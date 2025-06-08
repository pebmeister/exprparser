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
#include "ANSI_esc.h"

static ANSI_ESC esc;

int main()
{
    std::string input;    
    auto  parser = std::make_shared<ExpressionParser>(ExpressionParser());

    while (true) {
        std::cout
            << "\n"
            << esc.gr(esc.BLUE_FOREGROUND)
            << "Enter 6502 asm (or 'exit' to exit): "
            << esc.gr(esc.WHITE_FOREGROUND);
        if (!std::getline(std::cin, input)) break;
        if (input == "exit") break;
        if (input.empty()) continue;

        try {
            auto ast = parser->parse(input);
            if (ast != nullptr) {
                std::cout 
                    << esc.gr(esc.GREEN_FOREGROUND)
                    << "Parsing successful! AST: value " << ast->value << "\n";
                ast->color_print();
            }
        }
        catch (const std::exception& e) {
            std::cerr 
                << esc.gr(esc.RED_FOREGROUND)
                << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
