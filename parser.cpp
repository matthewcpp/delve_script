#include "parser.h"

#include <cassert>
#include <sstream>

namespace Delve { namespace Script {
	Parser::Parser()
	{
		init();
	}

	Parser::Parser(const Token::Vector& tokenVec)
	{
		init();
		parse(tokenVec);
	}

	void Parser::init()
	{
		if (prefixParseFuncs.size() == 0) {
			initParsingFuncs();
		}

		tokens = nullptr;
		currentToken = nullptr;
		currentTokenPos = 0;
		currentTokenReadPos = 0;
	}

	void Parser::clear()
	{
		init();
		program.reset(nullptr);
		errors.clear();
	}

	void Parser::parse(const Token::Vector& tokenVec)
	{
		if (tokens) {
			clear();
		}

		
		if (tokenVec.size() > 0) {
			tokens = &tokenVec;
		}
		else {
			return;
		}

		nextToken();
		program = std::make_unique<Ast::Program>();

		while (currentToken->type != Token::Type::Eof) {
			std::unique_ptr<Ast::Statement> statement = parseStatement();

			if (statement) {
				program->statements.push_back(std::move(statement));
			}
			else
			{
				// if there was error parsing the statement, we will eat all the tokens that
				// remain from the error location to the end of that statement.
				while (currentToken->type != Token::Type::Semicolon) {
					nextToken();
				}
			}

			nextToken();
		}
	}

	std::unique_ptr<Ast::Statement> Parser::parseStatement()
	{
		std::unique_ptr<Ast::Statement> statement;

		switch (currentToken->type) {
		case Token::Type::Let:
			statement = parseLetStatement();
			break;

		case Token::Type::Return:
			statement = parseReturnStatement();
			break;

		default:
			statement = parseExpressionStatement();
		}

		return statement;
	}

	/*
	* Parses a let statement in the form of `let <identifier> = <statement>;
	* Precondition: current token has token type of Let.
	* @returns unique pointer containing an AST hierarchy for this statement.  Pointer will be initialized to nullptr if there is a parsing error.
	*/
	std::unique_ptr<Ast::LetStatement> Parser::parseLetStatement()
	{
		assert(currentToken->type == Token::Type::Let);
		std::unique_ptr<Ast::LetStatement> statement = std::make_unique<Ast::LetStatement>(currentToken);

		nextToken();

		if (currentToken->type == Token::Type::Identifier) {
			statement->identifier = parseIdentifer();

			if (!statement->identifier) {
				statement.reset(nullptr);
				return statement;
			}
		}
		else {
			expectedTypeError(Token::Type::Identifier, currentToken);
			statement.reset(nullptr);
			return statement;
		}

		nextToken();
		
		if (currentToken->type != Token::Type::Assign) {
			expectedTypeError(Token::Type::Assign, currentToken);

			statement.reset(nullptr);
			return statement;
		}

		nextToken();

		// TODO: skip expression till we encounter a semicolon
		while (currentToken->type != Token::Type::Semicolon) {
			nextToken();
		}
		//statement->expression = parseExpression();

	
		return statement;
	}

	std::unique_ptr<Ast::ReturnStatement> Parser::parseReturnStatement()
	{
		assert(currentToken->type == Token::Type::Return);
		std::unique_ptr<Ast::ReturnStatement> statement = std::make_unique<Ast::ReturnStatement>(currentToken);

		nextToken();

		// TODO: skip expression till we encounter a semicolon
		while (currentToken->type != Token::Type::Semicolon) {
			nextToken();
		}
		//statement->expression = parseExpression();

		return statement;
	}

	std::unique_ptr<Ast::ExpressionStatement> Parser::parseExpressionStatement()
	{
		std::unique_ptr<Ast::ExpressionStatement> statement;

		auto* expressionStartToken = currentToken;
		auto expression = parseExpression(Precidence::Lowest);

		if (expression) {
			statement = std::make_unique<Ast::ExpressionStatement>(expressionStartToken);
			statement->expression = std::move(expression);
		}
		else {
			return statement;
		}

		if (peekToken->type == Token::Type::Semicolon) {
			nextToken();
		}

		return statement;
	}

	std::unique_ptr<Ast::Expression> Parser::parseExpression(Precidence precedence) {
		std::unique_ptr<Ast::Expression> leftExpression;

		auto result = prefixParseFuncs.find(currentToken->type);
		if (result == prefixParseFuncs.end()) {
			return leftExpression;
		}

		leftExpression.reset(result->second());

		return leftExpression;
	}

	std::unique_ptr<Ast::Identifier> Parser::parseIdentifer() {
		assert(currentToken->type == Token::Type::Identifier);

		return std::make_unique<Ast::Identifier>(currentToken);
	}

	/*
	* Advances the current token and the peek token.  If the end of the input is reached, it will 
	* continuously return the Eof Token.
	* Precondition: tokens->size() > 0
	*/
	void Parser::nextToken()
	{
		if (currentTokenReadPos < tokens->size() -1) {
			currentToken = tokens->at(currentTokenReadPos).get();
			currentTokenPos = currentTokenReadPos;

			currentTokenReadPos += 1;
			peekToken = tokens->at(currentTokenReadPos).get();
		}
		else {
			currentToken = tokens->at(currentTokenReadPos).get();
			peekToken = currentToken;
		}
	}

	void Parser::expectedTypeError(Token::Type expectedType, const Token* actualToken)
	{
		std::ostringstream error;
		error << "Expected " << Token::getTokenName(expectedType) << " at " << actualToken->lineNum << ", " << actualToken->colNum << '.';

		parseError(error.str());
	}

	void Parser::parseError(const std::string& message)
	{
		errors.push_back(message);
	}

	Ast::Expression* Parser::parseIdentifierExpression()
	{
		assert(currentToken->type == Token::Type::Identifier);
		return new Ast::Expression{ this->currentToken };
	}

	Ast::Expression* Parser::parseIntegerLiteralExpression()
	{
		assert(currentToken->type == Token::Type::Integer);
		auto* integerLiteral = new Ast::IntegerLiteral{ currentToken };
		integerLiteral->value = std::stoll(currentToken->literal);

		return integerLiteral;
	}

	Ast::Expression* Parser::parsePrefixExpression()
	{
		//TODO: better handling of case where right expression fails to parse?

		assert(currentToken->type == Token::Type::Negate || currentToken->type == Token::Type::Minus);
		auto* prefixExpression = new Ast::PrefixExpression{ currentToken };

		nextToken();

		prefixExpression->rightExpression = parseExpression(Precidence::Prefix);

		return prefixExpression;
	}

	void Parser::initParsingFuncs()
	{
		prefixParseFuncs[Token::Type::Identifier] = [this]() ->Ast::Expression* {
			return this->parseIdentifierExpression();
		};

		prefixParseFuncs[Token::Type::Integer] = [this]() ->Ast::Expression* {
			return this->parseIntegerLiteralExpression();
		};

		prefixParseFuncs[Token::Type::Negate] = [this]() ->Ast::Expression* {
			return this->parsePrefixExpression();
		};

		prefixParseFuncs[Token::Type::Minus] = [this]() ->Ast::Expression* {
			return this->parsePrefixExpression();
		};
	}

}}