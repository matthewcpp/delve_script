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

		statement->expression.reset(parseExpression(Precedence::Lowest));
	
		return statement;
	}

	std::unique_ptr<Ast::ReturnStatement> Parser::parseReturnStatement()
	{
		assert(currentToken->type == Token::Type::Return);
		std::unique_ptr<Ast::ReturnStatement> statement = std::make_unique<Ast::ReturnStatement>(currentToken);

		nextToken();

		statement->expression.reset(parseExpression(Precedence::Lowest));

		return statement;
	}

	std::unique_ptr<Ast::ExpressionStatement> Parser::parseExpressionStatement()
	{
		std::unique_ptr<Ast::ExpressionStatement> statement;

		auto* expressionStartToken = currentToken;
		auto expression = parseExpression(Precedence::Lowest);

		if (expression) {
			statement = std::make_unique<Ast::ExpressionStatement>(expressionStartToken);
			statement->expression.reset(expression);
		}
		else {
			return statement;
		}

		if (peekToken->type == Token::Type::Semicolon) {
			nextToken();
		}

		return statement;
	}

	Ast::Expression* Parser::parseExpression(Precedence precedence) {
		auto result = prefixParseFuncs.find(currentToken->type);
		if (result == prefixParseFuncs.end()) {
			return nullptr;
		}

		auto* leftExpression =  result->second();

		while (peekToken->type != Token::Type::Semicolon && precedence < getTokenPrecedence(peekToken)) {
			auto infixFuncResult = infixParsingFuncs.find(peekToken->type);

			if (infixFuncResult == infixParsingFuncs.end()) {
				return leftExpression;
			}

			nextToken();

			leftExpression = infixFuncResult->second(leftExpression);
		}

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
		return new Ast::Identifier{ this->currentToken };
	}

	Ast::Expression* Parser::parseIntegerLiteralExpression()
	{
		assert(currentToken->type == Token::Type::Integer);
		auto* integerLiteral = new Ast::IntegerLiteral{ currentToken };
		integerLiteral->value = std::stoll(currentToken->literal);

		return integerLiteral;
	}

	Ast::Expression* Parser::parseBooleanLiteralExpression()
	{
		assert(currentToken->type == Token::Type::True || currentToken->type == Token::Type::False);
		auto* booleanLiteral = new Ast::BooleanLiteral{ currentToken };

		return booleanLiteral;
	}

	Ast::Expression* Parser::parsePrefixExpression()
	{
		//TODO: better handling of case where right expression fails to parse?

		assert(currentToken->type == Token::Type::Negate || currentToken->type == Token::Type::Minus);
		auto* prefixExpression = new Ast::PrefixExpression{ currentToken };

		nextToken();

		prefixExpression->rightExpression.reset(parseExpression(Precedence::Prefix));

		return prefixExpression;
	}

	Ast::Expression* Parser::parseInfixExpression(Ast::Expression* leftExpression) {
		auto* infixExpression = new Ast::InfixExpression{ currentToken };
		infixExpression->left.reset(leftExpression);

		Precedence currentPrecedence = getTokenPrecedence(currentToken);
		nextToken();
		infixExpression->right.reset(parseExpression(currentPrecedence));

		return infixExpression;
	}

	Ast::Expression* Parser::parseGroupedExpression()
	{
		assert(currentToken->type == Token::Type::LParen);
		
		nextToken();
		
		auto* expression = parseExpression(Precedence::Lowest);

		if (peekToken->type != Token::Type::RParen) {
			expectedTypeError(Token::Type::RParen, peekToken);
			delete expression;
			return nullptr;
		}
		else {
			// Here we ensure that when we are done parsing this infix token the current token is set to the Rparen of this statement.
			nextToken();
			return expression;
		}
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
		prefixParseFuncs[Token::Type::Identifier] = [this]() ->Ast::Expression* {
			return this->parseIdentifierExpression();
		};

		prefixParseFuncs[Token::Type::Integer] = [this]() ->Ast::Expression* {
			return this->parseIntegerLiteralExpression();
		};

		prefixParseFuncs[Token::Type::True] = [this]() ->Ast::Expression* {
			return this->parseBooleanLiteralExpression();
		};

		prefixParseFuncs[Token::Type::False] = [this]() ->Ast::Expression* {
			return this->parseBooleanLiteralExpression();
		};

		prefixParseFuncs[Token::Type::Negate] = [this]() ->Ast::Expression* {
			return this->parsePrefixExpression();
		};

		prefixParseFuncs[Token::Type::Minus] = [this]() ->Ast::Expression* {
			return this->parsePrefixExpression();
		};

		prefixParseFuncs[Token::Type::LParen] = [this]() ->Ast::Expression* {
			return this->parseGroupedExpression();
		};

		auto parseInfix = [this](Ast::Expression* expression) -> Ast::Expression* {
			return this->parseInfixExpression(expression);
		};

		for (auto& tokenType : infixTokenTypes) {
			infixParsingFuncs[tokenType] = parseInfix;
		}
	}

	// this structure holds all the tokens that need to be registers for infix expression parsing
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
	};

}