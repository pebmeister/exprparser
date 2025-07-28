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

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "expr_rules.h"
#include "expressionparser.h"
#include "grammar_rule.h"
#include "parser.h"

#pragma warning(disable:4996)


static ANSI_ESC esc;
extern std::map<int64_t, std::string> parserDict;

static std::string startdir = "C:\\Users\\Windows\\source\\repos\\exprparser\\src\\unittests\\testfiles\\";

std::mutex mutex;

namespace parser_unit_test
{
    inline std::shared_ptr<ASTNode> make_node(int64_t type, int32_t value, SourcePos pos, std::vector<RuleArg> children = {})
    {        
        auto node = std::make_shared<ASTNode>();
        node->type = type;
        node->value = value;
        node->position = pos;
        node->children = std::move(children);
        return node;
    }

    // Short aliases
    auto pos = [](const std::string file, size_t line) { return SourcePos{ file, line }; };

    inline auto tok = [](TOKEN_TYPE type, std::string txt, SourcePos pos, size_t val = 0, bool start = true)
        {
            return Token{ type, std::move(txt), pos, val, start };
        };

    inline auto node = [](RULE_TYPE type, int val, SourcePos pos, auto... children)
        {
            return make_node(type, val, pos, { children... });
        };

    inline auto make_accumulator = [](SourcePos pos, Token token, int opCode, int _, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Accumulator, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)
                            ),
                            tok(A, "a", pos, A)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_implied = [](SourcePos pos, Token token, int opCode, int _, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Implied, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)
                            )
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_immediate = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Immediate, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)
                            ),
                            tok(POUND, "#", pos, POUND),
                            node(RULE_TYPE::Expr, val, pos)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_zeropage = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_ZeroPage, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)
                            ),
                            node(RULE_TYPE::Expr, val, pos)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_zeropagex = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_ZeroPageX, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            tok(X, "x", pos, X))
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_absolute = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Absolute, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::Expr, val, pos)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_relative = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Relative, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::Expr, val, pos)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_absolutex = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_AbsoluteX, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            tok(X, "x", pos, X)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_absolutey = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_AbsoluteY, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            tok(Y, "y", pos, Y)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_indirectx = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_IndirectX, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            tok(LPAREN, "(", pos, LPAREN),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            tok(X, "x", pos, X),
                            tok(RPAREN, ")", pos, RPAREN)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto make_indirecty = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_IndirectY, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            tok(LPAREN, "(", pos, LPAREN),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(RPAREN, ")", pos, RPAREN),
                            tok(COMMA, ",", pos, COMMA),
                            tok(Y, "y", pos, Y)
                        )
                    )
                ),
                tok(EOL, "\n", pos)
            );
        };

    inline auto implied = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_implied(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto accumulator = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_accumulator(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto immediate = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_immediate(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto zeropage = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropage(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto zeropagex = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropagex(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto absolute = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolute(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto absolutex = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolutex(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto absolutey = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolutey(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto indirectx = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_indirectx(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto indirecty = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_indirecty(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto relative = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_relative(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    void compareAST(const std::shared_ptr<ASTNode>& a, const std::shared_ptr<ASTNode>& b)
    {
        if (!a || !b) {
            EXPECT_TRUE((a == nullptr) && (b == nullptr));
        }
        // Compare type and value
        EXPECT_EQ(a->type, b->type) << " [" << parserDict[a->type] << "]  [" << parserDict[b->type] << "]";
        EXPECT_EQ(a->value, b->value);

        // Compare position, ignoring filename
        EXPECT_EQ(a->position.line, b->position.line);

        // Compare children count
        EXPECT_EQ(a->children.size(), b->children.size());

        // Compare children recursively
        for (size_t i = 0; i < a->children.size(); ++i) {
            const auto& ac = a->children[i];
            const auto& bc = b->children[i];

            EXPECT_EQ(ac.index(), bc.index());
            if (std::holds_alternative<std::shared_ptr<ASTNode>>(ac)) {
                compareAST(std::get<std::shared_ptr<ASTNode>>(ac), std::get<std::shared_ptr<ASTNode>>(bc));
            }
            else if (std::holds_alternative<Token>(ac)) {
                const Token& atok = std::get<Token>(ac);
                const Token& btok = std::get<Token>(bc);

                // Compare token type, value, and line only (not filename or line_pos/start)
                EXPECT_EQ(atok.type, btok.type) << " [" << parserDict[atok.type] << "]  [" << parserDict[btok.type] << "]";
                EXPECT_EQ(atok.value, btok.value);
                EXPECT_EQ(atok.pos.line, btok.pos.line);
            }
        }
    }

    TEST(ast_unit_test, adc)
    {
        std::string op = "adc";
        std::string file = op + ".asm";
        auto tok = ADC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), tok, op, 0x69, 0x01),
                zeropage(pos(file, 2), tok, op, 0x65, 0x02),
                zeropagex(pos(file, 3), tok, op, 0x75, 0x03),
                absolute(pos(file, 4), tok, op, 0x6D, 0x0405),
                absolutex(pos(file, 5), tok, op, 0x7D, 0x0607),
                absolutey(pos(file, 6), tok, op, 0x79, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x61, 0x0A),
                indirecty(pos(file, 8), tok, op, 0x71, 0x0B)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, and)
    {
        std::string op = "and";
        std::string file = op + ".asm";
        auto tok = AND;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), tok, op, 0x29, 0x01),
                zeropage(pos(file, 2), tok, op, 0x25, 0x02),
                zeropagex(pos(file, 3), tok, op, 0x35, 0x03),
                absolute(pos(file, 4), tok, op, 0x2D, 0x0405),
                absolutex(pos(file, 5), tok, op, 0x3D, 0x0607),
                absolutey(pos(file, 6), tok, op, 0x39, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x21, 0x0A),
                indirecty(pos(file, 8), tok, op, 0x31, 0x0B)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, asl)
    {
        std::string op = "asl";
        std::string file = op + ".asm";
        auto tok = ASL;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 6),
            node(RULE_TYPE::LineList, 0, pos(file, 6),
                implied(pos(file, 1), tok, op, 0x0A, 0x00),
                accumulator(pos(file, 2), tok, op, 0x0A, 0x00),
                zeropage(pos(file, 3), tok, op, 0x06, 0x01),
                zeropagex(pos(file, 4), tok, op, 0x16, 0x02),
                absolute(pos(file, 5), tok, op, 0x0E, 0x0304),
                absolutex(pos(file, 6), tok, op, 0x1E, 0x0506)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bcc)
    {
        std::string op = "bcc";
        std::string file = op + ".asm";
        auto tok = BCC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x90, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, beq)
    {
        std::string op = "beq";
        std::string file = op + ".asm";
        auto tok = BEQ;

        ParserOptions options;
        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xF0, 0x1000)
            )
        );

        try {
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bcs)
    {
        std::string op = "bcs";
        std::string file = op + ".asm";
        auto tok = BCS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xB0, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bit)
    {
        std::string op = "bit";
        std::string file = op + ".asm";
        auto tok = BIT;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 2),
            node(RULE_TYPE::LineList, 0, pos(file, 2),
                zeropage(pos(file, 1), tok, op, 0x24, 0x01),
                absolute(pos(file, 2), tok, op, 0x2C, 0x0203)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bmi)
    {
        std::string op = "bmi";
        std::string file = op + ".asm";
        auto tok = BMI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x30, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bne)
    {
        std::string op = "bne";
        std::string file = op + ".asm";
        auto tok = BNE;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xD0, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bpl)
    {
        std::string op = "bpl";
        std::string file = op + ".asm";
        auto tok = BPL;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x10, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, brk)
    {
        std::string op = "brk";
        std::string file = op + ".asm";
        auto tok = BRK;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x00, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }

    TEST(ast_unit_test, bvc)
    {
        std::string op = "bvc";
        std::string file = op + ".asm";
        auto tok = BVC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x50, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bvs)
    {
        std::string op = "bvs";
        std::string file = op + ".asm";
        auto tok = BVS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x70, 0x1000)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);
            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, clc)
    {
        std::string op = "clc";
        std::string file = op + ".asm";
        auto tok = CLC;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x18, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }

    TEST(ast_unit_test, cld)
    {
        std::string op = "cld";
        std::string file = op + ".asm";
        auto tok = CLD;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xD8, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }

    TEST(ast_unit_test, cli)
    {
        std::string op = "cli";
        std::string file = op + ".asm";
        auto tok = CLI;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x58, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }

    TEST(ast_unit_test, clv)
    {
        std::string op = "clv";
        std::string file = op + ".asm";
        auto tok = CLV;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xB8, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }

    TEST(ast_unit_test, cmp)
    {
        std::string op = "cmp";
        std::string file = op + ".asm";
        auto tok = CMP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), tok, op, 0xC9, 0x01),
                zeropage(pos(file, 2), tok, op, 0xC5, 0x02),
                zeropagex(pos(file, 3), tok, op, 0xD5, 0x03),
                absolute(pos(file, 4), tok, op, 0xCD, 0x0405),
                absolutex(pos(file, 5), tok, op, 0xDD, 0x0607),
                absolutey(pos(file, 6), tok, op, 0xD9, 0x0809),
                indirectx(pos(file, 7), tok, op, 0xC1, 0x0A),
                indirecty(pos(file, 8), tok, op, 0xD1, 0x0B)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, cpx)
    {
        std::string op = "cpx";
        std::string file = op + ".asm";
        auto tok = CPX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 3),
            node(RULE_TYPE::LineList, 0, pos(file, 3),
                immediate(pos(file, 1), tok, op, 0xE0, 0x01),
                zeropage(pos(file, 2), tok, op, 0xE4, 0x02),
                absolute(pos(file, 3), tok, op, 0xEC, 0x0304)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, lda)
    {
        std::string op = "lda";
        std::string file = op + ".asm";
        auto tok = LDA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), tok, op, 0xA9, 0x01),
                zeropage(pos(file, 2), tok, op, 0xA5, 0x02),
                zeropagex(pos(file, 3), tok, op, 0xB5, 0x03),
                absolute(pos(file, 4), tok, op, 0xAD, 0x0405),
                absolutex(pos(file, 5), tok, op, 0xBD, 0x0607),
                absolutey(pos(file, 6), tok, op, 0xB9, 0x0809),
                indirectx(pos(file, 7), tok, op, 0xA1, 0x0A),
                indirecty(pos(file, 8), tok, op, 0xB1, 0x0B)
            )
        );
        
        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, nop)
    {
        std::string op = "nop";
        std::string file = op + ".asm";
        auto tok = NOP;

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xEA, 0)
            )
        );

        try {
            ParserOptions options;
            options.files.push_back(startdir + file);

            ExpressionParser parser(options);
            auto ast = parser.parse();

            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL() << "Test failed";
        }
    }
}