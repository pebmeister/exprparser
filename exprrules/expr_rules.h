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

enum RULE_TYPE {
    Factor = 10000,
    MulExpr,
    AddExpr,
    AndExpr,
    OrExpr,
    Expr,
};

extern Tokenizer tokenizer;
extern std::map<int64_t, std::string> parserDict;
extern const std::vector<std::shared_ptr<GrammarRule>> rules;

