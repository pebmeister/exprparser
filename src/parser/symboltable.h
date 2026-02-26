// written by Paul Baxter
#pragma once
#include <map>
#include <iostream>
#include <functional>
#include <set>

#include "common_types.h"
#include "sym.h"

/*
 symboltable.h
 -------------
 Symbol table abstraction used by the assembler/parser.

 Responsibilities:
  - Store symbol metadata (via Sym) keyed by uppercase symbol name.
  - Provide lookup, insertion and update operations used during parsing
    and multi-pass assembly.
  - Provide diagnostic access to unresolved symbols and notify listeners
    when symbols change.
 Notes:
  - Symbol names are normalized to upper-case for case-insensitive lookup.
  - Source positions recorded with symbols are used for diagnostics and
    for producing listings / error messages.
  - This header declares the API; implementations live in the corresponding .cpp.
*/

// Declared elsewhere; utility used to normalize symbol names for storage/lookup.
extern std::string toupper(std::string& input);

// Public view of unresolved symbols returned by getUnresolved():
// Vector of (symbolName, set-of-SourcePos) where the set contains all known
// source positions that reference the unresolved symbol.
typedef std::vector<std::pair<std::string, std::set<SourcePos>>> symaccess;

/*
 SymTable
 --------
 Thin wrapper around std::map<std::string, Sym> providing:
  - Normalized (upper-case) symbol indexing,
  - Helper APIs used by the parser to add symbols, set values, mark macros/EQU,
  - Notification callbacks when symbols change, and
  - Convenience methods for printing and querying unresolved symbols.
*/
class SymTable {
private:
    // Core storage mapping normalized symbol name -> Sym metadata.
    std::map<std::string, Sym> symtable;

    // Registered callbacks to invoke when a symbol changes (e.g., value/EQU set).
    std::vector<std::function<void(Sym&)>> symchangedfunctions;

    // Internal helper that invokes registered callbacks for a changed symbol.
    void notifyChanged(Sym& sym);

public:
    SymTable() {}
    void clear() { symtable.clear(); }

    // Incremented when the table experiences a change. Useful to detect
    // whether another assembly pass is required.
    int changes = 0;

    // Register a callback to be invoked when any symbol changes.
    // Signature: void(Sym&).
    void addsymchanged(std::function<void(Sym&)>onSymChanged);

    // Mark the named symbol as a "var" (variable) symbol. Implementation may
    // create the symbol if not present and set appropriate flags.
    void setSymVar(std::string & name);

    // Print the symbol table to stdout. If `all` is true, include symbols
    // that are unresolved or have auxiliary flags set.
    void print(bool all) const;

    // Return a view of unresolved symbols: vector of (name, set<SourcePos>).
    symaccess getUnresolved();

    // Add a symbol reference (no value), recording the source position where it was seen.
    void add(std::string& name, SourcePos pos);

    // Add a symbol and set its integer value immediately.
    void add(std::string& name, int value, SourcePos pos);

    // Retrieve the integer value of a symbol. Behavior for undefined symbols
    // is defined by the implementation (may throw or return 0).
    int getSymValue(std::string& name, SourcePos pos);

    // Set the numeric value of an existing symbol and record the position.
    void setSymValue(std::string& name, SourcePos pos, int value);

    // Mark the symbol as an EQU (constant) definition.
    void setSymEQU(std::string& name);

    // Mark the symbol as a macro name.
    void setSymMacro(std::string& name);

    // Return true if the name is considered a label (symbol defined as a label).
    bool isLabel(std::string& name);

    // Return true if the symbol is defined in this table.
    bool isDefined(const std::string& name) const;

    // Number of symbols stored.
    size_t size() { return symtable.size(); }

    // Provide convenient access operator that normalizes the given name to upper-case
    // and returns a reference to the stored Sym object (creating it if necessary).
    Sym& operator[](std::string name)
    {
        auto uppername = toupper(name);
        Sym& sym = symtable[uppername];
        return sym;
    }
};
