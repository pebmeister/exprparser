#include <iostream>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "ASTNode.h"
#include "grammar_rule.h"
#include "opcodedict.h"
#include "parser.h"
#include "sym.h"
#include "token.h"
#include "tokenizer.h"
#include <iomanip>

static size_t line = 0;

void throwError(std::string str, Parser& p)
{
    throw std::runtime_error(
        str + " " + p.get_token_error_info()
    );
}

static std::string join_segments(std::string num)
{
    std::string out;
    for (auto& ch : num) {
        if (!std::isspace(ch)) {
            out += ch;
        }
    }
    return out;
}

static void handle_symbol(std::shared_ptr<ASTNode>& node,
    Parser& p,
    const Token& tok,
    std::map<std::string, Sym>& symTable,
    bool isGlobal)
{
    Sym sym;
    if (tok.start) {
        node->type = RULE_TYPE::Label;
      
        if (isGlobal) {

            auto unresolved = p.GetUnresolvedLocalSymbols();
            if (!unresolved.empty()) {
                std::string err = "Unresolved local symbols:";
                for (const auto& s : unresolved) {
                    err += " " + s.name + " accessed at line(s) ";
                    for (auto l : s.accessed) err += std::to_string(l) + " ";
                    err += "\n";
                }
                throwError(err, p);
            }
            p.localSymbolTable.clear();
        }

        if (symTable.contains(tok.value)) {
            sym = symTable[tok.value];
            
            if (sym.defined_in_pass) {
                if (!sym.changed && isGlobal && (sym.isPC && sym.value != p.PC))
                    throwError("Error symbol " + tok.value + " already defined", p);

                sym.changed = false;
                sym.defined_in_pass = true;                
                if (sym.initialized) {
                    if (sym.isPC) {
                        if (sym.value != p.PC) {
                            sym.changed = true;
                            sym.value = p.PC;
                        }
                    }
                }
                else {
                    if (sym.isPC) {
                        sym.initialized = true;
                        sym.changed = !sym.accessed.empty();
                        sym.value = p.PC;
                    }
                }
            }
            else {
                sym.defined_in_pass = true;
                sym.isPC = true;
                sym.initialized = true;
                sym.value = p.PC;
                sym.changed = sym.accessed.size() > 0;
            }
        }
        else {
            sym.name = tok.value;
            sym.isPC = true;
            sym.initialized = true;
            sym.value = p.PC;
            sym.accessed.clear();
            sym.changed = false;
            sym.defined_in_pass = true;
        }
        symTable[tok.value] = sym;
        node->value = sym.value;
    }
    else {
        if (symTable.contains(tok.value)) {
            sym = symTable[tok.value];
            if (!sym.was_accessed_by(tok.line)) {
                sym.accessed.push_back(tok.line);
            }
            node->value = sym.value;
        }
        else {
            if (isGlobal && p.localSymbolTable.contains(tok.value)) {
                std::cout << "Warning: " + tok.value + " is defined as local. Ignore this warning with -nowarn.\n";
            }

            sym.name = tok.value;
            sym.initialized = false;
            sym.value = 0;
            if (!sym.was_accessed_by(tok.line)) {
                sym.accessed.push_back(tok.line);
            }
            sym.changed = false;
            sym.defined_in_pass = false;
            symTable[tok.value] = sym;
            node->value = sym.value;
        }
    }
}

static void handle_local_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok)
{
    handle_symbol(node, p, tok, p.localSymbolTable, false);
}

static void handle_global_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok)
{
    handle_symbol(node, p, tok, p.symbolTable, true);
}

static std::shared_ptr<ASTNode> processRule(RULE_TYPE ruleType, 
    const std::vector<RuleArg>& args, Parser& p, int count)
{
    auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
    TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);

    // Check if opcode is valid
    auto it = opcodeDict.find(opcode);
    if (it == opcodeDict.end()) {
        throwError("Unknown opcode ", p);
    }

    // Check if opcode is valid for implied mode
    const OpCodeInfo& info = it->second;
    auto inf = info.mode_to_opcode.find(ruleType);
    if (inf == info.mode_to_opcode.end()) {
        ruleType = Op_Accumulator;
        inf = info.mode_to_opcode.find(ruleType);
    }
    if (inf == info.mode_to_opcode.end()) {
        auto& mode = p.parserDict[ruleType];
        auto mode_name = mode.substr(7);
        throwError("Opcode '" + info.mnemonic + "' does not support addressing mode " + mode_name, p);
    }
    auto node = std::make_shared<ASTNode>(ruleType);
    for (const auto& arg : args) node->add_child(arg);

    if (count == 0) {
        //std::cout << "processRule " 
        //    << std::setw(20) << parserDict[ruleType] << std::setw(0) 
        //    << " PC: $"
        //    << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
        //    << p.PC
        //    << std::dec << std::nouppercase << std::setfill(' ') << std::setw(0);
        p.PC++;
        //std::cout << " => "
        //    << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
        //    << p.PC << "\n"
        //    << std::dec << std::nouppercase << std::setfill(' ') << std::setw(0);
    }

    node->value = inf->second;
    return node;
}

static std::shared_ptr<ASTNode> processRule(std::vector<RULE_TYPE> rule,
    RuleArg l, RuleArg r, Parser& p, int count)
{
    RULE_TYPE ruleType;
    auto& left = std::get<std::shared_ptr<ASTNode>>(l);
    auto& right = std::get<std::shared_ptr<ASTNode>>(r);
    TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);

    auto it = opcodeDict.find(opcode);
    if (it == opcodeDict.end()) {
        throwError("Unknown opcode", p);
    }

    const OpCodeInfo& info = it->second;
    bool supports_two_byte = (info.mode_to_opcode.find(rule[0]) != info.mode_to_opcode.end());
    bool supports_one_byte = (info.mode_to_opcode.find(rule[1]) != info.mode_to_opcode.end());
    bool supports_relative = rule.size() == 3
        ? (info.mode_to_opcode.find(rule[2]) != info.mode_to_opcode.end())
        : false;

    if (!(supports_two_byte || supports_one_byte || supports_relative)) {
        ruleType = (RULE_TYPE) - 1;
        for (auto& type : rule)
            if (type != -1)
                ruleType = type;

        auto mode = ruleType != -1 ? p.parserDict[ruleType] : "";
        auto mode_name = mode.substr(7);
        throwError("Opcode '" + info.mnemonic + "' does not support addressing mode " + mode_name, p);
    }

    int op_value = right->value;
    bool is_large = (op_value & ~0xFF) != 0;
    bool out_of_range = (op_value & ~0xFFFF) != 0 || (!(supports_two_byte || supports_relative) && is_large);

    if (out_of_range) {
        throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(op_value) + ")", p);
    }

    int sz = 0;
    // Select the correct addressing mode
    if (supports_relative) {
        auto rel_value = op_value - (p.PC + 3); // we have not incremented the PC yet so use 3 not 2.
        if (op_value != 0) {
            if (((rel_value + 127) & ~0xFF) != 0) {
                throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(op_value) + ")", p);
            }
        }
        ruleType = rule[2];
        sz = 2;
    }
    else if (!is_large && supports_one_byte) {
        ruleType = rule[1];
        sz = 2;
    }
    else {
        ruleType = rule[0];
        sz = 3;
    }
    auto inf = info.mode_to_opcode.find(ruleType);
    auto node = std::make_shared<ASTNode>(ruleType);

    node->value = inf->second;
    if (count == 0) {

        //std::cout << "processRule "
        //    << std::setw(20) << parserDict[ruleType] << std::setw(0)      
        //    << " PC: $"
        //    << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
        //    << p.PC
        //    << std::dec << std::nouppercase << std::setfill(' ') << std::setw(0);

        p.PC += sz;

        //std::cout << " => " 
        //    << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
        //    << p.PC << "\n"
        //    << std::dec << std::nouppercase << std::setfill(' ') << std::setw(0);
    }

    return node;
}

// Grammar rules MAP
const std::unordered_map<int64_t, RuleHandler> grammar_rules =
{
    {
        Symbol,             // Key
        RuleHandler{        // Value
            {  // Productions vector
                { Symbol, LOCALSYM },
                { Symbol, SYM }
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {   // Action
                // ToDo: Normalize symbal case
                auto node = std::make_shared<ASTNode>(Symbol);
                for (const auto& arg : args) {
                    node->add_child(arg);  // Now matches RuleArg type
                }

                const Token& tok = std::get<Token>(node->children[0]);
                if (tok.type == LOCALSYM) {
                    //  ToDo: strip @ off from of symbol 
                    handle_local_sym(node, p, tok);
                }
                else {
                    handle_global_sym(node, p, tok);
                }
                return node;
            }
        }
    },
    // Number
    {
        Number,
        RuleHandler{
            {
                { Number, DECNUM },
                { Number, HEXNUM },
                { Number, BINNUM },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
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
                                std::string n = join_segments(tok.value.substr(1));
                                node->value = std::stol(n, nullptr, 16);
                                break;
                            }

                            case BINNUM:
                            {
                                std::string n = join_segments(tok.value.substr(1));
                                node->value = std::stol(n, nullptr, 2);
                                break;
                            }

                            default:
                                throwError("Unknown token type in Factor rule", p);
                                break;
                        }
                    }
                }
                return node;
            }
        }
    },
    // Equate
    {
        Equate,
        RuleHandler {
            {
                { Equate, -Symbol, EQUAL, -Expr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Equate);
                for (const auto& arg : args) node->add_child(arg);
                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[2]);

                std::shared_ptr<ASTNode> lab = std::get<std::shared_ptr<ASTNode>>(args[0]);
                Token symtok = std::get<Token>(lab->children[0]);
                if (symtok.type == SYM) {
                    if (p.symbolTable.contains(symtok.value)) {
                        Sym sym = p.symbolTable[symtok.value];
                        sym.isPC = false;
                        sym.initialized = true;
                        if (sym.value != value->value) {
                            sym.value = value->value;
                            sym.changed = true;
                        }
                        p.symbolTable[symtok.value] = sym;
                        node->value = sym.value;
                    }
                }
                else {
                    if (p.localSymbolTable.contains(symtok.value)) {
                        Sym sym = p.localSymbolTable[symtok.value];
                        sym.isPC = false;
                        sym.initialized = true;
                        sym.value = value->value;
                        p.localSymbolTable[symtok.value] = sym;
                        node->value = sym.value;
                    }
                }
                return node;
            }
        }
    },
    // Factor
    {
        Factor,
        RuleHandler {
            {
                { Factor, -Number },
                { Factor, -Symbol },
                { Factor, LPAREN, -Expr, RPAREN },
                { Factor, MINUS, -Factor },
                { Factor, PLUS, -Factor },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Factor);
                for (const auto& arg : args) node->add_child(arg);
                Token tok;
                switch (args.size()) {
                    case 1:
                    {
                        auto& t = std::get<std::shared_ptr<ASTNode>>(args[0]);
                        node->value = t->value;
                        break;
                    }
                    case 2:
                    {
                        // -Factor (unary minus)
                        const Token& op = std::get<Token>(args[0]);
                        tok = std::get<Token>(args[0]);
                        auto& t = std::get<std::shared_ptr<ASTNode>>(args[1]);
                        switch (op.type) {
                            case MINUS:
                                node->value = -t->value;
                                break;

                            case PLUS:
                                node->value = t->value;
                                break;

                            default:
                                throwError("Unknown unary operator in Factor rule", p);
                                break;
                        }
                        break;
                    }
                    case 3:
                    {
                        // (Expr)
                        tok = std::get<Token>(args[0]);
                        auto t = std::get<std::shared_ptr<ASTNode>>(args[1]);
                        node->value = t->value;
                        break;
                    }
                    default:
                        throwError("Syntax error in Factor rule", p);
                }

                return node;
            }
        }
    },
    // MulExpr ( multiply expression )
    {
        MulExpr,
        RuleHandler {
            {
                { MulExpr, -Factor }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {MUL, DIV},
                    MulExpr,
                    [&p]() { return p.parse_rule(Factor); },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        if (op == DIV && r == 0) {
                            throwError("Division by zero", p);
                        }
                        return op == MUL ? l * r : l / r;
                    },
                    "a factor"
                );
            }
        }
    },
    // AddExpr (add expression)
    {
        AddExpr,
        RuleHandler {
            {
                { AddExpr, -MulExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { PLUS, MINUS },
                    AddExpr,
                    [&p]() { return p.parse_rule(MulExpr); },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return op == PLUS ? l + r : l - r;
                    },
                    "a term"
                );
            }
        }
    },
    // SExpr (shift expression)
    {
        SExpr,
        RuleHandler {
            {
                { SExpr, -AddExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { SLEFT, SRIGHT },
                    SExpr,
                    [&p]() { return p.parse_rule(AddExpr); },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return op == SLEFT ? l << r : l >> r;
                    },
                    "a term"
                );
            }
        }
    },
    // AndExpr
    {
        AndExpr,
        RuleHandler{
            {
                { AndExpr, -SExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {BIT_AND},
                    AndExpr,
                    [&p]() { return p.parse_rule(SExpr); },
                    [&p](int l, TOKEN_TYPE op, int r) { return l & r; },
                    "an and expression"
                );
            }
        }
    },
    // OrExpr
    {
        OrExpr,
        RuleHandler{
            {
                { OrExpr, -AndExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {BIT_OR},
                    OrExpr,
                    [&p]() { return p.parse_rule(AndExpr); },
                    [&p](int l, TOKEN_TYPE op, int r) { return l | r; },
                    "an or expression"
                );
            }
        }
    },
    // XOrExpr
    {
        XOrExpr,
        RuleHandler {
            {
                { OrExpr, -OrExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { BIT_XOR },
                    XOrExpr,
                    [&p]() { return p.parse_rule(OrExpr); },
                    [&p](int l, TOKEN_TYPE op, int r) { return l ^ r; },
                    "an xor expression"
                );
            }
        }
    },
    // OpCode
    {
        OpCode,
        RuleHandler {
            {
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
                { OpCode, ANC2 },
                { OpCode, ANC },
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
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                const Token& tok = std::get<Token>(args[0]);
                auto node = std::make_shared<ASTNode>(OpCode);
                for (const auto& arg : args) node->add_child(arg);
                switch (args.size()) {
                    case 1:
                    {
                        node->value = tok.type;
                        TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(node->value);

                        // Check if opcode is valid
                        auto it = opcodeDict.find(opcode);
                        if (it == opcodeDict.end()) {
                            throwError("Unknown opcode " + tok.value, p);
                        }
                        break;
                    }

                    default:
                        throwError("Unknown token type in Factor rule", p);
                        break;
                }
                return node;
            }
        },
    },
    // Op_Implied
    {
        Op_Implied,
        RuleHandler {
            {
                { Op_Implied, -OpCode }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                return processRule(Op_Implied, args, p, count);
            }
        }
    },
    // Op_Accumulator
    {
        Op_Accumulator,
        RuleHandler {
            {
                { Op_Accumulator, -OpCode, A }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                return processRule(Op_Accumulator, args, p, count);
            }
        }
    },
    // Op_Immediate
    {
        Op_Immediate,
        RuleHandler{
            {
                { Op_Immediate, -OpCode, POUND, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {(RULE_TYPE) - 1, Op_Immediate },
                    args[0], args[2], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_Absolute
    {
        Op_Absolute,
        RuleHandler{
            {
                { Op_Absolute, -OpCode, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_Absolute, Op_ZeroPage, Op_Relative}, 
                    args[0], args[1], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_AbsoluteX
    {
        Op_AbsoluteX,
        RuleHandler {
            {
                { Op_AbsoluteX, -OpCode, -AddrExpr, COMMA, X }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_AbsoluteX, Op_ZeroPageX}, 
                    args[0], args[1], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_AbsoluteY
    {
        Op_AbsoluteY,
        RuleHandler {
            {
                { Op_AbsoluteY, -OpCode, -AddrExpr, COMMA, Y }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_AbsoluteY, Op_ZeroPageY}, 
                    args[0], args[1], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_Indirect
    {
        Op_Indirect,
        RuleHandler{
            {
                { Op_Indirect, -OpCode, LPAREN, -AddrExpr, RPAREN }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_Indirect, (RULE_TYPE)-1}, 
                    args[0], args[2], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_IndirectX
    {
        Op_IndirectX,
        RuleHandler{
            {
                { Op_IndirectX, -OpCode, LPAREN, -AddrExpr, COMMA, X, RPAREN }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_IndirectX, (RULE_TYPE)-1}, 
                    args[0], args[2], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_IndirectY
    {
        Op_IndirectY,
        RuleHandler{
            {
                { Op_IndirectY, -OpCode, LPAREN, -AddrExpr, RPAREN, COMMA, Y }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = processRule(std::vector<RULE_TYPE> {Op_IndirectY, (RULE_TYPE)-1}, 
                    args[0], args[2], p, count);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },
    // Op_ZeroPageRelative
    {
        Op_ZeroPageRelative,
        RuleHandler{
            {
                { Op_ZeroPageRelative, -OpCode, -Expr, COMMA, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto& zp = std::get<std::shared_ptr<ASTNode>>(args[1]);
                auto& rel = std::get<std::shared_ptr<ASTNode>>(args[3]);
                TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);

                auto it = opcodeDict.find(opcode);
                if (it == opcodeDict.end()) {
                    throwError("Unknown opcode in Op_ZeroPageRelative rule", p);
                }
                const OpCodeInfo& info = it->second;
                if (info.mode_to_opcode.find(Op_ZeroPageRelative) == info.mode_to_opcode.end()) {
                    throwError("Opcode '" + info.mnemonic + "' does not support zero page relative addressing mode", p);
                }

                int zp_addr = zp->value;
                int rel_offset = rel->value;

                if (zp_addr < 0 || zp_addr > 0xFF) {
                    throwError("Zero page address out of range (0-255)", p);
                }
                if (rel_offset < -128 || rel_offset > 127) {
                    throwError("Relative branch target out of range (-128 to 127)", p);
                }

                auto node = std::make_shared<ASTNode>(Op_ZeroPageRelative);
                node->add_child(left);
                node->add_child(zp);
                node->add_child(rel);

                // You can encode the value as needed for your backend
                node->value = left->value;
                if (count == 0)
                    p.PC += 3;
                return node;
            }
        }
    },
    // AddrExpr
    {
        AddrExpr,
        RuleHandler{
            {
                { AddrExpr, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(AddrExpr);
                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->value = left->value;
                return node;
            }
        }
    },
    // Expr
    {
        Expr,
        RuleHandler{
            {
                { Expr, -XOrExpr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Expr);
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->value = left->value;
                return node;
            }
        }
    },
    // Op_Instruction
    {
        Op_Instruction,
        RuleHandler{
            {
                { Op_Instruction, -Op_Accumulator },
                { Op_Instruction, -Op_Immediate },
                { Op_Instruction, -Op_IndirectX },
                { Op_Instruction, -Op_IndirectY },
                { Op_Instruction, -Op_Indirect },
                { Op_Instruction, -Op_AbsoluteX },
                { Op_Instruction, -Op_AbsoluteY },
                { Op_Instruction, -Op_Absolute },
                { Op_Instruction, -Op_ZeroPageRelative },
                { Op_Instruction, -Op_Implied },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Op_Instruction);
                for (const auto& arg : args) node->add_child(arg);
                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->value = left->value;
                return node;
            }
        }
    },
    // Comment
    {
        Comment,
        RuleHandler {
            {
                { Comment, COMMENT },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Comment);
                for (const auto& arg : args) node->add_child(arg);
            
                const Token& tok = std::get<Token>(args[0]);
                node->value = 0;
                return node;
            }
        }
    },
    // Statement
    {
        Statement,
        RuleHandler{
            {
                { Statement, -Equate, -Comment},
                { Statement, -Symbol,  -Comment },
                { Statement, -Op_Instruction, -Comment },
                { Statement, -Comment },
                { Statement, -Equate },
                { Statement, -Symbol },
                { Statement, -Op_Instruction },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Statement);
                for (const auto& arg : args) node->add_child(arg);
                if (args[0].index() == 1) {
                    auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    node->value = left->value;
                }
                return node;
            }
        }
    },
    // Line
    {
        Line,
        RuleHandler{
            {
                { Line, EOL },
                { Line, -Statement, EOL },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Line);
                for (const auto& arg : args) node->add_child(arg);

                if (count == 0)
                    ++line;
                node->value = line;
                return node;
            }
        }
    },
    // Prog
    {
        Prog,
        RuleHandler{
            {
                { Prog, -Line, -Prog },
                { Prog, -Line },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                line = 0;
                auto node = std::make_shared<ASTNode>(Prog);
                if (args.size() == 2) {
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    auto progNode = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    if (lineNode) node->add_child(lineNode);
                    if (progNode) {
                        // Flatten children if progNode is also a Prog node
                        for (const auto& child : progNode->children)
                            node->add_child(child);
                    }
                }
                else if (args.size() == 1) {
                    // For { Prog, -Line }
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    if (lineNode) node->add_child(lineNode);
                }
                return node;
            }
        }
    }
};
