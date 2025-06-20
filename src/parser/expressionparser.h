#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <iomanip>
#include <sstream>

#include "ASTNode.h"
#include "expr_rules.h"
#include "grammar_rule.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"

class ExpressionParser {

private:
    int line = 0;
    std::shared_ptr<Parser> parser;
    std::vector<std::string>& lines;
    std::vector<std::string> asmlines;
    std::vector<std::string> byteOutput;

    void processNode(std::shared_ptr<ASTNode> node);
    void printPC(uint16_t pc)
    {
        auto& esc = Parser::es;
        std::stringstream ss;

        ss
            << "$"
            << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
            << (int)pc << ":"
            << std::dec << std::setw(0) << std::nouppercase << std::setfill(' ');

        std::string str;
        ss >> str;
        byteOutputLine += str;
    }

    void printbyte(uint8_t value)
    {
        auto& esc = Parser::es;
        std::stringstream ss;
        ss
            << "$"
            << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(value)
            << std::dec << std::nouppercase << std::setw(0) << std::setfill(' ');

        std::string str;
        ss >> str;
        byteOutputLine += parser->paddLeft(str, 4);
    }

    void printword(uint16_t value)
    {
        auto lo = (value & 0x00FF) >> 0;
        auto hi = (value & 0xFF00) >> 8;

        printbyte(lo);
        printbyte(hi);
    }

    std::string byteOutputLine;
    std::string asmOutputLine;
    size_t asmOutputLine_Pos = 0;

public:
    ExpressionParser(std::vector<std::string>& lines);

    void generate_output(std::shared_ptr<ASTNode> ast);
    void generate_asembly(std::shared_ptr<ASTNode> ast);
    std::shared_ptr<ASTNode> parse(const std::string& input);
};
