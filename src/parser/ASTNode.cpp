// ASTNode.cpp
#include <iostream>
#include <iomanip>

#include "ASTNode.h"
#include "ANSI_esc.h"

/// <summary>
/// A map that associates 64-bit integer keys with string values in the ASTNode class.
/// </summary>
std::map<int64_t, std::string> ASTNode::astMap;

/// <summary>
/// Prints a formatted and colorized representation of the AST node and its children to the standard output.
/// </summary>
/// <param name="color">True if printing using color eascape sequences
/// <param name="indent">The current indentation level, used to format the tree structure.</param>
/// <param name="prefix">A string prefix used to visually align the tree branches.</param>
/// <param name="isLast">Indicates whether this node is the last child of its parent, affecting branch formatting.</param>
void ASTNode::print(std::ostream& os, bool color, int indent, const std::string& prefix, bool isLast) const
{
    static ANSI_ESC esc;
    std::string branch = prefix;
    if (indent > 0) {
        branch += isLast ? "`-- " : "|-- ";
    }

    auto branch_color = color ? esc.gr({ esc.BOLD, esc.BRIGHT_WHITE_FOREGROUND }) : "";
    auto node_type_color = color ? esc.gr({ esc.BOLD, esc.BRIGHT_BLUE_FOREGROUND }) : "";
    auto token_type_color = color ? esc.gr(esc.YELLOW_FOREGROUND) : "";
    auto position_color = color ? esc.gr({ esc.WHITE_FOREGROUND }) : "";
    auto value_color = color ? esc.gr(esc.GREEN_FOREGROUND) : "";
    auto reset_color = color ? esc.gr(esc.RESET_ALL) : "";

    // Color the AST node name cyan and bold
    os
        << branch_color
        << branch
        << node_type_color
        << astMap[type]
        << reset_color
        << " [ '"
        << position_color
        << std::dec << position.filename << "' " << position.line 
        << reset_color
        << " ]"
        << " (value: "
        << value_color << "$" << std::hex << value
        << reset_color
        << ")\n";

    for (size_t i = 0; i < children.size(); ++i) {
        const auto& child = children[i];
        bool lastChild = (i == children.size() - 1);
        std::string newPrefix = prefix;
        if (indent > 0) {
            newPrefix += isLast ? "    " : "|   ";
        }

        if (std::holds_alternative<std::shared_ptr<ASTNode>>(child)) {
            auto& childnode = std::get<std::shared_ptr<ASTNode>>(child);
            childnode->print(os, color, indent + 1, newPrefix, lastChild);
        }
        else {
            const Token& tok = std::get<Token>(child);
            auto val = (tok.value == "\n") ? "\\n" : tok.value;
            os << branch_color 
                << newPrefix
                << (lastChild ? "`-- " : "|-- ")
                << token_type_color
                << astMap[tok.type]
                << reset_color
                << " ('"
                << value_color
                << val
                << reset_color
                << "')\n";
        }
    }
}
