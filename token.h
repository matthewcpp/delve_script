#pragma once

#include <string>
#include <string_view>
#include <stdint.h>

namespace Delve { namespace Script {
struct Token
{
	enum class Type
	{
		// Special
		Illegal,
		Eof,

		// Identifiers and Literals
		Identifier,
		Integer,
		True,
		False,

		// Control
		If,
		Else,
		Return,

		// Operators
		Assign,
		Plus,
		Minus,
		Multiply,
		Divide,
		Negate,

		// Comparisons
		GreaterThan,
		LessThan,
		Equal,
		NotEqual,

		// Delimiters
		Comma,
		Semicolon,

		LParen,
		RParen,
		LBrace,
		RBrace,

		// Keywords
		Function,
		Let
	};

	Type type;
	uint16_t lineNum;
	uint16_t colNum;
	std::string_view literal;

	Token():type(Type::Illegal), lineNum(0), colNum(0) {}
	static std::string getTokenName(const Type& tokenType);
};

}}