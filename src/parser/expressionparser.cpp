#include "ExpressionParser.h"
#include <opcodedict.h>
#include <cassert>

void ExpressionParser::extractExpressionList(std::shared_ptr<ASTNode>& node, std::vector<uint16_t>& data, bool word)
{
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            if (childnode->type == Expr) {
                if (word) {
                    auto lo = ((childnode->value & 0x00FF) >> 0);
                    auto hi = ((childnode->value & 0xFF00) >> 8);
                    data.push_back(lo);
                    data.push_back(hi);
                }
                else {
                    data.push_back(childnode->value);
                }
            }
            else {
                extractExpressionList(childnode, data, word);
            }
        }
    }
}

//  abstract syntax tree processing
void ExpressionParser::processNode(std::shared_ptr<ASTNode> node)
{
    auto pc = parser->org + parser->output_bytes.size();
    line = node->line;
    switch (node->type) {
        case Line:
            for (auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    processNode(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
            byteOutput.push_back({ line,  byteOutputLine });
            byteOutputLine = "";
            asmOutputLine = "";
            break;

        case ByteDirective:
        case WordDirective:
        {
            std::vector<uint16_t> bytes;
            std::shared_ptr<ASTNode> bytelistNode = std::get<std::shared_ptr<ASTNode>>(node->children[1]);
            extractExpressionList(bytelistNode, bytes, node->type == WordDirective);
            
            auto count = bytes.size();

            int col = 0;
            bool extra = false;
            for (auto& b : bytes) {
                if (col == 0) {
                    pc = parser->org + parser->output_bytes.size();
                    printPC(pc);
                }
                printbyte(b);
                outputbyte(b);
                ++col;
                extra = true;

                if (col == 3) {
                    line = node->line;
                    byteOutput.push_back({ line,  byteOutputLine });
                    byteOutputLine = "";
                    asmOutputLine = "";
                    col = 0;
                    extra = false;
                }
            }
            if (extra) {
                byteOutput.push_back({ line,  byteOutputLine });
                byteOutputLine = "";
                asmOutputLine = "";
            }
        }
        return;

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


void ExpressionParser::generate_assembly(std::shared_ptr<ASTNode> node)
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
                    generate_assembly(childnode);
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

            asmlines.push_back({ node->line, asmOutputLine });

            asmOutputLine = "";
            asmOutputLine_Pos = 0;
            return;

        case WordDirective:
        case ByteDirective:
        {
            std::vector<uint16_t> bytes;
            auto bytelistNode = std::get<std::shared_ptr<ASTNode>>(node->children[1]);
            extractExpressionList(bytelistNode, bytes);

            size_t i = 0;
            size_t remaining = bytes.size();

            const std::string colorKeyword = parser->es.gr({ parser->es.BOLD, parser->es.CYAN_FOREGROUND });
            const std::string colorByte = parser->es.gr({ parser->es.BOLD, parser->es.YELLOW_FOREGROUND });

            auto makeIndentedLine = [&]() -> std::string
                {
                    return std::string(instruction_indent, ' ');
                };

            while (remaining > 0) {
                asmOutputLine = makeIndentedLine();
                asmOutputLine_Pos = instruction_indent;

                // Start .byte directive with coloring
                ss.clear();
                ss << colorKeyword << (node->type == ByteDirective ? ".byte" : ".word") << colorByte;
                ss >> temp;
                asmOutputLine += temp;
                asmOutputLine_Pos += 5;

                // Determine how many bytes to output on this line
                size_t chunkSize = std::min<size_t>(3, remaining);

                for (size_t b = 0; b < chunkSize; ++b) {
                    ss.clear();
                    auto width = 2;
                    if (node->type == WordDirective) {
                        width = 4;
                    }
                    ss << "$"
                        << std::hex << std::uppercase << std::setw(width) << std::setfill('0')
                        << static_cast<int>(bytes[i + b]);
                    ss >> temp;
                    temp = (b > 0 ? "," : "") + std::string(" ") + temp;
                    asmOutputLine += temp;
                    asmOutputLine_Pos += temp.length();
                }
                ss.clear();
                // Pad to asm line width for alignment
                if (asmOutputLine_Pos < asmLineWidth) {
                    asmOutputLine += std::string(asmLineWidth - asmOutputLine_Pos, ' ');
                    asmOutputLine_Pos = asmLineWidth;
                }

                asmlines.push_back({ node->line, asmOutputLine });

                asmOutputLine.clear();
                asmOutputLine_Pos = 0;
                i += chunkSize;
                remaining -= chunkSize;
            }
            return;
        }

        case MacroDef:
            inMacrodefinition = true;
            break;

        case Equate:
        case Comment:
        case OrgDirective:
        case Label:
            return;

        case Expr:
        {
            if (!inMacrodefinition) {
                color = parser->es.gr({ parser->es.BOLD, parser->es.YELLOW_FOREGROUND });
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
            color = parser->es.gr({ parser->es.BOLD, parser->es.BLUE_FOREGROUND });
            break;

        case OpCode:
            color = parser->es.gr({ parser->es.BOLD, parser->es.BLUE_FOREGROUND });
            break;
    }

    for (const auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            generate_assembly(childnode);
        }
        else {
            const Token& tok = std::get<Token>(child);
            if (tok.type == EOL) {
                continue;
            };
            while (asmOutputLine_Pos < instruction_indent) {
                asmOutputLine += ' ';
                ++asmOutputLine_Pos;
            }

            if (!asmOutputLine.ends_with(' ')) {
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
    {
        inMacrodefinition = false;
    }
}

void ExpressionParser::print_lines()
{
    const size_t szl = lines.size();
    auto l = 1;
    while (l <= szl) {
        auto& line = lines[l -1];
        std::cout << std::setw(3) << l << ")" << line << "\n";
        l++;
    }
}

void ExpressionParser::print_asm()
{
    const size_t szl = lines.size();
    const size_t sza = asmlines.size(); // Optional: used only for bounds checking

    size_t l = 1;
    size_t out = 0;

    while (l <= szl && out < asmlines.size()) {
        auto& a = asmlines[out];

        std::cout << std::setw(3) << l << ") " << a.first << ", " << a.second << "\n";
        while (++out < asmlines.size() && asmlines[out].first == l) {
            a = asmlines[out];
            std::cout << std::setw(3) << l << ") " << a.first << ", " << a.second << "\n";
        }
        ++l;
    }
}

void ExpressionParser::print_outbytes()
{
    const size_t szl = lines.size();
    const size_t szb = byteOutput.size(); // Optional: used only for bounds checking

    size_t l = 1;
    size_t out = 0;

    while (l <= szl && out < byteOutput.size()) {
        auto& b = byteOutput[out];

        std::cout << std::setw(3) << l << ") " << b.first << ", " << b.second << "\n";
        while (++out < byteOutput.size() && byteOutput[out].first == l) {
            b = byteOutput[out];
            std::cout << std::setw(3) << l << ") " << b.first << ", " << b.second << "\n";
        }
        ++l;
    }
}

void ExpressionParser::generate_listing()
{
    const size_t max_lines = lines.size();
    const size_t max_bytes = byteOutput.size();
    const size_t max_asm = asmlines.size();
    size_t byte_idx = 0;
    size_t asm_idx = 0;

    auto& esc = parser->es;

    for (size_t line_num = 1; line_num <= max_lines; line_num++) {

        bool first = true; // first line of output for source line
        while (byte_idx < max_bytes && byteOutput[byte_idx].first == line_num) {
            std::cout << std::setw(3) <<
                esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND }) <<                
                line_num << ") ";

            auto& b = byteOutput[byte_idx++];
            auto byteout = parser->paddRight(b.second, byteOutputWidth);

            std::cout <<
                esc.gr({ esc.BOLD, esc.GREEN_FOREGROUND }) <<
                byteout.substr(0, 6) <<
                esc.gr({ esc.BOLD, esc.YELLOW_FOREGROUND }) <<
                byteout.substr(6);

            if (asmlines[asm_idx].first == line_num) {
                auto& a = asmlines[asm_idx++];
                std::cout << a.second;
            }
            else {
                std::cout << parser->paddLeft("", asmLineWidth);
            }
            if (first) {
                std::cout <<
                    esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND }) <<
                    lines[line_num - 1];
                first = false;
            }
            std::cout << "\n";
        }
    }
}

void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    auto& esc = Parser::es;
    byteOutput.clear();
    parser->output_bytes.clear();

    inMacrodefinition = false;
    processNode(ast);

    inMacrodefinition = false;
    generate_assembly(ast);

    generate_listing();
}

ExpressionParser::ExpressionParser(std::vector<std::string>& lines) : lines(lines)
{
    byteOutput.clear();
    asmOutputLine.clear();
    asmlines.clear();
    parser = std::make_shared<Parser>(Parser(parserDict, lines));
    ASTNode::astMap = parserDict;
}

std::shared_ptr<ASTNode> ExpressionParser::parse()
{
    std::string input;
    for (auto& line : lines) {
        input += line + "\n";
    }

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
