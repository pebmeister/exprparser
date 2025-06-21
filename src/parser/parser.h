// parser.h
#pragma once
#include <map>
#include <memory>
#include <minmax.h>
#include <set>
#include <stdexcept>
#include <vector>
#include <cinttypes>

#include "ANSI_esc.h"
#include "ASTNode.h"
#include "common_types.h"
#include "expr_rules.h"
#include "grammar_rule.h"
#include "sym.h"
#include "token.h"


class Parser {
public:
    void throwError(std::string str)
    {
        throw std::runtime_error(
            str + " " + get_token_error_info()
        );
    }

    std::vector<Token> tokens;
    uint16_t org = 0x1000;
    int32_t PC = org;
    size_t current_pos = 0;
    size_t current_line = 0;
    std::map<int64_t, std::string> parserDict;
    std::map<std::string, Sym> symbolTable;
    std::map<std::string, Sym> localSymbolTable;   
    std::vector<std::string> lines;
    std::vector<uint8_t> output_bytes;

    static ANSI_ESC es;

    // Add these:
    std::unordered_map<std::string, std::shared_ptr<ASTNode>> macroTable;
    std::set<std::string> currentMacros;
    int macroCallDepth = 0;

    std::shared_ptr<ASTNode> expandMacro(
        const std::string& macroName,
        const std::vector<std::string>& args,
        const std::shared_ptr<ASTNode>& macroBody)
    {
        auto expanded = std::make_shared<ASTNode>(*macroBody);

        for (auto& child : expanded->children) {
            if (auto stmt = std::get_if<std::shared_ptr<ASTNode>>(&child)) {
                processMacroParameters(*stmt, args);
            }
        }

        return expanded;
    }

    void processMacroParameters(
        std::shared_ptr<ASTNode> node,
        const std::vector<std::string>& args)
    {
        for (auto& child : node->children) {
            if (auto token = std::get_if<Token>(&child)) {
                // Handle \1, \2 parameters
                if (token->value.size() > 1 && token->value[0] == '\\') {
                    try {
                        size_t paramNum = std::stoul(token->value.substr(1));
                        if (paramNum > 0 && paramNum <= args.size()) {
                            token->value = args[paramNum - 1];
                        }
                        else {
                            throwError("Invalid macro parameter: " + token->value);
                        }
                    }
                    catch (...) {
                        throwError("Invalid macro parameter: " + token->value);
                    }
                }
            }
            else if (auto childNode = std::get_if<std::shared_ptr<ASTNode>>(&child)) {
                processMacroParameters(*childNode, args);
            }
        }
    }

    Parser(
        const std::map<int64_t, std::string>& parserDict,
        const std::vector<std::string>& lines)
        : parserDict(parserDict), lines(lines)
    {
        symbolTable.clear();
        localSymbolTable.clear();
        tokens.clear();
    }

    std::shared_ptr<ASTNode> Assemble();
    std::vector<Sym> GetUnresolvedLocalSymbols()
    {
        std::vector<Sym> unresolved;
        for (auto& symEntry : localSymbolTable) {
            Sym& sym = symEntry.second;
            if (sym.changed || !sym.initialized || !sym.defined_in_pass) {
                unresolved.emplace_back(sym);
            }
        }
        return unresolved;
    }

    std::vector<Sym> GetUnresolvedSymbols()
    {
        std::vector<Sym> unresolved;
        for (auto& symEntry : symbolTable) {
            Sym& sym = symEntry.second;
            if (sym.changed || !sym.initialized) {
                unresolved.emplace_back(sym);
            }
        }
        return unresolved;
    }

    std::string get_token_error_info() const
    {
        const int range = 3;

        if (current_pos >= tokens.size()) return "at end of input";
        const Token& tok = tokens[current_pos];
        std::string str = (tok.type != EOL ? ("at token type " + parserDict.at(tok.type)) +
            " ('" + tok.value + "') " : " ") + "[line " +
            std::to_string(tok.line) + ", col " +
            std::to_string(tok.line_pos) + "]";

        for (auto l = max(tok.line - range, 0); l < min(tok.line + range, lines.size() - 1); ++l) {
            str += es.gr(es.BLUE_FOREGROUND);
            auto ln = paddLeft(std::to_string(l + 1), 4);
            str += "\n" + ln + " ";
            if (l + 1 == tok.line) {
                str += es.gr(es.BRIGHT_RED_FOREGROUND);
            }
            else {
                str += es.gr(es.WHITE_FOREGROUND);
            }
            str += lines[l];
        }
        es.gr(es.RESET_ALL);
        str += '\n';
        return str;
    }

    std::shared_ptr<ASTNode> parse_rule(int64_t rule_type);

    void InitPass();
    std::shared_ptr<ASTNode> Pass();
    std::shared_ptr<ASTNode> parse();

    std::string paddLeft(const std::string& str, size_t totalwidth) const
    {
        std::string out = str;
        while (out.size() < totalwidth)
            out = ' ' + out;
        return out;
    }

    std::string paddRight(const std::string& str, size_t totalwidth) const
    {
        std::string out = str;
        while (out.size() < totalwidth)
            out += ' ';
        return out;
    }

    template<typename RuleFunc, typename CalcFunc>
    std::shared_ptr<ASTNode> handle_binary_operation(
        std::shared_ptr<ASTNode> left,
        const std::set<TOKEN_TYPE>& allowed_ops,
        int64_t rule_type,
        RuleFunc parse_right,
        CalcFunc calculate,
        const std::string& expected_name)
    {
        while (current_pos < tokens.size() &&
            allowed_ops.count(tokens[current_pos].type)) {
            Token op = tokens[current_pos++];
            auto right = parse_right();
            if (!right) {
                throw std::runtime_error(
                    "Syntax error: expected " + expected_name +
                    " after operator '" + op.value + "' " +
                    get_token_error_info()
                );
            }

            auto node = std::make_shared<ASTNode>(rule_type);
            node->add_child(left);
            node->add_child(op);
            node->add_child(right);
            node->value = calculate(left->value, op.type, right->value);
            left = node;
        }
        return left;
    }
};
