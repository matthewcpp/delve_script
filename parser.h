#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Delve::Script {

class Parser
{
public:
	using ErrorList = std::vector<std::string>;

public:
	Parser();
	Parser(const Token::Vector& tokenVec);

public:
	void parse(const Token::Vector& tokenVec);
	void clear();

	inline const Ast::Program* getProgram() const { return program.get(); }
	inline const ErrorList& getErrors() const { return errors; }

private:
	enum class Precedence
	{
		Lowest = 0,
		Equals,
		LessGreater,
		Sum,
		Product,
		Prefix,
		Call
	};

	static std::unordered_map<Token::Type, Precedence> precedenceMap;

	static Precedence getTokenPrecedence(const Token* token);

private:
	void init();

	void nextToken(uint32_t count = 1);

	std::unique_ptr<Ast::Statement> parseStatement();
	std::unique_ptr<Ast::LetStatement> parseLetStatement();
	std::unique_ptr<Ast::ReturnStatement> parseReturnStatement();
	std::unique_ptr<Ast::ExpressionStatement> parseExpressionStatement();
	std::unique_ptr<Ast::Expression> parseIfStatement();

	std::unique_ptr<Ast::BlockStatement> parseBlockStatement();

	std::unique_ptr<Ast::Identifier> parseIdentifer();
	std::unique_ptr<Ast::Expression> parseExpression(Precedence precedence);
	
	void advanceUntil(Token::Type tokenType);

	void parseError(const std::string& message);
	void expectedTypeError(Token::Type expectedType, const Token* actualToken);

private:
	using PrefixParsingFunc = std::function<std::unique_ptr<Ast::Expression>()>;
	using InfixParsingFunc = std::function <std::unique_ptr<Ast::Expression>(std::unique_ptr<Ast::Expression>)>;

	std::unordered_map<Token::Type, PrefixParsingFunc> prefixParseFuncs;
	std::unordered_map<Token::Type, InfixParsingFunc> infixParsingFuncs;
	
	static std::vector<Token::Type> infixTokenTypes;

	void initParsingFuncs();

private:
	std::unique_ptr<Ast::Identifier> parseIdentifierExpression();
	std::unique_ptr<Ast::IntegerLiteral> parseIntegerLiteralExpression();
	std::unique_ptr<Ast::BooleanLiteral> parseBooleanLiteralExpression();
	std::unique_ptr<Ast::FunctionLiteral> parseFunctionLiteralExpression();
	std::unique_ptr<Ast::Expression> parseGroupedExpression();
	std::unique_ptr<Ast::PrefixExpression> parsePrefixExpression();
	std::unique_ptr<Ast::InfixExpression> parseInfixExpression(std::unique_ptr<Ast::Expression> leftExpression);

private:
	const Token::Vector* tokens;
	const Token* currentToken;
	const Token* peekToken;

	std::unique_ptr<Ast::Program> program;
	std::vector<std::string> errors;

	uint32_t currentTokenPos;
	uint32_t currentTokenReadPos;
};

}