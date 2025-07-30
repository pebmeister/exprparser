#include <string>
#include <stdexcept>
#include <vector>
#include "common_types.h"
#include "symboltable.h"

void SymTable::add(std::string& name, int value, SourcePos pos)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        if ( !symtable[uppername].created.filename.empty() && symtable[uppername].created != pos) {
            throw std::runtime_error(
                "Multiple defined symbol " + name + " " + pos.filename + " " + std::to_string(pos.line)
            );
        }
        symtable[uppername].created = pos;
    }
    else {
        symtable[uppername] = Sym(name);
        symtable[uppername].value = value;
        symtable[uppername].created = pos;
        symtable[uppername].isPC = true;
        symtable[uppername].changed = false;
        symtable[uppername].initialized = true;
    }
    setSymValue(name, value);
}

int SymTable::getSymValue(std::string& name, SourcePos pos)
{
    auto uppername = toupper(name);
    if (!symtable.contains(uppername)) {
        symtable[uppername] = Sym(name);
        symtable[uppername].initialized = 0;
        symtable[uppername].value = 0;
        symtable[uppername].isPC = true;
        notifyChanged(symtable[uppername]);
    }
    if (symtable[uppername].accessed.find(pos) == symtable[uppername].accessed.end()) {
        symtable[uppername].accessed.insert(pos);
    }
    return symtable[uppername].value;
}

void SymTable::notifyChanged(Sym& sym)
{
    for (auto& sychandedfunc : symchangedfunctions) {
        sychandedfunc(sym);
    }
}

void SymTable::addsymchanged(std::function<void(Sym&)>onSymChanged)
{
    symchangedfunctions.emplace_back(onSymChanged);
}

symaccess SymTable::getUnresolved()
{
    symaccess unresolved;
    for (auto& symEntry : symtable) {
        Sym& sym = symEntry.second;
        if (!sym.isMacro && (sym.changed || !sym.initialized)) {
            unresolved.emplace_back(std::pair{ sym.name, sym.accessed });
        }
    }
    return unresolved;
}

void SymTable::setSymValue(std::string& name, int value)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        if (symtable[uppername].value != value) {
            symtable[uppername].changed = true;
            symtable[uppername].value = value;
            notifyChanged(symtable[uppername]);
        }
        else if (symtable[uppername].changed) {
            symtable[uppername].initialized = true;
            symtable[uppername].changed = false;
            notifyChanged(symtable[uppername]);
        }
    }
    else {
        throw std::runtime_error(
            "Undefined symbol " + name
        );
    }
}

void SymTable::setSymEQU(std::string& name)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        if (symtable[uppername].isPC) {
            symtable[uppername].isPC = false;
            notifyChanged(symtable[uppername]);
        }
    }
    else {
        throw std::runtime_error(
            "Undefined symbol " + name
        );
    }
}

void SymTable::setSymMacro(std::string& name)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        if (!symtable[uppername].isMacro) {
            symtable[uppername].isPC = false;
            symtable[uppername].isMacro = true;
            notifyChanged(symtable[uppername]);
        }
    }
    return;
}

bool SymTable::isLabel(std::string& name)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        return symtable[uppername].isPC;
        throw std::runtime_error(
            "Undefined symbol " + name
        );
    }
    return true;
}

void SymTable::print()
{
    std::cout << "=================Symbol Table===================";
    for (auto& symEntry : symtable) {
        auto& key = symEntry.first;
        auto& sym = symEntry.second;
        sym.print();
    }
}
