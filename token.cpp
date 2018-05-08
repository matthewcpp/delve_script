#include "token.h"

namespace Delve { namespace Script {

/**
* Returns a String representation of a TokenType enumeration.
* @param tokenType token to retrieve string representation for.
* @returns string representation for supplied token type.
*/
std::string Token::getTokenName(const Type& tokenType) {
	switch (tokenType)
	{	
	case Type::Eof:
		return "eof";

	case Type::Identifier:
		return "identifier";
	case Type::Integer:
		return "int";
	case Type::True:
		return "true";
	case Type::False:
		return "false";

	case Type::If:
		return "if";
	case Type::Else:
		return "else";
	case Type::Return:
		return "return";

	case Type::Assign:
		return "=";
	case Type::Plus:
		return "+";
	case Type::Minus:
		return "-";
	case Type::Multiply:
		return "*";
	case Type::Divide:
		return "/";
	case Type::Negate:
		return "!";

	case Type::GreaterThan:
		return ">";
	case Type::LessThan:
		return "<";
	case Type::Equal:
		return "==";
	case Type::NotEqual:
		return "!=";


	case Type::Comma:
		return ",";
	case Type::Semicolon:
		return ";";

	case Type::LParen:
		return "(";
	case Type::RParen:
		return ")";
	case Type::LBrace:
		return "{";
	case Type::RBrace:
		return "}";


	case Type::Function:
		return "function";
	case Type::Let:
		return "let";

	default:
		return "illegal";
	}
}

}}