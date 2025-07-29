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
    bool color = true;

    std::vector<RuleArg> children;
    static std::map<int64_t, std::string> astMap;

    ASTNode() : type(0), position({ "", 0 }) {}
    ASTNode(int64_t type) : type(type), position({"", 0}) {}
    ASTNode(int64_t type, SourcePos pos) : type(type), position(pos) {}
    ASTNode(int64_t type, SourcePos pos, int32_t v) : type(type), position(pos), value(v) {}

    void add_child(const RuleArg& child) { children.push_back(child); }
    void print(std::ostream& os, bool color, int indent = 0, const std::string& prefix = "", bool isLast = true) const;

    void resetLine(std::string file)
    {
        position.filename = file;
        for (auto& child : children) {
            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                auto& node = std::get<std::shared_ptr<ASTNode>>(child);
                node->resetLine(file);
            }
            else {
                Token& token = std::get<Token>(child);
                token.pos.filename = file;
            }
        }
    }

    void resetLine(SourcePos pos)
    {
        position = pos;
        for (auto& child : children) {
            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                auto& node = std::get<std::shared_ptr<ASTNode>>(child);
                node->resetLine(pos);
            }
            else {
                Token& token = std::get<Token>(child);
                token.pos = pos;
            }
        }
    }
};
