#include "parser.h"
#include "lexer.h"

#include <vector>
#include <string>

#include <gtest/gtest.h>

using namespace Delve::Script;

Token* createTokenForType(Token::Type type, const std::string& literal = "");
void compareStatementsToExpectedOutput(const std::vector<std::string>& statements, const std::vector<std::string>& expectedOutput);

/*
* Tests that an empty token set produces a null program with no errors.
*/
TEST(Parser, Empty) 
{
	Token::Vector tokens;
	Parser parser;
	parser.parse(tokens);

	const auto* program = parser.getProgram();
	ASSERT_EQ(program, nullptr);

	const auto& errors = parser.getErrors();
	ASSERT_EQ(errors.size(), 0);
}

/*
* Tests a token stream with only EOF will produce a program with no statements;
*/
TEST(Parser, EofOnly)
{
	Token::Vector tokens;
	tokens.emplace_back(createTokenForType(Token::Type::Eof));
	Parser parser;
	parser.parse(tokens);

	const auto* program = parser.getProgram();
	ASSERT_NE(program, nullptr);
	ASSERT_EQ(program->statements.size(), 0);

	const auto& errors = parser.getErrors();
	ASSERT_EQ(errors.size(), 0);
}

TEST(Parser, BasicLetStatement)
{
	std::string code = "let x = 7;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();

	ASSERT_EQ(errors.size(), 0);
	ASSERT_EQ(program->statements.size(), 1);

	const auto* statement = program->statements[0].get();
	ASSERT_EQ(statement->token->type, Token::Type::Let);
	const auto* letStatement = static_cast<const Ast::LetStatement*>(statement);
	ASSERT_EQ(letStatement->identifier->token->literal, "x");
}

TEST(Parser, BasicLetParseErrors)
{
	std::string code = "let = 7;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();

	ASSERT_EQ(program->statements.size(), 0);
	ASSERT_EQ(errors.size(), 1);
	ASSERT_NE(errors[0].find("identifier"), std::string::npos);

	code = "let x 7;";
	lexer.tokenize(code);
	parser.parse(lexer.tokens());

	ASSERT_EQ(program->statements.size(), 0);
	ASSERT_EQ(errors.size(), 1);
	ASSERT_NE(errors[0].find("="), std::string::npos);
}

TEST(Parser, ParseExpressionStatementIdentifier)
{
	std::string code = "foobar;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();

	ASSERT_EQ(program->statements.size(), 1);
	EXPECT_EQ(program->statements[0]->token->type, Token::Type::Identifier);
	
	ASSERT_EQ(errors.size(), 0);
}

TEST(Parser, ParseExpressionStatementInteger)
{
	std::string code = "5;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();

	ASSERT_EQ(program->statements.size(), 1);
	ASSERT_EQ(program->statements[0]->token->type, Token::Type::Integer);

	ASSERT_EQ(errors.size(), 0);
}

TEST(Parser, ParsePrefixExpressionStatementMinus)
{
	std::string code = "-5;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();


	ASSERT_EQ(program->statements.size(), 1);
	ASSERT_EQ(program->statements[0]->token->type, Token::Type::Minus);

	const auto* expressionStatement = static_cast<const Ast::ExpressionStatement*>(program->statements[0].get());
	const auto* prefixExpression = static_cast<const Ast::PrefixExpression*>(expressionStatement->expression.get());

	ASSERT_EQ(prefixExpression->token->type, Token::Type::Minus);
	ASSERT_EQ(prefixExpression->rightExpression->token->type, Token::Type::Integer);
}

TEST(Parser, ParsePrefixExpressionStatementBang)
{
	std::string code = "!cool;";
	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();


	ASSERT_EQ(program->statements.size(), 1);
	ASSERT_EQ(program->statements[0]->token->type, Token::Type::Negate);

	const auto* expressionStatement = static_cast<const Ast::ExpressionStatement*>(program->statements[0].get());
	const auto* prefixExpression = static_cast<const Ast::PrefixExpression*>(expressionStatement->expression.get());

	ASSERT_EQ(prefixExpression->token->type, Token::Type::Negate);
	ASSERT_EQ(prefixExpression->rightExpression->token->type, Token::Type::Identifier);
}

TEST(Parser, ParseBasicInfixExpressions)
{
	std::string code =
		"5 + 5;"
		"5 - 5;"
		"5 * 5;"
		"5 / 5;"
		"5 > 5;"
		"5 < 5;"
		"5 == 5;"
		"5 != 5;";

	std::vector<Token::Type> infixOperatorTypes = {
		Token::Type::Plus, Token::Type::Minus, Token::Type::Multiply, Token::Type::Divide,
		Token::Type::GreaterThan, Token::Type::LessThan, Token::Type::Equal, Token::Type::NotEqual
	};

	Lexer lexer(code);
	Parser parser(lexer.tokens());

	const auto* program = parser.getProgram();
	const auto& errors = parser.getErrors();

	ASSERT_EQ(program->statements.size(), infixOperatorTypes.size());

	for (size_t i = 0; i < infixOperatorTypes.size(); ++i) {
		const auto* expressionStatement = dynamic_cast<const Ast::ExpressionStatement*>(program->statements[i].get());
		ASSERT_NE(expressionStatement, nullptr);

		const auto* infixExpression = dynamic_cast<const Ast::InfixExpression*>(expressionStatement->expression.get());
		ASSERT_NE(infixExpression, nullptr);

		ASSERT_EQ(infixExpression->token->type, infixOperatorTypes[i]);
	}
		
}

TEST(Parser, BasicStatementsWithExpressions)
{
	std::vector<std::string> statements = {
		"let x = y + z;",
		"let y = true;",
		"return 5-my_var;",
		"return false;",
		"my_var2 * 17;"
	};

	std::vector<std::string> expectedOutput = {
		"let x = (y + z);",
		"let y = true;",
		"return (5 - my_var);",
		"return false;",
		"(my_var2 * 17);"
	};

	compareStatementsToExpectedOutput(statements, expectedOutput);
}

TEST(Parser, Precedence)
{
	std::vector<std::string> statements = {
		"x + y + z;",
		"x - y + z;",
		"x + y * z;",
		"x / y - z;",
		"x * y * z + w;",
		"x - y * z / w;"
	};

	std::vector<std::string> expectedOutput = {
		"((x + y) + z);",
		"((x - y) + z);",
		"(x + (y * z));",
		"((x / y) - z);",
		"(((x * y) * z) + w);",
		"(x - ((y * z) / w));"
	};

	compareStatementsToExpectedOutput(statements, expectedOutput);
}

TEST(Parser, GroupedExpression)
{
	std::vector<std::string> statements = {
		"(x == true);",
		"x * (y + z);",
		"(3 + x) * (y - 4);",
		"let w=(x + y * z) - (3 * w) + 7;",
		"return((my_var * (7 + z)) / 3);"
	};

	std::vector<std::string> expectedOutput = {
		"(x == true);",
		"(x * (y + z));",
		"((3 + x) * (y - 4));",
		"let w = (((x + (y * z)) - (3 * w)) + 7);",
		"return ((my_var * (7 + z)) / 3);"
	};

	compareStatementsToExpectedOutput(statements, expectedOutput);
}


Token* createTokenForType(Token::Type type, const std::string& literal)
{
	Token* token = new Token();
	token->type = type;
	token->literal = literal;

	return token;
}

void compareStatementsToExpectedOutput(const std::vector<std::string>& statements, const std::vector<std::string>& expectedOutput)
{
	ASSERT_EQ(statements.size(), expectedOutput.size());

	Lexer lexer;
	Parser parser;

	for (size_t i = 0; i < statements.size(); ++i) {
		lexer.tokenize(statements[i]);
		parser.parse(lexer.tokens());

		ASSERT_EQ(parser.getErrors().size(), 0);

		auto program = parser.getProgram();
		ASSERT_EQ(program->statements.size(), 1);

		auto statement = program->statements[0].get();
		ASSERT_EQ(expectedOutput[i], statement->toString());
	}
}

