
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "opcodedict.h"
#include "token.h"

void sanitizeString(const std::string& input, std::vector<uint8_t>& output)
{
    output.clear();
    size_t len = input.length();

    if (len < 2) return; // too short to be quoted

    char quote = input.front();
    if ((quote != '"' && quote != '\'') || input.back() != quote) {
        return; // invalid quotes
    }

    for (size_t i = 1; i < len - 1; ++i) {
        char c = input[i];
        if (c == '\\' && i + 1 < len - 1) {
            char next = input[++i];
            switch (next) {
                case 'n': output.push_back('\n'); break;
                case 'r': output.push_back('\r'); break;
                case 't': output.push_back('\t'); break;
                case '\\': output.push_back('\\'); break;
                case '\'': output.push_back('\''); break;
                case '\"': output.push_back('\"'); break;
                case 'x':
                {
                    if (i + 2 < len - 1 && std::isxdigit(input[i + 1]) && std::isxdigit(input[i + 2])) {
                        std::string hex = input.substr(i + 1, 2);
                        output.push_back(static_cast<uint8_t>(std::stoi(hex, nullptr, 16)));
                        i += 2;
                    }
                    break;
                }
                default:
                    output.push_back(next); // unrecognized escape — treat as literal
                    break;
            }
        }
        else {
            output.push_back(static_cast<uint8_t>(c));
        }
    }
}

std::string join_segments(std::string num)
{
    std::string out;
    for (auto& ch : num) {
        if (!std::isspace(ch)) {
            out += ch;
        }
    }
    return out;
}

void extractwords(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data)
{
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            if (childnode->type == Expr) {
                auto lo = ((childnode->value & 0x00FF) >> 0);
                auto hi = ((childnode->value & 0xFF00) >> 8);

                data.push_back(lo);
                data.push_back(hi);
            }
            else {
                extractwords(childnode, data);
            }
        }
    }
}

void extract(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data)
{
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            if (childnode->type == Expr) {
                data.push_back(childnode->value);
            }
            else {
                extract(childnode, data);
            }
        }
    }
}

void handle_symbol(std::shared_ptr<ASTNode>& node,
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
                //if (!sym.changed && isGlobal && sym.isPC && sym.value != p.PC)
                //    p.throwError("Error symbol " + symbolName + " already defined");
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

void handle_local_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok)
{
    handle_symbol(node, p, tok, p.localSymbolTable, false);
}

void handle_global_sym(std::shared_ptr<ASTNode>& node, Parser& p, const Token& tok)
{
    handle_symbol(node, p, tok, p.symbolTable, true);
}

std::shared_ptr<ASTNode> processRule(RULE_TYPE ruleType,
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

std::shared_ptr<ASTNode> processRule(std::vector<RULE_TYPE> rule,
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
    if (count == 0 && !p.inMacroDefinition) {
        p.PC += sz;
    }

    return node;
}
