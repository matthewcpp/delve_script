#include "parser.h"
#include "lexer.h"

#include <gtest/gtest.h>

using namespace Delve::Script;

/*
* Convenient way to quickly make a token object. 
*/
Token* createTokenForType(Token::Type type, const std::string& literal = "");

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

Token* createTokenForType(Token::Type type, const std::string& literal)
{
	Token* token = new Token();
	token->type = type;
	token->literal = literal;

	return token;
}

