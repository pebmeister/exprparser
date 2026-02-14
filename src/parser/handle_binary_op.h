#pragma once
#include <memory>
#include <set>

#include "grammar_rule.h"
#include "token.h"

/**
 * handle_binary_op
 * ----------------
 * Generic helper to parse a left-associative sequence of binary operators:
 *
 *   left ( op right )*
 *
 * It repeatedly checks whether the next token is one of the allowed
 * operator tokens and, if so, consumes the operator, parses the right
 * operand by calling `parse_right_operand`, builds a new AST node
 * of `rule_type`, computes the node->value using `calculate_value`,
 * and continues until no more allowed operators are present.
 *
 * Important: this template expects to be used inside the parser implementation
 * where the following names are available in scope (free variables):
 *  - `current_pos` (size_t) — current index in the `tokens` vector
 *  - `tokens` (std::vector<Token>) — token input stream
 *  - `get_token_info(size_t)` or similar diagnostic helper used for errors
 *
 * Template parameters:
 *  - RuleFunc
 *      Callable invoked to parse the right-hand operand. Expected signature:
 *          std::shared_ptr<ASTNode> parse_right_operand();
 *      The callable should advance `current_pos` as it consumes tokens and
 *      return a null/shared_ptr(nullptr) on parse failure.
 *
 *  - CalcFunc
 *      Callable used to compute the integer value for the created binary AST node.
 *      Expected signature (conceptually):
 *          int calculate_value(int leftValue, TOKEN_TYPE opType, int rightValue);
 *      The return value is stored in `node->value`. `opType` is provided so the
 *      calculator can distinguish operator variants (e.g. '+' vs '-').
 *
 * Function parameters:
 *  - left
 *      The already-parsed left operand (ownership via shared_ptr). The function
 *      will create a new AST node combining `left`, operator token and `right`
 *      each time it matches an operator, and then set `left` to that new node
 *      for the next iteration.
 *
 *  - allowed_ops
 *      Set of `TOKEN_TYPE` values considered valid binary operators for this
 *      production. Matching is performed against `tokens[current_pos].type`.
 *
 *  - rule_type
 *      Numeric tag (from your RULE_TYPE / AST conventions) used when creating
 *      the binary AST node: `auto node = std::make_shared<ASTNode>(rule_type);`
 *
 *  - parse_right_operand
 *      See RuleFunc above — callable that parses the right-hand side and
 *      returns its AST node. Must return nullptr if the right operand is missing
 *      or invalid.
 *
 *  - expected_operand_name
 *      Human-readable name used in the thrown error message when the right
 *      operand cannot be parsed after an operator (e.g. "expression", "term").
 *
 *  - calculate_value
 *      See CalcFunc above — callable that computes the combined node value
 *      from left.value, operator token type and right.value.
 *
 * Return value:
 *  - The final left-hand AST node after parsing zero or more binary operations.
 *    If no operators matched, the original `left` is returned unchanged.
 *
 * Error behavior:
 *  - If an operator is matched but `parse_right_operand` returns null, the
 *    function throws a std::runtime_error containing a descriptive message and
 *    token context (via `get_token_info(current_pos)`).
 *
 * Example usage (sketch):
 *   auto parsed = handle_binary_op(left, {PLUS, MINUS}, ExprRule, parseTerm, "term",
 *                                  [](int L, TOKEN_TYPE op, int R) { return compute(L,op,R); });
 */
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
