#include "AnonLabels.h"

void AnonLabels::add(SourcePos pos, bool forward, uint16_t value)
{
    std::vector<std::tuple<SourcePos, uint16_t>>& labels = (forward) ? forwardLabels : backwardLabels;
    for (std::tuple<SourcePos, uint16_t>& label : labels) {
        auto& [labpos, value] = label;
        if (labpos == pos) {
            auto update = std::make_tuple(labpos, value); 
            if (label != update) {
                changed = true;
                label = update;
            }
            return;
        }
    }
    auto update = std::make_tuple(pos, value);
    labels.push_back(update);
    changed = true;
}

std::optional<std::tuple<SourcePos, uint16_t>> AnonLabels::find(SourcePos pos, bool forward, int count)
{
    std::vector<std::tuple<SourcePos, uint16_t>>& labels = (forward) ? forwardLabels : backwardLabels;
    long sz = static_cast<long>(labels.size());
    
    auto& [curfile, curline] = pos;

    if (forward) {
        long i = 0;
        while (i < sz) {
            auto& [labpos, value] = labels[i];
            auto& [filename, line] = labpos;
            
            if (filename != curfile) {
                ++i;
                continue;
            }

            while (i < sz && line < curline) {
                ++i;
                auto& [labpos, value] = labels[i];
                auto& [filename, line] = labpos;
            }
            i += (count - 1);
            if (i >= sz) {
                continue;
            }

            return labels[i];
        }
        return std::nullopt;
    }
    else {
        long i = sz -1;
        while (i >= 0) {
            auto& [labpos, value] = labels[i];
            auto& [filename, line] = labpos;

            if (filename != curfile) {
                --i;
                continue;
            }

            while (i > 0 && line >= curline) {
                --i;
                auto& [labpos, value] = labels[i];
                auto& [filename, line] = labpos;
            }
            i -= (count - 1);
            if (i < 0) {
                continue;
            }

            return labels[i];
        }
        return std::nullopt;
    }
}
