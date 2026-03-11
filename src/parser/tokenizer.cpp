// written by Paul Baxter
// Tokenizer.cpp
#include <cctype>
#include <iostream>
#include <chrono>

#include "tokenizer.h"
#include "expr_rules.h"

/// <summary>
/// Constructs a Tokenizer and initializes it with a list of token type and pattern pairs.
/// </summary>
/// <param name="patterns">An initializer list of pairs, each containing a token type and its corresponding pattern string.</param>
Tokenizer::Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns)
{
    for (const auto& [type, pattern] : patterns) {
        token_patterns.push_back(std::make_pair(type, RegexType(pattern, RegexType::icase)));
    }
}

/// <summary>
/// Tokenizes a sequence of source lines into a vector of tokens.
/// </summary>
/// <param name="input">A vector of pairs, each containing a source position and the corresponding line of source code as a string.</param>
/// <returns>A vector containing all tokens extracted from the input lines.</returns>
std::vector<Token> Tokenizer::tokenize(const std::vector<std::pair<SourcePos, std::string>>& input)
{
#ifdef __SHOW_TOKINIZE_TIME__
    auto start_time = std::chrono::high_resolution_clock::now();
#endif

    std::vector<Token> tokens;
    for (const auto& [pos, str] : input) {
        tokenize(pos, str, tokens);
    }

    // ensure we always terminate the token stream with an EOL so the parser
    // will handle a final-line label even when the input file lacks a trailing newline.
    Token eolTok;
    eolTok.type = TOKEN_TYPE::EOL;
    eolTok.value.clear();
    // use the last source position if available so listings point to the correct line
    if (!input.empty()) {
        eolTok.pos = input.back().first;
    } else {
        eolTok.pos = SourcePos{}; // default
    }
    eolTok.line_pos = 0;
    eolTok.start = false;
    tokens.push_back(eolTok);
  
#ifdef __SHOW_TOKINIZE_TIME__
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    auto seconds = duration.count() / 1000000.0;

    std::cout << "tokenize " << input.size() << " lines took " << seconds << " seconds using ";
#ifdef __USE_STD_REGEX__
    std::cout << "std::regex\n";
#endif
#ifdef __USE_BOOST_REGEX__
    std::cout << "boost::regex\n";
#endif
#endif

    return tokens;
}

/// <summary>
/// Tokenizes the input string into a sequence of tokens based on predefined patterns.
/// </summary>
/// <param name="sourcepos">The source position information, including filename and line number, used for error reporting and token metadata.</param>
/// <param name="input">The input string to be tokenized.</param>
/// <returns>A vector of Token objects representing the tokens extracted from the input string.</returns>
void Tokenizer::tokenize(const SourcePos& sourcepos, const std::string& input, std::vector<Token>& tokens)
{
#ifdef __USE_STD_REGEX__
    using namespace std;
#endif
#ifdef __USE_BOOST_REGEX__
    using namespace boost;
#endif
    size_t pos = 0, line_pos = 1;
    const auto fullline = input + "\n";
    bool start = true;

    smatch bestMatch;

    while (pos < fullline.size()) {
        TOKEN_TYPE bestType;
        size_t bestLength = 0;

        std::string remaining = fullline.substr(pos);
        for (const auto& [type, regex] : token_patterns) {
            smatch match;
            if (regex_search(remaining, match, regex, regex_constants::match_continuous)) {
                if (match.length() > bestLength) {
                    bestMatch = match;
                    bestType = type;
                    bestLength = match.length();
                    if (bestLength == remaining.length())
                        break;
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
}
