#pragma once
// common_types.h
#include <memory>
#include <variant>
#include <iostream>

struct SourcePos {
    std::string filename;
    size_t line;
    SourcePos() : filename(""), line(0) {}
    SourcePos(const std::string& f, size_t l) : filename(f), line(l) {}
    bool operator==(const SourcePos& other) const { return filename == other.filename && line == other.line; }
    
    bool operator<(const SourcePos& other) const
    {
        return (filename < other.filename) ||
            (filename == other.filename && line < other.line);
    }

    void print() const
    {
        std::cout << filename << " " << line << "\n";
    }
};

// Forward declarations
class ASTNode;
struct Token;
using RuleArg = std::variant<std::shared_ptr<ASTNode>, Token>;
