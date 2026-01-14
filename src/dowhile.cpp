#if __HANDLE_DO_IN_RULES__
                bool savedDefer = p.deferVariableUpdates;
                p.deferVariableUpdates = false;

                auto node = std::make_shared<ASTNode>(DoDirective, p.sourcePos);
                if (count != 0) {
                    p.deferVariableUpdates = savedDefer;
                    return node;
                }



                // Extract parsed components
                const Token& doTok = std::get<Token>(args[0]);
                //const Token& eolTok = std::get<Token>(args[1]);
                auto& doBodyAst = std::get<std::shared_ptr<ASTNode>>(args[2]);
                const Token& whileTok = std::get<Token>(args[3]);
                auto& condAst = std::get<std::shared_ptr<ASTNode>>(args[4]);

                node->position = doTok.pos;
                node->pc_Start = doBodyAst->pc_Start;

                // Extract source from file cache - this is STABLE because:
                // 1. File cache is never modified during parsing
                // 2. AST positions reference original source lines
                // 3. Now that we add children, nested loop content is included
                auto bodySource = p.getSourceFromAST(doBodyAst);
                auto condSource = p.getSourceFromAST(condAst);

                // Fix condition source - strip the ".while" prefix
                for (auto& src : condSource) {
                    std::string& line = src.second;
                    size_t whilePos = line.find(".while");
                    if (whilePos != std::string::npos) {
                        size_t exprStart = whilePos + 6;  // Length of ".while"
                        while (exprStart < line.length() &&
                                std::isspace(static_cast<unsigned char>(line[exprStart]))) {
                            exprStart++;
                        }
                        line = line.substr(exprStart);
                    }
                }

#if __DEBUG_DO_DIRECTIVE__
                // Debug output
                std::cout << "[DEBUG] DO body source ("
                    << bodySource.size() << " lines):\n";
                p.printSource(bodySource);
                std::cout << "\n";

                std::cout << "[DEBUG] WHILE condition source ("
                    << condSource.size() << " lines):\n";
                p.printSource(condSource);
                std::cout << "\n";
#endif
                // Save state for restoration
                int loopPC = p.PC;
                auto savedTokens = p.tokens;
                auto savedPos = p.current_pos;
                auto savedBytesInLine = p.bytesInLine;

                // Clear pending expansions at outermost level only
                p.clearPendingExpansions();

                // Accumulate expansion tokens from all iterations
                std::vector<Token> expansionTokens;

                int iterations = 0;
                const int MAX_ITERATIONS = 100000;
                bool continueLoop = true;

                while (continueLoop) {

                    p.printVars();

                    if (++iterations > MAX_ITERATIONS) {
                        p.tokens = savedTokens;
                        p.current_pos = savedPos;
                        p.throwError(".do/.while exceeded maximum iterations (possible infinite loop)");
                    }

                    // Reset PC for each iteration
                    p.PC = loopPC;

                    //===========================================
                    // STEP 1: Tokenize body fresh
                    //===========================================
                    auto bodyTokens = tokenizer.tokenize(bodySource);

                    // Set token positions for error reporting
                    for (auto& tok : bodyTokens) {
                        tok.pos = doTok.pos;
                    }

                    //===========================================
                    // STEP 1.5: CRITICAL FIX - Expand varsyms to current values
                    // This captures the loop counter value for THIS iteration
                    // Skip assignment targets (symbol followed by =)
                    //===========================================
                    for (size_t i = 0; i < bodyTokens.size(); ++i) {
                        auto& tok = bodyTokens[i];

                        if (tok.type ==  TOKEN_TYPE::SYM || tok.type == TOKEN_TYPE::LOCALSYM) {
                            // Check if this symbol is a varsym
                            if (p.varSymbols.isDefined(tok.value)) {
                                // Don't expand if this is an assignment target (followed by =)
                                bool isAssignmentTarget = false;
                                if (i + 1 < bodyTokens.size()) {
                                    auto nextType = bodyTokens[i + 1].type;
                                    // Check for assignment operators - adjust token types as needed
                                    if (nextType == TOKEN_TYPE::EQUAL) {
                                        isAssignmentTarget = true;
                                    }
                                }

                                if (!isAssignmentTarget) {
                                    // Expand to current value - freeze it for this iteration
                                    auto& entry = p.varSymbols[tok.value];  // Adjust to your API
                                    tok.type = DECNUM;  // Convert to numeric literal
                                    tok.value = std::to_string(entry.value);
                                }
                            }
                        }
                    }

                    //===========================================
                    // STEP 2: Parse body in isolated context
                    // .do/.while handlers will run and store their
                    // expansions in pendingLoopExpansions (NOT modify our tokens)
                    //===========================================
                    p.tokens = bodyTokens;
                    p.current_pos = 0;
                    p.bytesInLine = 0;
                    p.reset_rules();

                    auto bodyResult = p.parse_rule(RULE_TYPE::LineList);

                    //===========================================
                    // STEP 3: Apply any pending nested expansions
                    // Nested handlers stored their expansions instead of modifying
                    // tokens mid-parse - now we safely apply them
                    //===========================================
                    auto expandedBodyTokens = p.applyPendingExpansions(p.tokens);

                    // Collect the expanded tokens
                    expansionTokens.insert(expansionTokens.end(),
                                        expandedBodyTokens.begin(),
                                        expandedBodyTokens.end());

                    for (auto& tok : expansionTokens) {
                        tok.pos = doTok.pos;
                    }

                    //===========================================
                    // STEP 4: Tokenize and evaluate condition
                    // Also uses fresh tokenization for updated varsyms
                    //===========================================
                    auto condTokens = tokenizer.tokenize(condSource);
                    for (auto& tok : condTokens) {
                        tok.pos = whileTok.pos;
                    }

                    p.tokens = condTokens;
                    p.current_pos = 0;
                    p.bytesInLine = 0;
                    p.reset_rules();

                    auto condResult = p.parse_rule(RULE_TYPE::Expr);

                    continueLoop = (condResult && condResult->value != 0);
#if __DEBUG_DO_DIRECTIVE__

                    // Debug output
                    std::cout << "[DEBUG] .do "
                                    << " iteration " << iterations
                                    << ", condition=" << (condResult ? condResult->value : -1)
                                    << ", continue=" << (continueLoop ? "yes" : "no") << "\n";
#endif
                }

                //===========================================
                // STEP 5: Handle expansion based on nesting level
                //===========================================
                // OUTERMOST LOOP: Modify the main token stream
                // Restore saved state first
                p.tokens = savedTokens;
                p.current_pos = savedPos;

                // Find our construct in the token stream
                int constructStart = p.findIndex(p.tokens, doTok);

                if (constructStart >= 0) {
                    // Find the matching .while by tracking nesting depth
                    int depth = 1;
                    size_t constructEndTokenIndex = constructStart + 1;
                    size_t whileTokenIndex = (size_t)-1;

                    for (size_t i = constructStart + 1; i < p.tokens.size(); i++) {
                        if (p.tokens[i].type == DO_DIR) {
                            depth++;
                        }
                        else if (p.tokens[i].type == WHILE_DIR) {
                            depth--;
                            if (depth == 0) {
                                // index of the WHILE_DIR token in the token stream
                                whileTokenIndex = i;
                                // findLineEnd returns the index of the EOL token for that line
                                constructEndTokenIndex = p.findLineEnd(p.tokens, i);
                                break;
                            }
                        }
                    }

#if __DEBUG_DO_DIRECTIVE__
                    std::cout << "[DEBUG] .do: constructStart=" << constructStart
                              << " whileTokenIndex=" << whileTokenIndex
                              << " constructEndTokenIndex=" << constructEndTokenIndex << "\n";

                    // Dump tokens around the region for inspection
                    size_t dumpStart = (constructStart >= 5) ? constructStart - 5 : 0;
                    size_t dumpEnd = std::min(constructEndTokenIndex + 5, p.tokens.size() - 1);
                    for (size_t k = dumpStart; k <= dumpEnd; ++k) {
                        const auto &t = p.tokens[k];
                        std::cout << "[DBG TOK " << k << "] type=" << static_cast<int>(t.type)
                                  << " value=\"" << t.value << "\" pos=(" << t.pos.line << "," << t.pos.col << ")\n";
                    }
#endif

                    // Determine erase range: erase from 'constructStart' up to and including the EOL after .while.
                    // p.findLineEnd() yields the index of the EOL token for the .while line.
                    size_t eraseEndExclusive = constructEndTokenIndex + 1; // one-past-end

                    // Safety clamp
                    if (eraseEndExclusive > p.tokens.size()) {
                        eraseEndExclusive = p.tokens.size();
                    }

                    size_t oldSize = eraseEndExclusive - constructStart;
                    size_t newSize = expansionTokens.size();

                    // Erase the old construct (inclusive of the .while line + EOL)
                    p.tokens.erase(p.tokens.begin() + constructStart,
                                   p.tokens.begin() + static_cast<ptrdiff_t>(eraseEndExclusive));

                    // Insert the expansion
                    p.tokens.insert(p.tokens.begin() + constructStart,
                                    expansionTokens.begin(), expansionTokens.end());

                    // Ensure inserted tokens use the original .do position for listing consistency
                    for (size_t k = 0; k < expansionTokens.size(); ++k) {
                        p.tokens[constructStart + static_cast<int>(k)].pos = doTok.pos;
                    }

                    // Adjust current_pos to continue parsing after the inserted expansion
                    p.current_pos = constructStart + static_cast<int>(newSize);

#if __DEBUG_DO_DIRECTIVE__
                    std::cout << "[DEBUG] .do: replaced " << oldSize << " tokens with " << newSize << " tokens\n\n";
#endif
                }

                // Clear pending expansions after outermost completes
                p.clearPendingExpansions();

                // Reset PC to loop start for code generation
                p.PC = loopPC;
                p.bytesInLine = 0; // savedBytesInLine;
                p.reset_rules();
                p.deferVariableUpdates = savedDefer;
#endif
