#pragma once

#include "ast.h"
#include "lexer.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Delve { namespace Script {

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
	enum class Precidence
	{
		Lowest = 0,
		Equals,
		LessGreater,
		Sum,
		Product,
		Prefix,
		Call
	};

private:
	void init();

	void nextToken();

	std::unique_ptr<Ast::Statement> parseStatement();
	std::unique_ptr<Ast::LetStatement> parseLetStatement();
	std::unique_ptr<Ast::ReturnStatement> parseReturnStatement();
	std::unique_ptr<Ast::ExpressionStatement> parseExpressionStatement();

	std::unique_ptr<Ast::Identifier> parseIdentifer();
	std::unique_ptr<Ast::Expression> parseExpression(Precidence precedence);
	

	void parseError(const std::string& message);
	void expectedTypeError(Token::Type expectedType, const Token* actualToken);

private:
	using PrefixParsingFunc = std::function<Ast::Expression*()>;
	using InfixParsingFunc = std::function <Ast::Expression*(Ast::Expression*)>;

	std::unordered_map<Token::Type, PrefixParsingFunc> prefixParseFuncs;
	std::unordered_map<Token::Type, InfixParsingFunc> infixParsingFuncs;

	void initParsingFuncs();

private:
	Ast::Expression* parseIdentifierExpression();
	Ast::Expression* parseIntegerLiteralExpression();
	Ast::Expression* parsePrefixExpression();

private:
	const Token::Vector* tokens;
	const Token* currentToken;
	const Token* peekToken;

	std::unique_ptr<Ast::Program> program;
	std::vector<std::string> errors;

	uint32_t currentTokenPos;
	uint32_t currentTokenReadPos;
};

}}