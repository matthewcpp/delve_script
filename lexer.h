#pragma once

#include "token.h"
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace Delve { namespace Script{

	class Lexer {
	public:
		Lexer(const std::string& inputStr);

	public:
		void tokenize();
		inline const std::vector<std::unique_ptr<Token>>& tokens() const;

	private:
		void nextToken();

		void readNextChar();
		void skipWhitespace();
		char peekNextChar() const;

		std::string_view readNextIdentifier();
		std::string_view readNextNumber();
		
		static bool isIdentifierFirstLetter(char ch);
		static bool isIdentifierLetter(char ch);

		static std::unordered_map<std::string, Token::Type> keywords;
		static Token::Type getIdentifierType(const std::string_view& identifier);
		

	private:
		uint16_t currentLine;
		uint16_t currentCol;

		uint32_t position;
		uint32_t readPosition;
		char currentChar;

		std::string input;
		std::vector<std::unique_ptr<Token>> tokenVec;
	};

	inline const std::vector<std::unique_ptr<Token>>& Lexer::tokens() const {
		return tokenVec;
	}

}}