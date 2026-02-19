// parser.h
//
// Central parser/assembler interface for the 6502/65C02 assembler.
// This header declares the Parser class which is responsible for:
//  - token stream management and manipulation,
//  - macro storage and expansion,
//  - symbol tables and anonymous label tracking,
//  - multi-pass assembly state (program counter history, passes),
//  - high-level helpers used by the expression/grammar rules.
//
// The header intentionally keeps member functions implemented in .cpp files
// where behavior is non-trivial; lightweight helpers and small inline
// utilities remain in the header.
//
// Important conventions:
//  - SourcePos::line is 1-based across the codebase.
//  - `tokens` includes EOL tokens so line boundaries can be located by scanning
//    for EOL token markers.
//  - Parser maintains a fileCache mapping filenames to cached source lines
//    (used for diagnostics, macro extraction and retokenization).
#pragma once
#include <map>
#include <algorithm>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <cinttypes>
#include <iomanip>

#include "ast_source_extractor.h"
#include "ANSI_esc.h"
#include "common_types.h"
#include "expr_rules.h"
#include "grammar_rule.h"

#include "sym.h"
#include "symboltable.h"
#include "AnonLabels.h"
#include "token.h"

// Utility padding helpers used by diagnostic printers (implemented elsewhere).
extern std::string paddLeft(const std::string& str, size_t totalwidth);
extern std::string paddRight(const std::string& str, size_t totalwidth);

/*
 MacroDefinition
 ---------------
 Represents a stored macro: the original source lines that make up the
 macro body, how many parameters the macro expects, and the source position
 where the macro was defined. The assembler stores macros in `macroTable`.
*/
class MacroDefinition {
public:
    // Body lines of the macro (source position + raw text)
    std::vector<std::pair<SourcePos, std::string>> bodyText;

    // Number of parameters the macro expects (parse from symbol/definition)
    int paramCount;

    // Position where the macro was defined (used for diagnostics)
    SourcePos definedAtLine;

    MacroDefinition(std::vector<std::pair<SourcePos, std::string>> text, int params, SourcePos line)
    {
        bodyText = text;
        paramCount = params;
        definedAtLine = line;
    }

    // Diagnostic printer for macro contents
    void print()
    {
        for (auto& [pos, src] : bodyText) {
            std::cout << pos.filename << "  " << pos.line << ")  " << src << "\n";
        }
        std::cout << "\n";
    }
};

// Helper used by external code to extract expressions into source lines.
extern void exprExtract(int& argNum, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines);
extern void setmacroscope(std::string name, int pc, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines);


/*
 ParseState
 ----------
 Lightweight POD capturing the parser's mutable runtime state so it can be
 saved/restored while performing nested parsing tasks (macro expansion,
 speculative parsing, etc.).
*/
struct ParseState {
    std::string filename;
    size_t current_pos;
    SourcePos current_source;
    std::vector<Token> tokens;
    int32_t PC;
    uint32_t bytesInLine;
};

/*
 Parser
 ------
 Core class that manages token streams, symbol tables, macros and assembly
 pass state. Many parser operations mutate the token list in-place (inserting
 tokens, removing ranges) to implement macro expansion and structured control
 flow (conditionals, loops, etc.).
*/
class Parser {

private:
    // (Intentionally left blank — internal helpers implemented in .cpp)
public:
    // Symbol tables: global and local (scope) symbols and "var" symbols
    SymTable globalSymbols;
    SymTable localSymbols;
    SymTable varSymbols;

    // Anonymous label storage (forward/backward anonymous labels)
    AnonLabels anonLabels;

    // History of PC values across passes (useful for debugging/diagnostics)
    std::vector<std::vector<int>> PCHistory;

    // Current assembly pass (0..N)
    int pass = 0;

    /*
     throwError
     ----------
     Throw a runtime_error containing the provided message plus token
     context information produced by get_token_error_info(). Marked [[noreturn]].
    */
    [[noreturn]]
    void throwError(std::string str) const
    {
        throw std::runtime_error(
            str + " " + get_token_error_info()
        );
    }

    /*
     getCurrentState / setCurrentState
     ---------------------------------
     Save and restore a compact ParseState snapshot. Used when exploring or
     re-writing token ranges (macro expansion, nested parsing, etc.).
    */
    ParseState getCurrentState()
    {
        return ParseState
        {
            .filename = filename,
            .current_pos = current_pos,
            .current_source = sourcePos,
            .tokens = tokens,
            .PC = PC,
            .bytesInLine = bytesInLine,
        };
    }

    void setCurrentState(ParseState& state)
    {
        filename = state.filename;
        current_pos = state.current_pos;
        sourcePos = state.current_source;
        tokens = state.tokens;
        PC = state.PC;
        bytesInLine = state.bytesInLine;
    }

    // Token stream manipulation helpers (implementations in parser.cpp)
    void RemoveCurrentLine();
    void RemoveLineRange(size_t start_pos, size_t end_pos);
    void InsertTokens(int pos, std::vector<Token>& tok);
    void printToken(int index);
    void printTokens(int start, int end);
    void printTokens(std::vector<Token> tokens);
    void printTokens();
    std::vector<std::pair<SourcePos, std::string>> readfile(std::string filename);

    /// <summary>
    /// When true, variable assignments are deferred (not executed).
    /// Used during initial structure parsing of loop bodies to prevent
    /// side effects before the loop handler takes control.
    /// </summary>
    bool deferVariableUpdates = false;

    // Current source/parse context
    std::string filename;
    std::vector<Token> tokens;
    std::string scope;

    // Default origin and program counter (org / PC)
    uint16_t org = 0x1000;
    int32_t PC = org;

    // Count of bytes emitted on the current source line (used for listing)
    uint32_t bytesInLine = 0;

    // True if parser is at beginning-of-line (affects directive handling)
    bool atBOL = true;

    // Current index into tokens vector
    size_t current_pos = 0;

    // Current SourcePos for diagnostic use
    SourcePos sourcePos;

    // Mapping of token types -> human-readable name used for diagnostics
    std::map<int64_t, std::string> parserDict;

    // Cached file contents: filename -> vector of (SourcePos, line_text)
    std::map<std::string, std::vector<std::pair<SourcePos, std::string>>> fileCache;

    // Indicates currently inside a macro definition (suppresses some side-effects)
    bool inMacroDefinition = false;

    // Push/pop full parse state for nested parsing contexts (e.g., macro expansion)
    void pushParseState(ParseState& state);
    ParseState popParseState();

    // Diagnostic helpers to print symbol tables
    void printSymbols(bool all) const
    {
        globalSymbols.print(all);
    }

    void printVars() const
    {
        varSymbols.print(true);
    }

    /*
     eval_number
     -----------
     Convert textual numeric token into a numeric value depending on
     the token type (DECNUM / HEXNUM / BINNUM / CHAR). Uses std::strtol
     with the appropriate base; for CHAR returns the first character code.
    */
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

    // Macro storage and re-entrancy tracking
    std::unordered_map<std::string, std::shared_ptr<MacroDefinition>> macroTable;
    std::set<std::string> currentMacros;
    int macroCallDepth = 0;

    /*
     expandMacro / processMacroParameters
     ------------------------------------
     Expand macro bodies by copying the macro AST and replacing parameter
     tokens (\1, \2, ...) with the provided argument strings. The expansion
     creates a shallow copy of the macro body AST and recursively replaces
     parameter tokens within Token children.
    */
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

    /*
     Constructor
     -----------
     Initialize the parser with a mapping from token-type ids to names.
     Symbol tables and token buffers are cleared to a known initial state.
    */
    Parser(
        const std::map<int64_t, std::string>& parserDict)
        : parserDict(parserDict)
    {
        globalSymbols.clear();
        localSymbols.clear();
        varSymbols.clear();
        tokens.clear();
    }

    // Access unresolved symbols for diagnostics / linking
    symaccess GetUnresolvedLocalSymbols()
    {
        return localSymbols.getUnresolved();
    }

    symaccess GetUnresolvedSymbols()
    {
        return globalSymbols.getUnresolved();
    }

    /*
     get_token_error_info
     --------------------
     Produce a helpful diagnostic string describing the current token,
     including a few nearby source lines with highlighting. Assumes the
     parser instance (`this`) is valid (non-null) — calling a member
     function on a null `this` is undefined behavior in C++ and therefore
     not checked here.
    */
    std::string get_token_error_info() const
    {
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

    // Reset internal grammar rules (implementation in .cpp)
    void reset_rules();

    // Parse a specific grammar rule, return AST node (implementation in .cpp)
    std::shared_ptr<ASTNode> parse_rule(int64_t rule_type);

    // Pass initialization and main pass functions used by the assembler driver
    void InitPass();
    std::shared_ptr<ASTNode> Pass();
    std::shared_ptr<ASTNode> parse();

    /*
     handle_binary_operation
     -----------------------
     Template helper that implements the common pattern of parsing left-associative
     binary operations (e.g. expression parsing where left op right repeats).
     - left: initial left operand (already parsed)
     - allowed_ops: set of token types representing operators (+, -, etc.)
     - rule_type: AST node type for the created binary node
     - parse_right: callable that parses the right operand and returns AST node
     - calculate: callable that computes node->value from left/right/op
     - expected_name: name used in the error message if the right operand is missing
    */
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
            node->pc_Start = left->pc_Start;
            node->add_child(left);
            node->add_child(op);
            node->add_child(right);
            node->value = calculate(left->value, op.type, right->value);
            left = node;
        }
        return left;
    }

    // Track which rules have been processed for a given token position to avoid infinite recursion
    static std::map<std::pair<size_t, int64_t>, int> rule_processed;

    /*
     findLineStart / findLineEnd
     ---------------------------
     Utility functions to locate the token index that marks the beginning or
     end (EOL token) of the logical source line that contains a given token index.
     Useful when splicing token ranges by line.
    */
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

    /*
     Conditional / loop matching helpers
     ----------------------------------
     FindMatchingElseEndif locates the .else (optional) and .endif that correspond
     to a conditional block starting at `from`. Indices refer to token indices
     (position of ELSE/ENDIF tokens in the tokens vector).
    */
    struct ElseEndif {
        std::optional<size_t> elseIdx;
        size_t endifIdx;
    };
    ElseEndif FindMatchingElseEndif(size_t from) const;
    size_t FindMatchingWhile(size_t from) const;

    // Delete inactive/structural parts of a parsed conditional starting immediately after the directive
    void SpliceConditional(bool cond, size_t afterDirectivePos);

    // Query symbol existence across local/global/var symbol tables
    bool IsSymbolDefined(const std::string& name) const
    {
        return localSymbols.isDefined(name) || globalSymbols.isDefined(name) || varSymbols.isDefined(name);
    }

    // Find index of a Token within a vector (linear search). Returns -1 if not found.
    int findIndex(const std::vector<Token>& v, const Token& val) const
    {
        for (auto i = 0; i < v.size(); i++) {
            if (v[i] == val) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    /**
    * @brief Extracts source lines from an AST node using this parser's file cache.
    *
    * Convenience wrapper that provides direct access to the parser's internal
    * fileCache. Use this for macro body extraction, conditional compilation
    * resolution, or any scenario requiring source reconstruction from AST.
    *
    * @param node The AST node (subtree root) to extract source from
    * @return Sorted vector of (SourcePos, source_text) pairs, ready for retokenization
    *
    * @see extractSourceFromAST() for detailed behavior documentation
    */
    [[nodiscard]]
    std::vector<std::pair<SourcePos, std::string>> getSourceFromAST(
        const std::shared_ptr<ASTNode>& node) const
    {
        return extractSourceFromAST(node, fileCache);
    }

    // Print a vector of source lines to stdout (utility for debugging)
    [[nodiscard]]
    int printSource(std::vector<std::pair<SourcePos, std::string>> source)
    {
        for (auto& src : source) {
            auto& line = src.second;
            std::cout << line << "\n";
        }
        return 0;
    }

    // Storage for deferred loop expansions (keyed by the DO_DIR token source identity)
    std::map<std::pair<std::string, size_t>, std::vector<Token>> pendingLoopExpansions;

    // Apply pending nested loop expansions to a token vector (implementation in .cpp)
    std::vector<Token> applyPendingExpansions(const std::vector<Token>& tokens);

    // Clear pending expansions
    void clearPendingExpansions() { pendingLoopExpansions.clear(); }

    // List of include directories used when resolving includes
    std::vector<std::string> includeDirectories;
};

