// parser.h
#pragma once
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <stdexcept>
#include "Token.h"
#include "common_types.h"
#include "grammar_rule.h"
#include "ASTNode.h"

class Parser {
public:
    std::vector<Token> tokens;
    size_t current_pos = 0;
    std::vector<std::shared_ptr<GrammarRule>> rules;
    std::map<int64_t, std::string> parserDict;

    Parser(
        const std::vector<std::shared_ptr<GrammarRule>>& rules,
        const std::map<int64_t, std::string>& parserDict)
        : rules(rules), parserDict(parserDict)
    {
        tokens.clear();
    }

    Parser(const std::vector<Token>& tokens,
        const std::vector<std::shared_ptr<GrammarRule>>& rules,
        const std::map<int64_t, std::string>& parserDict)
        : tokens(tokens), rules(rules), parserDict(parserDict)
    {
    }

    std::shared_ptr<ASTNode> parse_rule(int64_t rule_type);

    std::string get_token_error_info() const
    {
        if (current_pos >= tokens.size()) return "at end of input";
        const Token& tok = tokens[current_pos];
        return "at token type " +  parserDict.at(tok.type) +
            " ('" + tok.value + "') [line " +
            std::to_string(tok.line) + ", col " +
            std::to_string(tok.line_pos) + "]";
    }

    template<typename RuleFunc, typename CalcFunc>
    std::shared_ptr<ASTNode> handle_binary_operation(
        std::shared_ptr<ASTNode> left,
        const std::set<TOKEN_TYPE>& allowed_ops,
        int64_t rule_type,
        RuleFunc parse_right,
        CalcFunc calculate,
        const std::string& expected_name)
    {
        while (current_pos < tokens.size() &&
            allowed_ops.count(tokens[current_pos].type)) {
            Token op = tokens[current_pos++];
            auto right = parse_right();
            if (!right) {
                throw std::runtime_error(
                    "Syntax error: expected " + expected_name +
                    " after operator '" + op.value + "' " +
                    get_token_error_info()
                );
            }

            auto node = std::make_shared<ASTNode>(rule_type);
            node->add_child(left);
            node->add_child(op);
            node->add_child(right);
            node->value = calculate(left->value, op.type, right->value);
            left = node;
        }
        return left;
    }
};