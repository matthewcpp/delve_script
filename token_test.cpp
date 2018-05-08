#include "token.h"

#include <gtest/gtest.h>
#include <string>

TEST(Token, getTokenName) {
	using namespace Delve::Script;

	EXPECT_EQ(Token::getTokenName(Token::Type::Illegal), "illegal");
	EXPECT_EQ(Token::getTokenName(Token::Type::Eof), "eof");

	EXPECT_EQ(Token::getTokenName(Token::Type::Identifier), "identifier");
	EXPECT_EQ(Token::getTokenName(Token::Type::Integer), "int");
	EXPECT_EQ(Token::getTokenName(Token::Type::True), "true");
	EXPECT_EQ(Token::getTokenName(Token::Type::False), "false");

	EXPECT_EQ(Token::getTokenName(Token::Type::If), "if");
	EXPECT_EQ(Token::getTokenName(Token::Type::Else), "else");
	EXPECT_EQ(Token::getTokenName(Token::Type::Return), "return");

	EXPECT_EQ(Token::getTokenName(Token::Type::Assign), "=");
	EXPECT_EQ(Token::getTokenName(Token::Type::Plus), "+");
	EXPECT_EQ(Token::getTokenName(Token::Type::Minus), "-");
	EXPECT_EQ(Token::getTokenName(Token::Type::Multiply), "*");
	EXPECT_EQ(Token::getTokenName(Token::Type::Divide), "/");
	EXPECT_EQ(Token::getTokenName(Token::Type::Negate), "!");

	EXPECT_EQ(Token::getTokenName(Token::Type::GreaterThan), ">");
	EXPECT_EQ(Token::getTokenName(Token::Type::LessThan), "<");
	EXPECT_EQ(Token::getTokenName(Token::Type::Equal), "==");
	EXPECT_EQ(Token::getTokenName(Token::Type::NotEqual), "!=");

	EXPECT_EQ(Token::getTokenName(Token::Type::Comma), ",");
	EXPECT_EQ(Token::getTokenName(Token::Type::Semicolon), ";");

	EXPECT_EQ(Token::getTokenName(Token::Type::LParen), "(");
	EXPECT_EQ(Token::getTokenName(Token::Type::RParen), ")");
	EXPECT_EQ(Token::getTokenName(Token::Type::LBrace), "{");
	EXPECT_EQ(Token::getTokenName(Token::Type::RBrace), "}");

	EXPECT_EQ(Token::getTokenName(Token::Type::Function), "function");
	EXPECT_EQ(Token::getTokenName(Token::Type::Let), "let");
}