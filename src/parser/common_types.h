#pragma once
// common_types.h
#include <memory>
#include <variant>

struct SourcePos {
    std::string filename;
    size_t line;
    SourcePos() : filename(""), line(0) {}
    SourcePos(const std::string& f, size_t l) : filename(f), line(l) {}
    bool operator==(const SourcePos& other) const { return filename == other.filename && line == other.line; }
};

// Forward declarations
class ASTNode;
struct Token;
using RuleArg = std::variant<std::shared_ptr<ASTNode>, Token>;
