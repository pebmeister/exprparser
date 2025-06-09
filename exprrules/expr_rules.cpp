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
#include "expr_tokens.h"

// Grammar rules
const std::vector<std::shared_ptr<GrammarRule>> rules = {
    // Number
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Number, DECNUM },
            { Number, HEXNUM },
            { Number, BINNUM },
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Number);
            for (const auto& arg : args) node->add_child(arg);

            switch (args.size()) {
                case 1:
                {
                    const Token& tok = std::get<Token>(args[0]);
                    switch (tok.type) {
                        case DECNUM:
                        {
                            std::string n = tok.value.substr(0);
                            node->value = std::stol(n, nullptr, 10);
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
                }
            }
            return node;
        }
    ),

    // Factor: Number or (Expr)
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Factor, -Number },
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
                    auto t = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    node->value = t->value;
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
            { OpCode, CMP },
            { OpCode, CPX },
            { OpCode, CPY },
            { OpCode, DEC },
            { OpCode, DEX },
            { OpCode, DEY },
            { OpCode, INC },
            { OpCode, INX },
            { OpCode, INY },
            { OpCode, ASL },
            { OpCode, ROL },
            { OpCode, LSR },
            { OpCode, ROR },
            { OpCode, LDA },
            { OpCode, STA },
            { OpCode, LDX },
            { OpCode, STX },
            { OpCode, LDY },
            { OpCode, STY },
            { OpCode, RMB0 },
            { OpCode, RMB1 },
            { OpCode, RMB2 },
            { OpCode, RMB3 },
            { OpCode, RMB4 },
            { OpCode, RMB5 },
            { OpCode, RMB6 },
            { OpCode, RMB7 },
            { OpCode, SMB0 },
            { OpCode, SMB1 },
            { OpCode, SMB2 },
            { OpCode, SMB3 },
            { OpCode, SMB4 },
            { OpCode, SMB5 },
            { OpCode, SMB6 },
            { OpCode, SMB7 },
            { OpCode, STZ },
            { OpCode, TAX },
            { OpCode, TXA },
            { OpCode, TAY },
            { OpCode, TYA },
            { OpCode, TSX },
            { OpCode, TXS },
            { OpCode, PLA },
            { OpCode, PHA },
            { OpCode, PLP },
            { OpCode, PHP },
            { OpCode, PHX },
            { OpCode, PHY },
            { OpCode, PLX },
            { OpCode, PLY },
            { OpCode, BRA },
            { OpCode, BPL },
            { OpCode, BMI },
            { OpCode, BVC },
            { OpCode, BVS },
            { OpCode, BCC },
            { OpCode, BCS },
            { OpCode, BNE },
            { OpCode, BEQ },
            { OpCode, BBR0 },
            { OpCode, BBR1 },
            { OpCode, BBR2 },
            { OpCode, BBR3 },
            { OpCode, BBR4 },
            { OpCode, BBR5 },
            { OpCode, BBR6 },
            { OpCode, BBR7 },
            { OpCode, BBS0 },
            { OpCode, BBS1 },
            { OpCode, BBS2 },
            { OpCode, BBS3 },
            { OpCode, BBS4 },
            { OpCode, BBS5 },
            { OpCode, BBS6 },
            { OpCode, BBS7 },
            { OpCode, STP },
            { OpCode, WAI },
            { OpCode, BRK },
            { OpCode, RTI },
            { OpCode, JSR },
            { OpCode, RTS },
            { OpCode, JMP },
            { OpCode, BIT },
            { OpCode, TRB },
            { OpCode, TSB },
            { OpCode, CLC },
            { OpCode, SEC },
            { OpCode, CLD },
            { OpCode, SED },
            { OpCode, CLI },
            { OpCode, SEI },
            { OpCode, CLV },
            { OpCode, NOP },
            { OpCode, SLO },
            { OpCode, RLA },
            { OpCode, SRE },
            { OpCode, RRA },
            { OpCode, SAX },
            { OpCode, LAX },
            { OpCode, DCP },
            { OpCode, ISC },
            { OpCode, ANC },
            { OpCode, ANC2 },
            { OpCode, ALR },
            { OpCode, ARR },
            { OpCode, XAA },
            { OpCode, AXS },
            { OpCode, USBC },
            { OpCode, AHX },
            { OpCode, SHY },
            { OpCode, SHX },
            { OpCode, TAS },
            { OpCode, LAS },
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

    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Op_Instruction, -Op_Immediate },
            { Op_Instruction, -Op_IndirectX },
            { Op_Instruction, -Op_IndirectY },
            { Op_Instruction, -Op_Indirect },
            { Op_Instruction, -Op_AbsoluteX },
            { Op_Instruction, -Op_AbsoluteY },
            { Op_Instruction, -Op_Absolute },
            { Op_Instruction, -Op_Implied },
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Op_Instruction);
            for (const auto& arg : args) node->add_child(arg);
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            node->value = left->value;
            return node;
        }
    ),

    // Comment
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Comment, COMMENT },
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Comment);
            for (const auto& arg : args) node->add_child(arg);
            const Token& tok = std::get<Token>(args[0]);
            node->value = 0;
            return node;
        }
    ),
        
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Statement, -Comment },
            { Statement, -Op_Instruction, -Comment },
            { Statement, -Op_Instruction },
        },
        [](Parser& p, const auto& args)
        {
            auto node = std::make_shared<ASTNode>(Statement);
            for (const auto& arg : args) node->add_child(arg);
            auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
            node->value = left->value;
            return node;
        }
    ),

    // Prog
    std::make_shared<GrammarRule>(
        std::vector<std::vector<int64_t>>{
            { Prog, -Statement },
        },
        [](Parser& p, const auto& args)
        {
            return std::get<std::shared_ptr<ASTNode>>(args[0]);
        }
    )
};
