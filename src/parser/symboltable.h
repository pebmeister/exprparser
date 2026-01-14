 #pragma once
#include <map>
#include <iostream>
#include <functional>
#include <set>

#include "common_types.h"
#include "sym.h"

extern std::string toupper(std::string& input);

typedef std::vector<std::pair<std::string, std::set<SourcePos>>> symaccess;

class SymTable {
private:
    std::map<std::string, Sym> symtable;
    std::vector<std::function<void(Sym&)>> symchangedfunctions;
    void notifyChanged(Sym& sym);

public:
    SymTable() {}
    void clear() { symtable.clear(); }
    int changes = 0;

    void addsymchanged(std::function<void(Sym&)>onSymChanged);
    void setSymVar(std::string & name);
    void print() const;
    symaccess getUnresolved();
    void add(std::string& name, SourcePos pos);
    void add(std::string& name, int value, SourcePos pos);
    int getSymValue(std::string& name, SourcePos pos);
    void setSymValue(std::string& name, SourcePos pos, int value);
    void setSymEQU(std::string& name);
    void setSymMacro(std::string& name);
    bool isLabel(std::string& name);
    bool isDefined(const std::string& name) const;

    Sym& operator[](std::string name)
    {
        auto uppername = toupper(name);
        Sym& sym = symtable[uppername];
        return sym;
    }
};
