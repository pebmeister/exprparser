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
