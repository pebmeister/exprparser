// ASTNode.h
#pragma once
#include <map>
#include <memory>
#include <vector>

#include "common_types.h"
#include "token.h"

class ASTNode {
public:
    int64_t type = 0;
    int32_t value = 0;
    SourcePos position;

    std::vector<RuleArg> children;
    static std::map<int64_t, std::string> astMap;

    ASTNode(int64_t type) : type(type), position({"", 0}) {}

    ASTNode(int64_t type, SourcePos pos) : type(type), position(pos) {}

    ASTNode(int64_t type, SourcePos pos, int32_t v) : type(type), position(pos), value(v) {}


    void add_child(const RuleArg& child) { children.push_back(child); }
    void print(int indent = 0, const std::string& prefix = "", bool isLast = true) const;

    void resetLine(size_t newline)
    {
        position.line = newline;
        for (auto& child : children) {
            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                auto& node = std::get<std::shared_ptr<ASTNode>>(child);
                node->resetLine(newline);
            }
            else {
                Token& token = std::get<Token>(child);
                token.pos.line = newline;
            }
        }
    }
};
