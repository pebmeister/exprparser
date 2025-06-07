#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "Token.h"
#include "Tokenizer.h"
#include "parser.h"
#include "grammar_rule.h"
#include "ASTNode.h"

#include "expr_rules.h"

int main()
{

    std::string input;
    ASTNode::astMap = parserDict;

    while (true) {
        std::cout << "Enter expression (or 'quit' to exit): ";
        if (!std::getline(std::cin, input)) break;
        if (input == "quit") break;
        if (input.empty()) continue;

        try {
            auto tokens = tokenizer.tokenize(input);
            Parser parser(tokens, rules, parserDict);
            auto ast = parser.parse_rule(Expr);

            // Check for leftover tokens
            if (parser.current_pos < parser.tokens.size()) {
                const Token& tok = parser.tokens[parser.current_pos];
                throw std::runtime_error(
                    "Unexpected token after complete parse: '" + tok.value +
                    "' at line " + std::to_string(tok.line) +
                    ", col " + std::to_string(tok.line_pos)
                );
            }

            std::cout << "Parsing successful! AST: value " << ast->value << "\n";
            ast->print();
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
