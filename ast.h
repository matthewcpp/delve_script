#pragma once

#include "token.h"

#include <string>
#include <vector>
#include <memory>
#include <cstddef>

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

struct CallExpression : public Expression
{
	CallExpression(const Token* t) : Statement(t) {}
	
	std::unique_ptr<Expression> function;
	std::vector<std::unique_ptr<Expression>> arguments;

	virtual std::string toString() const override
	{
		std::string str = function->toString();
		str.append("(");

		for (size_t i = 0; i < arguments.size(); ++i) {
			if (i) {
				str.append(", ");
			}

			str.append(arguments[i]->toString());
		}

		str.append(")");

		return str;
	}
};

namespace Internal {
inline std::string statementVectorToString(const std::vector<std::unique_ptr<Statement>>& statements)
{
	std::string str;

	for (auto& statement : statements) {
		str.append(statement.get()->toString());
		str.append("\n");
	}

	return str;
}
}

struct BlockStatement : public Statement
{
	BlockStatement(const Token* t) : Statement(t) {}
	std::vector<std::unique_ptr<Statement>> statements;

	virtual std::string toString() const override
	{
		return Internal::statementVectorToString(statements);
	}

};

struct FunctionLiteral : public Expression
{
	FunctionLiteral(const Token* t) : Expression(t) {}
	std::vector<std::unique_ptr<Identifier>> parameters;
	std::unique_ptr<BlockStatement> body;

	virtual std::string toString() const override
	{
		std::string str = "function(";

		for (size_t i = 0; i < parameters.size(); ++i) {
			if (i) {
				str.append(", ");
			}

			str.append(parameters[i]->toString());
		}

		str.append(") {\n").append(body->toString()).append("}");

		return str;
	}
};

struct IfStatement : public Statement
{
	IfStatement(const Token* t) : Expression(t) {}
	std::unique_ptr<Expression> condition;
	std::unique_ptr<BlockStatement> consequence;
	std::unique_ptr<BlockStatement> alternative;

	virtual std::string toString() const override
	{
		std::string str;

		str.append("if ").append(condition->toString()).append(" {\n").append(consequence->toString()).append("}");

		if (alternative) {
			str.append(" else {\n").append(alternative->toString()).append("}");
		}

		return str;
	}
};

struct Program
{
	std::vector<std::unique_ptr<Statement>> statements;

	std::string toString() const
	{
		return Internal::statementVectorToString(statements);
	}
};

}