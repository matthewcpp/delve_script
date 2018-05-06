#include "token.h"

#include <gtest/gtest.h>
#include <string>

TEST(Token, getTokenName) {
	using namespace Delve::Script;

	EXPECT_EQ(Token::getTokenName(Token::Type::Illegal), "Illegal");
	EXPECT_EQ(Token::getTokenName(Token::Type::Eof), "Eof");

	EXPECT_EQ(Token::getTokenName(Token::Type::Identifier), "Identifier");
	EXPECT_EQ(Token::getTokenName(Token::Type::Integer), "Int");
	EXPECT_EQ(Token::getTokenName(Token::Type::True), "True");
	EXPECT_EQ(Token::getTokenName(Token::Type::False), "False");

	EXPECT_EQ(Token::getTokenName(Token::Type::If), "If");
	EXPECT_EQ(Token::getTokenName(Token::Type::Else), "Else");
	EXPECT_EQ(Token::getTokenName(Token::Type::Return), "Return");

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

	EXPECT_EQ(Token::getTokenName(Token::Type::Function), "Function");
	EXPECT_EQ(Token::getTokenName(Token::Type::Let), "Let");
}