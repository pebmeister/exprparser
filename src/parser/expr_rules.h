#pragma once
#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "ASTNode.h"
#include "grammar_rule.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"

enum RULE_TYPE {
    Factor = 10000,
    Number,
    MulExpr,
    AddExpr,
    AndExpr,
    OrExpr,
    XOrExpr,
    ShiftExpr,
    AddrExpr,
    TextExpr,
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
    Op_Accumulator,
    Op_Relative,
    Op_ZeroPageRelative,
    Symbol,
    Label,
    Equate,
    Line,
    Statement,
    Comment,
    OrgDirective,
    ByteDirective,
    MacroDef,
    MacroCall,
    MacroArgs,
    ExprList,
    LineList,
    Prog,
    TokenNode
};

extern Tokenizer tokenizer;
extern std::map<int64_t, std::string> parserDict;
