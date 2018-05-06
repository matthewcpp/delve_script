#include "token.h"

namespace Delve { namespace Script {

std::string Token::getTokenName(const Type& tokenType) {
	switch (tokenType)
	{	
	case Type::Eof:
		return "Eof";


	case Type::Identifier:
		return "Identifier";
	case Type::Integer:
		return "Int";


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
		return "Function";
	case Type::Let:
		return "Let";

	default:
		return "Illegal";
	}
}

}}