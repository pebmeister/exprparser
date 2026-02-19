// parser.cpp
//
// This file implements the recursive descent parser for a 6502/65C02 Assembler.
// It handles grammar rule matching, token processing, conditional assembly directives,
// macro expansion, file inclusion, and multi-pass assembly support.
//

#include <iomanip>
#include <stack>
#include <fstream>
#include <iostream>

/// <summary>
/// ANSI escape sequence handler for colored/formatted terminal output.
/// Used for error messages and debugging output.
/// </summary>
#define DEFINE_ANSI_ES

#include "ANSI_esc.h"
#include "parser.h"
#include "grammar_rule.h"
#include "token.h"
#include <expressionparser.h>

// Disable warning C4715: 'not all control paths return a value'
// This is intentionally suppressed as some functions use throwError() which doesn't return
#pragma warning(disable: 4715)

//=============================================================================
// Global Variables
//=============================================================================


/// <summary>
/// Tracks which grammar rules have been processed at specific token positions.
/// Key: pair of (token position, rule type)
/// Value: count of how many times this rule was processed at this position
/// This prevents infinite recursion and enables proper handling of repeated rules.
/// </summary>
std::map<std::pair<size_t, int64_t>, int> Parser::rule_processed;

/// <summary>
/// Stack for saving and restoring parser state during speculative parsing.
/// Enables backtracking when a grammar rule match fails.
/// </summary>
std::stack<ParseState> parseStack;

//=============================================================================
// Parse State Management
//=============================================================================

/// <summary>
/// Saves the current parser state onto the parse stack.
/// Used before attempting to match a grammar production that might fail,
/// allowing the parser to backtrack if necessary.
/// </summary>
/// <param name="state">The current parse state to preserve.</param>
void Parser::pushParseState(ParseState& state)
{
    parseStack.push(state);
}

/// <summary>
/// Restores a previously saved parser state from the stack.
/// Called when backtracking is needed after a failed grammar match.
/// </summary>
/// <returns>The most recently saved parse state.</returns>
ParseState Parser::popParseState()
{
    auto state = parseStack.top();
    parseStack.pop();
    return state;
}

//=============================================================================
// Main Parse Entry Point
//=============================================================================

/// <summary>
/// Entry point for parsing the token stream.
/// Initiates parsing starting from the top-level 'Prog' (Program) rule.
/// </summary>
/// <returns>
/// The root AST (Abstract Syntax Tree) node representing the entire program,
/// or nullptr if parsing fails.
/// </returns>
std::shared_ptr<ASTNode> Parser::parse()
{
    auto ast = parse_rule(RULE_TYPE::Prog);
    return ast;
}

//=============================================================================
// Token Stream Navigation Utilities
//=============================================================================

/// <summary>
/// Searches backward from the given index to find the previous End-Of-Line token.
/// Used for determining line boundaries when processing directives.
/// </summary>
/// <param name="idx">The token index to start searching backward from.</param>
/// <returns>
/// The index of the previous EOL token, or (size_t)-1 if no EOL is found
/// (indicating we're on the first line).
/// </returns>
size_t Parser::FindPrevEOL(size_t idx) const
{
    for (size_t i = idx; i > 0; --i) {
        if (tokens[i - 1].type == EOL) return i - 1;
    }
    return (size_t)-1;  // No previous EOL found - we're at the start
}

/// <summary>
/// Searches forward from the given index to find the next End-Of-Line token.
/// Essential for processing line-based assembly directives.
/// </summary>
/// <param name="idx">The token index to start searching forward from.</param>
/// <returns>The index of the next EOL token.</returns>
/// <exception cref="std::runtime_error">Thrown if no EOL is found (malformed input).</exception>
size_t Parser::FindNextEOL(size_t idx) const
{
    for (size_t i = idx; i < tokens.size(); ++i) {
        if (tokens[i].type == EOL) return i;
    }
    throwError("Missing EOL while scanning tokens");
}

/// <summary>
/// Erases a range of tokens from the token stream and adjusts the current position.
/// Used when removing processed directives or conditional blocks.
/// </summary>
/// <param name="start">The starting index of the range to erase (inclusive).</param>
/// <param name="endExclusive">The ending index of the range to erase (exclusive).</param>
/// <remarks>
/// After erasing, current_pos is adjusted to remain valid:
/// - If current_pos was within the erased range, it moves to 'start'
/// - If current_pos was after the erased range, it's decremented by the erase count
/// </remarks>
void Parser::EraseRange(size_t start, size_t endExclusive)
{
    // Validate range boundaries
    if (endExclusive <= start) return;
    if (start > tokens.size() || endExclusive > tokens.size()) return;

    // Erase the token range
    tokens.erase(tokens.begin() + start, tokens.begin() + endExclusive);

    // Adjust current parsing position to account for removed tokens
    if (current_pos >= start) {
        if (current_pos < endExclusive) {
            current_pos = start;  // Was within erased range
        }
        else {
            current_pos -= (endExclusive - start);  // Was after erased range
        }
    }
}

//=============================================================================
// Loop Directive Handling (.do/.while)
//=============================================================================

/// <summary>
/// Finds the matching .while directive for a .do directive.
/// Handles nested .do/.while constructs by tracking depth.
/// </summary>
/// <param name="from">The token index to start searching from (after the .do).</param>
/// <returns>The index of the matching .while directive.</returns>
/// <exception cref="std::runtime_error">Thrown if no matching .while is found.</exception>
size_t Parser::FindMatchingWhile(size_t from) const
{
    size_t depth = 1;  // Start at depth 1 for the initial .do

    for (size_t i = from; i < tokens.size(); ++i) {
        switch (tokens[i].type) {
            case DO_DIR:
                ++depth;  // Entering a nested .do block
                break;
            case WHILE_DIR:
                --depth;  // Exiting a .do block
                if (depth == 0) {
                    return i;  // Found our matching .while
                }
                break;
            default:
                break;
        }
    }
    throwError("Unmatched .do (missing .while)");
}

//=============================================================================
// Conditional Assembly Directive Handling (.if/.ifdef/.ifndef/.else/.endif)
//=============================================================================

/// <summary>
/// Finds the matching .else (if present) and .endif for a conditional directive.
/// Properly handles nested conditional blocks by tracking depth.
/// </summary>
/// <param name="from">The token index to start searching from (after the condition).</param>
/// <returns>
/// An ElseEndif struct containing:
/// - elseIdx: Optional index of the .else directive (if present at current depth)
/// - endifIdx: Index of the matching .endif directive
/// </returns>
/// <exception cref="std::runtime_error">Thrown if no matching .endif is found.</exception>
Parser::ElseEndif Parser::FindMatchingElseEndif(size_t from) const
{
    size_t depth = 1;  // Start at depth 1 for the initial conditional
    std::optional<size_t> foundElse;  // Track first .else at our depth level

    for (size_t i = from; i < tokens.size(); ++i) {
        switch (tokens[i].type) {
            case IF_DIR:
            case IFDEF_DIR:
            case IFNDEF_DIR:
                ++depth;  // Entering a nested conditional block
                break;

            case ENDIF_DIR:
                --depth;  // Exiting a conditional block
                if (depth == 0) {
                    return { foundElse, i };  // Found our matching .endif
                }
                break;

            case ELSE_DIR:
                // Only record .else at our depth level (depth == 1)
                // and only the first one encountered
                if (depth == 1 && !foundElse) {
                    foundElse = i;
                }
                break;

            default:
                break;
        }
    }
    throwError("Unmatched .if/.ifdef/.ifndef (missing .endif)");
}

/// <summary>
/// Processes a conditional assembly directive by removing inactive code blocks.
/// Handles .if, .ifdef, .ifndef directives with optional .else clauses.
///
/// This function performs "splicing" - removing the directive lines and
/// either the THEN block or ELSE block based on the condition result.
/// </summary>
/// <param name="cond">
/// The evaluated condition result:
/// - true: Keep the THEN block (between directive and .else/.endif)
/// - false: Keep the ELSE block (between .else and .endif), or remove all if no .else
/// </param>
/// <param name="afterDirectivePos">
/// Token position immediately after the directive's expression/symbol
/// (i.e., pointing at or near the EOL of the directive line).
/// </param>
/// <remarks>
/// The function removes tokens in reverse order (right to left) to maintain
/// valid indices throughout the removal process.
///
/// Example for ".if COND" with ELSE:
/// ```
///   .if COND      <- directive line (always removed)
///   THEN_CODE     <- kept if cond==true, removed if cond==false
///   .else         <- structure line (always removed)
///   ELSE_CODE     <- removed if cond==true, kept if cond==false
///   .endif        <- structure line (always removed)
/// ```
/// </remarks>
void Parser::SpliceConditional(bool cond, size_t afterDirectivePos)
{
    // afterDirectivePos is just after the expr/symbol on the directive line
    const size_t dirEOL = FindNextEOL(afterDirectivePos);
    const size_t bodyStart = dirEOL + 1;  // First token of the conditional body

    // Find matching .else (optional) and .endif BEFORE we start erasing
    // (indices become invalid after erasing)
    auto match = FindMatchingElseEndif(bodyStart);
    const size_t endifIdx = match.endifIdx;

    // Lambda: Find the start of the line containing a given token index
    auto lineStart = [this](size_t anyIdx) -> size_t
        {
            size_t prev = FindPrevEOL(anyIdx);
            return (prev == (size_t)-1) ? 0 : prev + 1;
        };

    // Lambda: Find the end of the line containing a given token index (exclusive)
    auto lineEndEx = [this](size_t anyIdx) -> size_t
        {
            return FindNextEOL(anyIdx) + 1;
        };

    // Collect all ranges to erase (will sort and erase in reverse order)
    std::vector<std::pair<size_t, size_t>> toErase;

    // 1) Always remove the directive line itself (.if/.ifdef/.ifndef ...)
    const size_t ifLineStart = lineStart(afterDirectivePos);
    const size_t ifLineEndEx = dirEOL + 1;
    toErase.emplace_back(ifLineStart, ifLineEndEx);

    // 2) Determine what else to remove based on condition and presence of .else
    const size_t endifLineStart = lineStart(endifIdx);
    const size_t endifLineEndEx = lineEndEx(endifIdx);

    if (cond) {
        // Condition is TRUE: Keep THEN body, remove ELSE..ENDIF section
        if (match.elseIdx) {
            // Has .else: Remove from .else line through .endif line
            const size_t elseIdx = *match.elseIdx;
            const size_t elseLineStart = lineStart(elseIdx);
            toErase.emplace_back(elseLineStart, endifLineEndEx);
        }
        else {
            // No .else: Only remove the .endif line
            toErase.emplace_back(endifLineStart, endifLineEndEx);
        }
    }
    else {
        // Condition is FALSE: Remove THEN body, keep ELSE body (if present)
        if (match.elseIdx) {
            // Has .else: Remove THEN body and .else line, then .endif line
            const size_t elseIdx = *match.elseIdx;
            const size_t elseLineEndEx = lineEndEx(elseIdx);
            toErase.emplace_back(bodyStart, elseLineEndEx);        // Drop THEN body and .else line
            toErase.emplace_back(endifLineStart, endifLineEndEx);  // Drop .endif line
        }
        else {
            // No .else: Remove entire THEN body through .endif
            toErase.emplace_back(bodyStart, endifLineEndEx);
        }
    }

    // Sort ranges in descending order by start position
    // This ensures we erase from right to left, keeping earlier indices valid
    std::sort(toErase.begin(), toErase.end(),
        [](auto& a, auto& b) { return a.first > b.first; });

    // Perform the erasures
    for (auto& r : toErase) {
        EraseRange(r.first, r.second);
    }
}

//=============================================================================
// Multi-Pass Assembly Support
//=============================================================================

/// <summary>
/// Executes a single assembly pass over the token stream.
/// 6502 assemblers typically require multiple passes to resolve forward references.
/// </summary>
/// <returns>The root AST node for this pass.</returns>
/// <remarks>
/// Pass 1: Collect all label addresses (some forward references unresolved)
/// Pass 2+: Resolve forward references and finalize addresses
/// Assembly continues until no symbol table changes occur between passes.
/// </remarks>
std::shared_ptr<ASTNode> Parser::Pass()
{
    InitPass();     // Reset parser state for new pass
    pass++;         // Increment pass counter
    return parse(); // Execute the parse
}

/// <summary>
/// Clears the rule processing history.
/// Called between passes or when restarting parsing.
/// </summary>
void Parser::reset_rules()
{
    rule_processed.clear();
}

/// <summary>
/// Initializes the parser state for a new assembly pass.
/// Resets all stateful components while preserving global symbols between passes.
/// </summary>
void Parser::InitPass()
{
    // Reset symbol change counter (used to detect when passes stabilize)
    globalSymbols.changes = 0;
    scope = "GLOBAL_";

    // Clear local (scope-limited) symbols - they're recomputed each pass
 //   if (localSymbols.size() > 0) {
 //       localSymbols.clear();
//    }

    // Clear rule processing history
    rule_processed.clear();

    // Reset anonymous label tracking (for +/- relative labels)
    anonLabels.reset();
    varSymbols.clear();

    // Reset program counter to default origin
    // 0x1000 is a common default for 6502 programs
    PC = 0x1000;

    // Reset token stream position to beginning
    current_pos = 0;

    // Clear source position tracking
    sourcePos = { "", 0 };

    // Reset macro definition state
    inMacroDefinition = false;

    // clear pending expansions
    clearPendingExpansions();
}

std::vector<Token> Parser::applyPendingExpansions(const std::vector<Token>& tokens)
{
    std::vector<Token> result;
    result.reserve(tokens.size() * 2);  // Reserve some extra space

    for (size_t i = 0; i < tokens.size(); ) {
        if (tokens[i].type == DO_DIR) {
            // Create key from token's position
            auto key = std::make_pair(tokens[i].pos.filename, tokens[i].pos.line);
            auto it = pendingLoopExpansions.find(key);

            if (it != pendingLoopExpansions.end()) {
                // Found a pending expansion - insert it instead of the loop construct
                result.insert(result.end(), it->second.begin(), it->second.end());

                // Skip past the entire loop construct (DO_DIR ... WHILE_DIR ... EOL)
                int depth = 1;
                i++;  // Move past DO_DIR
                while (depth > 0 && i < tokens.size()) {
                    if (tokens[i].type == DO_DIR) {
                        depth++;
                    }
                    else if (tokens[i].type == WHILE_DIR) {
                        depth--;
                        if (depth == 0) {
                            // Skip to end of .while line (including EOL)
                            while (i < tokens.size() && tokens[i].type != EOL) {
                                i++;
                            }
                            if (i < tokens.size()) {
                                i++;  // Skip the EOL
                            }
                            break;
                        }
                    }
                    i++;
                }
                // Don't erase from map yet - might need for multiple iterations
                continue;
            }
        }
        result.push_back(tokens[i]);
        i++;
    }
    return result;
}

//=============================================================================
// File Handling
//=============================================================================

/// <summary>
/// Reads a source file and returns its contents as a vector of lines.
/// Implements file caching to avoid re-reading files on subsequent passes.
/// Searches through includeDirectories if the file is not found in the current path.
/// </summary>
/// <param name="filename">The path to the file to read.</param>
/// <returns>
/// A vector of pairs, each containing:
/// - SourcePos: The file name and line number for error reporting
/// - std::string: The actual line content
/// </returns>
/// <exception cref="std::runtime_error">Thrown if the file cannot be opened.</exception>
std::vector<std::pair<SourcePos, std::string>> Parser::readfile(std::string filename)
{
    namespace fs = std::filesystem;
    std::vector<std::pair<SourcePos, std::string>> lines;

    // Check if file is already cached
    if (!fileCache.contains(filename)) {
        // Normalize the requested filename to an absolute path
        fs::path requested_path = fs::absolute(fs::path(filename)).lexically_normal();
        std::ifstream incfile;
        std::string resolved_filename = filename;

        // Try opening the file directly first
        incfile.open(requested_path);
        if (!incfile) {
            // File not found in direct path, search through includeDirectories
            bool found = false;
            for (const auto& dir : includeDirectories) {
                fs::path include_path = fs::absolute(fs::path(dir) / filename).lexically_normal();
                incfile.open(include_path);
                if (incfile) {
                    resolved_filename = include_path.string();
                    found = true;
                    break;
                }
            }

            if (!found) {
                throwError("Could not open file: " + filename);
            }
        }

        // Read line by line, preserving source positions for error reporting
        std::string line;
        int l = 0;
        while (std::getline(incfile, line)) {
            lines.push_back({ SourcePos(resolved_filename, ++l), line });
        }
        filename = resolved_filename;

        // Cache the file contents for subsequent passes
        fileCache[filename] = lines;
        return lines;
    }

    // Return cached contents
    return fileCache[filename];
}

//=============================================================================
// Debug Output Functions
//=============================================================================

/// <summary>
/// Prints a single token with its index, type, value, and source position.
/// Used for debugging the lexer output and parser state.
/// </summary>
/// <param name="index">The index of the token to print.</param>
void Parser::printToken(int index)
{
    auto& tok = tokens[index];
    std::cout <<
        "[" <<
        std::setw(4) <<
        index++ <<
        std::setw(0) <<
        "] " <<
        std::left <<
        std::setw(10) <<
        parserDict[tok.type] <<         // Token type name from dictionary
        std::right <<
        std::setw(0) <<
        " " <<
        (tok.type == EOL ? "\\n" : tok.value) << " ";  // Show \n for EOL tokens
    tok.pos.print();  // Print source file and line number
}

void Parser::printTokens(std::vector<Token> toks)
{
    auto temp = tokens;
    auto tempcur = current_pos;
    tokens = toks;
    current_pos = 0;
    printTokens();
    tokens = temp;
    current_pos = tempcur;
}

/// <summary>
/// Prints a range of tokens from the token stream.
/// Useful for debugging specific sections of the input.
/// </summary>
/// <param name="start">The starting index (inclusive).</param>
/// <param name="end">The ending index (inclusive).</param>
void Parser::printTokens(int start, int end)
{
    for (auto index = start; index <= end; ++index) {
        printToken(index);
    }
}

/// <summary>
/// Prints all tokens in the token stream along with the current parse position.
/// Primary debugging function for examining the complete lexer output.
/// </summary>
void Parser::printTokens()
{
    printTokens(0, tokens.size() - 1);
    std::cout << "current_pos " << current_pos << "\n";
}

//=============================================================================
// Token Stream Manipulation
//=============================================================================

/// <summary>
/// Removes the current line from the token stream.
/// Used after processing directives that should not produce output
/// (e.g., macro definitions, equates, conditionals).
/// </summary>
/// <remarks>
/// The function identifies the line containing the most recently parsed token
/// and removes all tokens from line start through the EOL (inclusive).
/// After removal, current_pos is set to the start position so parsing
/// continues with any newly inserted content or the next original line.
/// </remarks>
void Parser::RemoveCurrentLine()
{
    if (tokens.empty()) return;

    // Find the line containing the token we just processed
    // We may be at EOL or just past the last token of the line
    size_t around = current_pos;
    if (around > 0) --around;

    // Determine line boundaries
    size_t begin = findLineStart(tokens, around);
    size_t end = findLineEnd(tokens, begin);

    // Erase the entire line
    if (begin < end && end <= tokens.size()) {
        tokens.erase(tokens.begin() + begin, tokens.begin() + end);
        current_pos = begin;  // Position to parse any inserted expansion next
    }
}

/// <summary>
/// Removes a range of complete lines from the token stream.
/// Used after processing multi-line directives (IF/ELSE/ENDIF, macros, REPEAT blocks).
/// </summary>
/// <param name="start_pos">Token position within the first line to remove</param>
/// <param name="end_pos">Token position within the last line to remove</param>
/// <remarks>
/// Both positions identify lines - all tokens from the start of the line containing
/// start_pos through the EOL of the line containing end_pos are removed.
///
/// After removal, current_pos is adjusted:
/// - Before removed range: unchanged
/// - Within removed range: set to removal start (ready to parse subsequent content)
/// - After removed range: reduced by number of removed tokens
/// </remarks>
void Parser::RemoveLineRange(size_t start_pos, size_t end_pos)
{
    if (tokens.empty()) return;

    // Clamp to valid token indices
    start_pos = std::min(start_pos, tokens.size() - 1);
    end_pos = std::min(end_pos, tokens.size() - 1);

    // Normalize: ensure start <= end
    if (start_pos > end_pos)
        std::swap(start_pos, end_pos);

    // Expand to complete line boundaries
    size_t remove_begin = findLineStart(tokens, start_pos);
    size_t remove_end = findLineEnd(tokens, end_pos);

    // Sanity check
    if (remove_begin >= remove_end || remove_end > tokens.size())
        return;

    size_t removed_count = remove_end - remove_begin;

    // Perform the erasure
    tokens.erase(tokens.begin() + remove_begin,
        tokens.begin() + remove_end);

    // Adjust current_pos relative to the removed range
    if (current_pos < remove_begin) {
        // Before removed range: no adjustment needed
    }
    else if (current_pos < remove_end) {
        // Within removed range: position at removal point
        current_pos = remove_begin;
    }
    else {
        // After removed range: shift back by removed count
        current_pos -= removed_count;
    }
}

/// <summary>
/// Inserts a vector of tokens at the specified position in the token stream.
/// Used for macro expansion and file inclusion.
/// </summary>
/// <param name="pos">
/// The position at which to insert tokens.
/// Clamped to valid range [0, tokens.size()].
/// </param>
/// <param name="tok">The vector of tokens to insert.</param>
/// <remarks>
/// After insertion, current_pos is set to the insertion point so the
/// parser will process the newly inserted tokens immediately.
/// </remarks>
void Parser::InsertTokens(int pos, std::vector<Token>& tok)
{
    // Clamp position to valid range
    if (pos < 0) pos = 0;
    if (pos > static_cast<int>(tokens.size())) pos = static_cast<int>(tokens.size());

    // Insert the tokens
    tokens.insert(tokens.begin() + pos, tok.begin(), tok.end());

    // Set position to process the expansion immediately
    current_pos = pos;
}

//=============================================================================
// Core Recursive Descent Parser
//=============================================================================

/// <summary>
/// Attempts to match a grammar rule against the current position in the token stream.
/// This is the heart of the recursive descent parser.
/// </summary>
/// <param name="rule_type">
/// The type of grammar rule to match (e.g., RULE_TYPE::Prog, RULE_TYPE::Statement).
/// </param>
/// <returns>
/// An AST node representing the matched construct if successful, nullptr otherwise.
/// </returns>
/// <remarks>
/// For each rule, the parser tries each production alternative in order:
/// 1. For terminals (positive values): Match against current token
/// 2. For non-terminals (negative values): Recursively parse the sub-rule
///
/// If all elements of a production match, the rule's action handler is invoked
/// to construct the appropriate AST node.
///
/// If a production fails to match, the parser backtracks to try the next alternative.
/// </remarks>
std::shared_ptr<ASTNode> Parser::parse_rule(int64_t rule_type)
{
    // Look up the rule definition in the grammar
    auto rule_it = grammar_rules.find(rule_type);
    if (rule_it == grammar_rules.end()) {
        std::cout << "No rule found for type: " << parserDict[rule_type] << "\n";
        return nullptr;
    }

    const RuleHandler& rule = rule_it->second;
    // ===== NEW: Defer variable updates during loop structure parsing =====
   
    bool savedDefer = deferVariableUpdates;
    if (rule_type == DoDirective || rule_type == WhileDirective) {
        deferVariableUpdates = true;
    }

    // =====================================================================
    // Try each production alternative for this rule
    for (const auto& production : rule.productions) {
        size_t start_pos = current_pos;     // Save position for backtracking
        std::vector<RuleArg> args;          // Collect matched elements
        bool match = true;

        // production[0] is the rule type itself, actual elements start at index 1
        for (size_t i = 1; i < production.size(); ++i) {
            int64_t expected = production[i];

            if (expected < 0) {
                // Non-terminal: Recursively parse the sub-rule
                // Negative value encodes the rule type (negated)
                auto node = parse_rule(-expected);
                if (!node) {
                    match = false; 
                    break;
                }
                args.push_back(node);
            }
            else {
                // Terminal: Match against current token
                if (current_pos >= tokens.size() ||
                    tokens[current_pos].type != expected) {
                    match = false;
                    break;
                }
                // Consume the token and track source position for error reporting
                auto& tok = tokens[current_pos++];
                sourcePos = tok.pos;
                args.push_back(tok);
            }
        }

        if (match) {
            // Successfully matched this production
            // Track how many times this rule matched at this position
            // (used for handling repeated/recursive rules)
            auto pair = std::make_pair(current_pos, rule_type);
            int count = 0;
            if (rule_processed.find(pair) != rule_processed.end()) {
                count = rule_processed[pair];
            }
            
            //if (count == 0) {
            //    // Invoke the rule's semantic action to build the AST node
            //    auto& rulestr = parserDict[rule_type];
            //    if (rulestr.empty()) {
            //        throwError("missing name for " + std::to_string(rule_type));
            //    }
            //    std::cout << "Rule matched: " << parserDict[rule_type] << " linepos " << tokens[current_pos - 1].line_pos << " ";
            //    tokens[current_pos -1].pos.print();
            //}
            auto result = rule.action(*this, args, count);
            rule_processed[pair] = ++count;

            return result;
        }

        // Production didn't match - backtrack and try next alternative
        current_pos = start_pos;
    }

    // ===== NEW: Restore defer flag on no match =====
    deferVariableUpdates = savedDefer;

    // ================================================
    // No production matched for this rule
    return nullptr;
}

//=============================================================================
// String Utility Functions
//=============================================================================

/// <summary>
/// Replaces all occurrences of a target substring with a replacement string.
/// Used primarily for macro parameter substitution.
/// </summary>
/// <param name="src">The source string to process.</param>
/// <param name="target">The substring to search for.</param>
/// <param name="repl">The replacement string.</param>
/// <returns>A new string with all substitutions applied.</returns>
/// <remarks>
/// Handles edge cases:
/// - Empty target string: Returns source unchanged (prevents infinite loop)
/// - Empty source string: Returns empty string
/// - Replacement can be longer or shorter than target
/// </remarks>
static std::string string_replace(std::string src, std::string const& target, std::string const& repl)
{
    // Prevent infinite loop if searching for empty string
    if (target.length() == 0) {
        return src;
    }

    // Nothing to process in empty source
    if (src.length() == 0) {
        return src;
    }

    size_t idx = 0;

    // Find and replace all occurrences
    for (;;) {
        idx = src.find(target, idx);
        if (idx == std::string::npos) break;  // No more occurrences

        src.replace(idx, target.length(), repl);
        idx += repl.length();  // Move past replacement to avoid infinite loops
    }

    return src;
}

//=============================================================================
// Macro Argument Extraction
//=============================================================================

/// <summary>
/// Recursively extracts expression values from an AST and substitutes them
/// into macro body lines as positional arguments (\1, \2, etc.).
/// </summary>
/// <param name="argNum">
/// Reference to the current argument number. Incremented for each Expr node found.
/// </param>
/// <param name="node">The AST node to process.</param>
/// <param name="lines">
/// The macro body lines to perform substitution on.
/// Each line is a pair of (SourcePos, string content).
/// </param>
/// <remarks>
/// Macro arguments are referenced as \1, \2, \3, etc. in macro bodies.
/// This function walks the AST looking for expression nodes and replaces
/// the corresponding placeholder with the evaluated value.
///
/// Example:
/// ```
///   .macro foo
///     lda #\1
///     sta \2
///   .endm
///   foo $42, $0200   ; Expands to: lda #$42 / sta $0200
/// ```
/// </remarks>
void exprExtract(int& argNum, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines)
{
    // If this node is an expression, substitute its value
    if (node->type == Expr) {
        std::string target = "\\" + std::to_string(argNum);  // e.g., "\1"
        std::string repl = std::to_string(node->value);      // The evaluated value

        // Apply substitution to all lines in the macro body
        for (auto& text : lines) {
            text.second = string_replace(text.second, target, repl);
        }
        argNum++;  // Move to next argument number

    }

    // Recursively process child nodes
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childNode = std::get<std::shared_ptr<ASTNode>>(child);
            exprExtract(argNum, childNode, lines);
        }
    }

    return;
}

void setmacroscope(std::string name, int timesCalled, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines)
{
    //   std::cout << "\n======= MACRO " << name <<  " ========= \n";    
    std::string target = "@";
    std::string repl = "@M_" + name + std::to_string(timesCalled) + "_";
    for (auto& [_, text] : lines) {
        auto pos = text.find("@M_" + name);
        if (pos == std::string::npos) {
            text = string_replace(text, target, repl);
        }
    }
}