#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ASTNode.h"
#include "opcodedict.h"
#include "token.h"
#include "utils.h"

/// <summary>
/// convert a string to uppercase
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
std::string toupper(std::string& input)
{
    std::string out;
    for (auto& ch : input) {
        out += std::toupper(ch);
    }
    return out;
}

/// <summary>
/// convert a string to lowercase
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
std::string tolower(std::string& input)
{
    std::string out;
    for (auto& ch : input) {
        out += std::tolower(ch);
    }
    return out;
}

/// <summary>
/// Sanatize a formatted string
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
std::string sanitizeString(const std::string& input)
{
    std::vector<uint8_t>chars;
    sanitizeString(input, chars);
    std::string out;
    for (auto& ch : chars) {
        out += static_cast<char>(ch);
    }
    return out;
}

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

/// <summary>
/// join segments of a number
/// </summary>
/// <param name="num"></param>
/// <returns></returns>
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

void extractworddata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data)
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
                extractworddata(childnode, data);
            }
        }
    }
}

void extractdata(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data)
{
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            if (childnode->type == Expr) {
                data.push_back(childnode->value);
            }
            else {
                extractdata(childnode, data);
            }
        }
    }
}

void handle_sym(std::shared_ptr<ASTNode>& node, Parser& p, SymTable& table, const Token& tok)
{
    std::string name = tok.value;
    if (tok.start && table.isLabel(name)) {
        table.add(name, p.PC, p.sourcePos);
    }
    node->value = table.getSymValue(name, p.sourcePos);
}

std::shared_ptr<ASTNode> processRule(RULE_TYPE ruleType,
    const std::vector<RuleArg>& args, Parser& p, int count)
{
    auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
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

    if (p.inMacroDefinition) {
        ruleType = (RULE_TYPE)-1;
        for (auto& type : rule)
            if (type != -1) {
                ruleType = type;
                break;
            }
        auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
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
    if (supports_relative) {
        auto rel_value = op_value - (p.PC + 3); // we have not incremented the PC yet so use 3 not 2.
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
    auto node = std::make_shared<ASTNode>(ruleType, p.sourcePos);
    auto inf = info.mode_to_opcode.find(ruleType);
    node->value = inf->second;
    if (count == 0 && !p.inMacroDefinition) {
        p.PC += sz;
    }
    return node;
}

// helper for calcoutputsize for .byte or .word directive
static void processdata(int& sz, std::shared_ptr<ASTNode>& node, bool word)
{
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            if (childnode->type == Expr) {
                if (word) {
                    sz += 2;
                }
                else {
                    ++sz;
                }
            }
            else {
                processdata(sz, childnode, word);
            }
        }
    }
}

/// <summary>
/// calculate the number of output bytes generated from the given node
/// </summary>
/// <param name="size"></param>
/// <param name="node"></param>
void calcoutputsize(int& size, std::shared_ptr<ASTNode> node)
{
    auto processChildren = [&](const std::vector<RuleArg>& children)
        {
            for (const auto& child : children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    calcoutputsize(size, std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
        };

    switch (node->type) {
        case Prog:
        case IncludeDirective:
        case MacroCall:
        case LineList:
        case Statement:
        case Op_Instruction:
        case Line:
            processChildren(node->children);
            return;

        case MacroDef:
            return;

        case ByteDirective:
        case WordDirective:
        {
            auto bytesz = 0;
            auto& bytelistNode = std::get<std::shared_ptr<ASTNode>>(node->children[1]);
            processdata(bytesz, bytelistNode, node->type == WordDirective);
            size += bytesz;
            return;
        }

        case Op_Implied:
        case Op_Accumulator:
            ++size;
            return;

        case Op_Relative:
        case Op_Immediate:
        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
        case Op_IndirectX:
        case Op_IndirectY:
            size += 2;
            return;

        case Op_Absolute:
        case Op_AbsoluteX:
        case Op_AbsoluteY:
        case Op_Indirect:
        case Op_ZeroPageRelative:
            size += 3;
            return;
    }
}

std::string paddLeft(const std::string& str, size_t totalwidth)
{
    std::string out = str;
    while (out.size() < totalwidth)
        out = ' ' + out;
    return out;
}

std::string paddRight(const std::string& str, size_t totalwidth)
{
    std::string out = str;
    while (out.size() < totalwidth)
        out += ' ';
    return out;
}
