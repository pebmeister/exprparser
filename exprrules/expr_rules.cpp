#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "Token.h"
#include "Tokenizer.h"
#include "parser.h"
#include "grammar_rule.h"
#include "ASTNode.h"


#include "expr_rules.h"

// Tokenizer setup
Tokenizer tokenizer({
    { NUMBER,   "\\d+" },
    { PLUS,     "\\+"},
    { MINUS,    "\\-"},
    { MUL,      "\\*"},
    { DIV,      "/"},
    { BIT_AND,  "\\&"},
    { BIT_OR,   "\\|"},
    { LPAREN,   "\\("},
    { RPAREN,   "\\)"},
    { WS,       "[ \t]+" }
    });

// Parser dictionary
std::map<int64_t, std::string> parserDict = {
    { NUMBER,   "NUMBER"},
    { PLUS,     "PLUS"},
    { MINUS,    "MINUS"},
    { MUL,      "MUL"},
    { DIV,      "DIV"},
    { BIT_AND,  "BIT_AND"},
    { BIT_OR,   "BIT_OR"},
    { LPAREN,   "LEFT_PAREN"},
    { RPAREN,   "RIGHT_PAREN"},
    { WS,       "WHITE_SPACE" },
    { Factor,   "Factor" },
    { MulExpr,  "MulExpr"},
    { AddExpr,  "AddExpr"},
    { AndExpr,  "AndExpr"},
    { OrExpr,   "OrExpr"},
    { Expr,     "Expr"},
};

// Grammar rules
const std::vector<std::shared_ptr<GrammarRule>> rules = {
    // Factor: NUMBER or (Expr)
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Factor, NUMBER },
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
                    node->value = std::stoi(tok.value);
                    break;
                }
                case 2:
                {
                    // -Factor (unary minus)
                    const Token& op = std::get<Token>(args[0]);
                    auto t = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    if (op.type == MINUS)
                        node->value = -t->value;
                    else if (op.type == PLUS)
                        node->value = t->value;
                    else
                        throw std::runtime_error("Unknown unary operator in Factor rule");
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

    // AddExpr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { AddExpr, -MulExpr }
        },
        [](Parser& p, const auto& args)
        {
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            return p.handle_binary_operation(
                left,
                {PLUS, MINUS},
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

    // AndExpr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { AndExpr, -AddExpr }
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

    // Expr
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Expr, -OrExpr }
        },
        [](Parser& p, const auto& args)
        {
            return std::get<std::shared_ptr<ASTNode>>(args[0]);
        }
    )
};
