#include <iostream>
#include <iomanip>
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
#include "ExpressionParser.h"

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
    auto& symbolName = tok.value;
    Sym sym;

    if (tok.start) {
        node->type = RULE_TYPE::Label;

        // If this is a global label, validate unresolved locals
        if (isGlobal) {
            const auto& unresolved = p.GetUnresolvedLocalSymbols();
            if (!unresolved.empty()) {
                std::string err = "Unresolved local symbols:";
                for (const auto& s : unresolved) {
                    err += " " + s.name + " accessed at line(s) ";
                    for (auto line : s.accessed)
                        err += std::to_string(line) + " ";
                    err += "\n";
                }
                p.throwError(err);
            }
            p.localSymbolTable.clear();
        }

        auto found = symTable.find(symbolName);
        if (found != symTable.end()) {
            sym = found->second;

            // If it's a macro, skip it entirely
            if (sym.isMacro)
                return;

            // Already defined this pass
            if (sym.defined_in_pass) {
                if (!sym.changed && isGlobal && sym.isPC && sym.value != p.PC)
                    p.throwError("Error symbol " + symbolName + " already defined");

                sym.changed = false;
                sym.defined_in_pass = true;

                if (sym.initialized) {
                    if (sym.isPC && sym.value != p.PC) {
                        sym.changed = true;
                        sym.value = p.PC;
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
                // Previously known but not yet defined in this pass
                sym.defined_in_pass = true;
                sym.isPC = true;
                sym.initialized = true;
                sym.value = p.PC;
                sym.changed = !sym.accessed.empty();
            }
        }
        else {
            // First time we've seen this symbol
            sym = Sym{
                .name = symbolName,
                .value = p.PC,
                .accessed = {},
                .initialized = true,
                .changed = false,
                .defined_in_pass = true,
                .isPC = true,
                .isMacro = false

            };
        }

        symTable[symbolName] = sym;
        node->value = sym.value;
        return;
    }

    // If not a label — this is a symbol reference
    auto found = symTable.find(symbolName);
    if (found != symTable.end()) {
        sym = found->second;

        if (!sym.was_accessed_by(tok.line))
            sym.accessed.push_back(tok.line);

        node->value = sym.value;
    }
    else {
        // Symbol is unknown — issue local warning if necessary
        if (isGlobal && p.localSymbolTable.contains(symbolName)) {
            std::cout << "Warning: " << symbolName << " is defined as local. Ignore this warning with -nowarn.\n";
        }

        sym = Sym{
            .name = symbolName,
            .value = 0,
            .accessed = { tok.line },
            .initialized = false,
            .changed = false,
            .defined_in_pass = false
        };

        symTable[symbolName] = sym;
        node->value = sym.value;
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
        p.throwError("Unknown opcode ");
    }

    if (p.inMacroDefinition)
        return std::make_shared<ASTNode>(ruleType, p.current_line);

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
        p.throwError("Opcode '" + info.mnemonic + "' does not support addressing mode " + mode_name);
    }
    auto node = std::make_shared<ASTNode>(ruleType, p.current_line);
    for (const auto& arg : args) node->add_child(arg);

    if (count == 0 && !p.inMacroDefinition) {
        p.PC++;
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
        p.throwError("Unknown opcode ");
    }

    if (p.inMacroDefinition)
        return std::make_shared<ASTNode>(ruleType, p.current_line);

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
        p.throwError("Opcode '" + info.mnemonic + "' does not support addressing mode " + mode_name);
    }

    int op_value = right->value;
    bool is_large = (op_value & ~0xFF) != 0;
    bool out_of_range = (op_value & ~0xFFFF) != 0 || (!(supports_two_byte || supports_relative) && is_large);
    if (out_of_range) {
        p.throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(op_value) + ")");
    }

    int sz = 0;
    // Select the correct addressing mode
    if (supports_relative) {
        auto rel_value = op_value - (p.PC + 3); // we have not incremented the PC yet so use 3 not 2.
        if (op_value != 0) {
            if (((rel_value + 127) & ~0xFF) != 0) {
                p.throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(op_value) + ")");
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
    auto node = std::make_shared<ASTNode>(ruleType, p.current_line);

    node->value = inf->second;
    if (count == 0 && ! p.inMacroDefinition) {
        p.PC += sz;
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
                { Symbol, LOCALSYM, COLAN },
                { Symbol, SYM, COLAN },
                { Symbol, LOCALSYM },
                { Symbol, SYM },
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {   // Action
                // ToDo: Normalize symbal case

                auto node = std::make_shared<ASTNode>(Symbol);
                for (const auto& arg : args) node->add_child(arg);
        
                const Token& tok = std::get<Token>(node->children[0]);
                node->line = tok.line;
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
                { Number, CHAR },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Number, p.current_line);
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

                            case CHAR:
                                node->value = tok.value[1];
                                break;

                            default:
                                p.throwError("Unknown token type in Factor rule");
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
                auto node = std::make_shared<ASTNode>(Equate, p.current_line);
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
                { Factor, MACRO_PARAM },
                { Factor, LPAREN, -Expr, RPAREN },
                { Factor, MINUS, -Factor },
                { Factor, PLUS, -Factor },
                { Factor, ONESCOMP, -Factor }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Factor, p.current_line);
                for (const auto& arg : args) node->add_child(arg);

                Token tok;
                switch (args.size()) {
                    case 1:
                    {
                        if (std::holds_alternative<std::shared_ptr<ASTNode>>(args[0])) {
                            auto& t = std::get<std::shared_ptr<ASTNode>>(args[0]);
                            node->value = t->value;
                        }
                            // node->value = std::stol(tok.value, nullptr, 10);                        
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

                            case ONESCOMP:
                                node->value = (~t->value)& 0xFFFF;
                                break;

                            default:
                                p.throwError("Unknown unary operator in Factor rule");
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
                        p.throwError("Syntax error in Factor rule");
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
                auto node = std::make_shared<ASTNode>(MulExpr, p.current_line);
                for (const auto& arg : args) node->add_child(arg);

                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child = 
                p.handle_binary_operation(
                    left,
                    {MUL, DIV, MOD},
                    MulExpr,
                    [&p]() {
                        return p.parse_rule(Factor);
                    },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        if ((op == DIV || op == MOD) && r == 0) {
                            p.throwError("Division by zero");
                        }
                        return 
                            (op == MOD) 
                            ? l % r 
                            :
                                ((op == MUL) 
                                ? l * r 
                                : l / r);
                    },
                    "a factor"
                );
                node->add_child(child);
                node->value = child->value;
                return node;
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
                auto node = std::make_shared<ASTNode>(AddExpr, p.current_line);
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child =                
                p.handle_binary_operation(
                    left,
                    { PLUS, MINUS },
                    AddExpr,
                    [&p]() {
                        auto node = p.parse_rule(MulExpr);
                        return node; 
                    },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return op == PLUS ? l + r : l - r;
                    },
                    "a term"
                );
                node->value = child->value;
                node->add_child(child);
                return node;
            }
        }        
    },
    // SExpr (shift expression)
    {
        ShiftExpr,
        RuleHandler {
            {
                { ShiftExpr, -AddExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(ShiftExpr, p.current_line);
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child = p.handle_binary_operation(
                    left,
                    { SLEFT, SRIGHT },
                    ShiftExpr,
                    [&p]() {
                        return p.parse_rule(AddExpr);
                    },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return op == SLEFT ? l << r : l >> r;
                    },
                    "a term"
                );
                node->add_child(child);
                node->value = child->value;
                return node;
            }
        }
    },
    // AndExpr
    {
        AndExpr,
        RuleHandler{
            {
                { AndExpr, -ShiftExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {BIT_AND},
                    AndExpr,
                    [&p]() {
                        auto node = p.parse_rule(ShiftExpr);
                        return node; 
                    },
                    [&p](int l, TOKEN_TYPE op, int r) { return l & r; },
                    "an and expression"
                );
            }
        }
    },

    // XOrExpr
    {
        XOrExpr,
        RuleHandler{
            {
                { XOrExpr, -AndExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {BIT_XOR},
                    OrExpr,
                    [&p]() {
                        auto node = p.parse_rule(AndExpr);
                        return node; 
                    },
                    [&p](int l, TOKEN_TYPE op, int r) { return l ^ r; },
                    "an or expression"
                );
            }
        }
    },
    
    // OrExpr
    {
        OrExpr,
        RuleHandler {
            {
                { OrExpr, -XOrExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { BIT_OR },
                    XOrExpr,
                    [&p]() { 
                        auto node = p.parse_rule(OrExpr);
                        return node; 
                    },
                    [&p](int l, TOKEN_TYPE op, int r) { return l ^ r; },
                    "an xor expression"
                );
            }
        }
    },
    // Expr
    {
        Expr,
        RuleHandler{
            {
                { Expr, -OrExpr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Expr, p.current_line);
                // uncomment if you want Expr detail
                // for (const auto& arg : args) node->add_child(arg);
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->value = left->value;
                return node;
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
                auto node = std::make_shared<ASTNode>(OpCode, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
                switch (args.size()) {
                    case 1:
                    {
                        node->value = tok.type;
                        TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(node->value);

                        // Check if opcode is valid
                        auto it = opcodeDict.find(opcode);
                        if (it == opcodeDict.end()) {
                            p.throwError("Unknown opcode " + tok.value);
                        }
                        break;
                    }

                    default:
                        p.throwError("Unknown token type in Factor rule");
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
                    p.throwError("Unknown opcode in Op_ZeroPageRelative rule");
                }
                const OpCodeInfo& info = it->second;
                if (info.mode_to_opcode.find(Op_ZeroPageRelative) == info.mode_to_opcode.end()) {
                    p.throwError("Opcode '" + info.mnemonic + "' does not support zero page relative addressing mode");
                }

                int zp_addr = zp->value;
                int rel_offset = rel->value;

                if (zp_addr < 0 || zp_addr > 0xFF) {
                    p.throwError("Zero page address out of range (0-255)");
                }
                if (rel_offset < -128 || rel_offset > 127) {
                    p.throwError("Relative branch target out of range (-128 to 127)");
                }

                auto node = std::make_shared<ASTNode>(Op_ZeroPageRelative, p.current_line);
                node->add_child(left);
                node->add_child(zp);
                node->add_child(rel);

                // You can encode the value as needed for your backend
                node->value = left->value;
                if (count == 0 && !p.inMacroDefinition)
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
                auto node = std::make_shared<ASTNode>(AddrExpr, p.current_line);
                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
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
                auto node = std::make_shared<ASTNode>(Op_Instruction, p.current_line);
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
                auto node = std::make_shared<ASTNode>(Comment, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
            
                const Token& tok = std::get<Token>(args[0]);
                node->value = 0;
                return node;
            }
        }
    },

    {
        MacroDef,
        RuleHandler{
            {
                { MacroDef, MACRO_DIR, -Symbol, -Line, -LineList, ENDMACRO_DIR }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MacroDef, p.current_line);
                for (const auto& arg : args) node->add_child(arg);

                auto sym = std::get<std::shared_ptr<ASTNode>>(args[1]);
                Token nameTok = std::get<Token>(sym->children[0]);
                std::string macroName = nameTok.value;

                // The macro name was parsed as a symbol so remove it from
                // the symbol table since it will look like an unresolved symbol
                if (p.symbolTable.contains(macroName)) {
                    p.symbolTable[macroName].isMacro = true;
                }
                Token endTok = std::get<Token>(args[4]);

                // Check for recursive macro definition
                if (p.currentMacros.contains(macroName)) {
                    p.throwError("Recursive macro definition: " + macroName);
                }

                // Get the line range of the macro body
                // line numbers start at 1 so adjust
                size_t start_line = nameTok.line; // Skip MACRO directive line
                size_t end_line = endTok.line - 2; // Before ENDMACRO

                // Extract raw text lines
                std::vector<std::string> macro_body;
                for (size_t i = start_line; i <= end_line; i++) {
                    macro_body.push_back(p.lines[i]);
                }

                // Store macro definition with raw text
                MacroDefinition macro(macro_body, 0, nameTok.line);
                p.macroTable[macroName] = std::make_shared <MacroDefinition>(macro);

                return node;
            }
        }
    },

    {
        MacroCall,
        RuleHandler{
            {
                { MacroCall, SYM, -ExprList }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MacroCall, p.current_line);

                // Recursion depth check
                if (p.macroCallDepth > 100) {
                    p.throwError("Macro recursion depth exceeded (possible infinite recursion)");
                }

                const Token& nameTok = std::get<Token>(args[0]);
                std::string macroName = nameTok.value;

                if (!p.macroTable.count(macroName)) {
                    p.throwError("Unknown macro: " + macroName);
                }

                // Make a copy of the macro body text
                auto macrolines = p.macroTable[macroName]->bodyText;
                auto exprList = std::get<std::shared_ptr<ASTNode>>(args[1]);
                auto argNum = 1;

                // Substitute arguments
                for (auto& expr : exprList->children) {
                    if (std::holds_alternative<std::shared_ptr<ASTNode>>(expr)) {
                        std::shared_ptr<ASTNode> node = std::get<std::shared_ptr<ASTNode>>(expr);
                        exprExtract(argNum, node, macrolines);
                    }
                }

                // Tokenize and parse the expanded macro
                try {
                    p.macroCallDepth++; // Track recursion depth

                    // Create a temporary parser for the macro content
                    ExpressionParser macroParser(macrolines);

                    // Parse the macro content
                    auto macroAST = macroParser.parse();

                    // set the line numbers for listings
                    macroAST->resetLine(node->line);

                    // Add the parsed AST as our first child
                    // NOTE: 1st node is Prog so get first child
                    node->add_child(macroAST->children[0]);

                    p.macroCallDepth--;
                }
                catch (const std::exception& e) {
                    p.macroCallDepth--;
                    p.throwError(std::string("In macro '") + macroName + "': " + e.what());                
                }

                return node;
            }
        }
    },

    // Expression List (for macro arguments)
    {
        ExprList,
        RuleHandler{
            {
                { ExprList, -Expr, COMMA, -ExprList },
                { ExprList, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(ExprList, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },

    // ByteDirective
    {
        ByteDirective,
        RuleHandler{
            {
                { ByteDirective, BYTE, -ExprList },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(ByteDirective, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                switch (tok.type) {
                    case BYTE:
                        node->value = value->value;
                        break;
                }
                return node;
            }
        }
    },

    // OrgDirective
    {
        OrgDirective,
        RuleHandler{
            {
                { OrgDirective, ORG, -Expr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(OrgDirective, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                switch (tok.type) {
                    case ORG:
                        node->value = value->value;
                        p.PC = node->value;
                        p.org = node->value;
                        break;
                }
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
                { Statement, -OrgDirective, -Comment },
                { Statement, -ByteDirective, -Comment },
                { Statement, -Comment },
                { Statement, -MacroCall },
                { Statement, -MacroDef },
                { Statement, -Equate },
                { Statement, -Symbol },
                { Statement, -Op_Instruction },
                { Statement, -OrgDirective },
                { Statement, -ByteDirective }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Statement, p.current_line);
                for (const auto& arg : args) node->add_child(arg);
                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->value = left->value;
                return node;
            }
        }
    },

    // Line
    {
        Line,
        RuleHandler{
            {
                { Line, EOL, },
                { Line, -Statement, EOL },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Line);
                for (const auto& arg : args) node->add_child(arg);
                Token tok;
                if (args.size() == 1) {
                    tok = std::get<Token>(args[0]);
                }
                else {
                    tok = std::get<Token>(args[1]);
                }
                node->line = tok.line;
                node->value = tok.line;
                return node;
            }
        }
    },

    {
        LineList,
        RuleHandler{
            {
                { LineList, -Line, -LineList },
                { LineList, -Line },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(LineList, p.current_line);
                if (args.size() == 2) {
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    auto progNode = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    if (lineNode) node->add_child(lineNode);
                    if (progNode) {
                        for (const auto& child : progNode->children)
                            node->add_child(child);
                    }
                }
                else if (args.size() == 1) {
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    if (lineNode) node->add_child(lineNode);
                }
                return node;
            }
        }
    },

    // Prog
    {
        Prog,
        RuleHandler{
            {
                { Prog, -LineList },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                p.current_line = 1;
                auto node = std::make_shared<ASTNode>(Prog, p.current_line);
                auto lineListNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                if (lineListNode) node->add_child(lineListNode);
                return node;
            }
        }
    }
};
