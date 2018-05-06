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

		// Operators
		Assign,
		Plus,

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
	std::string_view literal;
	uint16_t lineNum;
	uint16_t colNum;

	Token():type(Type::Illegal), lineNum(0), colNum(0) {}

	/**
	* Returns a String representation of a TokenType enumeration.
	* @param tokenType token to retrieve string representation for.
	* @returns string representation for supplied token type.
	*/
	static std::string getTokenName(const Type& tokenType);
};
}}