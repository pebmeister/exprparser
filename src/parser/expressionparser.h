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
    std::vector<std::string> byteOutput;
    void processNode(std::shared_ptr<ASTNode> node);

    void printPC(uint16_t pc)
    {
        std::stringstream ss;
        ss << "$"
            << std::hex << std::uppercase << std::setw(4) << std::setfill('0') 
            << (int)pc << ":"
            << std::dec << std::setw(0) << std::nouppercase << std::setfill(' ');
        std::string s;
        ss >> s;
        byteOutputLine += s;
    }

    void printbyte(uint8_t value)
    {
        std::stringstream ss;
        ss << "$"
            << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(value)
            << std::dec << std::nouppercase << std::setw(0) << std::setfill(' ');
        std::string s;
        ss >> s;
        byteOutputLine += parser->paddLeft(s, 4);
    }

    std::string byteOutputLine;

public:
    void generate_output(std::shared_ptr<ASTNode> ast);
    ExpressionParser(std::vector<std::string>& lines);
    std::shared_ptr<ASTNode> parse(const std::string& input);
};
