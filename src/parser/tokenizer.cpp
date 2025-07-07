
// Tokenizer.cpp
#include <cctype>
#include <iostream>

#include "tokenizer.h"
#include "expr_rules.h"

Tokenizer::Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns)
{
    for (const auto& [type, pattern] : patterns) {
        add_token_pattern(type, pattern);
    }
}

void Tokenizer::add_token_pattern(TOKEN_TYPE type, const std::string& pattern)
{
    token_patterns.push_back(
        std::make_pair(type, RegexType(pattern, std::regex::icase))
    );
}

std::vector<Token> Tokenizer::tokenize(const std::vector<std::pair<SourcePos, std::string>>& input)
{
    std::vector<Token> tokens;

    for (auto& line : input) {
        const SourcePos& pos = line.first;
        const std::string& str = line.second;
        auto linetoks = tokenize(pos, str);

        std::copy(linetoks.begin(), linetoks.end(), std::back_inserter(tokens));
    }

    return tokens;
}

std::vector<Token> Tokenizer::tokenize(const SourcePos& sourcepos, const std::string& input)
{
    std::vector<Token> tokens;
    size_t pos = 0, line_pos = 1;
    auto fullline = input + "\n";
    bool start = true;
    while (pos < fullline.size()) {
        std::smatch bestMatch;
        TOKEN_TYPE bestType;
        size_t bestLength = 0;

        std::string remaining = fullline.substr(pos);

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
            throw std::runtime_error("Unknown token at position " + sourcepos.filename + " " +  std::to_string(sourcepos.line));
        }

        std::string value = bestMatch.str();
        if (bestType != WS) {
            tokens.push_back(Token{ bestType, value, sourcepos, line_pos, start });
        }

        for (char c : value) {
            if (c == '\n') {
                line_pos = 1;
                start = true;
            }
            else {
                ++line_pos;
                if (bestType != WS) {
                    start = false;
                }
            }
        }

        pos += bestLength;
    }

    return tokens;
}
