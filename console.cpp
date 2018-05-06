#include "console.h"

#include <iostream>
#include <string>

namespace Delve { namespace Script {

	void Console::runInteractive() {
		std::cout << "Delve Script Interactive Terminal." << std::endl;
		std::string input;

		while (true) {
			std::cout << "> ";
			std::getline(std::cin, input);

			Lexer lexer(input);
			lexer.tokenize();

			const auto& tokens = lexer.tokens();

			for (const auto& token : tokens) {
				using namespace Delve::Script;
				if (token->type != Token::Type::Eof) {
					std::cout << "Token (" << Delve::Script::Token::getTokenName(token->type) << ')';

					if (token->type == Token::Type::Integer || token->type == Token::Type::Identifier) {
						std::cout << ": " << token->literal;
					}

					std::cout << std::endl;
				}
			}
		}
	}
}}