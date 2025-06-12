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
        std::smatch bestMatch;
        TOKEN_TYPE bestType;
        size_t bestLength = 0;

        std::string remaining = input.substr(pos);

        for (const auto& [type, regex] : token_patterns) {
            std::smatch match;
            if (std::regex_search(remaining, match, regex, std::regex_constants::match_continuous)) {
                if (match.length() > bestLength) {
                    bestMatch = match;
                    bestType = type;
                    bestLength = match.length();
                }
            }
        }

        if (bestLength == 0) {
            throw std::runtime_error("Unknown token at position " + std::to_string(pos));
        }

        std::string value = bestMatch.str();

        if (bestType != WS) {
            tokens.push_back(Token{ bestType, value, line, line_pos });
        }

        for (char c : value) {
            if (c == '\n') {
                ++line;
                line_pos = 1;
            }
            else {
                ++line_pos;
            }
        }

        pos += bestLength;
    }

    return tokens;
}
