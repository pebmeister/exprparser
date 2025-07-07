#include "re2/re2.h"
#include "re2/prefilter.h"
using RegexType = re2::RE2;



void Tokenizer::add_token_pattern(TOKEN_TYPE type, const std::string& pattern)
{

    RE2::Options opts;
    opts.set_case_sensitive(false);
    token_patterns.emplace_back(type, RE2("^" + pattern, opts));
}

std::vector<Token> Tokenizer::tokenize(const std::string& input)
{
    std::vector<Token> tokens;
    size_t pos = 0, line = 1, line_pos = 1;

    bool start = true;
    std::string_view view(input);

    while (!view.empty()) {
        std::string bestValue;
        TOKEN_TYPE bestType = TOKEN_TYPE::INVALID;
        size_t bestLength = 0;

        for (const auto& [type, regex] : token_patterns) {
            std::string match_value;

            if (RE2::PartialMatch(view, regex, &match_value)) {
                size_t len = match_value.size();
                if (len > bestLength) {
                    bestLength = len;
                    bestValue = match_value;
                    bestType = type;
                }
            }
        }

        if (bestLength == 0) {
            throw std::runtime_error("Unknown token at position " + std::to_string(pos));
        }

        if (bestType != WS) {
            tokens.push_back(Token{ bestType, bestValue, line, line_pos, start });
        }

        for (char c : bestValue) {
            if (c == '\n') {
                ++line;
                line_pos = 1;
                start = true;
            }
            else {
                ++line_pos;
                if (bestType != WS) start = false;
            }
        }

        pos += bestLength;
        view.remove_prefix(bestLength);
    }

    return tokens;
}

