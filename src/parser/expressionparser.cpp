#include <cassert>
#include <opcodedict.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <filesystem>

#include "ExpressionParser.h"
#include "ANSI_esc.h"

namespace fs = std::filesystem;
extern ANSI_ESC es;

#pragma warning( disable : 6031 )
// #define __DEBUG_SYM__ 1

void ExpressionParser::TestParserDict() const
{
#ifdef DEBUG
    for (auto tokId = (int)TOKEN_TYPE::ORA; tokId < TOKEN_TYPE::LAST; tokId++) {
        auto& rulestr = parserDict[tokId];
        if (rulestr.empty()) {
            parser->throwError("missing parserdict entry for " + std::to_string(tokId));
        }
    }

    for (auto ruleId = (int)RULE_TYPE::Factor; ruleId <= RULE_TYPE::Prog; ruleId++) {
        auto& rulestr = parserDict[ruleId];
        if (rulestr.empty()) {
            parser->throwError("missing parserdict entry for " + std::to_string(ruleId));
        }
    }
#endif // DEBUG
}

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

void ExpressionParser::generate_output_bytes(std::shared_ptr<ASTNode> node)
{
    if (inMacrodefinition) return;

    pos = node->sourcePosition;

    auto processChildren = [&](const std::vector<RuleArg>& children)
        {
            for (const auto& child : children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    generate_output_bytes(std::get<std::shared_ptr<ASTNode>>(child));
                }
            }
        };

    auto flushOutputLine = [&]()
        {
            if (!byteOutputLine.empty()) {
                byteOutput.push_back({ pos, byteOutputLine });
                byteOutputLine.clear();
            }
            lastpos = pos;
        };

    switch (node->type) {
        case Prog:
        case LineList:
        case Statement:
        case Op_Instruction:
            processChildren(node->children);
            return;

        case WhileDirective:
            // WhileDirective children: [WHILE_DIR, -Expr, -EOLOrComment, -LineList, WEND_DIR]
            if (!inMacrodefinition && node->children.size() >= 5) {
                const auto& whileTok = std::get<Token>(node->children[0]);
                const auto& wendTok = std::get<Token>(node->children[4]);
                const auto& loopBody = std::get<std::shared_ptr<ASTNode>>(node->children[3]);

                looplevel++;
                if (looplevel == 1) {
                    loopOutputpos = wendTok.pos;
                }

                auto bodySource = parser->getSourceFromAST(loopBody);
                auto conditionSource = parser->getSourceFromAST(std::get<std::shared_ptr<ASTNode>>(node->children[1]));

                // Clean up the while condition (remove the ".while" keyword)
                if (!conditionSource.empty()) {
                    auto& condLine = conditionSource[0].second;
                    auto off = condLine.find(".while");
                    if (off != std::string::npos) {
                        condLine = condLine.substr(off + 6);
                        // Trim leading whitespace
                        size_t start = condLine.find_first_not_of(" \t");
                        if (start != std::string::npos) {
                            condLine = condLine.substr(start);
                        }
                    }
                }

                // Create a parser for the loop iterations
                if (looplevel == 1) {
                    doParser->anonLabels = parser->anonLabels;
                    doParser->localSymbols = parser->localSymbols;
                    doParser->globalSymbols = parser->globalSymbols;
                    doParser->varSymbols = parser->varSymbols;  // Copy initial variable state
                }

                const int maxIterations = 0xFFFF;  // Safety limit to prevent infinite loops
                int iterations = 0;

                while (iterations < maxIterations) {

                    auto conditionTokens = tokenizer.tokenize(conditionSource);
                    doParser->tokens = conditionTokens;
                    doParser->current_pos = 0;
                    doParser->deferVariableUpdates = false;

                    auto varTempSymbols = doParser->varSymbols;
                    doParser->InitPass();  // Reset pass-specific state
                    doParser->varSymbols = varTempSymbols;

                    auto condition_ast = doParser->parse_rule(RULE_TYPE::Expr);
                    auto continueLoop = (condition_ast && condition_ast->value != 0);

                    // check for loop exit
                    if (!continueLoop)
                        break;

                    // Parse and execute the loop body
                    auto bodyTokens = tokenizer.tokenize(bodySource);

                    doParser->tokens = bodyTokens;
                    doParser->current_pos = 0;
                    doParser->deferVariableUpdates = false;

                    varTempSymbols = doParser->varSymbols;
                    doParser->InitPass();  // Reset pass-specific state
                    doParser->varSymbols = varTempSymbols;

                    auto loop_ast = doParser->parse_rule(RULE_TYPE::LineList);
                    if (loop_ast) {
                        auto sz = byteOutput.size();
                        generate_output_bytes(loop_ast);
                        auto i = 0;
                        for (auto& [pos, line] : byteOutput) {
                            if (i++ < sz)
                                continue;
                            pos = loopOutputpos;
                        }
                    }
                    iterations++;
                }
                looplevel--;
            }
            return;

        case DoDirective:
            // DoDirective children: [DO_DIR, -EOLOrComment, LineList, WHILE_DIR, Expr]
            if (!inMacrodefinition && node->children.size() >= 5) {

                const auto& doTok = std::get<Token>(node->children[0]);
                const auto& whileTok = std::get<Token>(node->children[3]);
                const auto& loopBody = std::get<std::shared_ptr<ASTNode>>(node->children[2]);

                looplevel++;
                if (looplevel == 1) {
                    loopOutputpos = whileTok.pos;
                }

                auto bodySource = parser->getSourceFromAST(loopBody);
                auto conditionSource = parser->getSourceFromAST(std::get<std::shared_ptr<ASTNode>>(node->children[4]));

                // Clean up the while condition (remove the ".while" keyword)
                if (!conditionSource.empty()) {
                    auto& condLine = conditionSource[0].second;
                    auto off = condLine.find(".while");
                    if (off != std::string::npos) {
                        condLine = condLine.substr(off + 6);
                        // Trim leading whitespace
                        size_t start = condLine.find_first_not_of(" \t");
                        if (start != std::string::npos) {
                            condLine = condLine.substr(start);
                        }
                    }
                }

                // Create a parser for the loop iterations
                if (looplevel == 1) {
                    doParser->anonLabels = parser->anonLabels;
                    doParser->localSymbols = parser->localSymbols;
                    doParser->globalSymbols = parser->globalSymbols;
                    doParser->varSymbols = parser->varSymbols;  // Copy initial variable state
                }

                bool continueLoop = true;
                const int maxIterations = 0xFFFF;  // Safety limit to prevent infinite loops
                int iterations = 0;

                while (continueLoop && iterations < maxIterations) {

                    // Parse and execute the loop body
                    auto bodyTokens = tokenizer.tokenize(bodySource);

                    doParser->tokens = bodyTokens;
                    doParser->current_pos = 0;
                    doParser->deferVariableUpdates = false;

                    auto varTempSymbols = doParser->varSymbols;
                    doParser->InitPass();  // Reset pass-specific state
                    doParser->varSymbols = varTempSymbols;

                    auto loop_ast = doParser->parse_rule(RULE_TYPE::LineList);
                    if (loop_ast) {
                        auto sz = byteOutput.size();
                        generate_output_bytes(loop_ast);
                        auto i = 0;
                        for (auto& [pos, line] : byteOutput) {
                            if (i++ < sz)
                                continue;
                            pos = loopOutputpos;
                        }
                    }

                    // Evaluate the while condition
                    auto conditionTokens = tokenizer.tokenize(conditionSource);
                    doParser->tokens = conditionTokens;
                    doParser->current_pos = 0;
                    doParser->deferVariableUpdates = false;

                    varTempSymbols = doParser->varSymbols;
                    doParser->InitPass();  // Reset pass-specific state
                    doParser->varSymbols = varTempSymbols;

                    auto condition_ast = doParser->parse_rule(RULE_TYPE::Expr);
                    continueLoop = (condition_ast && condition_ast->value != 0);

                    iterations++;
                }

                // Synchronize variable changes back to the main parser
                parser->varSymbols = doParser->varSymbols;
                parser->anonLabels = doParser->anonLabels;
                parser->globalSymbols = doParser->globalSymbols;
                parser->localSymbols = doParser->localSymbols;

                if (iterations >= maxIterations) {
                    parser->throwError("Do-While loop exceeded maximum iterations (possible infinite loop)");
                }
                looplevel--;
            }
            return;

        case Line:
            pos = node->sourcePosition;  // Capture the line's actual position
            processChildren(node->children);
            // Flush any pending bytes for this line
            flushOutputLine();
            return;

        case PCAssign:
            parser->PC = node->value;
            currentPC = parser->PC;
            expected_pc = parser->PC;
            printPC(currentPC);
            return;

        case OrgDirective:
            if (output_bytes.size() > 0) {
                throw std::runtime_error(".org not allowed after bytes ar generated.");
            }
            parser->PC = node->value;
            currentPC = parser->PC;
            expected_pc = parser->PC;
            printPC(currentPC);
            return;

        case StorageDirective:
        {
            printPC(currentPC);
            if (node->value < 0) {
                parser->throwError(".ds argument must be non-negative");
            }
            for (int i = 0; i < node->value; ++i) {
                currentPC++;
                expected_pc++;
            }

            if (allowbytes && output_bytes.size() > 0)
                allowbytes = false;
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
                    printPC(currentPC);
                }
                printbyte(b);
                outputbyte(b);
                ++col;
                extra = true;

                if (col == 3) {
                    pos = node->sourcePosition;
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
            inMacrodefinition = true;
            processChildren(node->children);
            inMacrodefinition = false;
            return;

        case Op_Implied:
        case Op_Accumulator:
            printPC(currentPC);
            printbyte(node->value);
            outputbyte(node->value);
            flushOutputLine();
            return;

        case Op_Relative:
            printPC(currentPC);
            printbyte(node->value);
            outputbyte(node->value);

            if (node->children.size() == 2) {
                const auto& value = node->children[1];
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(value)) {
                    auto& value_token = std::get<std::shared_ptr<ASTNode>>(value);
                    int n = value_token->value - (currentPC + 1);
                    bool out_of_range = ((n + 127) & ~0xFF) != 0;
                    if (out_of_range) {
                        auto& left = std::get<std::shared_ptr<ASTNode>>(node->children[0]);
                        TOKEN_TYPE opcode = static_cast<TOKEN_TYPE>(left->value);
                        auto it = opcodeDict.find(opcode);
                        if (it == opcodeDict.end()) {
                            parser->throwError("Unknown opcode");
                        }
                        const OpCodeInfo& info = it->second;
                       
                        parser->printSymbols();
                        std::cout << "Local symbols\n";
                        parser->localSymbols.print();

                        std::cout << "ERROR PC = " << std::hex << "$" << parser->PC << std::dec << "\n";
                        // parser->throwError("Opcode '" + info.mnemonic + "' operand out of range (" + std::to_string(n) + ")");
                    }
                    uint8_t b = static_cast<uint8_t>(n & 0xFF);
                    printbyte(b);
                    outputbyte(b);
                }
            }
            flushOutputLine();
            return;

        case Op_Immediate:
        case Op_ZeroPage:
        case Op_ZeroPageX:
        case Op_ZeroPageY:
        case Op_IndirectX:
        case Op_IndirectY:
            printPC(currentPC);
            printbyte(node->value);
            outputbyte(node->value);

            for (const auto& child : node->children) {
                if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                    auto& valuenode = std::get<std::shared_ptr<ASTNode>>(child);
                    if (valuenode->type == Expr || valuenode->type == AddrExpr) {
                        uint16_t value = valuenode->value;
                        printbyte(value);
                        outputbyte(value);
                        break;
                    }
                }
            }
            flushOutputLine();
            return;

        case Op_Absolute:
        case Op_AbsoluteX:
        case Op_AbsoluteY:
        case Op_Indirect:
            printPC(currentPC);
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
            flushOutputLine();
            return;

        case Op_ZeroPageRelative:
            printPC(currentPC);
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
            flushOutputLine();
            return;
    }
}

/// <summary>
/// Generates assembly code from an abstract syntax tree (AST) node and appends the output to the assembly lines buffer.
/// </summary>
/// <param name="node">A shared pointer to the ASTNode representing the current node in the abstract syntax tree to process.</param>
void ExpressionParser::generate_assembly(std::shared_ptr<ASTNode> node)
{
    if (
        node->type == MacroDef ||
        node->type == VarDirective ||
        node->type == DoDirective ||
        node->type == WhileDirective ||
        node->type == FillDirective

        ) {
        return;
    }

    std::stringstream ss;
    std::string color = es.gr(es.WHITE_FOREGROUND);
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

        case Line:
            processChildren(node->children);

            if (asmOutputLine_Pos == 0) {
                return;
            }

            padAsmOutputLine();
            asmlines.push_back({ node->sourcePosition, asmOutputLine });

            asmOutputLine.clear();
            asmOutputLine_Pos = 0;
            return;

        case WordDirective:
        case ByteDirective:
        {
            std::vector<uint16_t> bytes;
            auto bytelistNode = std::get<std::shared_ptr<ASTNode>>(node->children[1]);
            extractExpressionList(bytelistNode, bytes, false);

            size_t i = 0;
            size_t remaining = bytes.size();

            const std::string colorKeyword = es.gr({ es.BOLD, es.CYAN_FOREGROUND });
            const std::string colorByte = es.gr({ es.BOLD, es.YELLOW_FOREGROUND });

            auto makeIndentedLine = [&]() -> std::string
                {
                    return std::string(instruction_indent, ' ');
                };

            while (remaining > 0) {
                asmOutputLine = makeIndentedLine();
                asmOutputLine_Pos = instruction_indent;

                ss.str("");
                ss.clear();
                ss << colorKeyword << (node->type == ByteDirective ? ".byte" : ".word") << colorByte;
                ss >> temp;
                asmOutputLine += temp;
                asmOutputLine_Pos += 5;

                size_t chunkSize = std::min<size_t>(3, remaining);

                for (size_t b = 0; b < chunkSize; ++b) {
                    ss.str("");
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

                padAsmOutputLine();
                asmlines.push_back({ node->sourcePosition, asmOutputLine });

                asmOutputLine.clear();
                asmOutputLine_Pos = 0;
                i += chunkSize;
                remaining -= chunkSize;
            }
            return;
        }

        case MacroStart:
            inMacrodefinition = true;
            break;

        case EndMacro:
            inMacrodefinition = false;
            return;

        case Equate:
        case Comment:
        case OrgDirective:
        case LabelDef:
        case SymbolRef:
        case SymbolName:
            return;

        case Expr:
        case AddrExpr:
            if (!inMacrodefinition) {
                color = es.gr({ es.BOLD, es.YELLOW_FOREGROUND });
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

        case StorageDirective:
            color = es.gr({ es.BOLD, es.CYAN_FOREGROUND });
            break;

        case Op_Instruction:
        case OpCode:
        case IncludeDirective:
            color = es.gr({ es.BOLD, es.BLUE_FOREGROUND });
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
}

/// <summary>
/// Prints the assembly lines with formatting and file tracking information.
/// </summary>
void ExpressionParser::print_asm()
{
    std::set<std::string> filesprocesseding;

    for (auto& line : asmlines) {
        std::cout <<
            es.gr({ es.BOLD, es.WHITE_FOREGROUND });

        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            auto visited = filesprocesseding.contains(currentfile);
            std::string prefix = visited ? "\nResuming " : "\nProcessing ";

            std::cout <<
                es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                prefix << currentfile << "\n\n";

            if (!visited)
                filesprocesseding.insert(currentfile);
        }

        std::cout << std::setw(3) << line.first.line << ") " << line.second << "\n";
    }
}

/// <summary>
/// Prints the contents of the list file, grouping lines by filename and displaying each line with its line number.
/// </summary>
void ExpressionParser::print_listfile()
{
    currentfile = "";
    auto lastline = -1;
    for (auto& line : listLines) {
        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            std::cout << "Processsing " << currentfile << "\n";
        }
        if (line.first.line != lastline) {
            std::cout << std::setw(3) << std::to_string(line.first.line) << ") " << line.second << "\n";
            lastline = line.first.line;
        }
    }
}

void ExpressionParser::print_printmap()
{
    for (const auto& [pos, val] : printMap) {
        std::cout << pos.filename << " " << pos.line << "   " << val << "\n";
    }
}

void ExpressionParser::generate_printmap(std::shared_ptr<ASTNode> node)
{
    if (node->type == DoDirective ||
        node->type == WhileDirective) {
        return;
    }

    if (node->type == PrintDirective) {
        const auto& tok = std::get<Token>(node->children[0]);
        printMap[tok.pos] = (tok.type == TOKEN_TYPE::PRINT_ON) ? 1 : 0;
        return;
    }

    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            generate_printmap(std::get<std::shared_ptr<ASTNode>>(child));
        }
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

void ExpressionParser::generate_listing()
{
    currentfile = "";
    size_t max_byte_index = byteOutput.size();
    size_t max_asm_index = asmlines.size();

    size_t byte_index = 0;
    size_t asm_index = 0;
    size_t source_index = 0;

    SourcePos bytesPos = (byte_index < max_byte_index) ? byteOutput[byte_index].first : SourcePos();
    SourcePos asmPos = (asm_index < max_asm_index) ? asmlines[asm_index].first : SourcePos();

    std::set<std::string> filesprocesseding;

    for (auto& line : listLines) {
        bool printstateChange = false;
        auto& pos = line.first;
        if (printMap.contains(pos)) {
            printstate = printMap[pos];
            printstateChange = true;
        }
        
        // new file
        if (line.first.filename != currentfile) {
            currentfile = line.first.filename;
            auto visited = filesprocesseding.contains(currentfile);
            std::string prefix = visited ? "Resuming " : "Processing ";

            std::cout <<
                es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                prefix << currentfile << "\n";

            if (!visited)
                filesprocesseding.insert(currentfile);
        }

        bool original_printed = false;

        while (byte_index < max_byte_index && byteOutput[byte_index].first == pos) {
            if (printstate && !printstateChange) {
                std::cout <<
                    es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                    std::dec << std::setfill(' ') << std::setw(3) << pos.line << ") " << std::setw(0);
            }
            // bytes
            auto byteout = paddRight(byteOutput[byte_index].second, byteOutputWidth);
            if (printstate && !printstateChange) {
                std::cout <<
                    es.gr({ es.BOLD, es.GREEN_FOREGROUND }) <<
                    byteout.substr(0, 6) <<
                    es.gr({ es.BOLD, es.YELLOW_FOREGROUND }) <<
                    byteout.substr(6);
            }

            if (asm_index < max_asm_index && asmlines[asm_index].first == pos) {
                if (printstate && !printstateChange) {
                    std::cout << asmlines[asm_index].second;
                }
                ++asm_index;
            }
            else {
                auto blank = paddRight("", asmLineWidth);
                std::cout << blank;
            }
            if (!original_printed) {
                original_printed = true;
                if (printstate && !printstateChange) {
                    std::cout <<
                        es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                        line.second;
                }
            }
            if (printstate && !printstateChange) {
                std::cout << "\n";
            }
            byte_index++;
        }

        if (!original_printed) {
            if (printstate && !printstateChange) {
                std::cout <<
                    es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                    std::dec << std::setw(3) << pos.line << ") " << std::setw(0);

                auto byteout = paddRight("", byteOutputWidth);
                std::cout <<
                    es.gr({ es.BOLD, es.GREEN_FOREGROUND }) <<
                    byteout.substr(0, 6) <<
                    es.gr({ es.BOLD, es.YELLOW_FOREGROUND }) <<
                    byteout.substr(6);
            }
            if (asm_index < max_asm_index && asmlines[asm_index].first == pos) {
                if (printstate && !printstateChange) {
                    std::cout << asmlines[asm_index].second;
                }
                ++asm_index;
            }
            else {                
                auto blank = paddRight("", asmLineWidth);
                if (printstate && !printstateChange) {
                    std::cout << blank;
                }
            }
            if (printstate && !printstateChange) {
                std::cout <<
                    es.gr({ es.BOLD, es.WHITE_FOREGROUND }) <<
                    line.second << "\n";
            }
        }
    }
}

/// <summary>
/// Generates a list of source lines associated with AST nodes, handling file loading and line tracking as needed.
/// </summary>
/// <param name="node">A shared pointer to the ASTNode to process.</param>
void ExpressionParser::generate_file_list(std::shared_ptr<ASTNode> node)
{
    if (node->type == DoDirective ||
        node->type == WhileDirective) {
        return;
    }

    if (node->type == Line) {
        pos = node->sourcePosition;

        if (pos.filename != currentfile) {
            currentfile = pos.filename;
            lines = parser->readfile(currentfile);
        }

        // Use the actual source line number from the node position instead of incrementing
        if (pos.line > 0 && pos.line <= lines.size()) {
            auto lastpos = (listLines.size() > 0)
                ? listLines[listLines.size() - 1].first
                : SourcePos();

            if (lastpos != pos) {
                if (pos.filename == lastpos.filename) {
                    // must use pos.line > lastpos.line because line is unsigned
                    while (pos.line > lastpos.line && pos.line - lastpos.line > 1) {
                        lastpos.line++;
                        auto insertline = std::pair{ lastpos, lines[lastpos.line - 1].second };
                        listLines.emplace_back(insertline);
                    }
                }

                auto newline = std::pair{ pos, lines[pos.line - 1].second };
                listLines.emplace_back(newline);
            }
        }
    }
    for (auto& child : node->children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            generate_file_list(std::get<std::shared_ptr<ASTNode>>(child));
        }
    }
}

/// <summary>
/// Constructs an ExpressionParser and initializes it with source files specified in the given ParserOptions.
/// </summary>
/// <param name="options">A reference to a ParserOptions object containing the list of source files to parse.</param>
ExpressionParser::ExpressionParser(ParserOptions& options) : options(options)
{
    asmOutputLine_Pos = 0;
    printstate = 1;
    parser = std::make_shared<Parser>(Parser(parserDict));
    doParser = std::make_shared<Parser>(Parser(parserDict));
    parser->includeDirectories = options.includeDirectories;
    doParser->includeDirectories = options.includeDirectories;

    for (auto& file : options.files) {
        fs::path full_path = fs::absolute(fs::path(file)).lexically_normal();
        lines = parser->readfile(full_path.string());
    }
    byteOutput.clear();
    asmOutputLine.clear();
    asmlines.clear();
    if (ASTNode::astMap.size() == 0)
        ASTNode::astMap = parserDict;
    currentPC = parser->org;
    expected_pc = currentPC;

    TestParserDict();
}

std::shared_ptr<ASTNode> ExpressionParser::Assemble() const
{
    std::shared_ptr<ASTNode> ast;

    auto pass = 1;
    bool needPass;
    symaccess unresolved;

#ifdef __DEBUG_SYM__
    if (options.verbose) {
        parser->globalSymbols.addsymchanged(
            [this, &pass](Sym& sym)
            {
                std::cout << "\nPass " << pass << "  sym changed\n";
                sym.print();
            }
        );
    }
#endif

    auto tokens = tokenizer.tokenize(lines);

    parser->tokens = tokens;
    parser->tokens.clear();

    do {
        if (options.verbose)
            std::cout << es.gr(es.BRIGHT_GREEN_FOREGROUND) << "Pass " << es.gr(es.BRIGHT_YELLOW_FOREGROUND) << pass << "\n";

        needPass = false;
        parser->tokens.assign(tokens.begin(), tokens.end());
        ast = parser->Pass();
        ++pass;

#if __DEBUG_TOKENS__
        if (options.verbose)
            parser->printTokens();
#endif
        auto unresolved_locals = parser->GetUnresolvedLocalSymbols();
        unresolved = parser->GetUnresolvedSymbols();
        if (!unresolved_locals.empty()) {
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

#ifdef __DEBUG_SYM__
        parser->globalSymbols.print(); 
#endif
        // We dont care if vars change
        needPass = unresolved.size() > 0 || parser->globalSymbols.changes != 0 || parser->anonLabels.isChanged();
    } while (pass < max_passes && needPass);

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
    auto ast = Assemble();
#if 0
    std::cout << "\nPC history\n";
    auto pass = 1;
    for (auto& PCpass : parser->PCHistory) {
        std::cout << "PASS " << pass++ << "\n";
        auto line = 1;
        for (auto& hist : PCpass) {
            std::cout << "Line "
                << std::dec << std::setfill(' ') << std::setw(4) << line++
                << "     $" << std::hex << std::setfill('0') << std::setw(4) << hist << "\n"
                << std::setw(0) << std::dec;
        }

    }
#endif
    if (parser->current_pos < parser->tokens.size()) {
        const Token& tok = parser->tokens[parser->current_pos];
        parser->throwError("Unexpected token: '" + tok.value + "'");
    }
    return ast;
}

/// <summary>
/// Generates the output for a parsed expression by processing the abstract syntax tree (AST) and producing file listings, output bytes, and assembly lines.
/// </summary>
/// <param name="ast">A shared pointer to the root ASTNode representing the parsed expression.</param>
void ExpressionParser::generate_output(std::shared_ptr<ASTNode> ast)
{
    // generate output bytes
    currentfile = "";
    inMacrodefinition = false;
    byteOutput.clear();
    output_bytes.clear();
    lastpos.line = -1;
    looplevel = 0;
    generate_output_bytes(ast);

    // if not verbose all we needed was the output
    if (!options.verbose) {
        return;
    }

    // generate list of file/lines
    generate_printmap(ast);
    inMacrodefinition = false;
    listLines.clear();
    currentfile = "";
    generate_file_list(ast);

    // generate simplified asm
    currentfile = "";
    inMacrodefinition = false;
    asmlines.clear();
    generate_assembly(ast);

#ifdef __DEBUG_AST__
    std::cout << "-------------- list file --------------\n";
    print_listfile();
    std::cout << "--------------  outbytes --------------\n";
    print_outbytes();
    std::cout << "--------------    asm    --------------\n";
    print_asm();
#endif

    generate_listing();
}