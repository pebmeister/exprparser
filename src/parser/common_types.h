// written by Paul Baxter
//
// common_types.h
//
// Lightweight common types used across the parser/assembler.
// This header defines SourcePos (source location), forward declarations
// for AST-related types and the RuleArg variant used to hold child nodes
// or tokens inside AST nodes.
//
// Conventions:
//  - SourcePos::line is treated as 1-based throughout the codebase.
//  - SourcePos comparisons are ordered first by filename (lexicographically)
//    and then by line number (ascending). This is used for deterministic
//    sorting and stable iteration when resolving source fragments.

#pragma once
#include <memory>
#include <variant>
#include <iostream>

// Represents a position within a source file.
// - `filename` is the full path or logical filename.
// - `line` is 1-based (line == 0 means an invalid/unspecified position).
struct SourcePos {
    std::string filename;
    size_t line;

    // Default: unspecified position.
    SourcePos() : filename(""), line(0) {}

    // Construct a position for a given file and (1-based) line number.
    SourcePos(const std::string& f, size_t l) : filename(f), line(l) {}

    // Equality considers both filename and line.
    bool operator==(const SourcePos& other) const { return filename == other.filename && line == other.line; }
    
    // Ordering used for sorting and containers (maps/sets).
    // Primary key: filename (lexicographic)
    // Secondary key: line (ascending)
    bool operator<(const SourcePos& other) const
    {
        return (filename < other.filename) ||
            (filename == other.filename && line < other.line);
    }

    // Convenience greater-than; follows the same tuple ordering semantics.
    bool operator>(const SourcePos& other) const
    {
        return (filename > other.filename) ||
            (filename == other.filename && line > other.line);
    }

    // Print a compact representation showing only the base filename and line.
    // Example output: "[main.cpp 42]"
    void print() const
    {
        auto path = filename;
        std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
        std::cout << "[" << base_filename << " " << line << "]\n";
    }
};

// Forward declarations to avoid circular headers in the parser implementation.
class ASTNode;
struct Token;

// RuleArg is the variant type stored in an ASTNode's children vector.
// It can hold either:
//  - a shared_ptr<ASTNode> (subtree), or
//  - a Token value (leaf token).
// Using std::variant keeps the children container type-safe and avoids raw pointers.
using RuleArg = std::variant<std::shared_ptr<ASTNode>, Token>;
