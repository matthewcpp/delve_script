#pragma once

#include "token.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Delve { namespace Script{

	class Lexer {
	public:
		using TokenVector = std::vector<std::unique_ptr<Token>>;

	public:
		Lexer(const std::string& inputStr);
		Lexer();

	public:
		void tokenize(const std::string& inputStr);
		inline const TokenVector& tokens() const { return tokenVec; };

		size_t tokenCount() const;
		const Token* getToken(size_t index) const;

	private:
		void init();
		void nextToken();

		void readNextChar();
		void skipWhitespace();
		char peekNextChar() const;

		std::string readNextIdentifier();
		std::string readNextNumber();
		
		static bool isIdentifierFirstLetter(char ch);
		static bool isIdentifierLetter(char ch);

	private:
		static std::unordered_map<std::string, Token::Type> keywords;
		static std::unordered_map<Token::Type, std::string> tokenLiterals;

		static Token::Type getIdentifierType(const std::string_view& identifier);
		

	private:
		uint16_t currentLine;
		uint16_t currentCol;

		uint32_t position;
		uint32_t readPosition;
		char currentChar;

		std::string input;
		TokenVector tokenVec;
	};

}}