// parser.cpp
#include <iomanip>
#include <stack>
#include <fstream>
#include <iostream>

#include "parser.h"
#include "grammar_rule.h"
#include "token.h"

/// <summary>
/// A static member variable representing an ANSI escape sequence parser.
/// </summary>
ANSI_ESC Parser::es;

/// <summary>
/// A static map that tracks processed rules using a pair of size_t and int64_t as the key and an int as the value.
/// </summary>
static std::map<std::pair<size_t, int64_t>, int> rule_processed;

/// <summary>
/// A stack used to store ParseState objects during parsing.
/// </summary>
std::stack<ParseState> parseStack;

/// <summary>
/// Pushes a parse state onto the parser's internal stack.
/// </summary>
/// <param name="state">The parse state to be pushed onto the stack.</param>
void Parser::pushParseState(ParseState& state)
{
    parseStack.push(state);
}

/// <summary>
/// Removes and returns the top parse state from the parse stack.
/// </summary>
/// <returns>The parse state that was at the top of the parse stack.</returns>
ParseState Parser::popParseState()
{
    auto state = parseStack.top();
    parseStack.pop();
    return state;
}

std::shared_ptr<ASTNode> Parser::parse()
{
    auto ast = parse_rule(RULE_TYPE::Prog);
    return ast;
}

size_t Parser::FindPrevEOL(size_t idx) const
{
    for (size_t i = idx; i > 0; --i) {
        if (tokens[i - 1].type == EOL) return i - 1;
    }
    return (size_t)-1;
}

size_t Parser::FindNextEOL(size_t idx) const
{
    for (size_t i = idx; i < tokens.size(); ++i) {
        if (tokens[i].type == EOL) return i;
    }
    throwError("Missing EOL while scanning tokens");
}

void Parser::EraseRange(size_t start, size_t endExclusive)
{
    if (endExclusive <= start) return;
    if (start > tokens.size() || endExclusive > tokens.size()) return;
    tokens.erase(tokens.begin() + start, tokens.begin() + endExclusive);
    if (current_pos >= start) {
        if (current_pos < endExclusive) current_pos = start;
        else current_pos -= (endExclusive - start);
    }
}

Parser::ElseEndif Parser::FindMatchingElseEndif(size_t from) const
{
    size_t depth = 1;
    std::optional<size_t> foundElse;
    for (size_t i = from; i < tokens.size(); ++i) {
        switch (tokens[i].type) {
            case IF_DIR:
            case IFDEF_DIR:
            case IFNDEF_DIR:
                ++depth;
                break;
            case ENDIF_DIR:
                --depth;
                if (depth == 0) {
                    return { foundElse, i };
                }
                break;
            case ELSE_DIR:
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


void Parser::SpliceConditional(bool cond, size_t afterDirectivePos)
{
    // afterDirectivePos is just after the expr/symbol on the directive line
    const size_t dirEOL = FindNextEOL(afterDirectivePos);
    const size_t bodyStart = dirEOL + 1;

    // Compute the matching else/endif before erasing anything
    auto match = FindMatchingElseEndif(bodyStart);
    const size_t endifIdx = match.endifIdx;

    auto lineStart = [this](size_t anyIdx) -> size_t
        {
            size_t prev = FindPrevEOL(anyIdx);
            return (prev == (size_t)-1) ? 0 : prev + 1;
        };
    auto lineEndEx = [this](size_t anyIdx) -> size_t
        {
            return FindNextEOL(anyIdx) + 1;
        };

    std::vector<std::pair<size_t, size_t>> toErase;

    // 1) Always remove the directive line itself (.if/.ifdef/.ifndef ...)
    const size_t ifLineStart = lineStart(afterDirectivePos);
    const size_t ifLineEndEx = dirEOL + 1;
    toErase.emplace_back(ifLineStart, ifLineEndEx);

    // 2) Remove the inactive part and structural lines
    const size_t endifLineStart = lineStart(endifIdx);
    const size_t endifLineEndEx = lineEndEx(endifIdx);

    if (cond) {
        // Keep THEN body; remove ELSE..ENDIF (if present) or just ENDIF
        if (match.elseIdx) {
            const size_t elseIdx = *match.elseIdx;
            const size_t elseLineStart = lineStart(elseIdx);
            toErase.emplace_back(elseLineStart, endifLineEndEx); // removes .else line and everything up to and including .endif
        }
        else {
            toErase.emplace_back(endifLineStart, endifLineEndEx); // no else: drop only .endif line
        }
    }
    else {
        // Keep ELSE body (if present); otherwise drop THEN..ENDIF
        if (match.elseIdx) {
            const size_t elseIdx = *match.elseIdx;
            const size_t elseLineEndEx = lineEndEx(elseIdx); // end of the .else line
            toErase.emplace_back(bodyStart, elseLineEndEx);   // drop THEN body and .else line
            toErase.emplace_back(endifLineStart, endifLineEndEx); // and drop .endif line
        }
        else {
            // no else: remove THEN body all the way through ENDIF
            toErase.emplace_back(bodyStart, endifLineEndEx);
        }
    }

    // Erase from right to left so indices remain valid
    std::sort(toErase.begin(), toErase.end(),
        [](auto& a, auto& b) { return a.first > b.first; });
    for (auto& r : toErase) {
        EraseRange(r.first, r.second);
    }
}

std::shared_ptr<ASTNode> Parser::Pass()
{
    InitPass();
    pass++;
    return parse();
}

void Parser::InitPass()
{
    globalSymbols.changes = 0;
    localSymbols.clear();
    rule_processed.clear();
    anonLables.reset();

    PC = 0x1000;
    current_pos = 0;
    sourcePos = { "", 0 };
    inMacroDefinition = false;
}

std::vector<std::pair<SourcePos, std::string>> Parser::readfile(std::string filename)
{
    std::vector<std::pair<SourcePos, std::string>> lines;
    if (!fileCache.contains(filename)) {

        // Read the file contents
        std::ifstream incfile(filename);
        if (!incfile) {
            throwError("Could not open file: " + filename);
        }
        std::string line;
        int l = 0;
        while (std::getline(incfile, line)) {
            lines.push_back({ SourcePos(filename, ++l), line });
        }
        fileCache[filename] = lines;
        return lines;
    }
    return fileCache[filename];
}

void Parser::printTokens()
{
    auto index = 0;
    for (auto& tok : tokens) {
        std::cout <<
            "[" <<
            std::setw(4) <<
            index++ <<
            std::setw(0) <<
            "] " <<
            std::left <<
            std::setw(10) <<
            parserDict[tok.type] <<
            std::right <<
            std::setw(0) <<
            " " <<
            (tok.type == EOL ? "\\n" : tok.value) << " ";
        tok.pos.print();
    }

    std::cout << "current_pos " << current_pos << "\n";
}

void Parser::RemoveLine(SourcePos& /*pos*/)
{
    if (tokens.empty()) return;

    // Remove the physical line that contains the token we just parsed.
    // We are typically at EOL or right after the last token of the line.
    size_t around = current_pos;
    if (around > 0) --around;

    size_t begin = findLineStart(tokens, around);
    size_t end = findLineEnd(tokens, begin);

    if (begin < end && end <= tokens.size()) {
        tokens.erase(tokens.begin() + begin, tokens.begin() + end);
        current_pos = begin; // so we will parse the inserted expansion next
    }
}

void Parser::InsertTokens(int pos, std::vector<Token>& tok)
{
    if (pos < 0) pos = 0;
    if (pos > static_cast<int>(tokens.size())) pos = static_cast<int>(tokens.size());

    tokens.insert(tokens.begin() + pos, tok.begin(), tok.end());
    current_pos = pos; // process expansion immediately
}

std::shared_ptr<ASTNode> Parser::parse_rule(int64_t rule_type)
{

    // Look up the rule in our map
    auto rule_it = grammar_rules.find(rule_type);
    if (rule_it == grammar_rules.end()) {
        std::cout << "No rule found for type: " << parserDict[rule_type] << "\n";
        return nullptr;
    }

    const RuleHandler& rule = rule_it->second;

    // Try each production
    for (const auto& production : rule.productions) {
        size_t start_pos = current_pos;
        std::vector<RuleArg> args;
        bool match = true;

        // Start from 1 because production[0] is the rule type
        for (size_t i = 1; i < production.size(); ++i) {
            int64_t expected = production[i];

            if (expected < 0) {  // Non-terminal
                auto node = parse_rule(-expected);
                if (!node) {
                    match = false;
                    break;
                }
                args.push_back(node);
            }
            else {  // Terminal
                if (current_pos >= tokens.size() ||
                    tokens[current_pos].type != expected) {
                    match = false;
                    break;
                }
                auto& tok = tokens[current_pos++];
                sourcePos = tok.pos;
                args.push_back(tok);
            }
        }

        if (match) {
            auto pair = std::make_pair(current_pos, rule_type);
            int count = 0;
            if (rule_processed.find(pair) != rule_processed.end()) {
                count = rule_processed[pair];
            }
            auto result = rule.action(*this, args, count);
            rule_processed[pair] = ++count;

            return result;
        }

        // Reset if no match
        current_pos = start_pos;
    }

    return nullptr;
}

static std::string string_replace(std::string src, std::string const& target, std::string const& repl)
{
    // handle error situations/trivial cases

    if (target.length() == 0) {
        // searching for a match to the empty string will result in 
        //  an infinite loop
        //  it might make sense to throw an exception for this case
        return src;
    }

    if (src.length() == 0) {
        return src;  // nothing to match against
    }

    size_t idx = 0;

    for (;;) {
        idx = src.find(target, idx);
        if (idx == std::string::npos)  break;

        src.replace(idx, target.length(), repl);
        idx += repl.length();
    }

    return src;
}

void exprExtract(int& argNum, std::shared_ptr<ASTNode> node, std::vector<std::pair<SourcePos, std::string>>& lines)
{
    if (node->type == Expr) {
        std::string target = "\\" + std::to_string(argNum);
        std::string repl = std::to_string(node->value);
        for (auto& text : lines)
            text.second = string_replace(text.second, target, repl);
        argNum++;
    }
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& node = std::get<std::shared_ptr<ASTNode>>(child);
            exprExtract(argNum, node, lines);
        }
    }

    return;
};
