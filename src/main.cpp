#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <chrono>
#include <cmath>

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "expr_rules.h"
#include "expressionparser.h"
#include "grammar_rule.h"

static ANSI_ESC esc;
ParserOptions options;

static int parseArgs(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr <<
            esc.gr(esc.BRIGHT_GREEN_FOREGROUND) <<
            "Usage: " << argv[0] << " <inputfile>\n" <<
            esc.gr(esc.RESET_ALL);

        return 1;
    }

    // Simple option parsing, ready for more options later
    auto i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        ++i;

        if (arg[0] != '-') {
            options.files.push_back(arg);
            continue;
        }
        auto option = arg.substr(1);
        if (option == "il") {
            options.allowIllegal = true;
        }
        else if (option == "c64") {
            options.c64 = true;
        }
        else if (option == "nowarn") {
            options.nowarn = true;
        }
        else if (option == "v") {
            options.verbose = true;
        }
        else if (option == "o") {
            if (i + 1 >= argc) {
                std::cerr <<
                    esc.gr(esc.BRIGHT_GREEN_FOREGROUND) <<
                    "No outputfile specified with " << arg << "\n" <<
                    esc.gr(esc.RESET_ALL);
                return 1;
            }
            options.outputfile = argv[i++];
        }
        else if (option == "65c02") {
            options.cpu65c02 = true;
        }
        else if (option == "6502") {
            options.cpu65c02 = false;
        }
        else {
            std::cerr <<
                esc.gr(esc.BRIGHT_GREEN_FOREGROUND) <<
                "Unknown option : " << arg << "\n" <<
                esc.gr(esc.RESET_ALL);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    auto ret = parseArgs(argc, argv);
    if (ret != 0)
        return ret;

    if (options.files.empty()) {
        std::cerr << 
            esc.gr(esc.BRIGHT_RED_FOREGROUND) <<
            "No input file specified.\n" <<
            esc.gr(esc.RESET_ALL);
        return 1;
    }
 
    try {
        ExpressionParser parser(options);
        auto start_time = std::chrono::high_resolution_clock::now();
        auto ast = parser.parse();
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        auto seconds = duration.count() / 1000000.0;
        std::cout << "parse took: " << seconds << " seconds\n";

        // ast->print(std::cout, true);

        // parser.printsymbols();
        std::cout << "\n";
        parser.generate_output(ast);
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
