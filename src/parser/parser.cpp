// parser.cpp
#include <iomanip>
#include <stack>

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

std::shared_ptr<ASTNode> Parser::Assemble()
{
    std::shared_ptr<ASTNode> ast;

    auto pass = 1;
    std::vector<Sym> unresolved;
    bool needPass;
    do {
        needPass = false;
        ast = Pass();
        auto unresolved_locals = GetUnresolvedLocalSymbols();
        unresolved = GetUnresolvedSymbols();
        pass++;
        if (!unresolved_locals.empty()) {
            std::string err = "Unresolved local symbols:";
            for (auto& sym : unresolved_locals) {
                err += " " + sym.name + " accessed at line(s) ";
                for (auto& line : sym.accessed) {
                    err += line.filename + " " + std::to_string(line.line) + " ";
                }
                err += "\n";
            }
            throwError(err);
        }
        needPass = unresolved.size() > 0;
    } while (pass < 10 && needPass);

    if (!unresolved.empty()) {
        std::string err = "Unresolved global symbols:";
        for (auto& sym : unresolved) {
            err += " " + sym.name;
        }
        throw std::runtime_error(err + " " + get_token_error_info());
    }
    return ast;
}

std::shared_ptr<ASTNode> Parser::Pass()
{
    InitPass();
    return parse();
}

void Parser::InitPass()
{
    output_bytes.clear();
    localSymbolTable.clear();
    rule_processed.clear();
    PC = org;
    current_pos = 0;
    sourcePos = { "", 0 };
}

std::shared_ptr<ASTNode> Parser::parse_rule(int64_t rule_type)
{
    if (rule_type == MacroDef) {
        inMacroDefinition = true;
    }
 
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
            if (rule_type == MacroDef) {
                inMacroDefinition = false;
            }

            return result;
        }

        // Reset if no match
        current_pos = start_pos;
    }

    if (rule_type == MacroDef) {
        inMacroDefinition = false;
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
