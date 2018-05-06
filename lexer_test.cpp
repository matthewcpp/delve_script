#include "lexer.h"

#include <gtest/gtest.h>

#include <vector>

using namespace Delve::Script;

// helper function that compares a token list to expected types and literal values
void compareTokenTypeAndValues(const Lexer& lexer, const std::vector<Delve::Script::Token::Type>& expectedTokens, const std::vector<std::string>& expectedLiterals);

/**
* Tests that initializing lexer with empty string and calling nextToken returns EOF
*/
TEST(Lexer, NextTokenEmpty)
{
	Delve::Script::Lexer lexer("");
	lexer.tokenize();

	const auto& tokens = lexer.tokens();
	ASSERT_EQ(tokens.size(), 1);

	if (tokens.size() > 0) {
		auto& token = tokens[0];

		ASSERT_EQ(token->type, Delve::Script::Token::Type::Eof);
		ASSERT_TRUE(token->literal.empty());
	}
}

/*
* Tests that tokens that are a single character are corretly created from an input stream
*/
TEST(Lexer, SingleCharacterTokens)
{
	std::string input = "=+-*/!<>(){},;";

	Delve::Script::Lexer lexer(input);
	lexer.tokenize();

	const auto& tokens = lexer.tokens();

	std::vector<Delve::Script::Token::Type> expectedTokens = {
		Token::Type::Assign, Token::Type::Plus, Token::Type::Minus, Token::Type::Multiply, Token::Type::Divide, 
		Token::Type::Negate, Token::Type::LessThan, Token::Type::GreaterThan, Token::Type::LParen, Token::Type::RParen,
		Token::Type::LBrace, Token::Type::RBrace, Token::Type::Comma, Token::Type::Semicolon, Token::Type::Eof
	};

	ASSERT_EQ(tokens.size(), input.length() + 1);

	for (size_t i = 0; i < input.length(); i++) {
		auto& token = tokens[i];

		ASSERT_EQ(token->type, expectedTokens[i]);

		if (token->type != Token::Type::Eof) {
			ASSERT_EQ(token->literal.length(), 1);
			ASSERT_EQ(token->literal[0], input[i]);
		}

		ASSERT_EQ(token->colNum, i + 1);
		ASSERT_EQ(token->lineNum, 1);
	}
}

/*
* Tests the Lexer can recognize the language keywords
*/
TEST(Lexer, Keywords)
{
	std::string input = "let\nfunction\ntrue\nfalse\nif\nelse\nreturn\n";

	Delve::Script::Lexer lexer(input);
	lexer.tokenize();

	const auto& tokens = lexer.tokens();

	std::vector<Token::Type> expectedTokens = {
		Token::Type::Let, Token::Type::Function, Token::Type::True, Token::Type::False, Token::Type::If, Token::Type::Else, Token::Type::Return, Token::Type::Eof
	};

	std::vector<std::string> expectedLiterals = {
		"let", "function", "true", "false", "if", "else", "return", ""
	};

	ASSERT_EQ(tokens.size(), expectedTokens.size());

	for (size_t i = 0; i < tokens.size(); i++) {
		auto& token = tokens[i];

		ASSERT_EQ(token->type, expectedTokens[i]);

		if (expectedLiterals[i].empty()) {
			ASSERT_TRUE(token->literal.empty());
		}
		else {
			ASSERT_EQ(token->literal, expectedLiterals[i]);
			ASSERT_EQ(token->colNum, 1);
		}

		ASSERT_EQ(token->lineNum, i + 1);
	}
}

/*
* Tests the Lexer can lex a simple let statement
*/
TEST(Lexer, SimpleLetStatement)
{
	std::string input = "let five = 10;";

	std::vector<Token::Type> expectedTokens = {
		Token::Type::Let, Token::Type::Identifier, Token::Type::Assign,
		Token::Type::Integer, Token::Type::Semicolon, Token::Type::Eof
	};

	std::vector<std::string> expectedLiterals = {
		"let", "five", "=", "10", ";", ""
	};

	Delve::Script::Lexer lexer(input);
	lexer.tokenize();

	compareTokenTypeAndValues(lexer, expectedTokens, expectedLiterals);
}

/*
* Tests the Lexer can lex a simple function statement
*/
TEST(Lexer, SimpleFunctionStatement)
{
	std::string input = "function(x, y) {\r\nreturn x + y; \r\n}";

	std::vector<Token::Type> expectedTokens = {
		Token::Type::Function, Token::Type::LParen, Token::Type::Identifier, Token::Type::Comma,
		Token::Type::Identifier, Token::Type::RParen, Token::Type::LBrace, Token::Type::Return, Token::Type::Identifier,
		Token::Type::Plus, Token::Type::Identifier, Token::Type::Semicolon, Token::Type::RBrace, Token::Type::Eof
	};

	std::vector<std::string> expectedLiterals = {
		"function", "(", "x", ",", "y", ")", "{", "return", "x", "+", "y", ";", "}", ""
	};

	Delve::Script::Lexer lexer(input);
	lexer.tokenize();

	compareTokenTypeAndValues(lexer, expectedTokens, expectedLiterals);
}

/*
* Tests that the lexer can handle token that are ocmprise dof two characters
*/
TEST(Lexer, TwoCharacterTokens)
{
	std::string input = "==\n!=";
	
	std::vector<Token::Type> expectedTokens = {
		Token::Type::Equal, Token::Type::NotEqual, Token::Type::Eof
	};

	std::vector<std::string> expectedLiterals = {
		"==", "!=", ""
	};

	Delve::Script::Lexer lexer(input);
	lexer.tokenize();

	compareTokenTypeAndValues(lexer, expectedTokens, expectedLiterals);
}

void compareTokenTypeAndValues(const Lexer& lexer, const std::vector<Delve::Script::Token::Type>& expectedTokens, const std::vector<std::string>& expectedLiterals)
{
	ASSERT_EQ(expectedTokens.size(), expectedLiterals.size());

	const auto& tokens = lexer.tokens();
	ASSERT_EQ(tokens.size(), expectedTokens.size());

	for (size_t i = 0; i < tokens.size(); i++) {
		auto& token = tokens[i];

		ASSERT_EQ(token->type, expectedTokens[i]);

		if (expectedLiterals[i].empty()) {
			ASSERT_TRUE(token->literal.empty());
		}
		else {
			ASSERT_EQ(token->literal, expectedLiterals[i]);
		}
	}
}