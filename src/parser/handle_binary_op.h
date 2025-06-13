#pragma once
#include <memory>
#include <set>

#include "grammar_rule.h"
#include "token.h"

template<typename RuleFunc, typename CalcFunc>
std::shared_ptr<ASTNode> handle_binary_op(
    std::shared_ptr<ASTNode> left,
    const std::set<TOKEN_TYPE>& allowed_ops,
    int64_t rule_type,
    RuleFunc parse_right_operand,
    const std::string& expected_operand_name,
    CalcFunc calculate_value)
{
    while (current_pos < tokens.size() &&
        allowed_ops.contains(tokens[current_pos].type)) {
        Token op = tokens[current_pos++];
        auto right = parse_right_operand();
        if (!right) {
            throw std::runtime_error(
                "Syntax error: expected " + expected_operand_name +
                " after operator '" + op.value + "' " +
                get_token_info(current_pos)
            );
        }

        auto node = std::make_shared<ASTNode>(rule_type);
        node->add_child(left);
        node->add_child(op);
        node->add_child(right);
        node->value = calculate_value(left->value, op.type, right->value);
        left = node;
    }
    return left;
}
