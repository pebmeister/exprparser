// ASTNode.h
#pragma once
#include <vector>
#include <map>
#include <memory>
#include "Token.h"
#include "common_types.h"

class ASTNode {
public:
    int64_t type;
    int value = 0;
    std::vector<RuleArg> children;
    static std::map<int64_t, std::string> astMap;

    ASTNode(int64_t type) : type(type) {}
    void add_child(const RuleArg& child) { children.push_back(child); }
    void print(int indent = 0) const;
    void color_print(int indent = 0, const std::string& prefix = "", bool isLast = true) const;
};

