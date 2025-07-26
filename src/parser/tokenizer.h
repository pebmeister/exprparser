// Tokenizer.h
#pragma once

#include <utility>
#include <vector>
#include <regex>
using RegexType = std::regex;

#include "common_types.h"
#include "token.h"

class Tokenizer {
public:
    std::vector<std::pair<TOKEN_TYPE, RegexType>> token_patterns;

    Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns);
    std::vector<Token> tokenize(const SourcePos& pos, const std::string& input);
    std::vector<Token> tokenize(const std::vector<std::pair<SourcePos, std::string>>  &input);

private:
    void add_token_pattern(TOKEN_TYPE type, const std::string& pattern);

};