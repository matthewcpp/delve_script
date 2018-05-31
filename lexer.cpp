#include "lexer.h"

#include <cctype>

namespace Delve::Script {
	Lexer::Lexer()
	{
		init();
	}

	Lexer::Lexer(const std::string& inputStr)
	{
		init();
		tokenize(inputStr);
	}

	/*
	* Initializes Lexer members to their default values.
	*/
	void Lexer::init()
	{
		input_ = nullptr;
		currentLine_ = 1;
		currentCol_ = 0;
		currentChar_ = EOF;
		position_ = 0;
		readPosition_ = 0;
	}

	/**
	* Resets the Lexer to its initial state.
	*/
	void Lexer::clear()
	{
		init();
		tokens_.clear();
	}

	/*
	* Reads the next character from the input string and advances the read position, currentPosition, and current column
	* Stores the result in currentChar.  When end of file is encountered, current char is set to 0 and no further action
	* will take place.
	*/
	void Lexer::readNextChar()
	{
		if (readPosition_ < input_->length()) {
			currentChar_ = input_->at(readPosition_);
			position_ = readPosition_;
			readPosition_ += 1;
			currentCol_ += 1;
		}
		else {
			currentChar_ = 0;
		}
	}

	/*
	* Returns the next character in the input stream or 0 if EOF is reached.  Does not advance readPosition.
	* @returns next character in the input stream
	*/
	char Lexer::peekNextChar() const 
	{
		if (readPosition_ < input_->length()) {
			return input_->at(readPosition_);
		}
		else {
			return 0;
		}
	}


	/*
	* Parses the next token from the input stream.  Returns a token with Token::Type::Eof when the end of the input is reached.
	* After Eof is reached, this function will continue to return Eof tokens on successive calls.
	*/
	void Lexer::nextToken()
	{
		skipWhitespace();

		auto token = std::make_unique<Token>();
		token->lineNum = currentLine_;
		token->colNum = currentCol_;

		switch (currentChar_)
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
			if (isIdentifierFirstLetter(currentChar_)) {
				token->literal = readNextIdentifier();
				token->type = getIdentifierType(token->literal);
				tokens_.push_back(std::move(token));
				return;
			}
			else if (std::isdigit(currentChar_)) {
				token->literal = readNextNumber();
				token->type = Token::Type::Integer;
				tokens_.push_back(std::move(token));
				return;
			}
			else {
				//in this case we do not have any idea what this token is.
				token->type = Token::Type::Illegal;
				tokens_.push_back(std::move(token));
				return;
			}
		}
		}

		tokens_.push_back(std::move(token));
		readNextChar();
	}

	/**
	* Turns the input text into a token vector.  Stops parsing when an illegal token is encountered or the file is complete.
	* @param inputStr the input text to tokenize
	*/
	void Lexer::tokenize(const std::string& inputStr)
	{
		if (input_) {
			clear();
		}

		input_ = &inputStr;
		readNextChar();

		while (true) {
			nextToken();

			auto& token = tokens_.back();
			
			if (token->type == Token::Type::Eof || token->type == Token::Type::Illegal) {
				break;
			}
		};
	}

	/*
	* Gets whether a character is valid as the first letter in an identifier.
	* @param ch the character to test
	* @returns value indicating this character is valid as first letter in an identifier
	*/
	bool Lexer::isIdentifierFirstLetter(char ch)
	{
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
	}

	/*
	* Gets whether a character is value as a subsequent character in an identifier
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
		uint32_t startingPosition = position_;

		if (isIdentifierFirstLetter(currentChar_)) {
			do {
				length += 1;
				readNextChar();
			} while (isIdentifierLetter(currentChar_));
		}

		return std::string(input_->data() + startingPosition, length);
	}

	// This map holds all the language keywords
	std::unordered_map<std::string, Token::Type> Lexer::keywords = { 
		{"function", Token::Type::Function}, {"let", Token::Type::Let}, {"true", Token::Type::True}, {"false", Token::Type::False},
		{"if", Token::Type::If}, {"else", Token::Type::Else}, {"return", Token::Type::Return}
	};

	// This map holds the string literals for each token type
	std::unordered_map<Token::Type, std::string> Lexer::tokenLiterals = {
		{ Token::Type::Equal, "==" },{ Token::Type::Assign, "=" },{ Token::Type::Semicolon, ";" },{ Token::Type::LParen, "(" },{ Token::Type::RParen, ")" },{ Token::Type::Comma, "," },
		{ Token::Type::Plus, "+" },{ Token::Type::Minus, "-" },{ Token::Type::Multiply, "*" },
		{ Token::Type::Divide, "/" },{ Token::Type::Negate, "!" },{ Token::Type::NotEqual, "!=" },
		{ Token::Type::GreaterThan, ">" },{ Token::Type::LessThan, "<" },{ Token::Type::LBrace, "{" },
		{ Token::Type::RBrace, "}" }
	};

	/*
	* Checks to see if an identifier is a language keyword.  If so returns the appropriate token type.  Otherwise returns identifier.
	* @param identifier name to test if a language keyword
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
		while (std::isspace(currentChar_)) {
			if (currentChar_ == 0x0a) {
				currentLine_ += 1;
				currentCol_ = 0;
			}

			readNextChar();
		}
	}

	/*
	* Reads from the input until a non number identifier is found.   The input position will be set to the character after
	* the final digit of the number when this method returns.
	* @return string of the next number in the input string
	*/
	std::string Lexer::readNextNumber() 
	{
		size_t length = 0;
		uint32_t startingPosition = position_;

		if (std::isdigit(currentChar_)) {
			do {
				length += 1;
				readNextChar();
			} while (isIdentifierLetter(currentChar_));
		}

		return std::string(input_->data() + startingPosition, length);
	}
}