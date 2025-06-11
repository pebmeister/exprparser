// Tokenizer.cpp
#include <cctype>
#include "Tokenizer.h"

Tokenizer::Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns)
{
    for (const auto& [type, pattern] : patterns) {
        add_token_pattern(type, pattern);
    }
}

void Tokenizer::add_token_pattern(TOKEN_TYPE type, const std::string& pattern)
{
    token_patterns.push_back(
        std::make_pair(type, std::regex(pattern, std::regex::optimize | std::regex::icase ))
    );
}

std::vector<Token> Tokenizer::tokenize(const std::string& input)
{
    std::vector<Token> tokens;
    size_t pos = 0, line = 1, line_pos = 1;
    while (pos < input.size()) {
        bool matched = false;
        for (const auto& [type, regex] : token_patterns) {
            std::smatch match;
            std::string remaining = input.substr(pos);
            if (std::regex_search(remaining, match, regex, std::regex_constants::match_continuous)) {
                std::string value = match.str();
                // Skip whitespace tokens
                if (type != WS) {
                    tokens.push_back(Token{ type, value, line, line_pos });
                }
                // Update line/col
                for (char c : value) {
                    if (c == '\n') {
                        ++line;
                        line_pos = 1;
                    }
                    else {
                        ++line_pos;
                    }
                }
                pos += value.size();
                matched = true;
                break;
            }
        }
        if (!matched) {
            throw std::runtime_error("Unknown token at position " + std::to_string(pos));
        }
    }
    return tokens;
}
