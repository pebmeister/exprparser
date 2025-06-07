// ASTNode.cpp
#include "ASTNode.h"
#include <iostream>

std::map<int64_t, std::string> ASTNode::astMap;

void ASTNode::print(int indent) const
{
    std::string indentStr(indent, ' ');
    std::cout << indentStr << astMap[type] << " (value: " << value << ")\n";
    for (const auto& child : children) {
        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            std::get<std::shared_ptr<ASTNode>>(child)->print(indent + 4);
        }
        else {
            const Token& tok = std::get<Token>(child);
            std::cout << indentStr << "  " << astMap[tok.type]
                << " ('" << tok.value << "')\n";
        }
    }
}