#pragma once
#include <iostream>
#include <set>
#include <map>
#include <vector>

/*
 expr_rules.h
 -------------
 Grammar rule identifiers and declarations used by the expression parser.

 This header defines the RULE_TYPE enumeration which assigns unique numeric
 tags to every grammar/AST node kind the parser produces. These tags are
 used when creating ASTNode instances (ASTNode::type) so downstream code
 (assemblers, printers, analyzers) can identify node semantics without
 relying on RTTI.

 Conventions:
  - Values start at a high base (10000) to avoid collisions with token ids
    or other integer-valued identifiers used elsewhere.
  - Related rules are grouped by comment and share contiguous enum values
    for readability (e.g. expression operators, addressing modes, directives).
  - The enum values map to human-readable names via `ASTNode::astMap` or
    `parserDict` for diagnostics.
*/

enum RULE_TYPE {
    // Primary expression building blocks
    Factor = 10000,
    Number,
    MulExpr,
    AddExpr,
    AndExpr,
    OrExpr,
    LogicalAndExpr,
    LogicalOrExpr,
    XOrExpr,
    ShiftExpr,
    AddrExpr,
    TextExpr,
    RelExpr,
    EqExpr,
    Expr,

    // Convenience / run-length markers used by the assembler parser
    PlusRun,
    MinusRun,

    // Opcode / instruction related node kinds
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

    // Symbol and label related rules
    LabelDef,
    SymbolRef,
    SymbolName,
    AnonLabelRef,

    // Directives and assembly-level constructs
    Equate,
    PCAssign,
    Line,
    Statement,
    Comment,
    EOLOrComment,
    OrgDirective,
    ByteDirective,
    WordDirective,
    StorageDirective,
    IncludeDirective,
    PrintDirective,
    IfDirective,
    FillDirective,
    VarDirective,

    // Variable declaration helpers
    VarItem,    // Single variable declaration
    VarList,    // Comma-separated list of VarItems

    // Control flow and macro constructs
    DoDirective,
    WhileDirective,
    MacroDef,
    MacroStart,
    MacroCall,
    EndMacro,
    MacroArgs,

    // Collections and wrappers
    ExprList,
    LineList,
    TokenNode,
    Prog,
};

#include "ASTNode.h"
#include "grammar_rule.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"

/*
 External objects shared across parser modules:
  - tokenizer: the lexical tokenizer used to produce tokens for parsing.
  - parserDict: readable names for token/rule ids used in diagnostics.
 These are declared `extern` here so expression-rule implementations
 (expr_rules.cpp, grammar handlers) can reference them without cyclic
 includes or duplicated storage.
*/
extern Tokenizer tokenizer;
extern std::map<int64_t, std::string> parserDict;
