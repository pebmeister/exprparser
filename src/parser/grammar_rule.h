// grammar_rule.h
#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "common_types.h"

// Forward declaration
class Parser;

struct GrammarRule {
    std::vector<std::vector<int64_t>> productions;
    std::function<std::shared_ptr<ASTNode>(Parser&, const std::vector<RuleArg>&)> action;

    GrammarRule(std::vector<std::vector<int64_t>> productions,
        std::function<std::shared_ptr<ASTNode>(Parser&, const std::vector<RuleArg>&)> action);
};