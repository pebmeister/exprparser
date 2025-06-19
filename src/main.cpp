#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "expr_rules.h"
#include "expressionparser.h"
#include "grammar_rule.h"
#include "parser.h"
#include "Token.h"
#include "Tokenizer.h"

static ANSI_ESC esc;
std::vector<std::string>lines;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << 
            esc.gr(esc.BRIGHT_GREEN_FOREGROUND) <<
            "Usage: " << argv[0] << " <inputfile>\n" <<
            esc.gr(esc.RESET_ALL);

        return 1;
    }

    std::string inputFile;
    // Simple option parsing, ready for more options later
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] != '-') {
            inputFile = arg;
            break;
        }
        // Future: handle other options here
    }

    if (inputFile.empty()) {
        std::cerr << 
            esc.gr(esc.BRIGHT_RED_FOREGROUND) <<
            "No input file specified.\n" <<
            esc.gr(esc.RESET_ALL);
        return 1;
    }

    std::ifstream file(inputFile);
    if (!file) {
        std::cerr <<
            esc.gr(esc.BRIGHT_RED_FOREGROUND) <<
            "Could not open file: " << inputFile << "\n" <<
            esc.gr(esc.RESET_ALL);

        return 1;
    }

    std::string line, fileContent;
    while (std::getline(file, line)) {
        lines.push_back(std::string(line));
        fileContent += line + "\n";
    }

    try {
        ExpressionParser parser(lines);
        
        auto ast = parser.parse(fileContent);        
        std::cout << 
            esc.gr(esc.BRIGHT_GREEN_FOREGROUND) <<
            "Parsing successful! AST:\n" <<
            esc.gr(esc.RESET_ALL);
 
        parser.generate_output(ast);

        ast->color_print();
    }
    catch (const std::exception& ex) {
        std::cerr <<
            esc.gr(esc.BRIGHT_RED_FOREGROUND) <<
            "Error: " << ex.what() << "\n" <<
            esc.gr(esc.RESET_ALL);
        return 1;
    }

    return 0;
}
