// grammar_rule.h
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cinttypes>

#include "parser.h"
#include "common_types.h"
#include "expr_rules.h"

// Forward declaration
class Parser;

struct RuleHandler {
    std::vector<std::vector<int64_t>> productions;
    std::function<std::shared_ptr<ASTNode>(Parser&, const std::vector<RuleArg>&, int)> action;
};

extern const std::unordered_map<int64_t, RuleHandler> grammar_rules;

