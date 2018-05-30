#include "token.h"
#include "ast.h"

#include <gtest/gtest.h>
#include <memory>

using namespace Delve::Script;

TEST(Ast, IdentifierToString)
{
	std::string expectedStr = "matthew";
	Token token(Token::Type::Identifier, "matthew");
	Ast::Identifier identifier(&token);

	ASSERT_EQ(identifier.toString(), expectedStr);
}

TEST(Ast, IntegerLiteralToString)
{
	std::string expectedStr = "55662187";
	Token token(Token::Type::Integer, "55662187");
	Ast::IntegerLiteral integerLiteral(&token);
	integerLiteral.value = std::stoll(integerLiteral.token->literal);

	ASSERT_EQ(integerLiteral.toString(), expectedStr);
}

TEST(Ast, PrefixExpressionToString)
{
	std::string expectedStr = "(!matthew)";
	Token bang(Token::Type::Negate, "!");
	Token identifier(Token::Type::Identifier, "matthew");
	Ast::PrefixExpression prefixExpression(&bang);
	prefixExpression.rightExpression = std::make_unique<Ast::Identifier>(&identifier);

	ASSERT_EQ(prefixExpression.toString(), expectedStr);
}

TEST(Ast, InfixExpressionToString)
{
	std::string expectedStr = "(matthew + heather)";
	Token identifier1(Token::Type::Identifier, "matthew");
	Token plus(Token::Type::Plus, "+");
	Token identifier2(Token::Type::Identifier, "heather");

	Ast::InfixExpression infixExpression(&plus);
	infixExpression.left = std::make_unique<Ast::Identifier>(&identifier1);
	infixExpression.right = std::make_unique<Ast::Identifier>(&identifier2);

	ASSERT_EQ(infixExpression.toString(), expectedStr);
}

TEST(Ast, LetStatementToString)
{
	std::string expectedStr = "let x = 5;";
	Token let(Token::Type::Let, "let");
	Token identifier(Token::Type::Identifier, "x");
	Token integer(Token::Type::Integer, "5");

	Ast::LetStatement letStatement(&let);
	letStatement.identifier = std::make_unique<Ast::Identifier>(&identifier);
	auto* integerLiteral = new Ast::IntegerLiteral(&integer);
	integerLiteral->value = std::stoll(integerLiteral->token->literal);
	letStatement.expression.reset(integerLiteral);

	ASSERT_EQ(letStatement.toString(), expectedStr);
}

TEST(Ast, ReturnStatementToString)
{
	std::string expectedStr = "return 5;";
	Token ret(Token::Type::Return, "return");
	Token integer(Token::Type::Integer, "5");

	Ast::ReturnStatement returnStatement(&ret);
	auto* integerLiteral = new Ast::IntegerLiteral(&integer);
	integerLiteral->value = std::stoll(integerLiteral->token->literal);
	returnStatement.expression.reset(integerLiteral);

	ASSERT_EQ(returnStatement.toString(), expectedStr);
}

TEST(Ast, ExpressionStatementToString)
{
	std::string expectedStr = "(matthew + heather);";
	Token identifier1(Token::Type::Identifier, "matthew");
	Token plus(Token::Type::Plus, "+");
	Token identifier2(Token::Type::Identifier, "heather");

	auto* infixExpression = new Ast::InfixExpression{ &plus };
	infixExpression->left = std::make_unique<Ast::Identifier>(&identifier1);
	infixExpression->right = std::make_unique<Ast::Identifier>(&identifier2);

	Ast::ExpressionStatement expressionStatement(&identifier1);
	expressionStatement.expression.reset(infixExpression);

	ASSERT_EQ(expressionStatement.toString(), expectedStr);
}

TEST(Ast, ProgramToString)
{
	std::string expectedStr = "let x = 5;\nreturn x;\n";

	Token let(Token::Type::Let, "let");
	Token identifier(Token::Type::Identifier, "x");
	Token integer(Token::Type::Integer, "5");
	Token ret(Token::Type::Return, "return");

	auto* letStatement = new Ast::LetStatement{ &let };
	letStatement->identifier = std::make_unique<Ast::Identifier>(&identifier);
	auto* integerLiteral = new Ast::IntegerLiteral(&integer);
	integerLiteral->value = std::stoll(integerLiteral->token->literal);
	letStatement->expression.reset(integerLiteral);

	auto* returnStatement = new Ast::ReturnStatement{ &ret };
	auto* identifier2 = new Ast::Identifier(&identifier);
	returnStatement->expression.reset(identifier2);

	Ast::Program program;
	program.statements.emplace_back(letStatement);
	program.statements.emplace_back(returnStatement);

	ASSERT_EQ(program.toString(), expectedStr);
}