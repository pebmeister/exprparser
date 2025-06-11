#pragma once

#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "Token.h"
#include "Tokenizer.h"
#include "parser.h"
#include "grammar_rule.h"
#include "ASTNode.h"

#include "expr_rules.h"

class ExpressionParser {
private:
    std::shared_ptr<Parser> parser;

public:
    ExpressionParser();
    std::shared_ptr<ASTNode> parse(const std::string& input);
};

