#include "ExpressionParser.h"

ExpressionParser::ExpressionParser()
{
    parser = std::make_shared<Parser>(Parser(rules, parserDict));
    ASTNode::astMap = parserDict;
}

std::shared_ptr<ASTNode> ExpressionParser::parse(const std::string& input)
{
    parser->current_pos = 0;
    parser->tokens.clear();
    parser->tokens = tokenizer.tokenize(input);
    auto ast = parser->parse_rule(Prog);
    if (parser->current_pos < parser->tokens.size()) {
        const Token& tok = parser->tokens[parser->current_pos];
        throw std::runtime_error(
            "Unexpected token after complete parse: '" + tok.value +
            "' at line " + std::to_string(tok.line) +
            ", col " + std::to_string(tok.line_pos)
        );
    }

    return ast;
}
