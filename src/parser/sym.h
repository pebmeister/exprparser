#pragma once
#include <string.h>
#include <algorithm>

class Sym {
public:
    std::string name;
    int value = 0;
    std::vector<size_t>accessed;
    bool was_accessed_by(size_t line)
    {
        return (std::find(accessed.begin(), accessed.end(), line) != accessed.end());
    }
    bool initialized = false;
    bool changed = false;
    bool defined_in_pass = false;
    bool isPC = false;
    bool isMacro = false;
};