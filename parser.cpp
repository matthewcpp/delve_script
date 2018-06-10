#include "parser.h"

#include <cassert>
#include <sstream>

namespace Delve::Script {
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
			try {
				std::unique_ptr<Ast::Statement> statement = parseStatement();
				program->statements.push_back(std::move(statement));
			}
			catch (const ParsingError& error) {
				errors.push_back(error.what());

				// if there was error parsing the statement, we will eat all the tokens that
				// remain from the error location to the end of that statement.
				advanceUntil(Token::Type::Semicolon);
			}

			nextToken();
		}
	}

	void Parser::advanceUntil(Token::Type tokenType)
	{
		while (currentToken->type != tokenType) {
			nextToken();

			if (currentToken->type == Token::Type::Eof) {
				break;
			}
		}
	}

	/*
	* Parses the next full statement from the token stream.
	* Postcondition: if a statement was parsed successfully, the current token will be set to the trailing semicolon or Rbrace of that statement.
	*/
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

		case Token::Type::LBrace:
			statement = parseBlockStatement();
			break;

		case Token::Type::If:
			statement = parseIfStatement();
			break;

		default:
			statement = parseExpressionStatement();
		}

		return statement;
	}

	/*
	* Parses a let statement in the form of let <identifier> = <statement>;
	* Precondition: current token has token type of Let.
	* @returns unique pointer containing an AST hierarchy for this statement.  Pointer will be initialized to nullptr if there is a parsing error.
	*/
	std::unique_ptr<Ast::LetStatement> Parser::parseLetStatement()
	{
		assert(currentToken->type == Token::Type::Let);
		std::unique_ptr<Ast::LetStatement> statement = std::make_unique<Ast::LetStatement>(currentToken);

		nextToken();

		if (currentToken->type == Token::Type::Identifier) {
			statement->identifier = parseIdentifierExpression();
		}
		else {
			expectedTypeError(Token::Type::Identifier, currentToken);
		}

		nextToken();
		
		if (currentToken->type != Token::Type::Assign) {
			expectedTypeError(Token::Type::Assign, currentToken);
		}

		nextToken();

		statement->expression = std::move(parseExpression(Precedence::Lowest));

		nextToken();
		if (currentToken->type != Token::Type::Semicolon) {
			expectedTypeError(Token::Type::Semicolon, currentToken);
		}
	
		return statement;
	}

	std::unique_ptr<Ast::ReturnStatement> Parser::parseReturnStatement()
	{
		assert(currentToken->type == Token::Type::Return);
		std::unique_ptr<Ast::ReturnStatement> statement = std::make_unique<Ast::ReturnStatement>(currentToken);

		nextToken();

		statement->expression = std::move(parseExpression(Precedence::Lowest));

		nextToken();
		if (currentToken->type != Token::Type::Semicolon) {
			expectedTypeError(Token::Type::Semicolon, currentToken);
		}

		return statement;
	}

	std::unique_ptr<Ast::ExpressionStatement> Parser::parseExpressionStatement()
	{
		std::unique_ptr<Ast::ExpressionStatement> statement;

		auto* expressionStartToken = currentToken;
		auto expression = parseExpression(Precedence::Lowest);

		if (expression) {
			statement = std::make_unique<Ast::ExpressionStatement>(expressionStartToken);
			statement->expression = std::move(expression);
		}
		else {
			return statement;
		}

		nextToken();
		if (currentToken->type != Token::Type::Semicolon) {
			expectedTypeError(Token::Type::Semicolon, currentToken);
		}

		return statement;
	}

	std::unique_ptr<Ast::BlockStatement> Parser::parseBlockStatement()
	{
		assert(currentToken->type == Token::Type::LBrace);

		auto blockStatement = std::make_unique<Ast::BlockStatement>(currentToken);

		nextToken();

		while (currentToken->type != Token::Type::RBrace && currentToken->type != Token::Type::Eof) {
			auto statement = parseStatement();
			if (statement) {
				blockStatement->statements.emplace_back(std::move(statement));
			}

			nextToken();
		}

		return blockStatement;
	}

	/*
	* Parses the next expression from the input token stream.
	* Postcondition: the current token will be set to the final token consumed by parsing the appropriate expression.  This will most likely be the token before a";" or "}"
	*/
	std::unique_ptr<Ast::Expression> Parser::parseExpression(Precedence precedence) {
		auto result = prefixParseFuncs.find(currentToken->type);
		if (result == prefixParseFuncs.end()) {
			return nullptr;
		}

		auto leftExpression =  result->second();

		while (peekToken->type != Token::Type::Semicolon && precedence < getTokenPrecedence(peekToken)) {
			auto infixFuncResult = infixParsingFuncs.find(peekToken->type);

			if (infixFuncResult == infixParsingFuncs.end()) {
				return leftExpression;
			}

			nextToken();

			leftExpression = infixFuncResult->second(std::move(leftExpression));
		}

		return leftExpression;
	}

	/*
	* Advances the current token and the peek token a given number of times.  If the end of the input is reached, it will continuously return the Eof Token.
	* Precondition: tokens->size() > 0
	* @param count the number of times to advance the token
	*/
	void Parser::nextToken(uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++) {
			if (currentTokenReadPos < tokens->size() - 1) {
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
	}

	void Parser::expectedTypeError(Token::Type expectedType, const Token* actualToken)
	{
		std::ostringstream error;
		error << "Expected " << Token::getTokenName(expectedType) << " at " << actualToken->lineNum << ", " << actualToken->colNum << '.';

		throw ParsingError(error.str());
	}

	std::unique_ptr<Ast::Identifier> Parser::parseIdentifierExpression()
	{
		assert(currentToken->type == Token::Type::Identifier);
		return std::make_unique<Ast::Identifier>( this->currentToken);
	}

	std::unique_ptr<Ast::IntegerLiteral> Parser::parseIntegerLiteralExpression()
	{
		assert(currentToken->type == Token::Type::Integer);
		auto integerLiteral = std::make_unique<Ast::IntegerLiteral>(currentToken);
		integerLiteral->value = std::stoll(currentToken->literal);

		return integerLiteral;
	}

	std::unique_ptr<Ast::BooleanLiteral> Parser::parseBooleanLiteralExpression()
	{
		assert(currentToken->type == Token::Type::True || currentToken->type == Token::Type::False);
		return std::make_unique<Ast::BooleanLiteral>(currentToken);
	}

	std::unique_ptr<Ast::FunctionLiteral> Parser::parseFunctionLiteralExpression()
	{
		assert(currentToken->type == Token::Type::Function);
		auto function = std::make_unique<Ast::FunctionLiteral>(currentToken);
		nextToken();

		if (currentToken->type != Token::Type::LParen) {
			expectedTypeError(Token::Type::LParen, peekToken);
		}

		nextToken();

		//parse the parameter list
		while (currentToken->type != Token::Type::RParen) {
			if (function->parameters.size() > 0) {
				if (currentToken->type == Token::Type::Comma) {
					nextToken();
				}
				else {
					expectedTypeError(Token::Type::Comma, currentToken);
				}
			}

			if (currentToken->type == Token::Type::Identifier) {
				function->parameters.emplace_back(parseIdentifierExpression());
			}
			else {
				expectedTypeError(Token::Type::Identifier, peekToken);
			}

			nextToken();
		}

		nextToken();

		function->body = parseBlockStatement();

		return function;
	}

	std::unique_ptr<Ast::CallExpression> Parser::parseCallExpression(std::unique_ptr<Ast::Expression> leftExpression)
	{
		assert(currentToken->type == Token::Type::LParen);
		auto callExpression = std::make_unique<Ast::CallExpression>(currentToken);
		callExpression->function = std::move(leftExpression);

		nextToken();
		while (currentToken->type != Token::Type::RParen) {
			if (callExpression->arguments.size() > 0) {
				if (currentToken->type == Token::Type::Comma) {
					nextToken();
				}
				else {
					expectedTypeError(Token::Type::Comma, currentToken);
				}
			}

			callExpression->arguments.emplace_back(parseExpression(Precedence::Lowest));

			// parsing argument expression leaves us at the last token of that expression.
			nextToken();
		}

		return callExpression;
	}

	std::unique_ptr<Ast::PrefixExpression> Parser::parsePrefixExpression()
	{
		assert(currentToken->type == Token::Type::Negate || currentToken->type == Token::Type::Minus);
		auto prefixExpression = std::make_unique<Ast::PrefixExpression>(currentToken);

		nextToken();

		prefixExpression->rightExpression = parseExpression(Precedence::Prefix);

		return prefixExpression;
	}

	std::unique_ptr<Ast::InfixExpression> Parser::parseInfixExpression(std::unique_ptr<Ast::Expression> leftExpression) {
		auto infixExpression = std::make_unique<Ast::InfixExpression>(currentToken);
		infixExpression->left = std::move(leftExpression);

		Precedence currentPrecedence = getTokenPrecedence(currentToken);
		nextToken();
		infixExpression->right = parseExpression(currentPrecedence);

		return infixExpression;
	}

	std::unique_ptr<Ast::Expression> Parser::parseGroupedExpression()
	{
		assert(currentToken->type == Token::Type::LParen);
		
		nextToken();
		
		auto expression = parseExpression(Precedence::Lowest);

		if (peekToken->type != Token::Type::RParen) {
			expectedTypeError(Token::Type::RParen, peekToken);
		}
		else {
			// Here we ensure that when we are done parsing this infix token the current token is set to the Rparen of this statement.
			nextToken();
		}

		return expression;
	}

	/*
	* Parses and If/Else expression.
	*/
	std::unique_ptr<Ast::IfStatement> Parser::parseIfStatement()
	{
		assert(currentToken->type == Token::Type::If);
		auto expression = std::make_unique<Ast::IfStatement>(currentToken);

		if (peekToken->type != Token::Type::LParen) {
			expectedTypeError(Token::Type::LParen, peekToken);
		}

		nextToken();

		expression->condition = parseExpression(Precedence::Lowest);

		if (peekToken->type != Token::Type::LBrace) {
			expectedTypeError(Token::Type::LBrace, peekToken);
		}

		nextToken();

		expression->consequence = parseBlockStatement();

		// else block is optional, if present then consume it and parse the alternative statement block.
		if (peekToken->type == Token::Type::Else) {
			nextToken(2);

			if (currentToken->type != Token::Type::LBrace) {
				expectedTypeError(Token::Type::LBrace, currentToken);
			}

			expression->alternative = parseBlockStatement();
		}

		return expression;
	}

	/*
	* Gets the precedence for a token type.  Note that if the precedence is not explicitly defined for a token, Precedence::Lowest will be returned.
	* @param token the token for which to get precedence
	* @returns the precedence for the supplied token
	*/
	Parser::Precedence Parser::getTokenPrecedence(const Token* token)
	{
		auto result = precedenceMap.find(token->type);

		if (result == precedenceMap.end())
		{
			return Precedence::Lowest;
		}
		else {
			return result->second;
		}
	}

	/*
	* Binds callbacks for expression parsing for each token type.  
	* Prefix expressions are those which the operator appears before the expression, e.x -5.
	* Infix expressions are those which the operator appears in between 2 expressions, e.x 5 + 5.
	*/
	void Parser::initParsingFuncs()
	{
		prefixParseFuncs[Token::Type::Identifier] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseIdentifierExpression();
		};

		prefixParseFuncs[Token::Type::Integer] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseIntegerLiteralExpression();
		};

		prefixParseFuncs[Token::Type::True] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseBooleanLiteralExpression();
		};

		prefixParseFuncs[Token::Type::False] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseBooleanLiteralExpression();
		};

		prefixParseFuncs[Token::Type::Negate] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parsePrefixExpression();
		};

		prefixParseFuncs[Token::Type::Minus] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parsePrefixExpression();
		};

		prefixParseFuncs[Token::Type::LParen] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseGroupedExpression();
		};

		prefixParseFuncs[Token::Type::Function] = [this]() -> std::unique_ptr<Ast::Expression> {
			return this->parseFunctionLiteralExpression();
		};

		// this function handles all generic infix operations, i.e.  a + b, a - b, a == b, etc
		auto parseInfix = [this](std::unique_ptr<Ast::Expression> expression) -> std::unique_ptr<Ast::Expression> {
			return this->parseInfixExpression(std::move(expression));
		};

		for (auto& tokenType : infixTokenTypes) {
			infixParsingFuncs[tokenType] = parseInfix;
		}

		infixParsingFuncs[Token::Type::LParen] = [this](std::unique_ptr<Ast::Expression> expression) -> std::unique_ptr<Ast::Expression> {
			return this->parseCallExpression(std::move(expression));
		};
	}

	// this structure holds all the tokens that need to be registered for infix expression parsing
	std::vector<Token::Type> Parser::infixTokenTypes = { Token::Type::Plus, Token::Type::Minus,
		Token::Type::Divide, Token::Type::Multiply, Token::Type::Equal, Token::Type::NotEqual,
		Token::Type::LessThan, Token::Type::GreaterThan
	};

	// this structure maps token operator types to their parsing precedence
	std::unordered_map<Token::Type, Parser::Precedence> Parser::precedenceMap = {
		{ Token::Type::Equal, Parser::Precedence::Equals },
		{ Token::Type::NotEqual, Parser::Precedence::Equals },
		{ Token::Type::LessThan, Parser::Precedence::LessGreater },
		{ Token::Type::GreaterThan, Parser::Precedence::LessGreater },
		{ Token::Type::Plus, Parser::Precedence::Sum },
		{ Token::Type::Minus, Parser::Precedence::Sum },
		{ Token::Type::Divide, Parser::Precedence::Product },
		{ Token::Type::Multiply, Parser::Precedence::Product },
		{ Token::Type::LParen, Parser::Precedence::Call}
	};

}