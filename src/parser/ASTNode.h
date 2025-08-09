// ASTNode.h
// written by Paul Baxter
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

    void add_child(const RuleArg& child) { children.push_back(child); }    void print(std::ostream& os, bool color, int indent = 0, const std::string& prefix = "", bool isLast = true) const;
};
