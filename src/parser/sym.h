#pragma once
#include <string.h>

class Sym {
public:
    std::string name;
    int value;
    bool initialized;
    bool changed;
};