// ASTNode.cpp
#include "ASTNode.h"
#include "ANSI_esc.h"
#include <iostream>

std::map<int64_t, std::string> ASTNode::astMap;

void ASTNode::color_print(int indent, const std::string& prefix, bool isLast) const
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
        << esc.gr(esc.RESET_ALL)
            << " (value: "
            << esc.gr(esc.GREEN_FOREGROUND) << value << esc.gr(esc.RESET_ALL)
            << ")\n";

        for (size_t i = 0; i < children.size(); ++i) {
            const auto& child = children[i];
            bool lastChild = (i == children.size() - 1);
            std::string newPrefix = prefix;
            if (indent > 0) {
                newPrefix += isLast ? "    " : "|   ";
            }

            if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
                std::get<std::shared_ptr<ASTNode>>(child)->color_print(indent + 1, newPrefix, lastChild);
            }
            else {
                const Token& tok = std::get<Token>(child);
                std::cout << newPrefix
                    << (lastChild ? "`-- " : "|-- ")
                    << esc.gr(esc.YELLOW_FOREGROUND)
                    << astMap[tok.type]
                    << esc.gr(esc.RESET_ALL)
                    << " ('"
                    << esc.gr(esc.GREEN_FOREGROUND)
                    << tok.value
                    << esc.gr(esc.RESET_ALL)
                    << "')\n";
            }
        }
}


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
