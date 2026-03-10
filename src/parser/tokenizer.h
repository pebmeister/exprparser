// written by Paul Baxter
// Tokenizer.h
//
// Lightweight, configurable tokenizer used by the assembler/parser.
// The Tokenizer converts source text (lines or cached file segments)
// into a sequence of `Token` instances using a list of regular expression
// patterns associated with TOKEN_TYPE values.
//
// Design notes:
//  - Patterns are stored as (TOKEN_TYPE, std::regex) pairs in the order
//    they are added. Token matching proceeds in that order so pattern
//    ordering can affect lexical disambiguation.
//  - Two convenience `tokenize` overloads are provided:
//      * tokenize(const SourcePos&, const std::string&) - tokenizes a single input line.
//      * tokenize(const std::vector<std::pair<SourcePos,std::string>>&)
//        - tokenizes multiple lines (useful when retokenizing macro bodies).
//  - Regex compilation happens when patterns are added (constructor / add_token_pattern).
//    Keep patterns reasonably specific to avoid performance regressions.
//  - Tokenizer is lightweight and stateless between calls except for the configured
//    `token_patterns` vector; calls are safe as long as the patterns vector is not mutated
//    concurrently from multiple threads.

#pragma once

#define __USE_BOOST_REGEX__
// #define __USE_STD_REGEX__

#include <utility>
#include <vector>
#include <map>

// Define default regex library if not already specified
#if !defined(__USE_STD_REGEX__) && !defined(__USE_BOOST_REGEX__)
#define __USE_STD_REGEX__
#endif

#ifdef __USE_STD_REGEX__
#include <regex>
using RegexType = std::regex;
#endif
#ifdef __USE_BOOST_REGEX__
#include <boost/regex.hpp>
using RegexType = boost::regex;
#endif

#include "common_types.h"
#include "token.h"

class Tokenizer {
public:
    // Ordered list of token patterns: (token type, compiled regex).
    // Public so callers may inspect or (rarely) mutate the patterns if needed.
    std::vector<std::pair<TOKEN_TYPE, RegexType>> token_patterns;

    // Construct a tokenizer from an initializer list of (TOKEN_TYPE, pattern string).
    // Patterns are compiled to std::regex and stored in `token_patterns`.
    Tokenizer(std::initializer_list<std::pair<TOKEN_TYPE, std::string>> patterns);

    // Tokenize a single input line. `pos` provides the SourcePos used for each produced Token.
    // Returns a vector of Tokens in lexical order. Implementations should include an EOL token
    // when appropriate so callers can rely on EOL to determine logical line boundaries.
    std::vector<Token> tokenize(const SourcePos& sourcepos, const std::string& input);

    // Tokenize multiple (SourcePos, line) pairs. Useful for retokenizing bodies of macros
    // or cached file segments. The returned tokens preserve the source position for each line.
    std::vector<Token> tokenize(const std::vector<std::pair<SourcePos, std::string>>  &input);

private:
    // Helper that compiles and appends a regex pattern for a token type.
    // Keeps the compilation centralized so the constructor can be simple.
    void add_token_pattern(TOKEN_TYPE type, const std::string& pattern);
};