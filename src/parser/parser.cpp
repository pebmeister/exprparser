// parser.cpp
#include "parser.h"
#include "grammar_rule.h"
#include "token.h"
#include <iomanip>

ANSI_ESC Parser::es;

static std::map<std::pair<size_t, int64_t>, int> rule_processed;

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
                    err += std::to_string(line) + " ";
                }
                err += "\n";
            }
            Parser p = *this;
            throwError(err, p);
        }
        needPass = unresolved.size() > 0;
    } while (pass < 10 && needPass);

    if (!unresolved.empty()) {
        std::string err = "Unresolved local symbols:";
        for (auto& sym : unresolved) {
            err += " " + sym.name;
        }
        throw std::runtime_error(err + " " + get_token_error_info());
    }
    for (auto& symEntry : symbolTable) {
        auto& sym = symEntry.second;
        std::cout << paddLeft(sym.name, 10) 
            << " $"
            << std::hex << std::setw(4) << std::setfill('0')
            <<  sym.value
            << std::dec << std::setfill(' ') << std::setw(0);
    }
    std::cout << "\n";
    ast = Pass();
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
                args.push_back(tokens[current_pos++]);
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
