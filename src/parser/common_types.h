#pragma once
// common_types.h
#include <memory>
#include <variant>

// Forward declarations
class ASTNode;
struct Token;
using RuleArg = std::variant<std::shared_ptr<ASTNode>, Token>;
