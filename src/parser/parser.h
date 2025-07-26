// parser.h
#pragma once
#include <map>
#include <memory>
#include <minmax.h>
#include <set>
#include <stdexcept>
#include <vector>
#include <cinttypes>
#include <iomanip>

#include "ANSI_esc.h"
#include "common_types.h"
#include "expr_rules.h"
#include "grammar_rule.h"

#include "sym.h"
#include "symboltable.h"
#include "token.h"

class MacroDefinition {
public:
    std::vector<std::pair<SourcePos, std::string>> bodyText;
    int paramCount;         // You'll need to parse parameters from Symbol
    SourcePos definedAtLine;

    MacroDefinition(std::vector<std::pair<SourcePos, std::string>> text, int params, SourcePos line)
    {
        bodyText = text;
        paramCount = params;
        definedAtLine = line;
    }
};

extern void exprExtract(int& argNum, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines);

struct ParseState {
    std::string filename;
    size_t current_pos;
    SourcePos current_source;
    std::vector<Token> tokens;
    std::vector<std::pair<SourcePos, std::string>> lines;
};

class Parser {

public:
    SymTable globalSymbols;
    SymTable localSymbols;

    void throwError(std::string str) const
    {
        throw std::runtime_error(
            str + " " + get_token_error_info()
        );
    }

    void symchanged(Sym& sym)
    {

    }

    ParseState getCurrentState()
    {
        return ParseState
        {
            .filename = filename,
            .current_pos = current_pos,
            .current_source = sourcePos,
            .tokens = tokens,
            .lines = lines
        };
    }

    std::string filename;
    std::vector<Token> tokens;
    uint16_t org = 0x1000;
    int32_t PC = org;
    size_t current_pos = 0;
    SourcePos sourcePos;
    std::vector<std::pair<SourcePos, std::string>> lines;
    std::vector<uint8_t> output_bytes;
    std::map<int64_t, std::string> parserDict;
    std::map<std::string, std::vector<Token>> tokenCache;
    std::map<std::string, std::vector<std::pair<SourcePos, std::string>>> fileCache;

    bool inMacroDefinition = false;
    static ANSI_ESC es;

    void pushParseState(ParseState& state);
    ParseState popParseState();

    void printSymbols()
    {
        globalSymbols.print();
    }

    uint16_t eval_number(std::string num, TOKEN_TYPE tok)
    {
        switch (tok) {
            case DECNUM:
                return std::strtol(num.substr(0).c_str(), nullptr, 10);

            case HEXNUM:
                return std::strtol(num.substr(1).c_str(), nullptr, 16);

            case BINNUM:
                return std::strtol(num.substr(1).c_str(), nullptr, 2);

            case CHAR:
                return num[0];

            default:
                break;
        }
        return 0;
    }

    std::unordered_map<std::string, std::shared_ptr<MacroDefinition>> macroTable;
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

            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                auto& childnode = std::get<std::shared_ptr<ASTNode >>(child);
                processMacroParameters(childnode, args);
            }
            else {
                Token token = std::get<Token>(child);
                if (token.value.size() > 1 && token.value[0] == '\\') {
                    try {
                        size_t paramNum = std::stoul(token.value.substr(1));
                        if (paramNum > 0 && paramNum <= args.size()) {
                            token.value = args[paramNum - 1];
                        }
                        else {
                            throwError("Invalid macro parameter: " + token.value);
                        }
                    }
                    catch (...) {
                        throwError("Invalid macro parameter: " + token.value);
                    }
                }
            }
        }
    }

    Parser(
        const std::map<int64_t, std::string>& parserDict,
        std::vector<std::pair<SourcePos, std::string>>& lines)
        : parserDict(parserDict), lines(lines)
    {
        globalSymbols.clear();
        localSymbols.clear();
        globalSymbols.addsymchanged([this](Sym& sym) {
            std::cout << "Symbol changed";
            sym.print();
            std::cout << "\n";
            });
        tokens.clear();
    }

    std::shared_ptr<ASTNode> Assemble();
    symaccess GetUnresolvedLocalSymbols()
    {
        return localSymbols.getUnresolved();
    }

    symaccess GetUnresolvedSymbols()
    {
        return globalSymbols.getUnresolved();
    }

    std::string get_token_error_info() const
    {
        if (this == nullptr) return "";

        const int range = 3;

        if (current_pos >= tokens.size()) return "at end of input";
        const Token& tok = tokens[current_pos];
        std::string str = (tok.type != EOL ? ("at token type " + parserDict.at(tok.type)) +
            " ('" + tok.value + "') " : " ") + "[line " +
            tok.pos.filename + " " + std::to_string(tok.pos.line) + ", col " +
            std::to_string(tok.line_pos) + "]";

        for (auto l = max(tok.pos.line - range, 0); l < min(tok.pos.line + range, lines.size() - 1); ++l) {
            str += es.gr(es.BLUE_FOREGROUND);
            auto ln = paddLeft(std::to_string(l + 1), 4);
            str += "\n" + ln + " ";
            if (l + 1 == tok.pos.line) {
                str += es.gr(es.BRIGHT_RED_FOREGROUND);
            }
            else {
                str += es.gr(es.WHITE_FOREGROUND);
            }
            str += lines[l].second;
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
