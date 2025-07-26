#pragma once
#include <string.h>
#include <algorithm>
#include <set>
#include <iomanip>

class Sym {
public:
    std::string name;
    int value = 0;
    std::set<SourcePos>accessed;
    bool initialized = false;
    bool changed = false;
    bool isPC = false;
    bool isMacro = false;
    SourcePos created;

    void print()
    {
        std::cout <<
            "\nname:        " << name <<
            "\nvalue:       " << "$" << std::setw(4) << std::setfill('0') << std::hex << value 
                                    << std::dec << std::setw(0) << std::setfill(' ') <<
            "\ninitialized: " << initialized <<
            "\ncreated:     " << created.filename << " " << created.line <<
            "\nchanged:     " << changed <<
            "\nisMacro:     " << isMacro <<
            "\nisPC         " << isPC <<
            "\naccessed:    ";
        for (auto& access : accessed) {
            std::cout << "[" << access.filename << " line " << access.line << "] ";
        }
        std::cout << "\n\n";
    }
};