#pragma once

#include "token.h"

#include <string>
#include <vector>
#include <memory>
#include <cstddef>

namespace Delve {
	namespace Script {
		namespace Ast {
			struct Node
			{
			public:
				Node(const Token* t) : token(t) {}
				virtual ~Node() {}

			public:
				const Token* token;
			};

			typedef Node Statement;
			typedef Node Expression;

			struct Identifier : public Expression
			{
				Identifier(const Token* t) : Expression(t) {}
			};

			struct IntegerLiteral : public Expression
			{
				IntegerLiteral(const Token* t) : Expression(t) {}
				int64_t value;
			};

			struct PrefixExpression : public Expression
			{
				PrefixExpression(const Token* t) : Expression(t) {}
				std::unique_ptr<Expression> rightExpression;
			};

			struct LetStatement : public Statement
			{
				LetStatement(const Token* t) : Statement(t) {}

				std::unique_ptr<Identifier> identifier;
				std::unique_ptr<Expression> expression;
			};

			struct ReturnStatement : public Statement
			{
				ReturnStatement(const Token* t) : Statement(t) {}
				std::unique_ptr<Expression> expression;
			};

			struct ExpressionStatement : public Statement
			{
				ExpressionStatement(const Token* t) : Statement(t) {}

				std::unique_ptr<Expression> expression;
			};

			struct Program
			{
				std::vector<std::unique_ptr<Statement>> statements;
			};
		}
	}
}