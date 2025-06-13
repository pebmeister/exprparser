// Tokenizer.h
#pragma once
#include <regex>
#include <utility>
#include <vector>

#include "token.h"

class Tokenizer {
public:
    std::vector<std::pair<TOKEN_TYPE, std::regex>> token_patterns;

public:
    Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns);
    void add_token_pattern(TOKEN_TYPE type, const std::string& pattern);
    std::vector<Token> tokenize(const std::string& input);
};