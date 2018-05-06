#include "token.h"

#include <gtest/gtest.h>
#include <string>

TEST(Token, getTokenName) {
	using namespace Delve::Script;

	EXPECT_EQ(Token::getTokenName(Token::Type::Illegal), "Illegal");
	EXPECT_EQ(Token::getTokenName(Token::Type::Eof), "Eof");

	EXPECT_EQ(Token::getTokenName(Token::Type::Identifier), "Identifier");
	EXPECT_EQ(Token::getTokenName(Token::Type::Integer), "Int");

	EXPECT_EQ(Token::getTokenName(Token::Type::Assign), "=");
	EXPECT_EQ(Token::getTokenName(Token::Type::Plus), "+");

	EXPECT_EQ(Token::getTokenName(Token::Type::Comma), ",");
	EXPECT_EQ(Token::getTokenName(Token::Type::Semicolon), ";");

	EXPECT_EQ(Token::getTokenName(Token::Type::LParen), "(");
	EXPECT_EQ(Token::getTokenName(Token::Type::RParen), ")");
	EXPECT_EQ(Token::getTokenName(Token::Type::LBrace), "{");
	EXPECT_EQ(Token::getTokenName(Token::Type::RBrace), "}");

	EXPECT_EQ(Token::getTokenName(Token::Type::Function), "Function");
	EXPECT_EQ(Token::getTokenName(Token::Type::Let), "Let");
}