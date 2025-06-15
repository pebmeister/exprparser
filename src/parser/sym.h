#pragma once
#include <string.h>
#include <algorithm>

class Sym {
public:
    std::string name;
    int value;
    std::vector<size_t>accessed;
    bool was_accessed_by(size_t line)
    {
        return (std::find(accessed.begin(), accessed.end(), line) != accessed.end());
    }
    bool initialized;
    bool changed;
    bool defined_in_pass;
    bool isPC;
};