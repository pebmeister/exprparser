// Token.h
#pragma once
#include <string>

enum TOKEN_TYPE {
    ORA,        AND,        EOR,        ADC,        SBC,

    NUMBER,     HEXNUM,     BINNUM,     PLUS,       MINUS, 
    MUL,        DIV,        BIT_AND,    BIT_OR,     LPAREN, 
    RPAREN,     WS,         SLEFT,      SRIGHT,     COMMA,
    POUND,      X,          Y,
    EOL
};

struct Token {
    TOKEN_TYPE type;
    std::string value;
    size_t line;
    size_t line_pos;
};
