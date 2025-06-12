// parser.cpp
#include "parser.h"
#include "grammar_rule.h"


std::shared_ptr<ASTNode> Parser::parse()
{
    return parse_rule(RULE_TYPE::Prog);
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
