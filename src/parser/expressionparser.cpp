#include "ExpressionParser.h"
#include <opcodedict.h>
#include <cassert>

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

        case MacroDef:
            byteOutputLine = "";
            inMacrodefinition = true;

        case Prog:
        case LineList:
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
            if (inMacrodefinition)
                break;

            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);
        }
        break;

        case Op_Relative:
        {
            if (inMacrodefinition)
                break;

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
                            p.throwError("Unknown opcode");
                        }
                        const OpCodeInfo& info = it->second;
                        p.throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(n) + ")");
                    }
                    uint8_t b = static_cast<uint8_t>(n & 0xFF);
                    parser->output_bytes.push_back(b);
                    printbyte(b);
                }
            }
        }
        break;

        case Op_Immediate:
        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
            if (inMacrodefinition)
                break;
            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    std::shared_ptr<ASTNode> valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr) {
                        uint16_t value = valuenode->value;
                        parser->output_bytes.push_back(value);
                        printbyte(value);
                        break;
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
            if (inMacrodefinition)
                break;

            printPC(pc);
            parser->output_bytes.push_back(node->value);
            printbyte(node->value);

            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    std::shared_ptr<ASTNode> valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr) {
                        uint16_t value = valuenode->value;
                        auto lo = (value & 0x00FF) >> 0;
                        auto hi = (value & 0xFF00) >> 8;
                        parser->output_bytes.push_back(lo);
                        parser->output_bytes.push_back(hi);

                        printword(value);
                        break;
                    }
                }
            }
            break;
        }
        break;

        case Op_ZeroPageRelative:
        {
            if (inMacrodefinition)
                break;
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
    }
    if (node->type == MacroDef)
        inMacrodefinition = false;
}


void ExpressionParser::generate_asembly(std::shared_ptr<ASTNode> node)
{
    std::stringstream ss;
    std::string color = parser->es.gr(parser->es.WHITE_FOREGROUND);
    std::string temp;

    switch (node->type) {

        case Line:
            asmOutputLine.clear();
            asmOutputLine_Pos = 0;
            break;

        case MacroDef:
            inMacrodefinition = true;
            break;

        case Equate:
        case Comment:
        case Directive:
        case Label:
            return;

        case Expr:
        {
            color = parser->es.gr({ parser->es.BOLD, parser->es.GREEN_FOREGROUND });
            size_t sz = 2;
            if ((int)node->value & 0xFF00) {
                sz = 4;
            }

            ss
                << "$"
                << std::hex << std::uppercase << std::setw(sz) << std::setfill('0')
                << (int)node->value;
            ss >> temp;
            if (asmOutputLine[asmOutputLine.size() - 1] == '#') {
                asmOutputLine_Pos += sz + 1;
                asmOutputLine += color + temp;
            }
            else {
                asmOutputLine_Pos += sz + 2;
                asmOutputLine += color + " " + temp;
            }
            if (node->type == MacroDef)
                inMacrodefinition = false;
        }
        return;

        case Op_Instruction:
            color = parser->es.gr({ parser->es.BOLD, parser->es.YELLOW_FOREGROUND });
            break;

        case OpCode:
            color = parser->es.gr({ parser->es.BOLD, parser->es.YELLOW_FOREGROUND });
            break;
    }

    for (const auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            generate_asembly(childnode);
        }
        else {
            const Token& tok = std::get<Token>(child);
            if (tok.type == EOL) {
                if (inMacrodefinition) {
                    asmOutputLine = "";
                    asmOutputLine_Pos = 0;
                }

                while (asmOutputLine_Pos < 30) {
                    ++asmOutputLine_Pos;
                    asmOutputLine += ' ';
                }
                asmlines.push_back(asmOutputLine);
                continue;
            };
            while (asmOutputLine_Pos < tok.line_pos) {
                asmOutputLine += ' ';
                ++asmOutputLine_Pos;
            }
            if (!inMacrodefinition) {
                auto tokenLen = tok.value.size();
                asmOutputLine_Pos += tokenLen;
                asmOutputLine += color;
                asmOutputLine += tok.value;
            }
        }
    }
    if (node->type == MacroDef)
        inMacrodefinition = false;
}

void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    auto& esc = Parser::es;
    byteOutput.clear();
    parser->output_bytes.clear();

    inMacrodefinition = false;
    processNode(ast);
    inMacrodefinition = false;
    generate_asembly(ast);

    auto szl =  lines.size();
    for (auto l = 0; l < szl; ++l) {

        auto str = parser->paddRight(byteOutput[l], 20);
        auto& asmstr = asmlines[l];

        std::cout
            << esc.gr({ esc.BOLD, esc.CYAN_FOREGROUND })
            << str.substr(0, 6)
            << esc.gr(esc.GREEN_FOREGROUND)
            << str.substr(6)
            << esc.gr(esc.RESET_ALL)
            << asmstr
            << esc.gr(esc.RESET_ALL)
            << lines[l]
            << "\n";
    }
}

ExpressionParser::ExpressionParser(std::vector<std::string>& lines) : lines(lines)
{
    byteOutput.clear();
    asmOutputLine.clear();
    asmlines.clear();
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
