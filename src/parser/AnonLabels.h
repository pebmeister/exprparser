#pragma once
#include <vector>
#include <optional>
#include <stdint.h>
#include <tuple>

#include "common_types.h"
#include "symboltable.h"

class AnonLabels {
private:
    std::vector<std::tuple<SourcePos, uint16_t>> forwardLabels;
    std::vector<std::tuple<SourcePos, uint16_t>> backwardLabels;
    bool changed = false;

public:
    void add(SourcePos pos, bool forwardLabels, uint16_t value);
    std::optional<std::tuple<SourcePos, uint16_t>> find(SourcePos pos, bool forward, int count);
    bool isChanged() const { return changed; }
    void reset() { changed = false; }
};
