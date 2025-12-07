#pragma once
#include <string.h>
#include <algorithm>
#include <set>
#include <iomanip>
#include <string>

class Sym {
public:
    std::string name;
    int value = 0;
    std::set<SourcePos>accessed;
    bool initialized = false;
    bool changed = false;
    bool isPC = false;
    bool isMacro = false;
    bool isVar = false;
    SourcePos created;

    void print()
    {
        std::string createdstr = created.filename.empty() ? "" : created.filename + " " + std::to_string(created.line);

        std::cout <<
            "\nname:        " << name <<
            "\nvalue:       " << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << value 
                                    << std::dec << std::setw(0) << std::setfill(' ') <<
            "\ninitialized: " << initialized <<
            "\ncreated:     " << createdstr <<
            "\nchanged:     " << changed <<
            "\nisMacro:     " << isMacro <<
            "\nisVar        " << isVar <<
            "\nisPC         " << isPC <<
            "\naccessed:    \n";
        for (auto& access : accessed) {
            std::cout << "[" << access.filename << " line " << access.line << "]\n";
        }
        std::cout << "\n";
    }
};
