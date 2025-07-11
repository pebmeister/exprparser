// ASTNode.cpp
#include <iostream>
#include <iomanip>

#include "ASTNode.h"
#include "ANSI_esc.h"

/// <summary>
/// A static map that associates 64-bit integer keys with string values in the ASTNode class.
/// </summary>
std::map<int64_t, std::string> ASTNode::astMap;

/// <summary>
/// Prints a formatted and colorized representation of the AST node and its children to the standard output.
/// </summary>
/// <param name="indent">The current indentation level, used to format the tree structure.</param>
/// <param name="prefix">A string prefix used to visually align the tree branches.</param>
/// <param name="isLast">Indicates whether this node is the last child of its parent, affecting branch formatting.</param>
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
        << " [ '" << std::dec << position.filename << "' " << position.line << " ]"
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
