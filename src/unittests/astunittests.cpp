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

#pragma warning(disable:4996)

namespace fs = std::filesystem;

static ANSI_ESC esc;
extern std::map<int64_t, std::string> parserDict;

#ifdef TESTFILES_DIR
static std::string startdir = TESTFILES_DIR "/";
#else
static std::string startdir = "./testfiles/";
#endif

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

    inline auto make_zeropagerelative = [](SourcePos pos, Token token, int opCode, int zp_val, int rel_val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_ZeroPageRelative, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)
                            ),
                            node(RULE_TYPE::Expr, zp_val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            node(RULE_TYPE::Expr, rel_val, pos)
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

    inline auto make_zeropagey = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_ZeroPageY, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(COMMA, ",", pos, COMMA),
                            tok(Y, "y", pos, Y))
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

    inline auto make_indirect = [](SourcePos pos, Token token, int opCode, int val, size_t line)
        {
            return node(RULE_TYPE::Line, pos.line, pos,
                node(RULE_TYPE::Statement, opCode, pos,
                    node(RULE_TYPE::Op_Instruction, opCode, pos,
                        node(RULE_TYPE::Op_Indirect, opCode, pos,
                            node(RULE_TYPE::OpCode, token.type, pos,
                                tok(token.type, token.value, pos, token.type)),
                            tok(LPAREN, "(", pos, LPAREN),
                            node(RULE_TYPE::AddrExpr, val, pos),
                            tok(RPAREN, ")", pos, RPAREN)
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

    inline auto zeropagerelative = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int zp_val, int rel_val)
        {
            return make_zeropagerelative(pos, tok(t, tokstr, pos, zp_val, false), opCode, zp_val, rel_val, pos.line);
        };
        
    inline auto zeropagex = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropagex(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    inline auto zeropagey = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropagey(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
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

    inline auto indirect = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_indirect(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
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
        std::ostringstream ossa;
        a->print(ossa, false);
        std::string a_astStr = ossa.str();

        std::ostringstream ossb;
        b->print(ossb, false);
        std::string b_astStr = ossb.str();

        EXPECT_TRUE(a_astStr == b_astStr) << a_astStr << b_astStr;

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

    TEST(ast_unit_test, ora)
    {
        std::string op = "ora";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ORA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x09, 0x01),
                absolute(pos(file, 2), tok, op, 0x0d, 0x0203),
                zeropage(pos(file, 3), tok, op, 0x05, 0x04),
                absolutex(pos(file, 4), tok, op, 0x1d, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0x15, 0x07),
                absolutey(pos(file, 6), tok, op, 0x19, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x01, 0x0a),
                indirecty(pos(file, 8), tok, op, 0x11, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = AND;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x29, 0x01),
                absolute(pos(file, 2), tok, op, 0x2d, 0x0203),
                zeropage(pos(file, 3), tok, op, 0x25, 0x04),
                absolutex(pos(file, 4), tok, op, 0x3d, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0x35, 0x07),
                absolutey(pos(file, 6), tok, op, 0x39, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x21, 0x0a),
                indirecty(pos(file, 8), tok, op, 0x31, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, eor)
    {
        std::string op = "eor";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = EOR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x49, 0x01),
                absolute(pos(file, 2), tok, op, 0x4d, 0x0203),
                zeropage(pos(file, 3), tok, op, 0x45, 0x04),
                absolutex(pos(file, 4), tok, op, 0x5d, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0x55, 0x07),
                absolutey(pos(file, 6), tok, op, 0x59, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x41, 0x0a),
                indirecty(pos(file, 8), tok, op, 0x51, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, adc)
    {
        std::string op = "adc";
        std::string file = fs::absolute(fs::path(fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string())).lexically_normal().string();

        auto tok = ADC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x69, 0x01),
                absolute(pos(file, 2), tok, op, 0x6d, 0x0203),
                zeropage(pos(file, 3), tok, op, 0x65, 0x04),
                absolutex(pos(file, 4), tok, op, 0x7d, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0x75, 0x07),
                absolutey(pos(file, 6), tok, op, 0x79, 0x0809),
                indirectx(pos(file, 7), tok, op, 0x61, 0x0a),
                indirecty(pos(file, 8), tok, op, 0x71, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sbc)
    {
        std::string op = "sbc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SBC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xe9, 0x01),
                absolute(pos(file, 2), tok, op, 0xed, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xe5, 0x04),
                absolutex(pos(file, 4), tok, op, 0xfd, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0xf5, 0x07),
                absolutey(pos(file, 6), tok, op, 0xf9, 0x0809),
                indirectx(pos(file, 7), tok, op, 0xe1, 0x0a),
                indirecty(pos(file, 8), tok, op, 0xf1, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, cmp)
    {
        std::string op = "cmp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CMP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xc9, 0x01),
                absolute(pos(file, 2), tok, op, 0xcd, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xc5, 0x04),
                absolutex(pos(file, 4), tok, op, 0xdd, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0xd5, 0x07),
                absolutey(pos(file, 6), tok, op, 0xd9, 0x0809),
                indirectx(pos(file, 7), tok, op, 0xc1, 0x0a),
                indirecty(pos(file, 8), tok, op, 0xd1, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CPX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xe0, 0x01),
                absolute(pos(file, 2), tok, op, 0xec, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xe4, 0x04)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, cpy)
    {
        std::string op = "cpy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CPY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xc0, 0x01),
                absolute(pos(file, 2), tok, op, 0xcc, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xc4, 0x04)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, dec)
    {
        std::string op = "dec";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = DEC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0xce, 0x0102),
                zeropage(pos(file, 2), tok, op, 0xc6, 0x03),
                absolutex(pos(file, 3), tok, op, 0xde, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0xd6, 0x06)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, dex)
    {
        std::string op = "dex";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = DEX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xca, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, dey)
    {
        std::string op = "dey";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = DEY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x88, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, inc)
    {
        std::string op = "inc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = INC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0xee, 0x0102),
                zeropage(pos(file, 2), tok, op, 0xe6, 0x03),
                absolutex(pos(file, 3), tok, op, 0xfe, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0xf6, 0x06)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, inx)
    {
        std::string op = "inx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = INX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xe8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, iny)
    {
        std::string op = "iny";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = INY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xc8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ASL;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x0e, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x06, 0x03),
                absolutex(pos(file, 3), tok, op, 0x1e, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x16, 0x06),
                accumulator(pos(file, 5), tok, op, 0x0a, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rol)
    {
        std::string op = "rol";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ROL;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x2e, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x26, 0x03),
                absolutex(pos(file, 3), tok, op, 0x3e, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x36, 0x06),
                accumulator(pos(file, 5), tok, op, 0x2a, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, lsr)
    {
        std::string op = "lsr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LSR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x4e, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x46, 0x03),
                absolutex(pos(file, 3), tok, op, 0x5e, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x56, 0x06),
                accumulator(pos(file, 5), tok, op, 0x4a, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, ror)
    {
        std::string op = "ror";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ROR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x6e, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x66, 0x03),
                absolutex(pos(file, 3), tok, op, 0x7e, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x76, 0x06),
                accumulator(pos(file, 5), tok, op, 0x6a, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LDA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xa9, 0x01),
                absolute(pos(file, 2), tok, op, 0xad, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xa5, 0x04),
                absolutex(pos(file, 4), tok, op, 0xbd, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0xb5, 0x07),
                absolutey(pos(file, 6), tok, op, 0xb9, 0x0809),
                indirectx(pos(file, 7), tok, op, 0xa1, 0x0a),
                indirecty(pos(file, 8), tok, op, 0xb1, 0x0b)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sta)
    {
        std::string op = "sta";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = STA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x8d, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x85, 0x03),
                absolutex(pos(file, 3), tok, op, 0x9d, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x95, 0x06),
                absolutey(pos(file, 5), tok, op, 0x99, 0x0708),
                indirectx(pos(file, 6), tok, op, 0x81, 0x09),
                indirecty(pos(file, 7), tok, op, 0x91, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, ldx)
    {
        std::string op = "ldx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LDX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xa2, 0x01),
                absolute(pos(file, 2), tok, op, 0xae, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xa6, 0x04),
                absolutey(pos(file, 4), tok, op, 0xbe, 0x0506),
                zeropagey(pos(file, 5), tok, op, 0xb6, 0x07)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, stx)
    {
        std::string op = "stx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = STX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x8e, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x86, 0x03),
                zeropagey(pos(file, 3), tok, op, 0x96, 0x04)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, ldy)
    {
        std::string op = "ldy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LDY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xa0, 0x01),
                absolute(pos(file, 2), tok, op, 0xac, 0x0203),
                zeropage(pos(file, 3), tok, op, 0xa4, 0x04),
                absolutex(pos(file, 4), tok, op, 0xbc, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0xb4, 0x07)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sty)
    {
        std::string op = "sty";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = STY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x8c, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x84, 0x03),
                zeropagex(pos(file, 3), tok, op, 0x94, 0x04)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb0)
    {
        std::string op = "rmb0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB0;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x07, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb1)
    {
        std::string op = "rmb1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB1;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x17, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb2)
    {
        std::string op = "rmb2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB2;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x27, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb3)
    {
        std::string op = "rmb3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB3;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x37, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb4)
    {
        std::string op = "rmb4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB4;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x47, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb5)
    {
        std::string op = "rmb5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB5;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x57, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb6)
    {
        std::string op = "rmb6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB6;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x67, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rmb7)
    {
        std::string op = "rmb7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RMB7;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x77, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb0)
    {
        std::string op = "smb0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB0;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x87, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb1)
    {
        std::string op = "smb1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB1;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0x97, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb2)
    {
        std::string op = "smb2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB2;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xa7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb3)
    {
        std::string op = "smb3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB3;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xb7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb4)
    {
        std::string op = "smb4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB4;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xc7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb5)
    {
        std::string op = "smb5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB5;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xd7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb6)
    {
        std::string op = "smb6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB6;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xe7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, smb7)
    {
        std::string op = "smb7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SMB7;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropage(pos(file, 1), tok, op, 0xf7, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, stz)
    {
        std::string op = "stz";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = STZ;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x9c, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x64, 0x03),
                absolutex(pos(file, 3), tok, op, 0x9e, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x74, 0x06)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, tax)
    {
        std::string op = "tax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TAX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xaa, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, txa)
    {
        std::string op = "txa";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TXA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x8a, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, tay)
    {
        std::string op = "tay";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TAY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xa8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, tya)
    {
        std::string op = "tya";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TYA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x98, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bra)
    {
        std::string op = "bra";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BRA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x80, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BPL;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x10, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BMI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x30, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bvc)
    {
        std::string op = "bvc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BVC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x50, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BVS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x70, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BCC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0x90, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BCS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xb0, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BNE;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xd0, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BEQ;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                relative(pos(file, 1), tok, op, 0xf0, 0x1000)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr0)
    {
        std::string op = "bbr0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR0;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x0f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr1)
    {
        std::string op = "bbr1";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR1;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x1f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr2)
    {
        std::string op = "bbr2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR2;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x2f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr3)
    {
        std::string op = "bbr3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR3;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x3f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr4)
    {
        std::string op = "bbr4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR4;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x4f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr5)
    {
        std::string op = "bbr5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR5;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x5f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr6)
    {
        std::string op = "bbr6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR6;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x6f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbr7)
    {
        std::string op = "bbr7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBR7;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x7f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs0)
    {
        std::string op = "bbs0";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS0;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x8f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs1)
    {
        std::string op = "bbs1";
        std::string file =  fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS1;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0x9f, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs2)
    {
        std::string op = "bbs2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS2;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xaf, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs3)
    {
        std::string op = "bbs3";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS3;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xbf, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs4)
    {
        std::string op = "bbs4";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS4;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xcf, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs5)
    {
        std::string op = "bbs5";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS5;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xdf, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs6)
    {
        std::string op = "bbs6";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS6;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xef, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, bbs7)
    {
        std::string op = "bbs7";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BBS7;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                zeropagerelative(pos(file, 1), tok, op, 0xff, 0x01, 0x02)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, stp)
    {
        std::string op = "stp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = STP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xdb, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, wai)
    {
        std::string op = "wai";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = WAI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xcb, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BRK;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x00, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rti)
    {
        std::string op = "rti";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RTI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x40, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, jsr)
    {
        std::string op = "jsr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = JSR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x20, 0x0102)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rts)
    {
        std::string op = "rts";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RTS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x60, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, jmp)
    {
        std::string op = "jmp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = JMP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x4c, 0x0102),
                indirect(pos(file, 2), tok, op, 0x6c, 0x0304),
                indirectx(pos(file, 3), tok, op, 0x7c, 0x05)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = BIT;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x89, 0x01),
                absolute(pos(file, 2), tok, op, 0x2c, 0x0203),
                zeropage(pos(file, 3), tok, op, 0x24, 0x04),
                absolutex(pos(file, 4), tok, op, 0x3c, 0x0506),
                zeropagex(pos(file, 5), tok, op, 0x34, 0x07)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, trb)
    {
        std::string op = "trb";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TRB;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x1c, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x14, 0x03)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, tsb)
    {
        std::string op = "tsb";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TSB;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x0c, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x04, 0x03)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CLC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x18, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sec)
    {
        std::string op = "sec";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SEC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x38, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, cld)
    {
        std::string op = "cld";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CLD;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xd8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sed)
    {
        std::string op = "sed";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SED;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xf8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, cli)
    {
        std::string op = "cli";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CLI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x58, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sei)
    {
        std::string op = "sei";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SEI;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0x78, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, clv)
    {
        std::string op = "clv";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = CLV;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xb8, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

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
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = NOP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                implied(pos(file, 1), tok, op, 0xea, 0x0)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, slo)
    {
        std::string op = "slo";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SLO;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x0f, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x07, 0x03),
                absolutex(pos(file, 3), tok, op, 0x1f, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x17, 0x06),
                absolutey(pos(file, 5), tok, op, 0x1b, 0x0708),
                indirectx(pos(file, 6), tok, op, 0x03, 0x09),
                indirecty(pos(file, 7), tok, op, 0x13, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rla)
    {
        std::string op = "rla";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RLA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x2f, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x27, 0x03),
                absolutex(pos(file, 3), tok, op, 0x3f, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x37, 0x06),
                absolutey(pos(file, 5), tok, op, 0x3b, 0x0708),
                indirectx(pos(file, 6), tok, op, 0x23, 0x09),
                indirecty(pos(file, 7), tok, op, 0x33, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sre)
    {
        std::string op = "sre";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SRE;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x4f, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x47, 0x03),
                absolutex(pos(file, 3), tok, op, 0x5f, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x57, 0x06),
                absolutey(pos(file, 5), tok, op, 0x5b, 0x0708),
                indirectx(pos(file, 6), tok, op, 0x43, 0x09),
                indirecty(pos(file, 7), tok, op, 0x53, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, rra)
    {
        std::string op = "rra";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = RRA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x6f, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x67, 0x03),
                absolutex(pos(file, 3), tok, op, 0x7f, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0x77, 0x06),
                absolutey(pos(file, 5), tok, op, 0x7b, 0x0708),
                indirectx(pos(file, 6), tok, op, 0x63, 0x09),
                indirecty(pos(file, 7), tok, op, 0x73, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, sax)
    {
        std::string op = "sax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SAX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0x8f, 0x0102),
                zeropage(pos(file, 2), tok, op, 0x87, 0x03),
                zeropagey(pos(file, 3), tok, op, 0x97, 0x04),
                indirectx(pos(file, 4), tok, op, 0x83, 0x05)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, lax)
    {
        std::string op = "lax";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LAX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0xaf, 0x0102),
                zeropage(pos(file, 2), tok, op, 0xa7, 0x03),
                absolutey(pos(file, 3), tok, op, 0xbf, 0x0405),
                zeropagey(pos(file, 4), tok, op, 0xb7, 0x06),
                indirectx(pos(file, 5), tok, op, 0xa3, 0x07),
                indirecty(pos(file, 6), tok, op, 0xb3, 0x08)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, dcp)
    {
        std::string op = "dcp";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = DCP;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0xcf, 0x0102),
                zeropage(pos(file, 2), tok, op, 0xc7, 0x03),
                absolutex(pos(file, 3), tok, op, 0xdf, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0xd7, 0x06),
                absolutey(pos(file, 5), tok, op, 0xdb, 0x0708),
                indirectx(pos(file, 6), tok, op, 0xc3, 0x09),
                indirecty(pos(file, 7), tok, op, 0xd3, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, isc)
    {
        std::string op = "isc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ISC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolute(pos(file, 1), tok, op, 0xef, 0x0102),
                zeropage(pos(file, 2), tok, op, 0xe7, 0x03),
                absolutex(pos(file, 3), tok, op, 0xff, 0x0405),
                zeropagex(pos(file, 4), tok, op, 0xf7, 0x06),
                absolutey(pos(file, 5), tok, op, 0xfb, 0x0708),
                indirectx(pos(file, 6), tok, op, 0xe3, 0x09),
                indirecty(pos(file, 7), tok, op, 0xf3, 0x0a)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, anc)
    {
        std::string op = "anc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ANC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x0b, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, anc2)
    {
        std::string op = "anc2";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ANC2;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x2b, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, alr)
    {
        std::string op = "alr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ALR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x4b, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, arr)
    {
        std::string op = "arr";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = ARR;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x6b, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, xaa)
    {
        std::string op = "xaa";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = XAA;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0x8b, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, axs)
    {
        std::string op = "axs";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = AXS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xcb, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, usbc)
    {
        std::string op = "usbc";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = USBC;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                immediate(pos(file, 1), tok, op, 0xeb, 0x01)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, ahx)
    {
        std::string op = "ahx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = AHX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolutey(pos(file, 1), tok, op, 0x9f, 0x0102),
                indirecty(pos(file, 2), tok, op, 0x93, 0x03)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, shy)
    {
        std::string op = "shy";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SHY;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolutex(pos(file, 1), tok, op, 0x9c, 0x0102)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, shx)
    {
        std::string op = "shx";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = SHX;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolutey(pos(file, 1), tok, op, 0x9e, 0x0102)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, tas)
    {
        std::string op = "tas";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = TAS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolutey(pos(file, 1), tok, op, 0x9b, 0x0102)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }

    TEST(ast_unit_test, las)
    {
        std::string op = "las";
        std::string file = fs::absolute(fs::path(startdir + op + ".asm")).lexically_normal().string();
        auto tok = LAS;

        const auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 1, pos(file, 1),
                absolutey(pos(file, 1), tok, op, 0xbb, 0x0102)
            )
        );
        try {
            ParserOptions options;
            options.files.push_back(file);

            ExpressionParser parser(options);
            auto ast = parser.parse();
            compareAST(ast, expected);
        }
        catch (const std::exception& ex) {
            FAIL();
        }
    }
}