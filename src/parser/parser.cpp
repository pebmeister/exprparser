// parser.cpp
#include "parser.h"
#include "grammar_rule.h"


ANSI_ESC Parser::es;

std::shared_ptr<ASTNode> Parser::parse()
{
    auto ast = parse_rule(RULE_TYPE::Prog);
    return ast;
}


std::shared_ptr<ASTNode> Parser::Assemble()
{
    std::shared_ptr<ASTNode> ast;

    auto pass = 0;
    std::vector<Sym> unresolved;
    do {
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

    } while (pass < 10 && unresolved.size() > 0);

    if (!unresolved.empty()) {
        std::string err = "Unresolved local symbols:";
        for (auto& sym : unresolved) {
            err += " " + sym.name;
        }
        throw std::runtime_error(err + " " + get_token_error_info());
    }
    for (auto& symEntry : symbolTable) {
        auto& sym = symEntry.second;
        std::cout << paddLeft(sym.name, 10) << paddLeft(std::to_string(sym.accessed[0]), 4);
    }
    std::cout << "\n";
    return ast;
}

std::shared_ptr<ASTNode> Parser::Pass()
{
    InitPass();
    return parse();
}

void Parser::InitPass()
{
    localSymbolTable.clear();
    PC = 0x1000;
    current_pos = 0;
}

std::shared_ptr<ASTNode> Parser::parse_rule(int64_t rule_type)
{
    for (const auto& rule : rules) {
        for (const auto& production : rule->productions) {
            if (production[0] == rule_type) {
                size_t start_pos = current_pos;
                std::vector<RuleArg> args;
                bool match = true;

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
                    return rule->action(*this, args);
                }
                current_pos = start_pos;
            }
        }
    }
    return nullptr;
}
