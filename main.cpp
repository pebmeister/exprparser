#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "Token.h"
#include "Tokenizer.h"
#include "parser.h"
#include "grammar_rule.h"
#include "ASTNode.h"

#include "ExpressionParser.h"

int main()
{

    std::string input;    
    auto  parser = std::make_shared<ExpressionParser>(ExpressionParser());

    while (true) {
        std::cout << "Enter expression (or 'quit' to exit): ";
        if (!std::getline(std::cin, input)) break;
        if (input == "quit") break;
        if (input.empty()) continue;

        try {
            auto ast = parser->parse(input);
            if (ast != nullptr) {
                std::cout << "Parsing successful! AST: value " << ast->value << "\n";
                ast->print();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
