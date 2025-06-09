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
    Number,
    MulExpr,
    AddExpr,
    AndExpr,
    OrExpr,
    SExpr,
    AddrExpr,
    Expr,
    OpCode,
    Op_Instruction,
    Op_Implied,
    Op_Immediate,
    Op_Absolute,
    Op_ZeroPage,
    Op_AbsoluteX,
    Op_ZeroPageX,
    Op_AbsoluteY,
    Op_ZeroPageY,
    Op_Indirect,
    Op_IndirectX,
    Op_IndirectY,
    Statement,
    Comment,
    Prog
};

extern Tokenizer tokenizer;
extern std::map<int64_t, std::string> parserDict;
extern const std::vector<std::shared_ptr<GrammarRule>> rules;
