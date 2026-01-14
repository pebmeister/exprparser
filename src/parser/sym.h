#pragma once
#include <string.h>
#include <algorithm>
#include <set>
#include <iomanip>
#include <string>

class Sym {
private:
    std::vector <std::pair<SourcePos, int>> history;

public:
    std::string name;
    int value = 0;
    std::set<SourcePos>accessed;
    bool initialized = false;
    bool changed = false;
    bool isPC = false;
    bool isMacro = false;
    bool isVar = false;
    SourcePos created;

    int GetValueBefore(SourcePos pos, int iteration = 1)
    {
        if (history.empty()) return 0;

        int val = history[0].second;
        int currentIteration = 0;
        SourcePos lastPos = history[0].first;

        for (const auto& [historyPos, historyValue] : history) {
            // Reset iteration counter when we move to a new position
            if (historyPos.filename != lastPos.filename || historyPos.line != lastPos.line) {
                currentIteration = 0;
                lastPos = historyPos;
            }

            // If we've reached or passed the target position, check iteration
            if (historyPos.filename == pos.filename && historyPos.line >= pos.line) {
                // If this is the exact position and iteration we're looking for, return previous value
                if (historyPos.line == pos.line && currentIteration == iteration) {
                    return val;  // Return value BEFORE this change
                }
                // If we've gone past the position, stop searching
                if (historyPos.line > pos.line) {
                    return val;
                }
            }

            // Update value and iteration counter
            val = historyValue;
            currentIteration++;
        }

        return val;
    }

    void AddHistory(SourcePos pos, int value)
    {
        history.push_back({ pos, value });
    }

    void print()
    {
        std::string createdstr = created.filename.empty() ? "" : created.filename + " " + std::to_string(created.line);

        std::cout <<
            "\nname:        " << name <<
            "\nvalue:       " << "$" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << value 
                                    << std::dec << std::setw(0) << std::setfill(' ') <<
            "\ninitialized: " << initialized <<
            "\ncreated:     " << createdstr <<
            "\nchanged:     " << changed <<
            "\nisMacro:     " << isMacro <<
            "\nisVar        " << isVar <<
            "\nisPC         " << isPC <<
            "\naccessed:    \n";
   /*     for (auto& access : accessed) {
            std::cout << "[" << access.filename << " line " << access.line << "]\n";
        }*/
        std::cout << "\nHistory\n";
        for (auto& entry : history) {
            auto& pos = entry.first;
            auto& val = entry.second;
            std::cout << "[" << pos.filename << " line " << pos.line << "] " << val << "\n";
        }

        std::cout << "\n";
    }
};
