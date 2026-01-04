#include "AnonLabels.h"
void AnonLabels::add(SourcePos pos, bool forward, uint16_t newValue)  // Renamed parameter
{
    std::vector<std::tuple<SourcePos, uint16_t>>& labels = (forward) ? forwardLabels : backwardLabels;

    for (auto& label : labels) {
        auto& [labpos, existingValue] = label;  // Different name avoids shadowing
        if (labpos == pos) {
            if (existingValue != newValue) {
                changed = true;
                existingValue = newValue;  // Actually updates now!
            }
            return;
        }
    }

    labels.push_back(std::make_tuple(pos, newValue));
    changed = true;
}

std::optional<std::tuple<SourcePos, uint16_t>> AnonLabels::find(SourcePos pos, bool forward, int count)
{
    std::vector<std::tuple<SourcePos, uint16_t>>& labels = (forward) ? forwardLabels : backwardLabels;
    long sz = static_cast<long>(labels.size());

    auto& [curfile, curline] = pos;
    int matchesFound = 0;

    if (forward) {
        // Forward search - looking for '+' labels AFTER curline
        for (long i = 0; i < sz; ++i) {
            auto& [labpos, value] = labels[i];
            auto& [filename, line] = labpos;

            if (filename != curfile) {
                continue;
            }

            if (line > curline) {  // Must be strictly AFTER
                matchesFound++;
                if (matchesFound == count) {
                    return labels[i];
                }
            }
        }
    }
    else {
        // Backward search - looking for '-' labels BEFORE curline
        for (long i = sz - 1; i >= 0; --i) {
            auto& [labpos, value] = labels[i];
            auto& [filename, line] = labpos;

            if (filename != curfile) {
                continue;
            }

            if (line < curline) {  // Must be strictly BEFORE
                matchesFound++;
                if (matchesFound == count) {
                    return labels[i];
                }
            }
        }
    }

    return std::nullopt;
}