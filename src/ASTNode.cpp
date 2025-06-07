// ASTNode.cpp
#include "ASTNode.h"
#include <iostream>

std::map<int64_t, std::string> ASTNode::astMap;

void ASTNode::print(int indent) const
{
    static thread_local std::vector<bool> lastFlags;

    for (int i = 0; i < indent; ++i) {
        if (i == indent - 1) {
            std::cout << (lastFlags[i] ? "+-- " : "|-- ");
        }
        else {
            std::cout << (lastFlags[i] ? "    " : "|   ");
        }
    }

    std::cout << astMap[type] << " (value: " << value << ")\n";

    size_t count = children.size();
    for (size_t i = 0; i < count; ++i) {
        bool isLast = (i == count - 1);
        if (indent >= (int)lastFlags.size())
            lastFlags.push_back(isLast);
        else
            lastFlags[indent] = isLast;

        if (std::holds_alternative<std::shared_ptr<ASTNode>>(children[i])) {
            std::get<std::shared_ptr<ASTNode>>(children[i])->print(indent + 1);
        }
        else {
            const Token& tok = std::get<Token>(children[i]);
            for (int j = 0; j <= indent; ++j) {
                if (j == indent) {
                    std::cout << (isLast ? "+-- " : "|-- ");
                }
                else {
                    std::cout << (lastFlags[j] ? "    " : "|   ");
                }
            }
            std::cout << astMap[tok.type] << " ('" << tok.value << "')\n";
        }
    }

    if (indent < (int)lastFlags.size())
        lastFlags.resize(indent);
}

