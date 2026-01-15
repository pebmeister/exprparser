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

struct ParserOptions {
    std::vector<std::string> files;
    bool allowIllegal = false;
    bool c64 = false;
    bool nowarn = false;
    bool verbose = false;
    std::string outputfile = "";
    bool cpu65c02 = false;
    bool printAst = false;
};

class ExpressionParser {
public:
    int looplevel;
    SourcePos loopOutputpos;

    std::shared_ptr<Parser> parser;
    std::shared_ptr<Parser> doParser;

    bool inMacrodefinition = false;
    void generate_output(std::shared_ptr<ASTNode> node);
    std::vector<std::pair<SourcePos, std::string>> lines;
    std::vector<uint8_t> output_bytes;
 
    void resetExpectedPC()  {  expected_pc = currentPC; }

private:
    ParserOptions options;
    bool allowbytes = true;
    SourcePos pos;
    SourcePos lastpos;
    uint16_t currentPC;
    uint16_t expected_pc;
    std::string currentfile;
    std::shared_ptr<ASTNode> Assemble() const;
    std::map<std::string, int> filelistmap;

    std::vector<std::pair<SourcePos, std::string>> listLines;
    std::vector<std::pair<SourcePos, std::string>> byteOutput;
    std::vector<std::pair<SourcePos, std::string>> asmlines;

    void print_outbytes();
    void print_asm();
    void print_listfile();
    void printfilelistmap();

    void extractExpressionList(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data, bool word = false);

    void printPC(uint16_t pc)
    {
        if (inMacrodefinition)
            return;

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
        if (inMacrodefinition)
            return;

        std::stringstream ss;
        ss
            << "$"
            << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(value)
            << std::dec << std::nouppercase << std::setw(0) << std::setfill(' ');

        std::string str;
        ss >> str;
        byteOutputLine += paddLeft(str, 4);
    }

    void outputbyte(uint8_t value)
    {
        if (currentPC != expected_pc) {
            std::stringstream expected;
            std::stringstream actual;

            expected << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << expected_pc;
            actual << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << currentPC;
            throw std::runtime_error("Expected PC value " + expected.str() + " but got " + actual.str());
        }
        expected_pc++;
        currentPC++;

        if (!inMacrodefinition) {
            output_bytes.push_back(value);
        }
    }

    void printword(uint16_t value)
    {
        if (inMacrodefinition)
            return;

        auto lo = (value & 0x00FF) >> 0;
        auto hi = (value & 0xFF00) >> 8;

        printbyte(lo);
        printbyte(hi);
    }

    std::string byteOutputLine;
    std::string asmOutputLine;
    size_t asmOutputLine_Pos;

    const size_t instruction_indent = 4;
    const size_t byteOutputWidth = 23;
    const size_t asmLineWidth = 35;

    const int max_passes = 5;

public:     
    ExpressionParser(ParserOptions& options);
    void printsymbols() const { parser->printSymbols(); }
    
    void generate_file_list(std::shared_ptr<ASTNode> ast);
    void generate_output_bytes(std::shared_ptr<ASTNode> ast);
    void generate_assembly(std::shared_ptr<ASTNode>as);
    void generate_listing();

    std::shared_ptr<ASTNode> parse() const;

    void TestParserDict() const;
};
