#pragma once

#include "token.h"

#include <string>
#include <vector>
#include <memory>
#include <cstddef>
#include <sstream>

namespace Delve::Script::Ast {

struct Node
{
	Node(const Token* t) : token(t) {}
	virtual ~Node() {}
	virtual std::string toString() const = 0;

	const Token* token;
};

typedef Node Expression;

struct Identifier : public Expression
{
	Identifier(const Token* t) : Expression(t) {}

	virtual std::string toString() const override
	{
		return token->literal;
	}
};

struct IntegerLiteral : public Expression
{
	IntegerLiteral(const Token* t) : Expression(t) {}
	
	int64_t value;

	virtual std::string toString() const override
	{
		return std::to_string(value);
	}
};

struct BooleanLiteral : public Expression
{
	BooleanLiteral(const Token* t) : Expression(t) {}

	virtual std::string toString() const override
	{
		return token->type == Token::Type::True ? "true" : "false";
	}
};

struct PrefixExpression : public Expression
{
	PrefixExpression(const Token* t) : Expression(t) {}

	std::unique_ptr<Expression> rightExpression;

	virtual std::string toString() const override
	{
		return '(' + token->literal + rightExpression->toString() + ')';
	}
};

struct InfixExpression : public Expression
{
	InfixExpression(const Token* t) : Expression(t) {}

	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;

	virtual std::string toString() const override
	{
		return '(' + left->toString() + ' ' + token->literal + ' ' + right->toString() + ')';
	}
};

typedef Node Statement;

struct LetStatement : public Statement
{
	LetStatement(const Token* t) : Statement(t) {}

	std::unique_ptr<Identifier> identifier;
	std::unique_ptr<Expression> expression;

	virtual std::string toString() const override
	{
		return "let " + identifier->toString() + " = " + expression->toString() + ';';
	}
};

struct ReturnStatement : public Statement
{
	ReturnStatement(const Token* t) : Statement(t) {}

	std::unique_ptr<Expression> expression;

	virtual std::string toString() const override
	{
		return "return " + expression->toString() + ';';
	}
};

struct ExpressionStatement : public Statement
{
	ExpressionStatement(const Token* t) : Statement(t) {}

	std::unique_ptr<Expression> expression;

	virtual std::string toString() const override
	{
		return expression->toString() + ';';
	}
};

struct Program
{
	std::vector<std::unique_ptr<Statement>> statements;

	std::string toString() const
	{
		std::ostringstream str;

		for (auto& statement : statements) {
			str << statement.get()->toString() << '\n';
		}

		return str.str();
	}
	

};

}