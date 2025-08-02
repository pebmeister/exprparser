#include <cassert>
#include <opcodedict.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <filesystem>

namespace fs = std::filesystem;

#include "ExpressionParser.h"

#pragma warning( disable : 6031 )

/// <summary>
/// Extracts a list of expression values from an abstract syntax tree (AST) node and appends them to a data vector, optionally splitting values into bytes.
/// </summary>
/// <param name="node">A shared pointer reference to the AST node from which to extract expression values.</param>
/// <param name="data">A reference to a vector where the extracted values will be appended.</param>
/// <param name="word">If true, each expression value is split into two bytes (low and high) before being added to the data vector; if false, the value is added as a single 16-bit value.</param>
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


void ExpressionParser::buildOutput(std::shared_ptr<ASTNode> node)
{
    auto pc = parser->org + parser->output_bytes.size();

    if (node->type == Line || node->type == EndMacro || node->type == MacroStart)
        pos = node->position;

    auto processChildren = [&](const std::vector<RuleArg>& children)
        {
            for (const auto& child : children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    buildOutput(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
        };

    auto flushOutputLine = [&]()
        {
            byteOutput.push_back({ pos, byteOutputLine });
            byteOutputLine.clear();
            asmOutputLine.clear();
        };

    switch (node->type) {
        case Prog:
        case IncludeDirective:
        case MacroCall:
        case LineList:
        case Statement:
        case Op_Instruction:
        case Line:
            processChildren(node->children);
            if (node->type == Line) {
                flushOutputLine();
            }
            return;

        case ByteDirective:
        case WordDirective:
        {
            std::vector<uint16_t> bytes;
            auto bytelistNode = std::get<std::shared_ptr<ASTNode>>(node->children[1]);
            extractExpressionList(bytelistNode, bytes, node->type == WordDirective);

            int col = 0;
            bool extra = false;
            for (const auto& b : bytes) {
                if (col == 0) {
                    pc = parser->org + parser->output_bytes.size();
                    printPC(pc);
                }
                printbyte(b);
                outputbyte(b);
                ++col;
                extra = true;

                if (col == 3) {
                    pos = node->position;
                    flushOutputLine();
                    col = 0;
                    extra = false;
                }
            }
            if (extra) {
                flushOutputLine();
            }
            return;
        }

        case MacroDef:
            byteOutputLine.clear();
            asmOutputLine.clear();
            inMacrodefinition = true;
            processChildren(node->children);
            inMacrodefinition = false;
            return;

        case Op_Implied:
        case Op_Accumulator:
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);
            return;

        case Op_Relative:
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            if (node->children.size() == 2) {
                const auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    auto value_token = std::get<std::shared_ptr<ASTNode>>(value);
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
            return;

        case Op_Immediate:
        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
        case Op_IndirectX:
        case Op_IndirectY:
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            for (const auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    auto valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr || valuenode->type == AddrExpr) {
                        uint16_t value = valuenode->value;
                        printbyte(value);
                        outputbyte(value);
                        break;
                    }
                }
            }
            return;

        case Op_Absolute:
        case Op_AbsoluteX:
        case Op_AbsoluteY:
        case Op_Indirect:
            printPC(pc);
            printbyte(node->value);
            outputbyte(node->value);

            for (const auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    auto valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr || valuenode->type == AddrExpr) {
                        uint16_t value = valuenode->value;
                        printword(value);
                        auto lo = value & 0x00FF;
                        auto hi = (value & 0xFF00) >> 8;
                        outputbyte(lo);
                        outputbyte(hi);
                        break;
                    }
                }
            }
            return;

        case Op_ZeroPageRelative:
            printPC(pc);
            printbyte(node->value);
           
            outputbyte(node->value);

            if (node->children.size() == 4) {
                const auto& value1 = node->children[1];
                const auto& value2 = node->children[3];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value1) &&
                    std::holds_alternative<std::shared_ptr<ASTNode>>(value2)) {

                    auto& value_token1 = std::get<std::shared_ptr<ASTNode>>(value1);
                    auto& value_token2 = std::get<std::shared_ptr<ASTNode>>(value2);
                    printbyte(value_token1->value);
                    printbyte(value_token2->value);

                    outputbyte(value_token1->value);
                    outputbyte(value_token2->value);
                }
            }
            return;
    }
}

/// <summary>
/// Generates assembly code from an abstract syntax tree (AST) node and appends the output to the assembly lines buffer.
/// </summary>
/// <param name="node">A shared pointer to the ASTNode representing the current node in the abstract syntax tree to process.</param>
void ExpressionParser::generate_assembly(std::shared_ptr<ASTNode> node)
{
    std::stringstream ss;
    std::string color = parser->es.gr(parser->es.WHITE_FOREGROUND);
    std::string temp;
    std::shared_ptr<ASTNode> listnode;

    auto processChildren = [&](const std::vector<RuleArg>& children)
        {
            for (const auto& child : children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    generate_assembly(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
        };

    auto padAsmOutputLine = [&]()
        {
            if (asmOutputLine_Pos < asmLineWidth) {
                asmOutputLine += std::string(asmLineWidth - asmOutputLine_Pos, ' ');
                asmOutputLine_Pos = asmLineWidth;
            }
        };

    switch (node->type) {
        case Prog:
            asmOutputLine.clear();
            asmOutputLine_Pos = 0;
            asmlines.clear();
            break;

        case IncludeDirective:
            listnode = std::get<std::shared_ptr<ASTNode>>(node->children[0]);
            generate_assembly(listnode);
            return;

        case Line:
            processChildren(node->children);

            if (inMacrodefinition) {
                asmOutputLine.clear();
                asmOutputLine_Pos = 0;
            }

            padAsmOutputLine();
            asmlines.push_back({ node->position, asmOutputLine });

            asmOutputLine.clear();
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

                ss.clear();
                ss << colorKeyword << (node->type == ByteDirective ? ".byte" : ".word") << colorByte;
                ss >> temp;
                asmOutputLine += temp;
                asmOutputLine_Pos += 5;

                size_t chunkSize = std::min<size_t>(3, remaining);

                for (size_t b = 0; b < chunkSize; ++b) {
                    ss.clear();
                    auto width = (node->type == WordDirective) ? 4 : 2;
                    ss << "$"
                        << std::hex << std::uppercase << std::setw(width) << std::setfill('0')
                        << static_cast<int>(bytes[i + b]);
                    ss >> temp;
                    temp = (b > 0 ? "," : "") + std::string(" ") + temp;
                    asmOutputLine += temp;
                    asmOutputLine_Pos += temp.length();
                }
                ss.clear();

                padAsmOutputLine();
                asmlines.push_back({ node->position, asmOutputLine });

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
        case Symbol:
            return;

        case Expr:
        case AddrExpr:
            if (!inMacrodefinition) {
                color = parser->es.gr({ parser->es.BOLD, parser->es.YELLOW_FOREGROUND });
                size_t sz = ((int)node->value & 0xFF00) ? 4 : 2;
                ss << "$"
                    << std::hex << std::uppercase << std::setw(sz) << std::setfill('0')
                    << (int)node->value;
                ss >> temp;
                auto last = asmOutputLine.empty() ? ' ' : asmOutputLine.back();
                if (last == '#' || last == '(' || last == ',') {
                    asmOutputLine_Pos += sz + 1;
                    asmOutputLine += color + temp;
                }
                else {
                    asmOutputLine_Pos += sz + 2;
                    asmOutputLine += color + " " + temp;
                }
            }
            break;
        
        case Op_Instruction:
        case OpCode:
            color = parser->es.gr({ parser->es.BOLD, parser->es.BLUE_FOREGROUND });
            break;
    }

    for (const auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            generate_assembly(std::get<std::shared_ptr<ASTNode>>(child));
        }
        else {
            const Token& tok = std::get<Token>(child);
            if (tok.type == EOL) continue;
            while (asmOutputLine_Pos < instruction_indent) {
                asmOutputLine += ' ';
                ++asmOutputLine_Pos;
            }
            if (tok.type == POUND || tok.type == LPAREN || tok.type == A) {
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
    if (node->type == MacroDef) {
        inMacrodefinition = false;
    }
}

/// <summary>
/// Prints the assembly lines with formatting and file tracking information.
/// </summary>
void ExpressionParser::print_asm()
{
    for (auto& line : asmlines) {
        std::cout <<
            parser->es.gr({ parser->es.BOLD, parser->es.WHITE_FOREGROUND });

        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            std::cout
                << "Processsing " << currentfile << "\n";
        }
        std::cout << std::setw(3) << line.first.line << ")" << line.second << "\n";
    }
}

/// <summary>
/// Prints the contents of the list file, grouping lines by filename and displaying each line with its line number.
/// </summary>
void ExpressionParser::print_listfile()
{
    currentfile = "";
    for (auto& line : listLines) {
        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            std::cout << "Processsing " << currentfile << "\n";
        }
        std::cout << std::setw(3) << std::to_string(line.first.line) << ") " << line.second << "\n";
    }
}

/// <summary>
/// Prints the contents of the byteOutput container, grouping lines by filename and displaying each line's number and associated output.
/// </summary>
void ExpressionParser::print_outbytes()
{
    currentfile = "";
    for (auto& line : byteOutput) {
        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            std::cout << "Processsing " << currentfile << "\n";
        }
        std::cout << std::setw(3) << line.first.line << ") " << line.second << "\n";
    }
}

/// <summary>
/// Generates and outputs a formatted listing of the parsed source code, including bytecode, assembly, and original source lines, with colorized terminal output.
/// </summary>
void ExpressionParser::generate_listing()
{    
    currentfile = "";
    size_t max_byte_index = byteOutput.size();
    size_t max_asm_index = asmlines.size();

    auto& esc = parser->es;    

    size_t byte_index = 0;
    size_t asm_index = 0;
    size_t source_index = 0;

    SourcePos bytesPos = byteOutput[byte_index].first;
    SourcePos asmPos = asmlines[asm_index].first;

    for (auto& line: listLines) {

        // new file
        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            std::cout <<
                parser->es.gr({ parser->es.BOLD, parser->es.WHITE_FOREGROUND }) <<
                "\nProcessing " << currentfile << "\n\n";
        }
        
        bool first = true; // first line of output for source line

        // Bytes
        while (bytesPos.filename == currentfile && bytesPos.line == line.first.line && byte_index < max_byte_index) {
            std::cout <<
                esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND }) <<
                std::dec <<
                std::setw(3) <<
                line.first.line << ") ";

            auto byteout = parser->paddRight(byteOutput[byte_index].second, byteOutputWidth);
            std::cout <<
                esc.gr({ esc.BOLD, esc.GREEN_FOREGROUND }) <<
                byteout.substr(0, 6) <<
                esc.gr({ esc.BOLD, esc.YELLOW_FOREGROUND }) <<
                byteout.substr(6);
    
            // asm line
            if (asmPos.filename == currentfile && asmPos.line == line.first.line && asm_index < max_asm_index) {
                auto asmoutput = parser->paddRight(asmlines[asm_index].second, asmLineWidth);
                std::cout << asmoutput;
                if (++asm_index < max_asm_index) {
                    asmPos = asmlines[asm_index].first;
                }
            }

            // original source
            if (first) {
                first = false;
                std::cout << 
                    esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND }) <<
                    line.second;
            }
            std::cout << "\n";

            if (++byte_index < max_byte_index) {
                bytesPos = byteOutput[byte_index].first;
            }
            else 
                break;
        }
        // original source
        if (first) {
            first = false;
            std::cout <<
                esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND }) <<
                std::dec <<
                std::setw(3) <<
                line.first.line << ") " <<
                parser->paddLeft("", byteOutputWidth) <<
                parser->paddLeft("", asmLineWidth) <<
                line.second << "\n";
        }
        ++source_index;
    }
}

/// <summary>
/// Generates a list of source lines associated with AST nodes, handling file loading and line tracking as needed.
/// </summary>
/// <param name="node">A shared pointer to the ASTNode to process.</param>
void ExpressionParser::generate_file_list(std::shared_ptr<ASTNode> node)
{
    if (node->type == Line) {
        auto ll = filelistmap[currentfile];

        pos = node->position;
        if (pos.filename != currentfile) {
            currentfile = pos.filename;
            ll = filelistmap[currentfile];

            if (!parser->fileCache.contains(currentfile)) {
                // Read the file contents
                std::ifstream file(currentfile);
                if (!file) {
                    parser->throwError("Could not open file: " + currentfile);
                }
                std::string line;

                lines.clear();
                int l = 0;
                while (std::getline(file, line)) {
                    lines.push_back({ SourcePos(currentfile, ++l), line });
                }
                parser->fileCache[currentfile] = lines;
            }
            lines = parser->fileCache[currentfile];
        }
        // This is nessary because in macros the .endm is parsed BEFORE the .macro causing the lines to be contiguous
        ++ll;
        if (ll <= lines.size()) {
            pos.line = ll;
            listLines.emplace_back( std::pair{pos, lines[ll - 1].second });
        }
        filelistmap[currentfile] = ll;
    }

    for (auto& child : node->children) {

        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            generate_file_list(std::get<std::shared_ptr<ASTNode>>(child));
        }
    }
}

void ExpressionParser::printfilelistmap()
{
    std::cout << "file line map\n";
    for (auto& fileline : filelistmap) {
        std::cout << fileline.first << " [" << fileline.second << "]\n";
    }
}

/// <summary>
/// Constructs an ExpressionParser and initializes it with source files specified in the given ParserOptions.
/// </summary>
/// <param name="options">A reference to a ParserOptions object containing the list of source files to parse.</param>
ExpressionParser::ExpressionParser(ParserOptions& options)
{
    asmOutputLine_Pos = 0;

    for (auto& file : options.files) {

        fs::path full_path = fs::absolute(fs::path(file)).lexically_normal();

        std::ifstream infile(file); 
        if (!infile) {
            parser->throwError("Could not open file: " + file);
        }
        std::string line;

        auto l = 0;
        while (std::getline(infile, line)) {
            lines.push_back({ SourcePos(full_path.string(), ++l), line});
        }
    }

    byteOutput.clear();
    asmOutputLine.clear();
    asmlines.clear();
    parser = std::make_shared<Parser>(Parser(parserDict, lines));
    if (ASTNode::astMap.size() == 0)
        ASTNode::astMap = parserDict;

}

std::shared_ptr<ASTNode> ExpressionParser::Assemble() const
{
    std::shared_ptr<ASTNode> ast;

    auto pass = 1;
    bool needPass;
    symaccess unresolved;

    do {
        std::cout << "Pass " << pass << "\n";

        needPass = false;
        
        parser->tokens = tokenizer.tokenize(lines);
        parser->lines = lines;
        parser->globalSymbols.changes = 0;        
        ast = parser->Pass();
        ++pass;

        auto unresolved_locals = parser->GetUnresolvedLocalSymbols();
        unresolved = parser->GetUnresolvedSymbols();
        if (!
            unresolved_locals.empty()) {
            std::string err = "Unresolved local symbols:";
            for (auto& sym : unresolved_locals) {
                err += " " + sym.first + " accessed at line(s) ";
                for (auto& line : sym.second) {
                    err += line.filename + " " + std::to_string(line.line) + " ";
                }
                err += "\n";
            }
            parser->throwError(err);
        }
        needPass = unresolved.size() > 0 || parser->globalSymbols.changes != 0;
    } while (pass < 20 && needPass);

    if (!unresolved.empty()) {
        std::string err = "Unresolved global symbols:";
        for (auto& sym : unresolved) {
            err += " " + sym.first;
        }
        throw std::runtime_error(err + " " + parser->get_token_error_info());
    }
    return ast;
}

/// <summary>
/// Parses the input lines into an abstract syntax tree (AST) and returns the root node.
/// </summary>
/// <returns>A shared pointer to the root ASTNode representing the parsed expression. Throws a runtime_error if there are unexpected tokens after parsing is complete.</returns>
std::shared_ptr<ASTNode> ExpressionParser::parse() const
{
    std::string input;


    auto ast = Assemble();

    if (parser->current_pos < parser->tokens.size()) {
        const Token& tok = parser->tokens[parser->current_pos];
        throw std::runtime_error(
            "Unexpected token after complete parse: '" + tok.value +
            " " + tok.pos.filename +
            "' at line " + std::to_string(tok.pos.line) +
            ", col " + std::to_string(tok.line_pos)
        );
    }
    return ast;
}

/// <summary>
/// Generates the output for a parsed expression by processing the abstract syntax tree (AST) and producing file listings, output bytes, and assembly lines.
/// </summary>
/// <param name="ast">A shared pointer to the root ASTNode representing the parsed expression.</param>
void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    auto& esc = Parser::es;

    // generate list of file/lines
    inMacrodefinition = false;
    listLines.clear();
    currentfile = "";
    generate_file_list(ast);

    // generate output bytes
    currentfile = "";
    inMacrodefinition = false;

    byteOutput.clear();
    parser->output_bytes.clear();
    buildOutput(ast);

    currentfile = "";
    inMacrodefinition = false;
    asmlines.clear();
    generate_assembly(ast);

    generate_listing();
}
