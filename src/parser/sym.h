#pragma once
#include <string.h>
#include <algorithm>

class Sym {
public:
    std::string name;
    int value = 0;
    std::vector<SourcePos>accessed;
    bool was_accessed_by(SourcePos pos)
    {
        return (std::find(accessed.begin(), accessed.end(), pos) != accessed.end());
    }
    bool initialized = false;
    bool changed = false;
    bool defined_in_pass = false;
    bool isPC = false;
    bool isMacro = false;
};