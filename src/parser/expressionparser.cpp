#include "ExpressionParser.h"
#include <opcodedict.h>

void ExpressionParser::processNode(std::shared_ptr<ASTNode> node)
{
    auto pc = parser->org + parser->output_bytes.size();
    switch (node->type) {
        case Line:
            line = node->value;
            byteOutputLine = "";
            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    processNode(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
            byteOutput.push_back(byteOutputLine);
            break;

        case Prog:
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
        {
            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);
        }
        break;

        case Op_Immediate:
        {
            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            if (node->children.size() == 3) {
                auto& value = node->children[2];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    std::shared_ptr<ASTNode> value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    parser->output_bytes.push_back(value_token->value);
                    printbyte(value_token->value);
                }
            }
        }
        break;

        case Op_Relative:
        {
            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            if (node->children.size() == 2) {
                auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    std::shared_ptr<ASTNode> value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    int n = value_token->value - (pc + 2);
                    bool out_of_range = ((n + 127) & ~0xFF) != 0;
                    if (out_of_range) {
                        auto& left = std::get<std::shared_ptr<ASTNode>>(node->children[0]);
                        TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);
                        auto it = opcodeDict.find(opcode);
                        auto p = *parser;
                        if (it == opcodeDict.end()) {
                            throwError("Unknown opcode", p);
                        }
                        const OpCodeInfo& info = it->second;
                        throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(n) + ")", p);
                    }
                    uint8_t b = static_cast<uint8_t>(n & 0xFF);
                    parser->output_bytes.push_back(b);
                    printbyte(b);
                }
            }
        }
        break;

        case Op_Absolute:
        case Op_AbsoluteX:
        case Op_AbsoluteY:
        case Op_Indirect:
        case Op_IndirectX:
        case Op_IndirectY:
        {
            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            if (node->children.size() == 2) {
                auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    auto& value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    uint16_t value = value_token->value;
                    auto lo = (value & 0x00FF) >> 0;
                    auto hi = (value & 0xFF00) >> 8;
                    parser->output_bytes.push_back(lo);
                    parser->output_bytes.push_back(hi);

                    printbyte(lo);
                    printbyte(hi);
                }
            }
        }
        break;

        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
        {
            printPC(pc);

            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            if (node->children.size() == 2) {
                auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    std::shared_ptr<ASTNode> value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    parser->output_bytes.push_back(value_token->value);
                    printbyte(value_token->value);
                }
            }
        }
        break;

        case Op_ZeroPageRelative:
        {
            printPC(pc);

            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            if (node->children.size() == 4) {
                auto& value1 = node->children[1];
                auto& value2 = node->children[3];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value1) &&
                    std::holds_alternative<std::shared_ptr<ASTNode>>(value2)) {

                    std::shared_ptr<ASTNode> value_token1 = std::get<std::shared_ptr<ASTNode>>(value1);
                    std::shared_ptr<ASTNode> value_token2 = std::get<std::shared_ptr<ASTNode>>(value2);
                    parser->output_bytes.push_back(value_token1->value);
                    parser->output_bytes.push_back(value_token2->value);

                    printbyte(value_token1->value);
                    printbyte(value_token2->value);
                }
            }
        }
        break;

        default:
            break;
    }
}

void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    byteOutput.clear();
    parser->output_bytes.clear();
    processNode(ast);

    for (auto l = 0; l < byteOutput.size(); ++l) {
        std::cout 
            << parser->paddRight(byteOutput[l],20) << parser->paddLeft("", 5) 
            << lines[l] << "\n";
    }
}

ExpressionParser::ExpressionParser(std::vector<std::string>& lines) : lines(lines)
{
    byteOutput.clear();
    parser = std::make_shared<Parser>(Parser(parserDict, lines));
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
