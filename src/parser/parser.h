// parser.h
#pragma once
#include <map>
#include <algorithm>
#include <memory>
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
#include "AnonLabels.h"
#include "token.h"

class ExpressionParser;


extern ANSI_ESC es;

extern std::string paddLeft(const std::string& str, size_t totalwidth);
extern std::string paddRight(const std::string& str, size_t totalwidth);

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
};

class Parser {

public:
    SymTable globalSymbols;
    SymTable localSymbols;
    SymTable varSymbols;
    AnonLabels anonLables;
    ExpressionParser* parent;

    std::vector<SourcePos> doStack;

    int pass = 0;

    void throwError(std::string str) const
    {
        throw std::runtime_error(
            str + " " + get_token_error_info()
        );
    }

    ParseState getCurrentState()
    {
        return ParseState
        {
            .filename = filename,
            .current_pos = current_pos,
            .current_source = sourcePos,
            .tokens = tokens,
        };
    }

    void setCurrentState(ParseState& state)
    {
        filename = state.filename;
        current_pos = state.current_pos;
        sourcePos = state.current_source;
        tokens = state.tokens;
    }

    void RemoveLine(SourcePos& pos);
    void InsertTokens(int pos, std::vector<Token>& tok);
    void printToken(int index);
    void printTokens(int start, int end);
    void printTokens();
    std::vector<std::pair<SourcePos, std::string>> readfile(std::string filename);

    std::string filename;
    std::vector<Token> tokens;
    uint16_t org = 0x1000;
    int32_t PC = org;
    uint32_t bytesInLine = 0;
    bool atBOL = true;
    size_t current_pos = 0;
    SourcePos sourcePos;
    std::map<int64_t, std::string> parserDict;
    std::map<std::string, std::vector<std::pair<SourcePos, std::string>>> fileCache;

    bool inMacroDefinition = false;

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
        const std::map<int64_t, std::string>& parserDict)
        : parserDict(parserDict)
    {
        globalSymbols.clear();
        localSymbols.clear();
        varSymbols.clear();
        tokens.clear();
        doStack.clear();
    }

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

        auto lines = fileCache.at(tok.pos.filename);

        for (auto l = std::max(tok.pos.line - range, static_cast<size_t>(0)); l < std::min(tok.pos.line + range, lines.size() - 1); ++l) {
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
                throwError(
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

    static size_t findLineStart(const std::vector<Token>& tokens, size_t idx)
    {
        // Move to the token just before idx if idx points at EOL or end
        if (idx > 0 && idx <= tokens.size() && idx < tokens.size() && tokens[idx].type == EOL) {
            --idx;
        }
        while (idx > 0 && tokens[idx - 1].type != EOL) {
            --idx;
        }
        return idx;
    }

    static size_t findLineEnd(const std::vector<Token>& tokens, size_t idx)
    {
        // Move forward to (just after) the EOL that terminates this line
        while (idx < tokens.size() && tokens[idx].type != EOL) {
            ++idx;
        }
        if (idx < tokens.size()) {
            ++idx; // include the EOL
        }
        return idx;
    }

    // Finds the index of the previous EOL before idx. Returns (size_t)-1 if none.
    size_t FindPrevEOL(size_t idx) const;

    // Finds the index of the next EOL at or after idx (search forward). Throws if none.
    size_t FindNextEOL(size_t idx) const;

    // Erase tokens [start, endExclusive). Adjust current_pos accordingly.
    void EraseRange(size_t start, size_t endExclusive);

    // From 'from' (first token of the body, i.e., token after the .if line EOL),
    // find matching .else (at depth 1) and the matching .endif (depth 0 from this start).
    // Returns { elseIdx (optional), endifIdx } where indices point to the ELSE/ENDIF tokens.
    struct ElseEndif {
        std::optional<size_t> elseIdx;
        size_t endifIdx;
    };
    ElseEndif FindMatchingElseEndif(size_t from) const;
    size_t FindMatchingWhile(size_t from) const;

    // Delete inactive/structural parts of this conditional, starting just after we parsed the directive.
    // 'afterDirectivePos' should be p.current_pos (which is right after the expr/symbol of the directive,
    // and just before the EOL token of the directive line).
    void SpliceConditional(bool cond, size_t afterDirectivePos);
    void ProcessDoLoop(size_t doPosition);

    bool IsSymbolDefined(const std::string& name) const
    {
        return localSymbols.isDefined(name) || globalSymbols.isDefined(name) || varSymbols.isDefined(name);
    }

};
