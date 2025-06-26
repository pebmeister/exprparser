#include "ExpressionParser.h"
#include <opcodedict.h>
#include <cassert>

void ExpressionParser::processNode(std::shared_ptr<ASTNode> node)
{
    auto pc = parser->org + parser->output_bytes.size();
    switch (node->type) {
        case Line:
            line = node->value;
            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    processNode(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
            byteOutput.push_back(byteOutputLine);
            byteOutputLine = "";
            asmOutputLine = "";
            break;

        case MacroDef:
            byteOutputLine = "";
            asmOutputLine = "";
            inMacrodefinition = true;

        case Prog:
        case MacroCall:
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
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);
        }
        break;

        case Op_Relative:
        {
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

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
                    printbyte(b);
                    outputbyte(b);
                }
            }
        }
        break;

        case Op_Immediate:
        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    std::shared_ptr<ASTNode> valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr) {
                        uint16_t value = valuenode->value;
                        printbyte(value);
                        outputbyte(value);
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
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    std::shared_ptr<ASTNode> valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr) {
                        uint16_t value = valuenode->value;
                        printword(value);
                        auto lo = (value & 0x00FF) >> 0;
                        auto hi = (value & 0xFF00) >> 8;
                        outputbyte(lo);
                        outputbyte(hi);
                        break;
                    }
                }
            }
            break;
        }
        break;

        case Op_ZeroPageRelative:
        {
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            if (node->children.size() == 4) {
                auto& value1 = node->children[1];
                auto& value2 = node->children[3];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value1) &&
                    std::holds_alternative<std::shared_ptr<ASTNode>>(value2)) {

                    std::shared_ptr<ASTNode> value_token1 = std::get<std::shared_ptr<ASTNode>>(value1);
                    std::shared_ptr<ASTNode> value_token2 = std::get<std::shared_ptr<ASTNode>>(value2);
                    printbyte(value_token1->value);
                    printbyte(value_token2->value);

                    outputbyte(value_token1->value);
                    outputbyte(value_token2->value);
                }
            }
        }
        break;
    }
    if (node->type == MacroDef)
        inMacrodefinition = false;
}

const int asmLineWidth = 30;

void ExpressionParser::generate_asembly(std::shared_ptr<ASTNode> node)
{
    std::stringstream ss;
    std::string color = parser->es.gr(parser->es.WHITE_FOREGROUND);
    std::string temp;

    switch (node->type) {

        case Prog:
            asmOutputLine.clear();
            asmOutputLine_Pos = 0;
            asmlines.clear();
            break;

        case Line:
            // iterate all children of the line
            for (const auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
                    generate_asembly(childnode);
                }
            }

            // If we are in a macro definition 
            // erase the line
            if (inMacrodefinition) {
                asmOutputLine = "";
                asmOutputLine_Pos = 0;
            }

            // padd to the width of asm for allignment
            while (asmOutputLine_Pos < asmLineWidth) {
                ++asmOutputLine_Pos;
                asmOutputLine += ' ';
            }
            asmlines.push_back(asmOutputLine);

            asmOutputLine = "";
            asmOutputLine_Pos = 0;
            return;

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
            if (!inMacrodefinition) {
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
            }
        }
        break;

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

//  abstract syntax tree

void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    auto& esc = Parser::es;
    byteOutput.clear();
    parser->output_bytes.clear();

    inMacrodefinition = false;
    processNode(ast);

    inMacrodefinition = false;
    generate_asembly(ast);

    auto szl = lines.size();
    auto szb = byteOutput.size();
    auto sza = asmlines.size();

    size_t l = 0;
    size_t out = 0;

    auto& map = parser->codeInjectionMap;
    while (l < szl) {
        std::string str;
        std::string asmstr;

        str = parser->paddRight(byteOutput[out], 20);
        asmstr = asmlines[out];

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

        if (map.contains(l)) {
            ++out;
            auto outsz = map[l];
            while (outsz > 0) {
                str = parser->paddRight(byteOutput[out], 20);
                asmstr = asmlines[out];

                std::cout
                    << esc.gr({ esc.BOLD, esc.CYAN_FOREGROUND })
                    << str.substr(0, 6)
                    << esc.gr(esc.GREEN_FOREGROUND)
                    << str.substr(6)
                    << esc.gr(esc.RESET_ALL)
                    << asmstr
                    << "\n";
                ++out;
                --outsz;
            }
            --out;
        }
        ++l;
        ++out;
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
