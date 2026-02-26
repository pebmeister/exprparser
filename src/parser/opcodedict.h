// written by Paul Baxter
#pragma once
#include <cstdint>
#include <map>
#include <string>

#include "expr_rules.h"
#include "parser.h"

struct OpCodeInfo {
    std::string mnemonic;
    std::map<RULE_TYPE, std::pair<uint8_t, int>> mode_to_opcode;
    bool is_65c02 = false;
    bool is_illegal = false;
    std::string description;

    // Constructor for convenience
    OpCodeInfo(
        std::string mnemonic,
        std::map<RULE_TYPE, std::pair<uint8_t, int>> mode_to_opcode,
        bool is_65c02 = false,
        bool is_illegal = false,
        std::string description = ""
    )
        : mnemonic(std::move(mnemonic)),
        mode_to_opcode(std::move(mode_to_opcode)),
        is_65c02(is_65c02),
        is_illegal(is_illegal),
        description(std::move(description))
    {
    }
};

extern std::map<TOKEN_TYPE, OpCodeInfo> opcodeDict;