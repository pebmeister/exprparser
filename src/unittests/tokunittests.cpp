// Written by Paul Baxter
#include <gtest/gtest.h>
#include <string>

#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <chrono>
#include <cmath>
#include <filesystem>

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "expr_rules.h"
#include "expressionparser.h"
#include "grammar_rule.h"
#include "parser.h"
#include "tokenizer.h"
#include "opcodedict.h"
#include "utils.h"
#include "token.h"

#pragma warning(disable:4996)

namespace fs = std::filesystem;

static ANSI_ESC esc;
extern std::map<int64_t, std::string> parserDict;

#ifdef TESTFILES_DIR
static std::string startdir = TESTFILES_DIR "/";
#else
static std::string startdir = "./testfiles/";
#endif

void CompareTokens(std::vector<Token>& expected, std::vector<Token>& actual)
{
    EXPECT_EQ(expected.size(), actual.size());
    for (auto i = 0; i < expected.size(); ++i) {
        auto& e = expected[i];
        auto& a = actual[i];

        EXPECT_EQ(e.type, a.type);
        EXPECT_EQ(e.value, a.value);
        EXPECT_EQ(e.start, a.start);
        EXPECT_EQ(e.pos.filename, a.pos.filename);
        EXPECT_EQ(e.pos.line, a.pos.line);        
    }
}


namespace parser_unit_test
{
    TEST(tok_unit_test, adc)
    {
        std::string op = "adc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ADC, "adc", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {ADC, "adc", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {ADC, "adc", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {ADC, "adc", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {ADC, "adc", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {ADC, "adc", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {ADC, "adc", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {ADC, "adc", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, ahx)
    {
        std::string op = "ahx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {AHX, "ahx", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {COMMA, ",", { file, 1}, 13, false },
            {Y, "y", { file, 1}, 14, false },
            {EOL, "\n", { file, 1}, 15, false },
            {AHX, "ahx", { file, 2}, 4, true },
            {LPAREN, "(", { file, 2}, 8, false },
            {HEXNUM, "$03", { file, 2}, 9, false },
            {RPAREN, ")", { file, 2}, 12, false },
            {COMMA, ",", { file, 2}, 13, false },
            {Y, "y", { file, 2}, 14, false },
            {EOL, "\n", { file, 2}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, alr)
    {
        std::string op = "alr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ALR, "alr", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, anc)
    {
        std::string op = "anc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ANC, "anc", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, anc2)
    {
        std::string op = "anc2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ANC2, "anc2", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 9, false },
            {HEXNUM, "$01", { file, 1}, 10, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, and)
    {
        std::string op = "and";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {AND, "and", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {AND, "and", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {AND, "and", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {AND, "and", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {AND, "and", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {AND, "and", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {AND, "and", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {AND, "and", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, arr)
    {
        std::string op = "arr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ARR, "arr", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, asl)
    {
        std::string op = "asl";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ASL, "asl", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {ASL, "asl", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {ASL, "asl", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {ASL, "asl", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {ASL, "asl", { file, 5}, 4, true },
            {A, "a", { file, 5}, 8, false },
            {EOL, "\n", { file, 5}, 9, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, axs)
    {
        std::string op = "axs";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {AXS, "axs", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr0)
    {
        std::string op = "bbr0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR0, "bbr0", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr1)
    {
        std::string op = "bbr1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR1, "bbr1", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr2)
    {
        std::string op = "bbr2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR2, "bbr2", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr3)
    {
        std::string op = "bbr3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR3, "bbr3", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr4)
    {
        std::string op = "bbr4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR4, "bbr4", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr5)
    {
        std::string op = "bbr5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR5, "bbr5", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr6)
    {
        std::string op = "bbr6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR6, "bbr6", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbr7)
    {
        std::string op = "bbr7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBR7, "bbr7", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs0)
    {
        std::string op = "bbs0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS0, "bbs0", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs1)
    {
        std::string op = "bbs1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS1, "bbs1", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs2)
    {
        std::string op = "bbs2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS2, "bbs2", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs3)
    {
        std::string op = "bbs3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS3, "bbs3", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs4)
    {
        std::string op = "bbs4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS4, "bbs4", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs5)
    {
        std::string op = "bbs5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS5, "bbs5", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs6)
    {
        std::string op = "bbs6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS6, "bbs6", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bbs7)
    {
        std::string op = "bbs7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BBS7, "bbs7", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {COMMA, ",", { file, 1}, 12, false },
            {DECNUM, "02", { file, 1}, 13, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bcc)
    {
        std::string op = "bcc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BCC, "bcc", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bcs)
    {
        std::string op = "bcs";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BCS, "bcs", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, beq)
    {
        std::string op = "beq";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BEQ, "beq", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bit)
    {
        std::string op = "bit";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BIT, "bit", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {BIT, "bit", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {BIT, "bit", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {BIT, "bit", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {BIT, "bit", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bmi)
    {
        std::string op = "bmi";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BMI, "bmi", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bne)
    {
        std::string op = "bne";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BNE, "bne", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bpl)
    {
        std::string op = "bpl";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BPL, "bpl", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bra)
    {
        std::string op = "bra";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BRA, "bra", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, brk)
    {
        std::string op = "brk";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BRK, "brk", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bvc)
    {
        std::string op = "bvc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BVC, "bvc", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, bvs)
    {
        std::string op = "bvs";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {BVS, "bvs", { file, 1}, 4, true },
            {HEXNUM, "$1000", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, clc)
    {
        std::string op = "clc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CLC, "clc", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, cld)
    {
        std::string op = "cld";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CLD, "cld", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, cli)
    {
        std::string op = "cli";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CLI, "cli", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, clv)
    {
        std::string op = "clv";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CLV, "clv", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, cmp)
    {
        std::string op = "cmp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CMP, "cmp", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {CMP, "cmp", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {CMP, "cmp", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {CMP, "cmp", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {CMP, "cmp", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {CMP, "cmp", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {CMP, "cmp", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {CMP, "cmp", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, cpx)
    {
        std::string op = "cpx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CPX, "cpx", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {CPX, "cpx", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {CPX, "cpx", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, cpy)
    {
        std::string op = "cpy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {CPY, "cpy", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {CPY, "cpy", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {CPY, "cpy", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, dcp)
    {
        std::string op = "dcp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {DCP, "dcp", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {DCP, "dcp", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {DCP, "dcp", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {DCP, "dcp", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {DCP, "dcp", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {DCP, "dcp", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {DCP, "dcp", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, dec)
    {
        std::string op = "dec";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {DEC, "dec", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {DEC, "dec", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {DEC, "dec", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {DEC, "dec", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, dex)
    {
        std::string op = "dex";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {DEX, "dex", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, dey)
    {
        std::string op = "dey";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {DEY, "dey", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, eor)
    {
        std::string op = "eor";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {EOR, "eor", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {EOR, "eor", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {EOR, "eor", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {EOR, "eor", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {EOR, "eor", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {EOR, "eor", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {EOR, "eor", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {EOR, "eor", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, inc)
    {
        std::string op = "inc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {INC, "inc", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {INC, "inc", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {INC, "inc", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {INC, "inc", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(tok_unit_test, inx)
    {
        std::string op = "inx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {INX, "inx", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, iny)
    {
        std::string op = "iny";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {INY, "iny", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, isc)
    {
        std::string op = "isc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ISC, "isc", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {ISC, "isc", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {ISC, "isc", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {ISC, "isc", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {ISC, "isc", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {ISC, "isc", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {ISC, "isc", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, jmp)
    {
        std::string op = "jmp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {JMP, "jmp", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {JMP, "jmp", { file, 2}, 4, true },
            {LPAREN, "(", { file, 2}, 8, false },
            {HEXNUM, "$0304", { file, 2}, 9, false },
            {RPAREN, ")", { file, 2}, 14, false },
            {EOL, "\n", { file, 2}, 15, false },
            {JMP, "jmp", { file, 3}, 4, true },
            {LPAREN, "(", { file, 3}, 8, false },
            {HEXNUM, "$05", { file, 3}, 9, false },
            {COMMA, ",", { file, 3}, 12, false },
            {X, "x", { file, 3}, 13, false },
            {RPAREN, ")", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, jsr)
    {
        std::string op = "jsr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {JSR, "jsr", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, las)
    {
        std::string op = "las";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LAS, "las", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {COMMA, ",", { file, 1}, 13, false },
            {Y, "y", { file, 1}, 14, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, lax)
    {
        std::string op = "lax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LAX, "lax", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {LAX, "lax", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {LAX, "lax", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {Y, "y", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {LAX, "lax", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {Y, "y", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {LAX, "lax", { file, 5}, 4, true },
            {LPAREN, "(", { file, 5}, 8, false },
            {HEXNUM, "$07", { file, 5}, 9, false },
            {COMMA, ",", { file, 5}, 12, false },
            {X, "x", { file, 5}, 13, false },
            {RPAREN, ")", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {LAX, "lax", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$08", { file, 6}, 9, false },
            {RPAREN, ")", { file, 6}, 12, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, lda)
    {
        std::string op = "lda";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LDA, "lda", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {LDA, "lda", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {LDA, "lda", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {LDA, "lda", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {LDA, "lda", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {LDA, "lda", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {LDA, "lda", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {LDA, "lda", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, ldx)
    {
        std::string op = "ldx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LDX, "ldx", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {LDX, "ldx", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {LDX, "ldx", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {LDX, "ldx", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {Y, "y", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {LDX, "ldx", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {Y, "y", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, ldy)
    {
        std::string op = "ldy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LDY, "ldy", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {LDY, "ldy", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {LDY, "ldy", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {LDY, "ldy", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {LDY, "ldy", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, lsr)
    {
        std::string op = "lsr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {LSR, "lsr", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {LSR, "lsr", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {LSR, "lsr", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {LSR, "lsr", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {LSR, "lsr", { file, 5}, 4, true },
            {A, "a", { file, 5}, 8, false },
            {EOL, "\n", { file, 5}, 9, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, nop)
    {
        std::string op = "nop";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {NOP, "nop", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, ora)
    {
        std::string op = "ora";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ORA, "ora", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {ORA, "ora", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {ORA, "ora", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {ORA, "ora", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {ORA, "ora", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {ORA, "ora", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {ORA, "ora", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {ORA, "ora", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rla)
    {
        std::string op = "rla";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RLA, "rla", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {RLA, "rla", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {RLA, "rla", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {RLA, "rla", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {RLA, "rla", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {RLA, "rla", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {RLA, "rla", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb0)
    {
        std::string op = "rmb0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB0, "rmb0", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb1)
    {
        std::string op = "rmb1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB1, "rmb1", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb2)
    {
        std::string op = "rmb2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB2, "rmb2", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb3)
    {
        std::string op = "rmb3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB3, "rmb3", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb4)
    {
        std::string op = "rmb4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB4, "rmb4", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb5)
    {
        std::string op = "rmb5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB5, "rmb5", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb6)
    {
        std::string op = "rmb6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB6, "rmb6", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rmb7)
    {
        std::string op = "rmb7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RMB7, "rmb7", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rol)
    {
        std::string op = "rol";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ROL, "rol", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {ROL, "rol", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {ROL, "rol", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {ROL, "rol", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {ROL, "rol", { file, 5}, 4, true },
            {A, "a", { file, 5}, 8, false },
            {EOL, "\n", { file, 5}, 9, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, ror)
    {
        std::string op = "ror";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {ROR, "ror", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {ROR, "ror", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {ROR, "ror", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {ROR, "ror", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {ROR, "ror", { file, 5}, 4, true },
            {A, "a", { file, 5}, 8, false },
            {EOL, "\n", { file, 5}, 9, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rra)
    {
        std::string op = "rra";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RRA, "rra", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {RRA, "rra", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {RRA, "rra", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {RRA, "rra", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {RRA, "rra", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {RRA, "rra", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {RRA, "rra", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rti)
    {
        std::string op = "rti";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RTI, "rti", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, rts)
    {
        std::string op = "rts";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {RTS, "rts", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sax)
    {
        std::string op = "sax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SAX, "sax", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {SAX, "sax", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {SAX, "sax", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 11, false },
            {Y, "y", { file, 3}, 12, false },
            {EOL, "\n", { file, 3}, 13, false },
            {SAX, "sax", { file, 4}, 4, true },
            {LPAREN, "(", { file, 4}, 8, false },
            {HEXNUM, "$05", { file, 4}, 9, false },
            {COMMA, ",", { file, 4}, 12, false },
            {X, "x", { file, 4}, 13, false },
            {RPAREN, ")", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sbc)
    {
        std::string op = "sbc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SBC, "sbc", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
            {SBC, "sbc", { file, 2}, 4, true },
            {HEXNUM, "$0203", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 13, false },
            {SBC, "sbc", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {EOL, "\n", { file, 3}, 11, false },
            {SBC, "sbc", { file, 4}, 4, true },
            {HEXNUM, "$0506", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 13, false },
            {X, "x", { file, 4}, 14, false },
            {EOL, "\n", { file, 4}, 15, false },
            {SBC, "sbc", { file, 5}, 4, true },
            {HEXNUM, "$07", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 11, false },
            {X, "x", { file, 5}, 12, false },
            {EOL, "\n", { file, 5}, 13, false },
            {SBC, "sbc", { file, 6}, 4, true },
            {HEXNUM, "$0809", { file, 6}, 8, false },
            {COMMA, ",", { file, 6}, 13, false },
            {Y, "y", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {SBC, "sbc", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {COMMA, ",", { file, 7}, 12, false },
            {X, "x", { file, 7}, 13, false },
            {RPAREN, ")", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
            {SBC, "sbc", { file, 8}, 4, true },
            {LPAREN, "(", { file, 8}, 8, false },
            {HEXNUM, "$0b", { file, 8}, 9, false },
            {RPAREN, ")", { file, 8}, 12, false },
            {COMMA, ",", { file, 8}, 13, false },
            {Y, "y", { file, 8}, 14, false },
            {EOL, "\n", { file, 8}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sec)
    {
        std::string op = "sec";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SEC, "sec", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sed)
    {
        std::string op = "sed";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SED, "sed", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sei)
    {
        std::string op = "sei";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SEI, "sei", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, shx)
    {
        std::string op = "shx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SHX, "shx", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {COMMA, ",", { file, 1}, 13, false },
            {Y, "y", { file, 1}, 14, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, shy)
    {
        std::string op = "shy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SHY, "shy", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {COMMA, ",", { file, 1}, 13, false },
            {X, "x", { file, 1}, 14, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, slo)
    {
        std::string op = "slo";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SLO, "slo", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {SLO, "slo", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {SLO, "slo", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {SLO, "slo", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {SLO, "slo", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {SLO, "slo", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {SLO, "slo", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb0)
    {
        std::string op = "smb0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB0, "smb0", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb1)
    {
        std::string op = "smb1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB1, "smb1", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb2)
    {
        std::string op = "smb2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB2, "smb2", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb3)
    {
        std::string op = "smb3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB3, "smb3", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb4)
    {
        std::string op = "smb4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB4, "smb4", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb5)
    {
        std::string op = "smb5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB5, "smb5", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb6)
    {
        std::string op = "smb6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB6, "smb6", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, smb7)
    {
        std::string op = "smb7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SMB7, "smb7", { file, 1}, 4, true },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sre)
    {
        std::string op = "sre";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {SRE, "sre", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {SRE, "sre", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {SRE, "sre", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {SRE, "sre", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {SRE, "sre", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {SRE, "sre", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {SRE, "sre", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sta)
    {
        std::string op = "sta";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {STA, "sta", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {STA, "sta", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {STA, "sta", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {STA, "sta", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
            {STA, "sta", { file, 5}, 4, true },
            {HEXNUM, "$0708", { file, 5}, 8, false },
            {COMMA, ",", { file, 5}, 13, false },
            {Y, "y", { file, 5}, 14, false },
            {EOL, "\n", { file, 5}, 15, false },
            {STA, "sta", { file, 6}, 4, true },
            {LPAREN, "(", { file, 6}, 8, false },
            {HEXNUM, "$09", { file, 6}, 9, false },
            {COMMA, ",", { file, 6}, 12, false },
            {X, "x", { file, 6}, 13, false },
            {RPAREN, ")", { file, 6}, 14, false },
            {EOL, "\n", { file, 6}, 15, false },
            {STA, "sta", { file, 7}, 4, true },
            {LPAREN, "(", { file, 7}, 8, false },
            {HEXNUM, "$0a", { file, 7}, 9, false },
            {RPAREN, ")", { file, 7}, 12, false },
            {COMMA, ",", { file, 7}, 13, false },
            {Y, "y", { file, 7}, 14, false },
            {EOL, "\n", { file, 7}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, stp)
    {
        std::string op = "stp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {STP, "stp", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, stx)
    {
        std::string op = "stx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {STX, "stx", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {STX, "stx", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {STX, "stx", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 11, false },
            {Y, "y", { file, 3}, 12, false },
            {EOL, "\n", { file, 3}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, sty)
    {
        std::string op = "sty";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {STY, "sty", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {STY, "sty", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {STY, "sty", { file, 3}, 4, true },
            {HEXNUM, "$04", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 11, false },
            {X, "x", { file, 3}, 12, false },
            {EOL, "\n", { file, 3}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, stz)
    {
        std::string op = "stz";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {STZ, "stz", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {STZ, "stz", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
            {STZ, "stz", { file, 3}, 4, true },
            {HEXNUM, "$0405", { file, 3}, 8, false },
            {COMMA, ",", { file, 3}, 13, false },
            {X, "x", { file, 3}, 14, false },
            {EOL, "\n", { file, 3}, 15, false },
            {STZ, "stz", { file, 4}, 4, true },
            {HEXNUM, "$06", { file, 4}, 8, false },
            {COMMA, ",", { file, 4}, 11, false },
            {X, "x", { file, 4}, 12, false },
            {EOL, "\n", { file, 4}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, tas)
    {
        std::string op = "tas";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TAS, "tas", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {COMMA, ",", { file, 1}, 13, false },
            {Y, "y", { file, 1}, 14, false },
            {EOL, "\n", { file, 1}, 15, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, tax)
    {
        std::string op = "tax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TAX, "tax", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, tay)
    {
        std::string op = "tay";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TAY, "tay", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, trb)
    {
        std::string op = "trb";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TRB, "trb", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {TRB, "trb", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, tsb)
    {
        std::string op = "tsb";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TSB, "tsb", { file, 1}, 4, true },
            {HEXNUM, "$0102", { file, 1}, 8, false },
            {EOL, "\n", { file, 1}, 13, false },
            {TSB, "tsb", { file, 2}, 4, true },
            {HEXNUM, "$03", { file, 2}, 8, false },
            {EOL, "\n", { file, 2}, 11, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, txa)
    {
        std::string op = "txa";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TXA, "txa", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, tya)
    {
        std::string op = "tya";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {TYA, "tya", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, usbc)
    {
        std::string op = "usbc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {USBC, "usbc", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 9, false },
            {HEXNUM, "$01", { file, 1}, 10, false },
            {EOL, "\n", { file, 1}, 13, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


    TEST(tok_unit_test, wai)
    {
        std::string op = "wai";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {WAI, "wai", { file, 1}, 4, true },
            {EOL, "\n", { file, 1}, 7, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(tok_unit_test, xaa)
    {
        std::string op = "xaa";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        std::vector<std::pair<SourcePos, std::string>> lines;
        std::vector<Token> expected =
        {
            {XAA, "xaa", { file, 1}, 4, true },
            {POUND, "#", { file, 1}, 8, false },
            {HEXNUM, "$01", { file, 1}, 9, false },
            {EOL, "\n", { file, 1}, 12, false },
        };
        try {
            ParserOptions options;
            options.files.push_back(file);
            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);
            CompareTokens(expected, actual);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }


#if 0
    TEST(maketests, tokens)
    {
        std::stringstream buff;
        std::ofstream fout(startdir + "tokens.tok");

        for (auto const& dir_entry : std::filesystem::directory_iterator{ startdir }) {
            auto& path = dir_entry.path();

            if (path.extension() != ".asm")
                continue;

            auto file = path.string();

            ParserOptions options;
            options.files.push_back(file);

            std::ifstream f(file);
            EXPECT_NE(&f, nullptr);

            std::vector<std::pair<SourcePos, std::string>> lines;
            std::string line;
            int l = 0;
            while (std::getline(f, line)) {
                lines.push_back({ SourcePos(file, ++l), line });
            }
            auto actual = tokenizer.tokenize(lines);

            auto& op = actual[0].value;
            fout <<
                "    TEST(tok_unit_test, " << tolower(op) << ")\n" <<
                "    {\n" <<
                "        std::string op = \"" << tolower(op) << "\";\n" <<
                "        std::string file = fs::absolute(fs::path(startdir + op + \".asm\")).lexically_normal().string();\n" <<
                "        std::vector<std::pair<SourcePos, std::string>> lines;\n" <<
                "        std::vector<Token> expected =\n" <<
                "        {\n";

            for (auto& tok : actual) {
                auto& opstr = parserDict[tok.type];
                auto& val = tok.value;
                if (val == "\n")
                    val = "\\n";
                fout <<
                    "            {" << opstr << ", \"" << val << "\", { file, " << tok.pos.line << "}, "
                    << tok.line_pos << ", " << (tok.start ? "true" : "false") << " },\n";
            }

            fout <<
                "        };\n" <<
                "        try {\n" <<
                "            ParserOptions options;\n" <<
                "            options.files.push_back(file);" <<
                "\n" <<
                "            std::ifstream f(file);\n" <<
                "            EXPECT_NE(&f, nullptr);\n" <<
                "\n" <<
                "            std::string line;\n" <<
                "            int l = 0;\n" <<
                "            while (std::getline(f, line)) {\n" <<
                "                lines.push_back({ SourcePos(file, ++l), line });\n" <<
                "            }\n" <<
                "            auto actual = tokenizer.tokenize(lines);" <<
                "\n" <<
                "            CompareTokens(expected, actual);\n" <<
                "        }\n" <<
                "        catch (const std::exception& ex) {\n" <<
                "            FAIL();\n" <<
                "        }\n" <<
                "    }\n" <<
                "\n\n";
        }
    }
#endif
}