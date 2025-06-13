#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "ASTNode.h"
#include "expr_rules.h"
#include "grammar_rule.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"


class ExpressionParser {
private:
    std::shared_ptr<Parser> parser;
    std::vector<std::string>& lines;

public:
    ExpressionParser(std::vector<std::string>& lines);
    std::shared_ptr<ASTNode> parse(const std::string& input);
};

