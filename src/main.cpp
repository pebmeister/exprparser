// written by Paul Baxter

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "expr_rules.h"
#include "expressionparser.h"
#include "grammar_rule.h"
#include "opcodedict.h"

/// <summary>
/// Display the usage
/// </summary>
void printUsage();

/// <summary>
/// Structure to process commandline arguments
/// </summary>
struct argHandler {
    std::string help;
    std::string helpdetail;
    std::function<int(int curArgc, int argc, char* argv[])> action;
};

/// <summary>
/// ANSI escape sequences for color
/// </summary>
extern ANSI_ESC es;

/// <summary>
/// Options passed to the parser
/// </summary>
ParserOptions options;

/// <summary>
/// Map for command line arguments to be processed
/// </summary>
static std::map<std::string, argHandler> argmap =
{
    {
        "h",
        argHandler {
            "",
            "Print help.",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                printUsage();
                return 0;
            }
        }
    },
    {
        "ast",
        argHandler {
            "",
            "Print abstract syntax tree",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.printAst = true;
                return 0;
            }
        }
    },
    {
        "v",
        argHandler {
            "",
            "Set verbose mode.",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.verbose = true;
                return 0;
            }
        }
    },
    {
        "c64",
        argHandler {
            "",
            "Commodore64 program mode. Set first 2 bytes as load address",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.c64 = true;
                return 0;
            }
        }
    },
    {
        "nowarn",
        argHandler {
            "",
            "Turn off warnings",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.nowarn = true;
                return 0;
            }
        }
    },
    {
        "65c02",
        argHandler {
            "",
            "Turn on 65C02 instructions.",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.cpu65c02 = true;
                return 0;
            }
        }
    },

    {
        "o",
        argHandler {
            " Output file",
            "Set the output file.",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                if (curArgc >= argc) {
                    std::cerr <<
                        es.gr(es.BRIGHT_RED_FOREGROUND) <<
                        "No outputfile specified with " << "-o" << "\n" <<
                        es.gr(es.RESET_ALL);
                    return -1;
                }
                options.outputfile = argv[curArgc++];
                return 1;
            }
        }
    },
    {
        "il",
        argHandler {
            "",
            "allow illegal instructions",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                options.allowIllegal = true;
                return 0;
            }
        }
    },
    {
        "li",
        argHandler {
            "",
            "list all valid instructions, modes and cycles.",
            [](int curArgc, int argc, char* argv[])  -> int
            {
                for (auto& opEntry : opcodeDict) {
                    auto& tokType = opEntry.first;
                    auto& opInfo = opEntry.second;
                    auto& mn = opInfo.mnemonic;

                    std::cout <<
                        es.gr({ es.BRIGHT_BLUE_FOREGROUND }) <<
                        mn << "\n" <<
                        es.gr({ es.BRIGHT_WHITE_FOREGROUND }) <<
                        opInfo.description << "\n\n";

                    std::cout <<
                        es.gr({ es.BRIGHT_YELLOW_FOREGROUND }) <<
                        std::left << std::setw(17) << "MODE" <<
                        es.gr({ es.BRIGHT_GREEN_FOREGROUND }) <<
                        "OPCODE" <<
                        es.gr({ es.BRIGHT_CYAN_FOREGROUND }) <<
                        std::right << std::setw(10) <<
                        "CYCLES" <<
                        std::dec << std::setw(0) << std::setfill(' ') <<
                        "\n" <<
                        es.gr({ es.BRIGHT_WHITE_FOREGROUND }) <<
                        "------------------------------------\n";

                    for (auto& modeEntry : opInfo.mode_to_opcode) {
                        auto& mode = modeEntry.first;
                        auto& opcode = modeEntry.second;

                        auto& modename = parserDict[mode];
                        std::cout  <<
                            es.gr({ es.BRIGHT_YELLOW_FOREGROUND }) <<
                            std::left << std::setw(17)   <<
                            modename.substr(7) <<
                            es.gr({ es.BRIGHT_GREEN_FOREGROUND }) <<
                            "$" << std::hex << std::setfill('0') << std::setw(2) <<
                            (int)opcode <<
                            es.gr({ es.BRIGHT_CYAN_FOREGROUND }) <<
                            std::dec << std::setfill(' ') << std::right << std::setw(8) <<
                            opInfo.mode_to_cycles[mode] <<
                            "\n";
                    }
                    std::cout << "\n";
                }
                return 0;
            }
        }
    }
};

/// <summary>
/// Print the usage
/// </summary>
void printUsage()
{
    std::cerr <<
        es.gr(es.BRIGHT_GREEN_FOREGROUND) <<
        "USAGE: " <<
        es.gr(es.BRIGHT_BLUE_FOREGROUND) <<
        "pasm" <<
        es.gr(es.BRIGHT_WHITE_FOREGROUND) <<
        " inputfile1 inputfile2 ... ";

    for (auto& a : argmap) {
        auto& op = a.first;
        auto& ophandler = a.second;
        std::cout <<
            es.gr(es.BRIGHT_YELLOW_FOREGROUND) <<
            " [-" <<
            op <<
            ophandler.help <<
            "]";
    };
    std::cout << "\n\n";
    for (auto& a : argmap) {
        auto& op = a.first;
        auto& ophandler = a.second;

        std::cout <<  
            es.gr(es.BRIGHT_YELLOW_FOREGROUND) <<
            std::left <<
            "-" <<
            std::setw(20) <<
            op  + ophandler.help <<
            std::right <<
            std::setw(0) <<
            es.gr(es.BRIGHT_WHITE_FOREGROUND) <<
            ophandler.helpdetail <<
            "\n";

    }
    es.gr(es.RESET_ALL);
}

/// <summary>
/// Parse the arguments
/// </summary>
/// <param name="argc">argument count</param>
/// <param name="argv">argument values</param>
/// <returns>0 for success</returns>
static int parseArgs(int argc, char* argv[])
{
    if (argc < 2) {
        printUsage();
        return 1;
    }

    auto arg = 1;
    while (arg < argc) {
        std::string curarg = argv[arg];
        arg++;

        if (curarg[0] != '-') {
            options.files.push_back(curarg);
            continue;
        }
        auto option = curarg.substr(1);
        if (!argmap.contains(option)) {

            std::cerr <<
                es.gr(es.BRIGHT_GREEN_FOREGROUND) <<
                "Unknown option : " << curarg << "\n" <<
                es.gr(es.RESET_ALL);

            printUsage();
            return -1;
        }
        auto& handler = argmap[option];
        auto r = handler.action(arg, argc, argv);
        if (r < 0)
            return r;
        arg += r;
    }
    return 0;
}

/// <summary>
/// main entry point
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char* argv[])
{
    auto ret = parseArgs(argc, argv);
    if (ret != 0)
        return ret;

    if (options.files.empty()) {
        std::cerr << 
            es.gr(es.BRIGHT_RED_FOREGROUND) <<
            "No input file specified.\n" <<
            es.gr(es.RESET_ALL);
        return 1;
    }
 
    try {
        ExpressionParser parser(options);
        auto start_time = std::chrono::high_resolution_clock::now();
        auto ast = parser.parse();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        auto seconds = duration.count() / 1000000.0;
        std::cout <<
            es.gr({ es.BRIGHT_CYAN_FOREGROUND }) <<
            "\nParse took: " <<
            es.gr(es.BRIGHT_YELLOW_FOREGROUND) <<
            seconds << 
            es.gr({ es.BRIGHT_CYAN_FOREGROUND }) <<
            " seconds\n" <<
            es.gr(es.RESET_ALL);

        std::cout << "\n";
        if (options.printAst) {
            ast->print(std::cout, true);
        }

        if (options.verbose) {
            parser.printsymbols();
        }
        parser.generate_output(ast);
        if (!options.outputfile.empty()) {
            std::ofstream fs(options.outputfile, std::ios::out | std::ios::binary);
            if (fs) {  // Always check if the file opened successfully
                if (options.c64) {
                    parser.output_bytes.insert(parser.output_bytes.begin() + 0, static_cast<uint8_t>(parser.parser->org & 0xff));
                    parser.output_bytes.insert(parser.output_bytes.begin() + 1, static_cast<uint8_t>((parser.parser->org >> 8) & 0xff));
                }

                // Write the entire vector in one operation
                fs.write(reinterpret_cast<const char*>(parser.output_bytes.data()),
                    parser.output_bytes.size());

                std::cout << "\n"  << es.gr(es.BRIGHT_YELLOW_FOREGROUND) << fs.tellp() <<
                    es.gr(es.BRIGHT_CYAN_FOREGROUND) << " bytes written to " <<
                    es.gr(es.BRIGHT_BLUE_FOREGROUND) << options.outputfile << es.gr(es.RESET_ALL) << "\n";
            }
            else {
                throw std::runtime_error(
                    "Could not open file " + options.outputfile);
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr <<
            es.gr(es.BRIGHT_RED_FOREGROUND) <<
            "Error: " << ex.what() << "\n" <<
            es.gr(es.RESET_ALL);
        return 1;
    }
    return 0;
}
 