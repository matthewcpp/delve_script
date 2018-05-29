#pragma once

#include "token.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Delve { namespace Script{

	class Lexer {

	public:
		Lexer();
		Lexer(const std::string& inputStr);
		
	public:
		void tokenize(const std::string& inputStr);
		inline const Token::Vector& tokens() const { return tokens_; };

		void clear();

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
		uint16_t currentLine_;
		uint16_t currentCol_;

		uint32_t position_;
		uint32_t readPosition_;
		char currentChar_;

		const std::string* input_;
		Token::Vector tokens_;
	};

}}