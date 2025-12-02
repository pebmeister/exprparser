#include <string>
#include <stdexcept>
#include <vector>

#include "ANSI_esc.h"
#include "common_types.h"
#include "symboltable.h"
#include "parser.h"

extern ANSI_ESC es;

void SymTable::add(std::string& name, SourcePos pos)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        if (!symtable[uppername].created.filename.empty() && symtable[uppername].created != pos) {
            throw std::runtime_error(
                "Multiple defined symbol " + name + " " + pos.filename + " " + std::to_string(pos.line)
            );
        }
        return;
    }
    symtable[uppername] = Sym(name);
    Sym& sym = symtable[uppername];
    sym.created = pos;
    sym.isPC = true;
    sym.changed = false;
    sym.initialized = false;
}

void SymTable::add(std::string& name, int value, SourcePos pos)
{
    add(name, pos);
    auto uppername = toupper(name);
    Sym& sym = symtable[uppername];
    sym.created = pos;
    sym.isPC = true;
    sym.changed = false;
    setSymValue(name, value);
}

int SymTable::getSymValue(std::string& name, SourcePos pos)
{
    auto uppername = toupper(name);
    if (!symtable.contains(uppername)) {
        add(name, pos);
        notifyChanged(symtable[uppername]);
    }
    Sym& sym = symtable[uppername];
    if (sym.accessed.find(pos) == sym.accessed.end()) {
        sym.accessed.insert(pos);
    }
    return sym.value;
}

void SymTable::setSymValue(std::string& name, int value)
{
    auto uppername = toupper(name);
    if (symtable.contains(uppername)) {
        Sym& sym = symtable[uppername];
        if (!sym.initialized || sym.value != value) {
            if (sym.initialized && sym.value != value) {
                sym.changed = true;
            }
            sym.initialized = true;
            sym.value = value;
            notifyChanged(sym);
        }
        else if (sym.changed) {
            sym.initialized = true;
            sym.changed = false;
            notifyChanged(sym);
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
        Sym& sym = symtable[uppername];
        if (sym.isPC) {
            sym.isPC = false;
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
            // notifyChanged(symtable[uppername]);
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

bool SymTable::isDefined(const std::string& name) const
{
    
    auto namecopy = name;
    auto uppername = toupper(namecopy);
    return symtable.contains(uppername);
}

void SymTable::notifyChanged(Sym& sym)
{
    changes++;
    for (auto& symchanded : symchangedfunctions) {
        symchanded(sym);
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

void SymTable::print() const
{
    // Save formatting state so we don't leak flags/fill into callers
    auto old_flags = std::cout.flags();
    auto old_fill = std::cout.fill();

    std::cout << es.gr(es.BRIGHT_BLUE_FOREGROUND) << "================="
        << es.gr(es.BRIGHT_GREEN_FOREGROUND) << " Symbol Table "
        << es.gr(es.BRIGHT_BLUE_FOREGROUND) << "=================== \n";

    using Iter = decltype(symtable)::const_iterator;
    std::vector<Iter> rows;
    rows.reserve(symtable.size());

    // Collect only the entries you intend to print
    for (auto it = symtable.cbegin(); it != symtable.cend(); ++it) {
        const auto& sym = it->second;
        if (!sym.isMacro && !sym.accessed.empty() &&
            (sym.accessed.size() > 1 || (!sym.accessed.empty() && !sym.isPC))) {
            rows.push_back(it);
        }
    }

    // Sort by value ascending; tie-break by name to get a stable order
    std::sort(rows.begin(), rows.end(), [](Iter a, Iter b)
        {
            const auto& sa = a->second;
            const auto& sb = b->second;
            if (sa.value != sb.value) return sa.value < sb.value;
            return sa.name < sb.name;
        });

    // Print in the sorted order
    for (auto it : rows) {
        const auto& sym = it->second;
        std::cout
            << es.gr(es.BRIGHT_GREEN_FOREGROUND)
            << std::setw(20) << std::left << std::setfill(' ') << sym.name
            << es.gr(es.BRIGHT_YELLOW_FOREGROUND)
            << "$"
            << std::hex << std::uppercase
            << std::setw(4) << std::right << std::setfill('0') << sym.value
            << "\n";
    }

    // Restore stream state
    std::cout.flags(old_flags);
    std::cout.fill(old_fill);
}

