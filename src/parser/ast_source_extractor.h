// ast_source_extractor.h
// Utility for extracting source lines from AST nodes
// For 6502/65C02 Assembler lexer/parser

#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common_types.h"
#include "ASTNode.h"
#include "token.h"

/**
 * @brief Extracts all unique source lines referenced by an AST subtree.
 *
 * Performs a depth-first traversal of the AST starting from the given node,
 * collecting all unique source positions from:
 *   - ASTNode::position fields
 *   - Token::pos fields (for Token children in the variant)
 *
 * Returns the corresponding source lines retrieved from the file cache,
 * sorted by SourcePos (filename lexicographically, then line number ascending).
 * This ordering is critical for correct retokenization.
 *
 * @param node      Root of the AST subtree to extract source from
 * @param fileCache Map: filename → vector of (SourcePos, line_text) pairs
 *
 * @return Sorted vector of (SourcePos, source_text) pairs suitable for retokenization
 *
 * @note Sorting: Primary key = filename (lexicographic), Secondary key = line number
 * @note Invalid positions (empty filename or line == 0) are silently ignored
 * @note Missing files or out-of-bounds line numbers are silently skipped
 * @note Duplicate positions are automatically deduplicated
 *
 * @example
 *   // Extracting macro body source for expansion
 *   auto bodySource = extractSourceFromAST(macroBodyNode, parser.fileCache);
 *   for (const auto& [pos, lineText] : bodySource) {
 *       // Retokenize lineText...
 *   }
 */
[[nodiscard]]
inline std::vector<std::pair<SourcePos, std::string>> extractSourceFromAST(
    const std::shared_ptr<ASTNode>& node,
    const std::map<std::string, std::vector<std::pair<SourcePos, std::string>>>& fileCache)
{
    if (!node) {
        return {};
    }

    // std::set provides automatic deduplication and sorting via SourcePos::operator<
    std::set<SourcePos> positions;

    // Recursive depth-first traversal to collect all source positions
    std::function<void(const std::shared_ptr<ASTNode>&)> collectPositions;
    collectPositions = [&positions, &collectPositions](const std::shared_ptr<ASTNode>& current) {
        if (!current) {
            return;
        }

        // Collect this node's position if valid (non-empty filename and non-zero line)
        const auto& nodePos = current->sourcePosition;
        if (!nodePos.filename.empty() && nodePos.line > 0) {
            positions.insert(nodePos);
        }

        // Process each child in the variant vector
        for (const auto& child : current->children) {
            std::visit([&](const auto& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, std::shared_ptr<ASTNode>>) {
                    // Recurse into child ASTNode
                    collectPositions(arg);
                }
                else if constexpr (std::is_same_v<T, Token>) {
                    // Extract position from Token child
                    const auto& tokenPos = arg.pos;
                    if (!tokenPos.filename.empty() && tokenPos.line > 0) {
                        positions.insert(tokenPos);
                    }
                }
            }, child);
        }
    };

    // Begin traversal from root
    collectPositions(node);

    // Build result vector by resolving positions to source text via fileCache
    std::vector<std::pair<SourcePos, std::string>> result;
    result.reserve(positions.size());

    for (const SourcePos& pos : positions) {
        // Locate file in cache
        auto fileIt = fileCache.find(pos.filename);
        if (fileIt == fileCache.end()) {
            continue;  // File not cached, skip
        }

        const auto& lines = fileIt->second;

        // Convert 1-based line number to 0-based index
        // (SourcePos::line is 1-based per your codebase conventions)
        const size_t index = pos.line - 1;
        if (index >= lines.size()) {
            continue;  // Line number out of bounds, skip
        }

        // lines[index] is pair<SourcePos, string>; we want the string (actual source text)
        result.emplace_back(pos, lines[index].second);
    }

    // Result is pre-sorted: std::set iteration order matches SourcePos::operator<
    return result;
}
