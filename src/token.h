// Token.h
#pragma once
#include <string>

enum TOKEN_TYPE {
    NUMBER, PLUS, MINUS, MUL, DIV,
    BIT_AND, BIT_OR, LPAREN, RPAREN, WS,
    EOL  // Add this line
};

struct Token {
    TOKEN_TYPE type;
    std::string value;
    size_t line;
    size_t line_pos;
};