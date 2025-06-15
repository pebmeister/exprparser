#include "ExpressionParser.h"

void ExpressionParser::processNode(std::shared_ptr<ASTNode> node)
{
    switch (node->type) {
        case Prog:
        case Line:
        case Statement:
        case Op_Instruction:
            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    processNode(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
            break;

        case Op_Implied:
        case Op_Accumulator:
            parser->output_bytes.push_back(node->value);
            break;

        case Op_Immediate:
            parser->output_bytes.push_back(node->value);
            if (node->children.size() == 3) {
                auto& value = node->children[2];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    std::shared_ptr<ASTNode> value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    parser->output_bytes.push_back(value_token->value);
                }
            }
            break;

        case Op_Absolute:
        case Op_AbsoluteX:
        case Op_AbsoluteY:
        case Op_Indirect:
        case Op_IndirectX:
        case Op_IndirectY:
            parser->output_bytes.push_back(node->value);
            if (node->children.size() == 2) {
                auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    auto& value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    uint16_t value = value_token->value;
                    parser->output_bytes.push_back((value & 0x00FF) >> 0);
                    parser->output_bytes.push_back((value & 0xFF00) >> 8);
                }
            }
            break;

        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
            parser->output_bytes.push_back(node->value);
            if (node->children.size() == 2) {
                auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    std::shared_ptr<ASTNode> value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    parser->output_bytes.push_back(value_token->value);
                }
            }
            break;

        case Op_ZeroPageRelative:
            parser->output_bytes.push_back(node->value);
            if (node->children.size() == 4) {
                auto& value1 = node->children[1];
                auto& value2 = node->children[3];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value1) &&
                    std::holds_alternative<std::shared_ptr<ASTNode>>(value2)) {

                    std::shared_ptr<ASTNode> value_token1 = std::get<std::shared_ptr<ASTNode>>(value1);
                    std::shared_ptr<ASTNode> value_token2 = std::get<std::shared_ptr<ASTNode>>(value2);
                    parser->output_bytes.push_back(value_token1->value);
                    parser->output_bytes.push_back(value_token2->value);
                }
            }
            break;

        default:
            break;
    }
}

void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    parser->output_bytes.clear();
    processNode(ast);
}

ExpressionParser::ExpressionParser(std::vector<std::string>& lines) : lines(lines)
{
    parser = std::make_shared<Parser>(Parser(rules, parserDict, lines));
    ASTNode::astMap = parserDict;
}

std::shared_ptr<ASTNode> ExpressionParser::parse(const std::string& input)
{
    parser->current_pos = 0;
    parser->tokens.clear(); 
    parser->tokens = tokenizer.tokenize(input);
    auto ast = parser->Assemble();

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
