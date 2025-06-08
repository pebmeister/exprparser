#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <string>

#include "Token.h"
#include "Tokenizer.h"
#include "parser.h"
#include "grammar_rule.h"
#include "ASTNode.h"
#include "expr_rules.h"

// Tokenizer setup
Tokenizer tokenizer({

    { NUMBER,   "\\d+" },
    { HEXNUM,   "\\$[0-9a-fA-F]+" },
    { BINNUM,   "\\%[0-1]+" },
    { PLUS,     "\\+"},
    { MINUS,    "\\-"},
    { COMMA,    "\\,"},
    { X,        "[Xx]"},
    { Y,        "[Yy]"},
    { POUND,    "\\#"},
    { MUL,      "\\*"},
    { DIV,      "\\/"},
    { BIT_AND,  "\\&"},
    { BIT_OR,   "\\|"},
    { SLEFT,    "\\<<"},
    { SRIGHT,   "\\>>"},
    { LPAREN,   "\\("},
    { RPAREN,   "\\)"},
    { WS,       "[ \t]+" },

    { ORA,      "[Oo][Rr][Aa]" },
    { AND,      "[Aa][Nn][Dd]" },
    { EOR,      "[Ee][Oo][Rr]" },
    { ADC,      "[Aa][Dd][Cc]" },
    { SBC,      "[Ss][Bb][Cc]" },

    });


// Parser dictionary
std::map<int64_t, std::string> parserDict = {
    { NUMBER,       "NUMBER"},
    { HEXNUM,       "HEXNUM"},
    { BINNUM,       "BINNUM"},
    { PLUS,         "PLUS"},
    { MINUS,        "MINUS"},
    { COMMA,        "COMMA"},
    { X,            "X"},
    { Y,            "Y"},
    { POUND,        "POUND"},
    { MUL,          "MUL"},
    { DIV,          "DIV"},
    { BIT_AND,      "BIT_AND"},
    { BIT_OR,       "BIT_OR"},
    { SLEFT,        "SHIFT_LEFT"},
    { SRIGHT,       "SHIFT_LEFT" },
    { LPAREN,       "LEFT_PAREN"},
    { RPAREN,       "RIGHT_PAREN"},
    { WS,           "WHITE_SPACE" },
    { Factor,       "Factor" },
    { MulExpr,      "MulExpr" },
    { AddExpr,      "AddExpr" },
    { AddrExpr,     "AddrExpr" },
    { OrExpr,       "OrExpr" },
    { SExpr,        "ShiftExpr"},

    { ORA,          "ORA"},
    { AND,          "AND"},
    { EOR,          "EOR"},
    { ADC,          "ADC"},
    { SBC,          "SBC"},

    { OpCode,       "OpCode"},
    { Op_Implied,   "OpCode_Implied"},
    { Op_Immediate, "OpCode_Immediate"},
    { Op_Absolute,  "OpCode_Absolute"},
    { Op_AbsoluteX, "OpCode_AbsoluteX"},
    { Op_AbsoluteY, "OpCode_AbsoluteY"},
    { Op_Indirect,  "OpCode_Indirect"},
    { Op_IndirectX, "OpCode_IndirectX"},
    { Op_IndirectY, "OpCode_IndirectY"},
    { Expr,         "Expr"},
    { Prog,         "Prog"},
};

// Grammar rules
const std::vector<std::shared_ptr<GrammarRule>> rules = {
    // Factor: NUMBER or (Expr)
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Factor, NUMBER },
            { Factor, HEXNUM },
            { Factor, BINNUM },
            { Factor, LPAREN, -Expr, RPAREN },
            { Factor, MINUS, -Factor },
            { Factor, PLUS, -Factor }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Factor);
            for (const auto& arg : args) node->add_child(arg);

            switch (args.size()) {
                case 1:
                {
                    const Token& tok = std::get<Token>(args[0]);
                    switch (tok.type) {
                        case NUMBER:
                        {
                            node->value = std::stoi(tok.value);
                            break;
                        }

                        case HEXNUM:
                        {
                            std::string n = tok.value.substr(1);
                            node->value = std::stol(n, nullptr, 16);
                            break;
                        }

                        case BINNUM:
                        {
                            std::string n = tok.value.substr(1);
                            node->value = std::stol(n, nullptr, 2);
                            break;
                        }

                        default:
                            throw std::runtime_error("Unknown token type in Factor rule");
                            break;
                    }
                    break;
                }
                case 2:
                {
                    // -Factor (unary minus)
                    const Token& op = std::get<Token>(args[0]);
                    auto t = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    switch (op.type) {
                        case MINUS:
                            node->value = -t->value;
                            break;

                        case PLUS:
                            node->value = t->value;
                            break;

                        default:
                            throw std::runtime_error("Unknown unary operator in Factor rule");
                            break;
                    }
                    break;
                }
                case 3:
                {
                    // (Expr)
                    auto t = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    node->value = t->value;
                    break;
                }
                default:
                    throw std::runtime_error("Syntax error in Factor rule");
            }

            return node;
        }
    ),

    // MulExpr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { MulExpr, -Factor }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                {MUL, DIV},
                MulExpr,
                [&p]() { return p.parse_rule(Factor); },
                [](int l, TOKEN_TYPE op, int r)
                {
                    return op == MUL ? l * r : l / r;
                },
                "a factor"
            );
        }
    ),

    // AddExpr ::= MulExpr ( (PLUS|MINUS) MulExpr )*
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { AddExpr, -MulExpr }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                { PLUS, MINUS },
                AddExpr,
                [&p]() { return p.parse_rule(MulExpr); },
                [](int l, TOKEN_TYPE op, int r)
                {
                    return op == PLUS ? l + r : l - r;
                },
                "a term"
            );
        }
    ),

    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { SExpr, -AddExpr }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                { SLEFT, SRIGHT },
                AddExpr,
                [&p]() { return p.parse_rule(MulExpr); },
                [](int l, TOKEN_TYPE op, int r)
                {
                    return op == SLEFT ? l << r : l >> r;
                },
                "a term"
            );
        }
    ),

    // AndExpr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { AndExpr, -SExpr }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                {BIT_AND},
                AndExpr,
                [&p]() { return p.parse_rule(AddExpr); },
                [](int l, TOKEN_TYPE op, int r) { return l & r; },
                "an add expression"
            );
        }
    ),

    // OrExpr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { OrExpr, -AndExpr }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                {BIT_OR},
                OrExpr,
                [&p]() { return p.parse_rule(AndExpr); },
                [](int l, TOKEN_TYPE op, int r) { return l | r; },
                "an or expression"
            );
        }
    ),

    // OpCode
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { OpCode, ORA },
            { OpCode, AND },
            { OpCode, EOR },
            { OpCode, ADC },
            { OpCode, SBC },
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(OpCode);
            for (const auto& arg : args) node->add_child(arg);
            switch (args.size()) {
                case 1:
                {
                    const Token& tok = std::get<Token>(args[0]);
                    node->value = tok.type;
                    break;
                }

                default:
                    throw std::runtime_error("Unknown token type in Factor rule");
                    break;

            }
            return node;
        }
    ),

    // Op_Implied
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_Implied, -OpCode }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_Implied);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            node->value = (left->value * 1000 & 0xFFFF);
            return node;
        }
    ),

    // Op_Immediate
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_Immediate, -OpCode, POUND, -Expr }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_Immediate);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            auto right = std::get<std::shared_ptr<ASTNode>>(args[2]);
            node->value = (left->value * 1000 & 0xFFFF) + right->value;
            return node;
        }
    ),
        
    // Op_Absolute | Zero Page or Relative
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_Absolute, -OpCode, -Expr }
        },
        [](Parser& p, const auto& args)
        {
            // Lookahead: if next token is COMMA, fail this rule
            if (p.current_pos < p.tokens.size() && p.tokens[p.current_pos].type == COMMA) {
                throw std::runtime_error("Lookahead: COMMA after address, not Op_Absolute");
            }

            auto node = std::make_shared<ASTNode>(Op_Absolute);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            auto right = std::get<std::shared_ptr<ASTNode>>(args[1]);
            node->value = (left->value * 1000 & 0xFFFF) + right->value;
            return node;
        }
    ),
    
    // Op_AbsoluteX | Zero PageX
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_AbsoluteX, -OpCode, -AddrExpr, COMMA, X }
        },
        [](Parser& p, const auto& args)
        {
            std::cout << "Matched Op_AbsoluteX rule" << std::endl;
            auto node = std::make_shared<ASTNode>(Op_AbsoluteX);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            auto right = std::get<std::shared_ptr<ASTNode>>(args[1]);
            node->value = (left->value * 1000 & 0xFFFF) + right->value;
            return node;
        }
    ),

    // Op_AbsoluteY | Zero PageY
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_AbsoluteY, -OpCode, -AddrExpr, COMMA, Y }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_AbsoluteY);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            auto right = std::get<std::shared_ptr<ASTNode>>(args[1]);
            node->value = (left->value * 1000 & 0xFFFF) + right->value;
            return node;
        }
    ),

    // Op_Indirect
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_Indirect, -OpCode, LPAREN, -AddrExpr, RPAREN }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_Indirect);
            for (const auto& arg : args) node->add_child(arg);
            
            auto left = std::get<std::shared_ptr<ASTNode>>(args[2]);
            node->value = (left->value * 1000 & 0xFFFF);
            return node;
        }
    ),
        
    // Op_IndirectX
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>> {
            { Op_IndirectX, -OpCode, LPAREN, -AddrExpr, COMMA, X, RPAREN }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_IndirectX);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[2]);
            node->value = (left->value * 1000 & 0xFFFF);
            return node;
        }
    ),

    // Op_IndirectY
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>> {
            { Op_IndirectY, -OpCode, LPAREN, -AddrExpr, RPAREN, COMMA, Y }
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_IndirectY);
            for (const auto& arg : args) node->add_child(arg);

            auto left = std::get<std::shared_ptr<ASTNode>>(args[2]);
            node->value = (left->value * 1000 & 0xFFFF);
            return node;
        }
    ),
        
        // AddrExpr ::= Expr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { AddrExpr, -Expr }
        },
        [](Parser& p, const auto& args)
        {
            return std::get<std::shared_ptr<ASTNode>>(args[0]);
        }
    ),

    // Expr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Expr, -OrExpr },
        },
        [](Parser& p, const auto& args)
        {
            return std::get<std::shared_ptr<ASTNode>>(args[0]);
        }
    ),

    // Prog
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Prog, -Op_Immediate   },
            { Prog, -Op_IndirectX   },
            { Prog, -Op_IndirectY   },
            { Prog, -Op_Indirect    },
            { Prog, -Op_AbsoluteX   },
            { Prog, -Op_AbsoluteY   },
            { Prog, -Op_Absolute    },
            { Prog, -Op_Implied     },
        },
        [](Parser& p, const auto& args)
        {
            return std::get<std::shared_ptr<ASTNode>>(args[0]);
        }
    )

};
