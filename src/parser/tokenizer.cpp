
// Tokenizer.cpp
#include <cctype>
#include <iostream>

#include "tokenizer.h"
#include "expr_rules.h"

/// <summary>
/// Constructs a Tokenizer and initializes it with a list of token type and pattern pairs.
/// </summary>
/// <param name="patterns">An initializer list of pairs, each containing a token type and its corresponding pattern string.</param>
Tokenizer::Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns)
{
    for (const auto& [type, pattern] : patterns) {
        add_token_pattern(type, pattern);
    }
}

/// <summary>
/// Adds a new token pattern to the tokenizer with the specified type and regular expression pattern.
/// </summary>
/// <param name="type">The type to associate with the token pattern.</param>
/// <param name="pattern">The regular expression pattern used to match tokens.</param>
void Tokenizer::add_token_pattern(TOKEN_TYPE type, const std::string& pattern)
{
    token_patterns.push_back(
        std::make_pair(type, RegexType(pattern, std::regex::icase))
    );
}

/// <summary>
/// Tokenizes a sequence of source lines into a vector of tokens.
/// </summary>
/// <param name="input">A vector of pairs, each containing a source position and the corresponding line of source code as a string.</param>
/// <returns>A vector containing all tokens extracted from the input lines.</returns>
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

/// <summary>
/// Tokenizes the input string into a sequence of tokens based on predefined patterns.
/// </summary>
/// <param name="sourcepos">The source position information, including filename and line number, used for error reporting and token metadata.</param>
/// <param name="input">The input string to be tokenized.</param>
/// <returns>A vector of Token objects representing the tokens extracted from the input string.</returns>
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
