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

static ANSI_ESC esc;
ParserOptions options;

#pragma warning(disable:4996)

static std::string startdir = "C:\\Users\\Windows\\source\\repos\\exprparser\\src\\unittests\\testfiles\\";

namespace parser_unit_test
{
    static std::shared_ptr<ASTNode> make_node(int64_t type, int32_t value, SourcePos pos, std::vector<RuleArg> children = {})
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

    auto tok = [](TOKEN_TYPE type, std::string txt, SourcePos pos, size_t val = 0, bool start = true)
        {
            return Token{ type, std::move(txt), pos, val, start };
        };

    auto node = [](RULE_TYPE type, int val, SourcePos pos, auto... children)
        {
            return make_node(type, val, pos, { children... });
        };

    auto make_implied = [](SourcePos pos, Token token, int opCode, int _, size_t line)
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

    auto make_immediate = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_zeropage = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_zeropagex = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_absolute = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_absolutex = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_absolutey = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_indirectx = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto make_indirecty = [](SourcePos pos, Token token, int opCode, int val, size_t line)
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

    auto implied = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_implied(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto immediate = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_immediate(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto zeropage = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropage(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto zeropagex = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_zeropagex(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto absolute = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolute(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto absolutex = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolutex(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto absolutey = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_absolutey(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto indirectx = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_indirectx(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    auto indirecty = [](SourcePos pos, TOKEN_TYPE t, std::string tokstr, int opCode, int val)
        {
            return make_indirecty(pos, tok(t, tokstr, pos, val, false), opCode, val, pos.line);
        };

    static void compareAST(const std::shared_ptr<ASTNode>& a, const std::shared_ptr<ASTNode>& b)
    {
        if (!a || !b) {
            EXPECT_TRUE((a == nullptr) && (b == nullptr));
        }
        // Compare type and value
        EXPECT_EQ(a->type, b->type) << a->value;
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
                EXPECT_EQ(atok.type, btok.type);
                EXPECT_EQ(atok.value, btok.value);
                EXPECT_EQ(atok.pos.line, btok.pos.line);
            }
        }
    }

    TEST(ast_unit_test, adc)
    {
        std::string op = "adc";
        std::string file = op + ".asm";

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), ADC, op, 0x69, 0x01),
                zeropage(pos(file, 2), ADC, op, 0x65, 0x02),
                zeropagex(pos(file, 3), ADC, op, 0x75, 0x03),
                absolute(pos(file, 4), ADC, op, 0x6D, 0x0405),
                absolutex(pos(file, 5), ADC, op, 0x7D, 0x0607),
                absolutey(pos(file, 6), ADC, op, 0x79, 0x0809),
                indirectx(pos(file, 7), ADC, op, 0x61, 0x0A),
                indirecty(pos(file, 8), ADC, op, 0x71, 0x0B)
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

    TEST(ast_unit_test, lda)
    {
        std::string op = "lda";
        std::string file = op + ".asm";

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 8),
            node(RULE_TYPE::LineList, 0, pos(file, 8),
                immediate(pos(file, 1), LDA, op, 0xA9, 0x01),
                zeropage(pos(file, 2), LDA, op, 0xA5, 0x02),
                zeropagex(pos(file, 3), LDA, op, 0xB5, 0x03),
                absolute(pos(file, 4), LDA, op, 0xAD, 0x0405),
                absolutex(pos(file, 5), LDA, op, 0xBD, 0x0607),
                absolutey(pos(file, 6), LDA, op, 0xB9, 0x0809),
                indirectx(pos(file, 7), LDA, op, 0xA1, 0x0A),
                indirecty(pos(file, 8), LDA, op, 0xB1, 0x0B)
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

    TEST(ast_unit_test, nop)
    {
        std::string op = "nop";
        std::string file = op + ".asm";

        auto expected = node(RULE_TYPE::Prog, 0, pos(file, 1),
            node(RULE_TYPE::LineList, 0, pos(file, 1),
                implied(pos(file, 1), NOP, op, 0xEA, 0)
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
}