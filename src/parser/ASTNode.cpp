// ASTNode.cpp
#include <iostream>
#include <iomanip>

#include "ASTNode.h"
#include "ANSI_esc.h"

std::map<int64_t, std::string> ASTNode::astMap;

void ASTNode::print(int indent, const std::string& prefix, bool isLast) const
{
    static ANSI_ESC esc;
    std::string branch = prefix;
    if (indent > 0) {
        branch += isLast ? "`-- " : "|-- ";
    }

    // Color the AST node name cyan and bold
    std::cout << branch
        << esc.gr({ esc.BOLD, esc.CYAN_FOREGROUND })
        << astMap[type] 
        << esc.gr({ esc.BOLD, esc.WHITE_FOREGROUND })
        << " [line " << std::dec << line << " ]"
        << esc.gr(esc.RESET_ALL)
            << " (value: "
            << esc.gr(esc.GREEN_FOREGROUND) <<  "$" << std::hex << value << esc.gr(esc.RESET_ALL)
            << ")\n";

    for (size_t i = 0; i < children.size(); ++i) {
        const auto& child = children[i];
        bool lastChild = (i == children.size() - 1);
        std::string newPrefix = prefix;
        if (indent > 0) {
            newPrefix += isLast ? "    " : "|   ";
        }

        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            std::get<std::shared_ptr<ASTNode>>(child)->print(indent + 1, newPrefix, lastChild);
        }
        else {
            const Token& tok = std::get<Token>(child);
            auto val = (tok.value == "\n") ? "\\n" : tok.value;
            std::cout << newPrefix
                << (lastChild ? "`-- " : "|-- ")
                << esc.gr(esc.YELLOW_FOREGROUND)
                << astMap[tok.type]
                << esc.gr(esc.RESET_ALL)
                << " ('"
                << esc.gr(esc.GREEN_FOREGROUND)
                << val
                << esc.gr(esc.RESET_ALL)
                << "')\n";
        }
    }
}

