#include "lexer.h"

#include <cctype>

namespace Delve { namespace Script {
	Lexer::Lexer(const std::string& inputStr)
		:currentLine(1), currentCol(0), currentChar(EOF), position(0), readPosition(0)
	{
		init();
		tokenize(inputStr);
	}

	Lexer::Lexer()
	{
		init();
	}

	void Lexer::init()
	{
		currentLine = 0;
		currentCol = 0;
		currentChar = EOF;
		position = 0;
		readPosition = 0;
	}

	/**
	* Gets the number of tokens in the internal token list
	* @retuns the number of tokens in the internal list
	*/
	size_t Lexer::tokenCount() const
	{
		return tokenVec.size();
	}

	/**
	* Gets a token at a given position from the internal token list
	* @param index index of token to get
	* @returns token at the given index or nullptr if index is out of range
	*/
	const Token* Lexer::getToken(size_t index) const
	{
		if (index < tokenVec.size()) {
			return tokenVec[index].get();
		}
		else {
			return nullptr;
		}
	}

	/*
	* Reads the next character from the input string and advances the read position, currentPosition, and current column
	* Stores the result in currentChar.  When end of file is encountered, current char is set to 0 and no further action
	* will take place.
	*/
	void Lexer::readNextChar()
	{
		if (readPosition < input.length()) {
			currentChar = input[readPosition];
			position = readPosition;
			readPosition += 1;
			currentCol += 1;
		}
		else {
			currentChar = 0;
		}
	}

	/*
	* Returns the next character in the input stream or 0 if EOF is reached.  Does not advance readPosition.
	* @returns next character in the input stream
	*/
	char Lexer::peekNextChar() const 
	{
		if (readPosition < input.length()) {
			return input[readPosition];
		}
		else {
			return 0;
		}
	}


	/*
	* Parses the next token from the input stream.  Returns a token with Token::Type::Eof when the end of the input is reached.
	* After Eof is reached, this function will continue to return Eof tokens on sucessive calls.
	*/
	void Lexer::nextToken()
	{
		skipWhitespace();

		auto token = std::make_unique<Token>();
		token->lineNum = currentLine;
		token->colNum = currentCol;

		switch (currentChar)
		{
		case '=': {
			char nextChar = peekNextChar();
			if (nextChar == '=') {
				token->type = Token::Type::Equal;
				token->literal = tokenLiterals[token->type];
				readNextChar();
			}
			else {
				token->type = Token::Type::Assign;
				token->literal = tokenLiterals[token->type];
			}
			break;
		}
		case ';':
			token->type = Token::Type::Semicolon;
			token->literal = tokenLiterals[token->type];
			break;
		case '(':
			token->type = Token::Type::LParen;
			token->literal = tokenLiterals[token->type];
			break;
		case ')':
			token->type = Token::Type::RParen;
			token->literal = tokenLiterals[token->type];
			break;
		case ',':
			token->type = Token::Type::Comma;
			token->literal = tokenLiterals[token->type];
			break;
		case '+':
			token->type = Token::Type::Plus;
			token->literal = tokenLiterals[token->type];
			break;
		case '-':
			token->type = Token::Type::Minus;
			token->literal = tokenLiterals[token->type];
			break;
		case '*':
			token->type = Token::Type::Multiply;
			token->literal = tokenLiterals[token->type];
			break;
		case '/':
			token->type = Token::Type::Divide;
			token->literal = tokenLiterals[token->type];
			break;
		case '!': {
			char nextChar = peekNextChar();
			if (nextChar == '=') {
				token->type = Token::Type::NotEqual;
				token->literal = tokenLiterals[token->type];
				readNextChar();
			}
			else {
				token->type = Token::Type::Negate;
				token->literal = tokenLiterals[token->type];
			}
			break;
		}
		case '>':
			token->type = Token::Type::GreaterThan;
			token->literal = tokenLiterals[token->type];
			break;
		case '<':
			token->type = Token::Type::LessThan;
			token->literal = tokenLiterals[token->type];
			break;
		case '{':
			token->type = Token::Type::LBrace;
			token->literal = tokenLiterals[token->type];
			break;
		case '}':
			token->type = Token::Type::RBrace;
			token->literal = tokenLiterals[token->type];
			break;
		case 0:
			token->type = Token::Type::Eof;
			break;
		default: {
			if (isIdentifierFirstLetter(currentChar)) {
				token->literal = readNextIdentifier();
				token->type = getIdentifierType(token->literal);
				tokenVec.push_back(std::move(token));
				return;
			}
			else if (std::isdigit(currentChar)) {
				token->literal = readNextNumber();
				token->type = Token::Type::Integer;
				tokenVec.push_back(std::move(token));
				return;
			}
			else {
				//in this case we do not have any idea what this token is.
				token->type = Token::Type::Illegal;
				tokenVec.push_back(std::move(token));
				return;
			}
		}
		}

		tokenVec.push_back(std::move(token));
		readNextChar();
	}

	/**
	* turns the input text into a token vector.  Stops parsing when an illegal token is encountered or the file is complete.
	*/
	void Lexer::tokenize(const std::string& inputStr)
	{
		currentLine = 1;
		input = inputStr;
		readNextChar();

		while (true) {
			nextToken();

			auto& token = tokenVec.back();
			
			if (token->type == Token::Type::Eof || token->type == Token::Type::Illegal) {
				break;
			}
		};
	}

	/*
	* Gets whether a character is valid as the first letter in an identifer.
	* @param ch the character to test
	* @returns value indicating this character is valid as first letter in an identifier
	*/
	bool Lexer::isIdentifierFirstLetter(char ch)
	{
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
	}

	/*
	* Gets whether a character is value as a subsequent character in an identifer
	* @param ch the character to test
	* @returns value indicating this character is valid as subsequent letter in an identifier
	*/
	bool Lexer::isIdentifierLetter(char ch)
	{
		return (ch >= '0' && ch <= '9') || isIdentifierFirstLetter(ch);
	}

	/*
	* Reads from the input until a non identifier character is found.  The input position will be set to the character after
	* the final letter of the identifier when this method returns.
	* @return string of the next identifier name in the input string
	*/
	std::string Lexer::readNextIdentifier()
	{
		size_t length = 0;
		uint32_t startingPosition = position;

		if (isIdentifierFirstLetter(currentChar)) {
			do {
				length += 1;
				readNextChar();
			} while (isIdentifierLetter(currentChar));
		}

		return std::string(input.data() + startingPosition, length);
	}

	// This map holds all the language keywords
	std::unordered_map<std::string, Token::Type> Lexer::keywords = { 
		{"function", Token::Type::Function}, {"let", Token::Type::Let}, {"true", Token::Type::True}, {"false", Token::Type::False},
		{"if", Token::Type::If}, {"else", Token::Type::Else}, {"return", Token::Type::Return}
	};

	std::unordered_map<Token::Type, std::string> Lexer::tokenLiterals = {
		{ Token::Type::Equal, "==" },{ Token::Type::Assign, "=" },{ Token::Type::Semicolon, ";" },{ Token::Type::LParen, "(" },{ Token::Type::RParen, ")" },{ Token::Type::Comma, "," },
		{ Token::Type::Plus, "+" },{ Token::Type::Minus, "-" },{ Token::Type::Multiply, "*" },
		{ Token::Type::Divide, "/" },{ Token::Type::Negate, "!" },{ Token::Type::NotEqual, "!=" },
		{ Token::Type::GreaterThan, ">" },{ Token::Type::LessThan, "<" },{ Token::Type::LBrace, "{" },
		{ Token::Type::RBrace, "}" }
	};

	/*
	* Checks to see if an identifer is a language keyword.  If so returns the appropriate token type.  Otherwise returns identifier.
	* @param identifer name to test if a language keyword
	* @returns Token::Type representing the keyword or Token::Type::Identifier if not a language keyword
	*/
	Token::Type Lexer::getIdentifierType(const std::string_view& identifier)
	{
		std::string name(identifier.data(), identifier.length());

		auto result = keywords.find(name);
		if (result == keywords.end()) {
			return Token::Type::Identifier;
		}
		else {
			return result->second;
		}
	}

	/*
	* Consumes whitespace from the input stream.  Updates line and column counters along the way.  When this method returns
	* the current character will be set to the first non whitespace character encountered.
	*/
	void Lexer::skipWhitespace() 
	{
		while (std::isspace(currentChar)) {
			if (currentChar == 0x0a) {
				currentLine += 1;
				currentCol = 0;
			}

			readNextChar();
		}
	}

	std::string Lexer::readNextNumber() 
	{
		size_t length = 0;
		uint32_t startingPosition = position;

		if (std::isdigit(currentChar)) {
			do {
				length += 1;
				readNextChar();
			} while (isIdentifierLetter(currentChar));
		}

		return std::string(input.data() + startingPosition, length);
	}
}}