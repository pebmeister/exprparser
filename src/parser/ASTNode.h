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
    SourcePos sourcePosition;
    SourcePos listPosition;
    bool color = true;
    int pc_Start;

    std::vector<RuleArg> children;
    static std::map<int64_t, std::string> astMap;

    ASTNode() : type(0), sourcePosition({ "", 0 }), pc_Start(0) {}
    ASTNode(int64_t type) : type(type), sourcePosition({"", 0}), pc_Start(0) {}
    ASTNode(int64_t type, SourcePos pos) : type(type), sourcePosition(pos), pc_Start(0) {}
    ASTNode(int64_t type, SourcePos pos, int32_t v) : type(type), sourcePosition(pos), value(v), pc_Start(0) {}
    ASTNode(int64_t type, SourcePos pos, int32_t v, int pc) : type(type), sourcePosition(pos), value(v), pc_Start(pc) {}

    void add_child(const RuleArg& child) { children.push_back(child); }    void print(std::ostream& os, bool color, int indent = 0, const std::string& prefix = "", bool isLast = true) const;
};
