#pragma once

// written by Paul Baxter

#include <vector>
#include <optional>
#include <stdint.h>
#include <tuple>

#include "common_types.h"
#include "symboltable.h"

/*
 AnonLabels
 ----------
 Manage anonymous numeric labels used by the assembler.

 This class stores anonymous label definitions separated into two
 categories:
   - forwardLabels:  labels defined for forward references (e.g. "+", "+1")
   - backwardLabels: labels defined for backward references (e.g. "-", "-1")

 Each stored entry is a tuple of (SourcePos, uint16_t) where:
   - SourcePos: source location where the anonymous label was defined
   - uint16_t : the numeric value associated with the label (usually the address)

 Typical usage:
   - Call add() when an anonymous label is defined.
   - Call find() to resolve the Nth anonymous label relative to a given
     source position (forward or backward direction).
   - isChanged() / reset() are used by the assembler driver to track whether
     the label sets changed between passes (to determine whether another
     pass is needed).
*/
class AnonLabels {
private:
    // Anonymous labels defined for forward lookups.
    // Each tuple is (definition position, value/address).
    std::vector<std::tuple<SourcePos, uint16_t>> forwardLabels;

    // Anonymous labels defined for backward lookups.
    // Each tuple is (definition position, value/address).
    std::vector<std::tuple<SourcePos, uint16_t>> backwardLabels;

    // Flag set when add() mutates the label sets. Used to detect changes
    // across assembly passes.
    bool changed = false;

public:
    // Record a new anonymous label.
    // Parameters:
    //  - pos: source position of the label definition
    //  - forwardLabels: if true, add to forward label list; otherwise to backward list
    //  - value: numeric value associated with the label (typically an address)
    void add(SourcePos pos, bool forwardLabels, uint16_t value);

    // Find the 'count'-th anonymous label relative to 'pos' in the given direction.
    // Parameters:
    //  - pos: source position to search relative to
    //  - forward: if true search forwardLabels (labels defined after pos),
    //             otherwise search backwardLabels (labels defined before pos)
    //  - count: 1-based index selecting which matching anonymous label to return
    //
    // Returns:
    //  - optional tuple (SourcePos, uint16_t) for the resolved label, or
    //    std::nullopt if no matching label exists.
    std::optional<std::tuple<SourcePos, uint16_t>> find(SourcePos pos, bool forward, int count);

    // True if the label sets were modified since the last reset.
    bool isChanged() const { return changed; }

    // Clear the changed flag (called by the assembler after detecting/handling changes).
    void reset() { changed = false; }
};
