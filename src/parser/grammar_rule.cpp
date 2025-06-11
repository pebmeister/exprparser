// grammar_rule.cpp
#include "grammar_rule.h"

GrammarRule::GrammarRule(
    std::vector<std::vector<int64_t>> productions,
    std::function<std::shared_ptr<ASTNode>(Parser&, const std::vector<RuleArg>&)> action)
    : productions(std::move(productions)), action(std::move(action))
{
}
