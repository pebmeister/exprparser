#include <iostream>
#include <filesystem>
#include <fstream>
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
#include "symboltable.h"
#include "sym.h"
#include "token.h" 
#include "tokenizer.h"
#include "ExpressionParser.h"
#include "utils.h"

extern Tokenizer tokenizer;

namespace fs = std::filesystem;

static void handle_label_def(std::shared_ptr<ASTNode>& node, Parser& p, SymTable& table, const Token& tok)
{
    std::string name = tok.value;
    table.add(name, p.PC, p.sourcePos);
    node->value = table.getSymValue(name, p.sourcePos);
}

static std::shared_ptr<ASTNode> processOpCodeRule(RULE_TYPE ruleType,
    const std::vector<RuleArg>& args, Parser& p, int count)
{
    auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
    node->pc_Start = p.PC;
    if (p.inMacroDefinition) {
        node->value = 0;
        return node;
    }

    auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
    TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);

    // Check if opcode is valid
    auto it = opcodeDict.find(opcode);
    if (it == opcodeDict.end()) {
        p.throwError("Unknown opcode ");
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
        p.throwError("Opcode '" + info.mnemonic + "' does not support addressing mode " + mode_name);
    }
    for (const auto& arg : args) node->add_child(arg);
    if (count == 0) {
        p.bytesInLine = 1;
    }

    node->value = inf->second;
    return node;
}

static std::shared_ptr<ASTNode> processOpCodeRule(std::vector<RULE_TYPE> rule,
    RuleArg l, RuleArg r, Parser& p, int count)
{
    RULE_TYPE ruleType;
    auto& left = std::get<std::shared_ptr<ASTNode>>(l);
    auto& right = std::get<std::shared_ptr<ASTNode>>(r);
    TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);

    if (p.inMacroDefinition) {
        ruleType = (RULE_TYPE)-1;
        for (auto& type : rule)
            if (type != -1) {
                ruleType = type;
                break;
            }
        auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
        node->pc_Start = p.PC;
        node->value = 0;
        return node;
    }

    auto it = opcodeDict.find(opcode);
    if (it == opcodeDict.end()) {
        p.throwError("Unknown opcode ");
    }

    const OpCodeInfo& info = it->second;
    bool supports_two_byte = (info.mode_to_opcode.find(rule[0]) != info.mode_to_opcode.end());
    bool supports_one_byte = (info.mode_to_opcode.find(rule[1]) != info.mode_to_opcode.end());
    bool supports_relative = rule.size() == 3
        ? (info.mode_to_opcode.find(rule[2]) != info.mode_to_opcode.end())
        : false;

    if (!(supports_two_byte || supports_one_byte || supports_relative)) {
        ruleType = (RULE_TYPE)-1;
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
    if (out_of_range && p.globalSymbols.changes == 0) {
        p.throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(op_value) + ")");
    }

    int sz = 0;
    // Select the correct addressing mode
    if (!out_of_range) {
        if (supports_relative) {
            auto rel_value = op_value - (p.PC + 2);
            if (op_value != 0) {
                if (((rel_value + 127) & ~0xFF) != 0 && p.globalSymbols.changes == 0) {
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
    }
    auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
    node->pc_Start = p.PC;

    auto inf = info.mode_to_opcode.find(ruleType);
    if (inf != info.mode_to_opcode.end()) {
        node->value = inf->second;
        if (count == 0 && !p.inMacroDefinition) {
            p.bytesInLine = sz;
        }
    }
    return node;
}

/// <summary>
/// Defines grammar rules and their associated semantic actions for a parser, mapping rule symbols to their production patterns and handler functions.
/// </summary>
const std::unordered_map<int64_t, RuleHandler> grammar_rules =
{
    // LabelDef - FIXED: Use tok.pos for anonymous label positions
    {
        LabelDef,
        RuleHandler{
            {  // Productions vector
                { LabelDef, PLUS, COLAN },
                { LabelDef, PLUS },
                { LabelDef, MINUS, COLAN },
                { LabelDef, MINUS },
                { LabelDef, LOCALSYM, COLAN },
                { LabelDef, LOCALSYM },
                { LabelDef, SYM, COLAN },
                { LabelDef, SYM },
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {   // Action

                auto node = std::make_shared<ASTNode>(LabelDef);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                const Token& tok = std::get<Token>(node->children[0]);
                node->sourcePosition = tok.pos;

                if (tok.type == PLUS || tok.type == MINUS) {
                    // Anonymous label definition: "+" is forward anchor, "-" is backward anchor.
                    if (!p.inMacroDefinition) {
                        // FIX: Use tok.pos (the token's actual position) instead of p.sourcePos
                        p.anonLabels.add(tok.pos, tok.type == PLUS, p.PC);

                        node->value = p.PC;
                    }
                    return node;
                }

                if (tok.type == LOCALSYM) {
                    handle_label_def(node, p, p.localSymbols, tok);
                }
                else {
                    if (tok.start) {
                        p.localSymbols.clear();
                    }
                    handle_label_def(node, p, p.globalSymbols, tok);
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
                { Number, OCTNUM },
                { Number, CHAR },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Number, p.sourcePos);
                node->pc_Start = p.PC;
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

                            case OCTNUM:
                            {
                                // Accept multiple common octal prefixes:
                                // "0o123", "0O123", "o123", "&123", or plain leading '0' like "0123".
                                std::string s = tok.value;
                                if (s.rfind("0o", 0) == 0 || s.rfind("0O", 0) == 0) {
                                    s = s.substr(2);
                                }
                                else if (!s.empty() && (s[0] == 'o' || s[0] == 'O' || s[0] == '&')) {
                                    s = s.substr(1);
                                }
                                else if (s.size() > 1 && s[0] == '0') {
                                    // "0123" style octal - strip leading 0
                                    s = s.substr(1);
                                }
                                s = join_segments(s);
                                node->value = std::stol(s, nullptr, 8);
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

    // PC Assign
    {
        PCAssign,
        RuleHandler {
            {
                { PCAssign, MUL, EQUAL, -Expr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(PCAssign, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[2]);

                node->value = value->value;
                if (count == 0) {
                    p.PC = value->value;
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
                { Equate, -SymbolName, EQUAL, -Expr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Equate, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[2]);
                std::shared_ptr<ASTNode> lab = std::get<std::shared_ptr<ASTNode>>(args[0]);
                Token symtok = std::get<Token>(lab->children[0]);

                if (symtok.type == SYM) {
                    if (p.varSymbols.isDefined(symtok.value)) {
                        if (count == 0 && !p.deferVariableUpdates) {
                            p.varSymbols.setSymValue(symtok.value, p.sourcePos, value->value);
                            auto sym = p.varSymbols.getSymValue(symtok.value, p.sourcePos);
                        }
                    }
                    else {
                        p.globalSymbols.add(symtok.value, value->value, p.sourcePos);
                        p.globalSymbols.setSymEQU(symtok.value);
                    }
                }
                else if (symtok.type == LOCALSYM) {
                    p.localSymbols.add(symtok.value, value->value, p.sourcePos);
                    p.localSymbols.setSymEQU(symtok.value);
                }
                node->value = value->value;
                return node;
            }
        }
    },

    // Factor
    {
        Factor,
        RuleHandler {
            {
                { Factor, -AnonLabelRef },          // NEW: must come before unary +/−
                { Factor, -Number },
                { Factor, -SymbolRef },
                { Factor, MACRO_PARAM },
                { Factor, MUL },
                { Factor, LPAREN, -Expr, RPAREN },
                { Factor, MINUS, -Factor },
                { Factor, PLUS, -Factor },
                { Factor, ONESCOMP, -Factor },
                { Factor, LT, -Factor },            // unary low-byte operator '<'
                { Factor, GT, -Factor }             // unary high-byte operator '>'
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Factor, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                Token tok;
                switch (args.size()) {
                    case 1:
                    {
                        if (std::holds_alternative<Token>(args[0])) {
                            auto tok = std::get<Token>(args[0]);
                            if (tok.type == MUL) {
                                node->value = p.PC;
                            }
                            else if (tok.type == MACRO_PARAM) {
                                node->value = 1;
                                // DONT DO ANYTHING!!!
                                // This will be replaced when expanding the macro.
                                // It just needs to parse with no error
                            }
                            break;
                        }
                        else if (std::holds_alternative<std::shared_ptr<ASTNode>>(args[0])) {
                            auto& t = std::get<std::shared_ptr<ASTNode>>(args[0]);
                            node->value = t->value;
                        }
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
                                node->value = (~t->value) & 0xFFFF;
                                break;

                            case LT:
                                node->value = t->value & 0x00FF;
                                break;

                            case GT:
                                node->value = (t->value >> 8) & 0x00FF;
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

    // EndMacro
    {
        EndMacro,
        RuleHandler {
            {
                { EndMacro, ENDMACRO_DIR }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(EndMacro, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);
                node->value = 0;
                p.inMacroDefinition = false;

                return node;
            }
        }
    },

    // MulExpr (multiply expression)
    {
        MulExpr,
        RuleHandler {
            {
                { MulExpr, -Factor }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MulExpr, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child =
                p.handle_binary_operation(
                    left,
                    {MUL, DIV, MOD},
                    MulExpr,
                    [&p]()
                    {
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
                auto node = std::make_shared<ASTNode>(AddExpr, p.sourcePos);
                node->pc_Start = p.PC;

                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child =
                p.handle_binary_operation(
                    left,
                    { PLUS, MINUS },
                    AddExpr,
                    [&p]()
                    {
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

    // ShiftExpr (shift expression)
    {
        ShiftExpr,
        RuleHandler {
            {
                { ShiftExpr, -AddExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(ShiftExpr, p.sourcePos);
                node->pc_Start = p.PC;
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child = p.handle_binary_operation(
                    left,
                    { SLEFT, SRIGHT },
                    ShiftExpr,
                    [&p]()
                    {
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

    // RelExpr (relational expression: <, >, <=, >=)
    {
        RelExpr,
        RuleHandler {
            {
                { RelExpr, -ShiftExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(RelExpr, p.sourcePos);
                node->pc_Start = p.PC;
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child = p.handle_binary_operation(
                    left,
                    { LT, GT, LE, GE },
                    RelExpr,
                    [&p]()
                    {
                        return p.parse_rule(ShiftExpr);
                    },
                    [&p](int l, TOKEN_TYPE op, int r) -> int
                    {
                        switch (op) {
                            case LT: return (l < r) ? 1 : 0;
                            case GT: return (l > r) ? 1 : 0;
                            case LE: return (l <= r) ? 1 : 0;
                            case GE: return (l >= r) ? 1 : 0;
                            default: return 0;
                        }
                    },
                    "a shift expression"
                );
                node->add_child(child);
                node->value = child->value;
                return node;
            }
        }
    },

    // EqExpr (equality expression: ==, !=)
    {
        EqExpr,
        RuleHandler {
            {
                { EqExpr, -RelExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(EqExpr, p.sourcePos);
                node->pc_Start = p.PC;
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto child = p.handle_binary_operation(
                    left,
                    { DEQUAL, NOTEQUAL },
                    EqExpr,
                    [&p]()
                    {
                        return p.parse_rule(RelExpr);
                    },
                    [&p](int l, TOKEN_TYPE op, int r) -> int
                    {
                        return (op == DEQUAL) ? ((l == r) ? 1 : 0) : ((l != r) ? 1 : 0);
                    },
                    "a relational expression"
                );
                node->add_child(child);
                node->value = child->value;
                return node;
            }
        }
    },

    // AndExpr (bitwise AND) - MODIFIED: now parses EqExpr
    {
        AndExpr,
        RuleHandler{
            {
                { AndExpr, -EqExpr }  // Changed from -ShiftExpr
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    {BIT_AND},
                    AndExpr,
                    [&p]()
                    {
                        return p.parse_rule(EqExpr);  // Changed from ShiftExpr
                    },
                    [&p](int l, TOKEN_TYPE op, int r) { return l & r; },
                    "an equality expression"
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
                    XOrExpr,
                    [&p]()
                    {
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
                    OrExpr,
                    [&p]()
                    {
                        auto node = p.parse_rule(XOrExpr);
                        return node;
                    },
                    [&p](int l, TOKEN_TYPE op, int r) { return l | r; },
                    "an or expression"
                );
            }
        }
    },

    // LogicalAndExpr
    {
        LogicalAndExpr,
        RuleHandler {
            {
                { LogicalAndExpr, -OrExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { LOGICAL_AND },
                    LogicalAndExpr,
                    [&p]()
                    {
                        auto node = p.parse_rule(OrExpr);
                        return node;
                    },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return l && r ? 1 : 0;
                    },
                    "a logical and expression"
                );
            }
        }
    },

    // LogicalOrExpr
    {
        LogicalOrExpr,
        RuleHandler {
            {
                { LogicalOrExpr, -LogicalAndExpr }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto& left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                return p.handle_binary_operation(
                    left,
                    { LOGICAL_OR },
                    LogicalOrExpr,
                    [&p]()
                    {
                        auto node = p.parse_rule(LogicalAndExpr);
                        return node;
                    },
                    [&p](int l, TOKEN_TYPE op, int r)
                    {
                        return l || r ? 1 : 0;
                    },
                    "a logical or expression"
                );
            }
        }
    },

    // Expr
    {
        Expr,
        RuleHandler{
            {
                { Expr, -LogicalOrExpr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Expr, p.sourcePos);
                node->pc_Start = p.PC;

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
                auto node = std::make_shared<ASTNode>(OpCode, p.sourcePos);
                node->pc_Start = p.PC;

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
                auto node = processOpCodeRule(Op_Implied, args, p, count);
                return node;
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
                return processOpCodeRule(Op_Accumulator, args, p, count);
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {(RULE_TYPE)-1, Op_Immediate },
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_Absolute, Op_ZeroPage, Op_Relative},
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_AbsoluteX, Op_ZeroPageX},
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_AbsoluteY, Op_ZeroPageY},
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_Indirect, (RULE_TYPE)-1},
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_IndirectX, (RULE_TYPE)-1},
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
                auto node = processOpCodeRule(std::vector<RULE_TYPE> {Op_IndirectY, (RULE_TYPE)-1},
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
                auto inf = info.mode_to_opcode.find(Op_ZeroPageRelative);

                if (inf == info.mode_to_opcode.end()) {
                    p.throwError("Opcode '" + info.mnemonic + "' does not support zero page relative addressing mode");
                }
                auto opCode = inf->second;
                int zp_addr = zp->value;
                int target = rel->value;
                int rel_offset = target - (p.PC + 3); // opcode + zp + rel
                if (((rel_offset + 128) & ~0xFF) != 0) {
                    p.throwError("Relative branch target out of range (-128 to 127)");
                }
                if (zp_addr < 0 || zp_addr > 0xFF) {
                    p.throwError("Zero page address out of range (0-255)");
                }
                if (rel_offset < -128 || rel_offset > 127) {
                    p.throwError("Relative branch target out of range (-128 to 127)");
                }

                auto node = std::make_shared<ASTNode>(Op_ZeroPageRelative, p.sourcePos);
                for (const auto& arg : args) node->add_child(arg);

                // You can encode the value as needed for your backend
                node->value = opCode;
                if (count == 0 && !p.inMacroDefinition)
                    p.bytesInLine += 3;
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
                auto node = std::make_shared<ASTNode>(AddrExpr, p.sourcePos);
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
                auto node = std::make_shared<ASTNode>(Op_Instruction, p.sourcePos);
                node->pc_Start = p.PC;

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
                auto node = std::make_shared<ASTNode>(Comment, p.sourcePos);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                const Token& tok = std::get<Token>(args[0]);
                node->value = 0;
                return node;
            }
        }
    },

    // MacroStart
    {
        MacroStart,
        RuleHandler{
            {
                { MacroStart, MACRO_DIR }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MacroStart, p.sourcePos);
                node->value = 0;
                node->pc_Start = p.PC;

                p.inMacroDefinition = true;
                return node;
            }
        }
    },

    // SymName: a raw symbol name (no side effects) for LHS of EQU and macro names
    {
        SymbolName,
        RuleHandler{
            {
                { SymbolName, LOCALSYM },
                { SymbolName, SYM },
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(SymbolName, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                const Token& tok = std::get<Token>(args[0]);
                node->sourcePosition = tok.pos;
                node->value = 0;
                return node;
            }
        }
    },

    // SymRef: reference in an expression; do not mutate symbol tables
    {
        SymbolRef,
        RuleHandler{
            {
                { SymbolRef, LOCALSYM },
                { SymbolRef, SYM },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(SymbolRef, p.sourcePos);
                node->pc_Start = p.PC;
                const Token& tok = std::get<Token>(args[0]);
                std::string name = tok.value;

                int val = 0;

                if (p.varSymbols.isDefined(name)) {
                    // FIX: variables are runtime values; ignore source position when reading
                    val = p.varSymbols[name].value;
                    node->add_child(tok);      // keep the original token as child
                }
                else if (tok.type == LOCALSYM) {
                    val = p.localSymbols.getSymValue(name, tok.pos);
                    node->add_child(tok);      // keep the original token as child
                }
                else if (p.globalSymbols.isDefined(name)) {
                    val = p.globalSymbols.getSymValue(name, tok.pos);
                    node->add_child(tok);      // keep the original token as child
                }
                node->sourcePosition = tok.pos;
                node->value = val;
                return node;
            }
        }
    },

    // MacroDef
    {
        MacroDef,
        RuleHandler{
            {
                { MacroDef, -MacroStart, -SymbolName, -Line, -LineList, -EndMacro }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MacroDef, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                auto startm = std::get<std::shared_ptr<ASTNode>>(args[0]);
                auto sym = std::get<std::shared_ptr<ASTNode>>(args[1]);
                auto macroBodyAst = std::get<std::shared_ptr<ASTNode>>(args[3]);
                auto endm = std::get<std::shared_ptr<ASTNode>>(args[4]);

                Token nameTok = std::get<Token>(sym->children[0]);
                std::string macroName = nameTok.value;

                node->sourcePosition = startm->sourcePosition;

                // The macro name was parsed as a symbol so mark it as a macro
                p.globalSymbols.setSymMacro(macroName);

                // Check for recursive macro definition
                if (p.currentMacros.contains(macroName)) {
                    p.throwError("Recursive macro definition: " + macroName);
                }

                // Extract raw text lines   
                std::vector<std::pair<SourcePos, std::string>> macro_body = p.getSourceFromAST(macroBodyAst);

                // Store macro definition with raw text
                MacroDefinition macro(macro_body, 0, nameTok.pos);
                p.macroTable[macroName] = std::make_shared <MacroDefinition>(macro);

                return node;
            }
        }
    },

    // MacroCall
    {
        MacroCall,
        RuleHandler{
            {
                { MacroCall, -SymbolName, -ExprList },
                { MacroCall, -SymbolName, LPAREN, RPAREN }, // allow MYMACRO()
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MacroCall, p.sourcePos);
                node->pc_Start = p.PC;

                if (p.macroCallDepth > 100) {
                    p.throwError("Macro recursion depth exceeded (possible infinite recursion)");
                }

                auto nameNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                Token nameTok = std::get<Token>(nameNode->children[0]);
                node->sourcePosition = nameTok.pos;
                std::string macroName = nameTok.value;

                if (!p.macroTable.count(macroName)) {
                    p.throwError("Unknown macro: " + macroName);
                }

                // Copy macro body and expand parameters
                std::vector<std::pair<SourcePos, std::string>> macrolines =
                    p.macroTable[macroName]->bodyText;

                if (args.size() == 2) {
                    auto exprList = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    int argNum = 1;
                    for (auto& expr : exprList->children) {
                        if (std::holds_alternative<std::shared_ptr<ASTNode>>(expr)) {
                            auto exprNode = std::get<std::shared_ptr<ASTNode>>(expr);
                            exprExtract(argNum, exprNode, macrolines);
                        }
                    }
                }

                // Expansion with proper cleanup on exceptions
                p.currentMacros.insert(macroName);
                try {
                    // Remove entire original call line. After this call p.current_pos
                    // is the correct insertion anchor for the expanded tokens.
                    p.RemoveCurrentLine();
                    //p.current_pos++;
                    const int insertPos = static_cast<int>(p.current_pos);

                    // Tokenize expanded macro body (tokenizer must emit EOL tokens)
                    auto expanded = tokenizer.tokenize(macrolines);
                    Token eolTok;
                    eolTok.type = TOKEN_TYPE::EOL;
                    expanded.insert(expanded.begin(), eolTok);

                    // Anchor tokens to the call site for listing
                    for (auto& t : expanded) t.pos = nameTok.pos;

                    // Insert expanded tokens at the canonical anchor and begin parsing them
                    p.InsertTokens(insertPos, expanded);
                    p.current_pos = insertPos;

                    // Successful expansion; done.
                    p.currentMacros.erase(macroName);
                    p.macroCallDepth--;
                }
                catch (const std::exception& e) {
                    // Ensure cleanup before propagating
                    p.currentMacros.erase(macroName);
                    p.macroCallDepth--;
                    p.throwError(std::string("In macro '") + macroName + "': " + e.what());
                }

                node->value = nameTok.pos.line;
                return node;
            }
        }
    },

    // .inc "filename"
    {
        IncludeDirective,
        RuleHandler{
            {
                { IncludeDirective, INCLUDE, TEXT },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                // Get the filename from the TEXT token
                const Token incTok = std::get<Token>(args[0]);
                const Token filenameTok = std::get<Token>(args[1]);
                std::string filename = sanitizeString(filenameTok.value);

                // Remove quotes if present
                if (!filename.empty() && filename.front() == '"' && filename.back() == '"') {
                    filename = filename.substr(1, filename.size() - 2);
                }

                if (count == 0) {
                    auto eolpos = p.FindNextEOL(p.current_pos);
                    p.current_pos = eolpos - 1;
                    auto pos = p.current_pos;

                    // Read the included file
                    std::vector<std::pair<SourcePos, std::string>> includedLines = p.readfile(filename);

                    auto inctokens = tokenizer.tokenize(includedLines);
                    Token eolTok;
                    eolTok.type = TOKEN_TYPE::EOL;
                    inctokens.insert(inctokens.begin(), eolTok);

                    // Insert included tokens at current position
                    p.InsertTokens(pos + 2, inctokens);
                }

                // Return a dummy node - this directive is transparent to the AST
                auto node = std::make_shared<ASTNode>(IncludeDirective, p.sourcePos);
                node->pc_Start = p.PC;
                node->value = 0;
                node->sourcePosition = incTok.pos;

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
                { ExprList, TEXT, COMMA, -ExprList },
                { ExprList, -Expr },
                { ExprList, TEXT },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(ExprList, p.sourcePos);
                node->pc_Start = p.PC;
                for (auto arg : args) {
                    if (std::holds_alternative<Token>(arg)) {
                        const Token& tok = std::get<Token>(arg);
                        if (tok.type == TEXT) {
                            std::vector<uint8_t> chars;
                            chars.clear();
                            sanitizeString(tok.value, chars);
                            for (auto ch : chars) {
                                auto child = std::make_shared<ASTNode>(Expr, p.sourcePos);
                                child->value = ch;
                                node->add_child(child);
                            }
                        }
                        else {
                            node->add_child(arg);
                        }
                    }
                    else {
                        node->add_child(arg);
                    }
                }
                return node;
            }
        }
    },

    // VarItem - a single variable declaration (name with optional value)
    {
        VarItem,
        RuleHandler{
            {
                { VarItem, SYM, EQUAL, -Expr },  // name = value
                { VarItem, SYM },                // name only
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(VarItem, p.sourcePos);
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },

    // VarList - comma-separated list of variable declarations
    {
        VarList,
        RuleHandler{
            {
                { VarList, -VarItem, COMMA, -VarList },  // item, more items
                { VarList, -VarItem },                   // base case: single item
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(VarList, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                return node;
            }
        }
    },

    // FillDirective
    {
        FillDirective,
        RuleHandler{
            {
                { FillDirective, FILL_DIR, -Expr, COMMA, -Expr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(FillDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                std::shared_ptr<ASTNode> fillByte = std::get<std::shared_ptr<ASTNode>>(args[1]);
                std::shared_ptr<ASTNode> fillCount = std::get<std::shared_ptr<ASTNode>>(args[3]);

                std::string bytval = std::to_string(fillByte->value);
                auto bytelineCount = 0;
                std::vector<std::pair<SourcePos, std::string>> fillLines;
                while (fillCount->value > 0) {
                    if (fillCount->value >= 3) {
                        fillLines.push_back({ p.sourcePos, ".byte " + bytval + ", " + bytval + ", " + bytval });
                        fillCount->value -= 3;
                    }
                    else if (fillCount->value == 2) {
                        fillLines.push_back({ p.sourcePos, ".byte " + bytval + ", " + bytval });
                        fillCount->value -= 2;
                    }
                    else {
                        fillLines.push_back({ p.sourcePos, ".byte " + bytval });
                        fillCount->value -= 1;
                    }
                }

                auto expanded = tokenizer.tokenize(fillLines);

                // Anchor listing to call site (keep the line numbers for display),
                // but do NOT rely on them for removal (RemoveLine will use EOLs).
                for (auto& t : expanded) {
                    t.pos = node->sourcePosition;
                }

                // Remove the original call line right around current_pos
                p.RemoveCurrentLine();

                // Insert expanded tokens where the call was
                p.InsertTokens(static_cast<int>(p.current_pos), expanded);

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
                auto node = std::make_shared<ASTNode>(ByteDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                auto sz = 0;
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                switch (tok.type) {
                    case BYTE:
                        node->value = value->value;
                        if (count == 0) {
                            std::vector<uint16_t> data;
                            extractdata(value, data);
                            if (!p.inMacroDefinition)
                                p.bytesInLine += data.size();
                        }
                        break;
                }
                return node;
            }
        }
    },

    // WordDirective
    {
        WordDirective,
        RuleHandler{
            {
                { WordDirective, WORD, -ExprList },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(WordDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                auto sz = 0;
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                switch (tok.type) {
                    case WORD:
                        node->value = value->value;
                        if (count == 0) {
                            std::vector<uint16_t> data;
                            extractworddata(value, data);
                            if (!p.inMacroDefinition)
                                p.bytesInLine += data.size();
                        }
                        break;
                }
                return node;
            }
        }
    },

    // StoreageDirective
    {
        StorageDirective,
        RuleHandler{
            {
                { StorageDirective, DS, -Expr },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(StorageDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                node->value = value->value;
                if (count == 0)
                    p.bytesInLine = node->value;

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
                auto node = std::make_shared<ASTNode>(OrgDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                const auto& tok = std::get<Token>(args[0]);

                std::shared_ptr<ASTNode> value = std::get<std::shared_ptr<ASTNode>>(args[1]);
                switch (tok.type) {
                    case ORG:
                        node->value = value->value;
                        if (!p.inMacroDefinition)
                            p.PC = node->value;
                        p.org = node->value;
                        break;
                }
                return node;
            }
        }
    },

    // IfDirective
    {
        IfDirective,
        RuleHandler{
            {
                { IfDirective, IF_DIR, -Expr },          // .if <expr>
                { IfDirective, IFDEF_DIR, -SymbolName }, // .ifdef <sym>
                { IfDirective, IFNDEF_DIR, -SymbolName } // .ifndef <sym>
            },
            [](Parser& p, const std::vector<RuleArg>& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(IfDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                if (p.inMacroDefinition) {
                    auto foundEndInf = p.current_pos;
                    for (size_t i = p.current_pos; i < p.tokens.size(); ++i) {
                        if (p.tokens[i].type == TOKEN_TYPE::ENDIF_DIR) {
                            foundEndInf = i;  // Found our matching .endif
                            break;
                        }
                    }
                    p.current_pos = foundEndInf + 1;
                    return node;
                }

                const Token& first = std::get<Token>(args[0]);
                bool cond = false;

                switch (first.type) {
                    case IF_DIR:
                    {
                        auto& expr = std::get<std::shared_ptr<ASTNode>>(args[1]);
                        cond = (expr->value != 0);
                        break;
                    }

                    case IFDEF_DIR:
                    {
                        auto& nameNode = std::get<std::shared_ptr<ASTNode>>(args[1]);
                        Token nameTok = std::get<Token>(nameNode->children[0]);
                        cond = p.IsSymbolDefined(nameTok.value);
                        break;
                    }

                    case IFNDEF_DIR:
                    {
                        auto& nameNode = std::get<std::shared_ptr<ASTNode>>(args[1]);
                        Token nameTok = std::get<Token>(nameNode->children[0]);
                        cond = !p.IsSymbolDefined(nameTok.value);
                        break;
                    }

                    default:
                        p.throwError("Internal: bad IfDirective dispatch");
                }

                // Important: splice out the inactive half BEFORE we try to parse those lines.
                // current_pos is right after the expression/symbol of the directive;
                // the next token should be the EOL for this directive line.
                if (!p.inMacroDefinition) {
                    p.SpliceConditional(cond, p.current_pos);
                }
                node->value = cond ? 1 : 0;
                return node;
            }
        }
    },

    // DoDirective - Proper nested loop handling with deferred expansion
    {
        DoDirective,
        RuleHandler{
            {
                { DoDirective,  DO_DIR, -EOLOrComment, -LineList, WHILE_DIR, -Expr  },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(DoDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                node->value = DO_DIR;
                return node;
            }
        }
    },

    // WhileDirective - Proper nested loop handling with deferred expansion
    {
        WhileDirective,
        RuleHandler{
            {
                { WhileDirective, WHILE_DIR, -Expr, -EOLOrComment, -LineList, WEND_DIR },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(WhileDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                node->value = WHILE_DIR;
                return node;
            }
        }
    },

    // VarDirective - now uses VarList
    {
        VarDirective,
        RuleHandler{
            {
                { VarDirective, VAR_DIR, -VarList },
            },
            [](Parser& p, const std::vector<RuleArg>& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(VarDirective, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);

                if (count == 0) {
                    // Helper: register a single variable from a VarItem node
                    auto registerVar = [&p](const std::shared_ptr<ASTNode>& item)
                    {
                        const Token& symTok = std::get<Token>(item->children[0]);
                        std::string name = symTok.value;
                        int value = 0;

                        // children: [SYM] or [SYM, EQUAL, Expr]
                        if (item->children.size() > 2) {
                            auto& expr = std::get<std::shared_ptr<ASTNode>>(item->children[2]);
                            value = expr->value;
                        }

                        if (!p.varSymbols.isDefined(name)) {
                            p.varSymbols.add(name, value, p.sourcePos);
                            p.varSymbols.setSymVar(name);
                        }
                        else {
                            p.varSymbols[name].value = value;
                        }
                    };

                    // Recursively walk VarList to process all VarItems
                    std::function<void(const std::shared_ptr<ASTNode>&)> processVarList;
                    processVarList = [&](const std::shared_ptr<ASTNode>& listNode)
                    {
                        for (const auto& child : listNode->children) {
                            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                                auto& childNode = std::get<std::shared_ptr<ASTNode>>(child);
                                if (childNode->type == VarItem) {
                                    registerVar(childNode);
                                }
                                else if (childNode->type == VarList) {
                                    processVarList(childNode);
                                }
                            }
                        }
                    };

                    auto& varList = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    processVarList(varList);
                }
                return node;
            }
        }
    },

    // PrintDirective
    {
        PrintDirective,
        RuleHandler{
            {
                { PrintDirective, PRINT_ON },
                { PrintDirective, PRINT_OFF },            // base case
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(PrintDirective, p.sourcePos);
                node->value = 1;
                node->add_child(args[0]); // token first

                return node;
            }
        }
    },

    // Statement
    {
        Statement,
        RuleHandler{
            {
                { Statement, -MacroCall },
                { Statement, -MacroDef },
                { Statement, -PCAssign },
                { Statement, -Equate },
                { Statement, -Op_Instruction },
                { Statement, -OrgDirective },
                { Statement, -ByteDirective },
                { Statement, -WordDirective },
                { Statement, -StorageDirective },
                { Statement, -FillDirective },
                { Statement, -IncludeDirective },
                { Statement, -IfDirective },
                { Statement, -VarDirective },
                { Statement, -PrintDirective },
                { Statement, -DoDirective },
                { Statement, -WhileDirective },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(Statement, p.sourcePos);
                node->pc_Start = p.PC;
                for (const auto& arg : args) node->add_child(arg);
                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->sourcePosition.line = left->sourcePosition.line;
                node->value = left->value;

                return node;
            }
        }
    },

    // PlusRun
    {
        PlusRun,
        RuleHandler{
            {
                { PlusRun, PLUS, -PlusRun }, // recursive first (greedy)
                { PlusRun, PLUS },           // base case
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(PlusRun, p.sourcePos);
                if (args.size() == 2) {
                    auto tail = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    node->value = 1 + tail->value;
                    // Ensure the first child is the PLUS token
                    node->add_child(args[0]);
                    node->add_child(tail);
                }
                else {
                    node->value = 1;
                    node->add_child(args[0]); // token first
                }
                return node;
            }
        }
    },

    // MinusRun
    {
        MinusRun,
        RuleHandler{
            {
                { MinusRun, MINUS, -MinusRun }, // recursive first (greedy)
                { MinusRun, MINUS },            // base case
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(MinusRun, p.sourcePos);
                if (args.size() == 2) {
                    auto tail = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    node->value = 1 + tail->value;
                    // Ensure the first child is the MINUS token
                    node->add_child(args[0]);
                    node->add_child(tail);
                }
                else {
                    node->value = 1;
                    node->add_child(args[0]); // token first
                }
                return node;
            }
        }
    },

    // AnonLabelRef - FIXED: Use token position for lookup, not p.sourcePos
    {
        AnonLabelRef,
        RuleHandler{
            {
                { AnonLabelRef, -PlusRun },
                { AnonLabelRef, -MinusRun },
            },
            [](Parser& p, const auto& args, int /*count*/) -> std::shared_ptr<ASTNode>
            {
                auto run = std::get<std::shared_ptr<ASTNode>>(args[0]);

                // Get the first token from the run - this has the correct position
                const Token& first = std::get<Token>(run->children[0]);

                // FIX: Create node with the token's position, not p.sourcePos
                auto node = std::make_shared<ASTNode>(AnonLabelRef, first.pos);

                bool forward = (first.type == PLUS);
                int n = run->value;

                // FIX: Use first.pos (the token's actual position) for the lookup
                auto result = p.anonLabels.find(first.pos, forward, n);
                if (result.has_value()) {
                    auto& value = result.value();
                    node->value = std::get<1>(value); // anchor address
                }
                else {
                    if (p.pass > 1) {
                        p.throwError("Unable to find anonymous label.");
                    }
                    node->value = 0; // first pass or unresolved
                }
                node->add_child(run);
                return node;
            }
        }
    },

    // EOLOrComment
    {
        EOLOrComment,
        RuleHandler{
            {
                { EOLOrComment, -Comment, EOL },
                { EOLOrComment, EOL },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(EOLOrComment);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);
                auto eolIndex = args.size() - 1;
                const Token& eolTok = std::get<Token>(args[eolIndex]);

                node->sourcePosition = eolTok.pos;
                node->value = eolTok.pos.line;
                return node;
            }
        }
    },

    // Line
    {
        Line,
        RuleHandler{
            {
                { Line, -Statement, -EOLOrComment },
                { Line, -EOLOrComment },
                { Line, -LabelDef, -Statement, -EOLOrComment },
                { Line, -LabelDef, -EOLOrComment }
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                p.PC += p.bytesInLine;
                auto node = std::make_shared<ASTNode>(Line);
                node->pc_Start = p.PC;

                for (const auto& arg : args) node->add_child(arg);

                auto left = std::get<std::shared_ptr<ASTNode>>(args[0]);
                node->sourcePosition = left->sourcePosition;
                node->value = left->sourcePosition.line;

                p.bytesInLine = 0;

                return node;
            }
        }
    },

    // LineList
    {
        LineList,
        RuleHandler{
            {
                { LineList, -Line, -LineList },
                { LineList, -Line },
            },
            [](Parser& p, const auto& args, int count) -> std::shared_ptr<ASTNode>
            {
                auto node = std::make_shared<ASTNode>(LineList, p.sourcePos);
                node->pc_Start = p.PC;

                if (args.size() == 2) {
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    auto progNode = std::get<std::shared_ptr<ASTNode>>(args[1]);
                    if (lineNode) node->add_child(lineNode);
                    if (progNode) {
                        for (const auto& child : progNode->children) {
                            node->add_child(child);
                        }
                    }
                    node->sourcePosition = lineNode->sourcePosition;
                    node->value = lineNode->sourcePosition.line;
                }
                else if (args.size() == 1) {
                    auto lineNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                    if (lineNode) node->add_child(lineNode);
                    node->sourcePosition = lineNode->sourcePosition;
                    node->value = lineNode->sourcePosition.line;
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
                auto node = std::make_shared<ASTNode>(Prog, p.sourcePos);
                node->pc_Start = p.PC;
                auto lineListNode = std::get<std::shared_ptr<ASTNode>>(args[0]);
                if (lineListNode) {
                    node->add_child(lineListNode);
                    node->sourcePosition.line = lineListNode->sourcePosition.line;
                }
                return node;
            }
        }
    }
};
