#include  <map>

#include "expr_rules.h"
#include "opcodedict.h"
#include "token.h"

/// <summary>
/// A mapping of 6502 CPU instruction tokens to their opcode information, including mnemonic, addressing modes, opcodes, instruction lengths, and descriptions.
/// </summary>
std::map<TOKEN_TYPE, OpCodeInfo> opcodeDict = {
    // ORA (Logical Inclusive OR with Accumulator)
    { ORA, OpCodeInfo{
        "ORA",
        {
            { Op_Immediate, 0x09 },
            { Op_ZeroPage,  0x05 },
            { Op_ZeroPageX, 0x15 },
            { Op_Absolute,  0x0D },
            { Op_AbsoluteX, 0x1D },
            { Op_AbsoluteY, 0x19 },
            { Op_IndirectX, 0x01 },
            { Op_IndirectY, 0x11 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Logical Inclusive OR with Accumulator"
    }},
    // AND (Logical AND with Accumulator)
    { AND, OpCodeInfo{
        "AND",
        {
            { Op_Immediate, 0x29 },
            { Op_ZeroPage,  0x25 },
            { Op_ZeroPageX, 0x35 },
            { Op_Absolute,  0x2D },
            { Op_AbsoluteX, 0x3D },
            { Op_AbsoluteY, 0x39 },
            { Op_IndirectX, 0x21 },
            { Op_IndirectY, 0x31 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Logical AND with Accumulator"
    }},
    // PHA (Push Accumulator)
    { PHA, OpCodeInfo{
        "PHA",
        {
            { Op_Implied, 0x48 }
        },
        {
            { Op_Implied, 3 }
        },
        false, // is_65c02
        false, // is_illegal
        "Push Accumulator"
    } },
    // PHP (Push Processor Status)
    { PHP, OpCodeInfo{
        "PHP",
        {
            { Op_Implied, 0x08 }
        },
        {
            { Op_Implied, 3 }
        },
        false, // is_65c02
        false, // is_illegal
        "Push Processor Status"
    } },
    // PLA (Pull Accumulator)
    { PLA, OpCodeInfo{
        "PLA",
        {
            { Op_Implied, 0x68 }
        },
        {
            { Op_Implied, 4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Pull Accumulator"
    } },
    // PLP (Pull Processor Status)
    { PLP, OpCodeInfo{
        "PLP",
        {
            { Op_Implied, 0x28 }
        },
        {
            { Op_Implied, 4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Pull Processor Status"
    } },
    // TSX (Transfer Stack Pointer to X)
    { TSX, OpCodeInfo{
        "TSX",
        {
            { Op_Implied, 0xBA }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer Stack Pointer to X"
    } },
    // TXS (Transfer X to Stack Pointer)
    { TXS, OpCodeInfo{
        "TXS",
        {
            { Op_Implied, 0x9A }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer X to Stack Pointer"
    } },
    // EOR (Exclusive OR with Accumulator)
    { EOR, OpCodeInfo{
        "EOR",
        {
            { Op_Immediate, 0x49 },
            { Op_ZeroPage,  0x45 },
            { Op_ZeroPageX, 0x55 },
            { Op_Absolute,  0x4D },
            { Op_AbsoluteX, 0x5D },
            { Op_AbsoluteY, 0x59 },
            { Op_IndirectX, 0x41 },
            { Op_IndirectY, 0x51 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Exclusive OR with Accumulator"
    }},
    // ADC (Add with Carry)
    { ADC, OpCodeInfo{
        "ADC",
        {
            { Op_Immediate, 0x69 },
            { Op_ZeroPage,  0x65 },
            { Op_ZeroPageX, 0x75 },
            { Op_Absolute,  0x6D },
            { Op_AbsoluteX, 0x7D },
            { Op_AbsoluteY, 0x79 },
            { Op_IndirectX, 0x61 },
            { Op_IndirectY, 0x71 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Add with Carry"
    }},
    // SBC (Subtract with Carry)
    { SBC, OpCodeInfo{
        "SBC",
        {
            { Op_Immediate, 0xE9 },
            { Op_ZeroPage,  0xE5 },
            { Op_ZeroPageX, 0xF5 },
            { Op_Absolute,  0xED },
            { Op_AbsoluteX, 0xFD },
            { Op_AbsoluteY, 0xF9 },
            { Op_IndirectX, 0xE1 },
            { Op_IndirectY, 0xF1 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Subtract with Carry"
    }},
    // CMP (Compare Accumulator)
    { CMP, OpCodeInfo{
        "CMP",
        {
            { Op_Immediate, 0xC9 },
            { Op_ZeroPage,  0xC5 },
            { Op_ZeroPageX, 0xD5 },
            { Op_Absolute,  0xCD },
            { Op_AbsoluteX, 0xDD },
            { Op_AbsoluteY, 0xD9 },
            { Op_IndirectX, 0xC1 },
            { Op_IndirectY, 0xD1 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Compare Accumulator"
    } },
    // CPX (Compare X Register)
    { CPX, OpCodeInfo{
        "CPX",
        {
            { Op_Immediate, 0xE0 },
            { Op_ZeroPage,  0xE4 },
            { Op_Absolute,  0xEC }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_Absolute,  4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Compare X Register"
    } },
    // CPY (Compare Y Register)
    { CPY, OpCodeInfo{
        "CPY",
        {
            { Op_Immediate, 0xC0 },
            { Op_ZeroPage,  0xC4 },
            { Op_Absolute,  0xCC }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_Absolute,  4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Compare Y Register"
    } },
    // DEC (Decrement Memory)
    { DEC, OpCodeInfo{
        "DEC",
        {
            { Op_ZeroPage,  0xC6 },
            { Op_ZeroPageX, 0xD6 },
            { Op_Absolute,  0xCE },
            { Op_AbsoluteX, 0xDE }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Decrement Memory"
    } },
    // DEX (Decrement X Register)
    { DEX, OpCodeInfo{
        "DEX",
        {
            { Op_Implied, 0xCA }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Decrement X Register"
    }},
    // DEY (Decrement Y Register)
    { DEY, OpCodeInfo{
        "DEY",
        {
            { Op_Implied, 0x88 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Decrement Y Register"
    } },
    // INC (Increment Memory)
    { INC, OpCodeInfo{
        "INC",
        {
            { Op_ZeroPage,  0xE6 },
            { Op_ZeroPageX, 0xF6 },
            { Op_Absolute,  0xEE },
            { Op_AbsoluteX, 0xFE }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Increment Memory"
    } },
    // INX (Increment X Register)
    { INX, OpCodeInfo{
        "INX",
        {
            { Op_Implied, 0xE8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Increment X Register"
    } },
    // INY (Increment Y Register)
    { INY, OpCodeInfo{
        "INY",
        {
            { Op_Implied, 0xC8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Increment Y Register"
    } },
    // ASL (Arithmetic Shift Left)
    { ASL, OpCodeInfo{
        "ASL",
        {
            { Op_Accumulator, 0x0A },
            { Op_ZeroPage,    0x06 },
            { Op_ZeroPageX,   0x16 },
            { Op_Absolute,    0x0E },
            { Op_AbsoluteX,   0x1E }
        },
        {
            { Op_Accumulator, 2 },
            { Op_ZeroPage,    5 },
            { Op_ZeroPageX,   6 },
            { Op_Absolute,    6 },
            { Op_AbsoluteX,   7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Arithmetic Shift Left"
    } },
    // ROL (Rotate Left)
    { ROL, OpCodeInfo{
        "ROL",
        {
            { Op_Accumulator, 0x2A },
            { Op_ZeroPage,    0x26 },
            { Op_ZeroPageX,   0x36 },
            { Op_Absolute,    0x2E },
            { Op_AbsoluteX,   0x3E }
        },
        {
            { Op_Accumulator, 2 },
            { Op_ZeroPage,    5 },
            { Op_ZeroPageX,   6 },
            { Op_Absolute,    6 },
            { Op_AbsoluteX,   7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Rotate Left"
    } },
    // LSR (Logical Shift Right)
    { LSR, OpCodeInfo{
        "LSR",
        {
            { Op_Accumulator, 0x4A },
            { Op_ZeroPage,    0x46 },
            { Op_ZeroPageX,   0x56 },
            { Op_Absolute,    0x4E },
            { Op_AbsoluteX,   0x5E }
        },
        {
            { Op_Accumulator, 2 },
            { Op_ZeroPage,    5 },
            { Op_ZeroPageX,   6 },
            { Op_Absolute,    6 },
            { Op_AbsoluteX,   7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Logical Shift Right"
    } },
    // ROR (Rotate Right)
    { ROR, OpCodeInfo{
        "ROR",
        {
            { Op_Accumulator, 0x6A },
            { Op_ZeroPage,    0x66 },
            { Op_ZeroPageX,   0x76 },
            { Op_Absolute,    0x6E },
            { Op_AbsoluteX,   0x7E }
        },
        {
            { Op_Accumulator, 2 },
            { Op_ZeroPage,    5 },
            { Op_ZeroPageX,   6 },
            { Op_Absolute,    6 },
            { Op_AbsoluteX,   7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Rotate Right"
    } },
    // LDA (Load Accumulator)
    { LDA, OpCodeInfo{
        "LDA",
        {
            { Op_Immediate, 0xA9 },
            { Op_ZeroPage,  0xA5 },
            { Op_ZeroPageX, 0xB5 },
            { Op_Absolute,  0xAD },
            { Op_AbsoluteX, 0xBD },
            { Op_AbsoluteY, 0xB9 },
            { Op_IndirectX, 0xA1 },
            { Op_IndirectY, 0xB1 }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 }, // +1 if page crossed
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Load Accumulator"
    } },
    // STA (Store Accumulator)
    { STA, OpCodeInfo{
        "STA",
        {
            { Op_ZeroPage,  0x85 },
            { Op_ZeroPageX, 0x95 },
            { Op_Absolute,  0x8D },
            { Op_AbsoluteX, 0x9D },
            { Op_AbsoluteY, 0x99 },
            { Op_IndirectX, 0x81 },
            { Op_IndirectY, 0x91 }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 5 },
            { Op_AbsoluteY, 5 },
            { Op_IndirectX, 6 },
            { Op_IndirectY, 6 }
        },
        false, // is_65c02
        false, // is_illegal
        "Store Accumulator"
    } },
    // LDX (Load X Register)
    { LDX, OpCodeInfo{
        "LDX",
        {
            { Op_Immediate, 0xA2 },
            { Op_ZeroPage,  0xA6 },
            { Op_ZeroPageY, 0xB6 },
            { Op_Absolute,  0xAE },
            { Op_AbsoluteY, 0xBE }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageY, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteY, 4 } // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Load X Register"
    } },
    // STX (Store X Register)
    { STX, OpCodeInfo{
        "STX",
        {
            { Op_ZeroPage,  0x86 },
            { Op_ZeroPageY, 0x96 },
            { Op_Absolute,  0x8E }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageY, 4 },
            { Op_Absolute,  4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Store X Register"
    } },
    // LDY (Load Y Register)
    { LDY, OpCodeInfo{
        "LDY",
        {
            { Op_Immediate, 0xA0 },
            { Op_ZeroPage,  0xA4 },
            { Op_ZeroPageX, 0xB4 },
            { Op_Absolute,  0xAC },
            { Op_AbsoluteX, 0xBC }
        },
        {
            { Op_Immediate, 2 },
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 4 } // +1 if page crossed
        },
        false, // is_65c02
        false, // is_illegal
        "Load Y Register"
    } },
    // STY (Store Y Register)
    { STY, OpCodeInfo{
        "STY",
        {
            { Op_ZeroPage,  0x84 },
            { Op_ZeroPageX, 0x94 },
            { Op_Absolute,  0x8C }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 }
        },
        false, // is_65c02
        false, // is_illegal
        "Store Y Register"
    } },
    // RMB0 (Reset Memory Bit 0, 65C02 only)
    { RMB0, OpCodeInfo{
        "RMB0",
        {
            { Op_ZeroPage, 0x07 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 0 (65C02 only)"
    } },
    // RMB1 (Reset Memory Bit 1, 65C02 only)
    { RMB1, OpCodeInfo{
        "RMB1",
        {
            { Op_ZeroPage, 0x17 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 1 (65C02 only)"
    } },
    // RMB2 (Reset Memory Bit 2, 65C02 only)
    { RMB2, OpCodeInfo{
        "RMB2",
        {
            { Op_ZeroPage, 0x27 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 2 (65C02 only)"
    } },
    // RMB3 (Reset Memory Bit 3, 65C02 only)
    { RMB3, OpCodeInfo{
        "RMB3",
        {
            { Op_ZeroPage, 0x37 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 3 (65C02 only)"
    } },
    // RMB4 (Reset Memory Bit 4, 65C02 only)
    { RMB4, OpCodeInfo{
        "RMB4",
        {
            { Op_ZeroPage, 0x47 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 4 (65C02 only)"
    } },
    // RMB5 (Reset Memory Bit 5, 65C02 only)
    { RMB5, OpCodeInfo{
        "RMB5",
        {
            { Op_ZeroPage, 0x57 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 5 (65C02 only)"
    } },
    // RMB6 (Reset Memory Bit 6, 65C02 only)
    { RMB6, OpCodeInfo{
        "RMB6",
        {
            { Op_ZeroPage, 0x67 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 6 (65C02 only)"
    } },
    // RMB7 (Reset Memory Bit 7, 65C02 only)
    { RMB7, OpCodeInfo{
        "RMB7",
        {
            { Op_ZeroPage, 0x77 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Reset Memory Bit 7 (65C02 only)"
    } },
    // SMB0 (Set Memory Bit 0, 65C02 only)
    { SMB0, OpCodeInfo{
        "SMB0",
        {
            { Op_ZeroPage, 0x87 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 0 (65C02 only)"
    } },
    // SMB1 (Set Memory Bit 1, 65C02 only)
    { SMB1, OpCodeInfo{
        "SMB1",
        {
            { Op_ZeroPage, 0x97 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 1 (65C02 only)"
    } },
    // SMB2 (Set Memory Bit 2, 65C02 only)
    { SMB2, OpCodeInfo{
        "SMB2",
        {
            { Op_ZeroPage, 0xA7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 2 (65C02 only)"
    } },

    // SMB3 (Set Memory Bit 3, 65C02 only)
    { SMB3, OpCodeInfo{
        "SMB3",
        {
            { Op_ZeroPage, 0xB7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 3 (65C02 only)"
    } },
    // SMB4 (Set Memory Bit 4, 65C02 only)
    { SMB4, OpCodeInfo{
        "SMB4",
        {
            { Op_ZeroPage, 0xC7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 4 (65C02 only)"
    } },
    // SMB5 (Set Memory Bit 5, 65C02 only)
    { SMB5, OpCodeInfo{
        "SMB5",
        {
            { Op_ZeroPage, 0xD7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 5 (65C02 only)"
    } },
    // SMB6 (Set Memory Bit 6, 65C02 only)
    { SMB6, OpCodeInfo{
        "SMB6",
        {
            { Op_ZeroPage, 0xE7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 6 (65C02 only)"
    } },
    // SMB7 (Set Memory Bit 7, 65C02 only)
    { SMB7, OpCodeInfo{
        "SMB7",
        {
            { Op_ZeroPage, 0xF7 }
        },
        {
            { Op_ZeroPage, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Set Memory Bit 7 (65C02 only)"
    } },
    // STZ (Store Zero, 65C02 only)
    { STZ, OpCodeInfo{
        "STZ",
        {
            { Op_ZeroPage,  0x64 },
            { Op_ZeroPageX, 0x74 },
            { Op_Absolute,  0x9C },
            { Op_AbsoluteX, 0x9E }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageX, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteX, 5 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Store Zero (65C02 only)"
    } },
    // TAX (Transfer Accumulator to X)
    { TAX, OpCodeInfo{
        "TAX",
        {
            { Op_Implied, 0xAA }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer Accumulator to X"
    } },
    // TXA (Transfer X to Accumulator)
    { TXA, OpCodeInfo{
        "TXA",
        {
            { Op_Implied, 0x8A }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer X to Accumulator"
    } },
    // TAY (Transfer Accumulator to Y)
    { TAY, OpCodeInfo{
        "TAY",
        {
            { Op_Implied, 0xA8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer Accumulator to Y"
    } },
    // TYA (Transfer Y to Accumulator)
    { TYA, OpCodeInfo{
        "TYA",
        {
            { Op_Implied, 0x98 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Transfer Y to Accumulator"
    } },
    // BRA (Branch Always, 65C02 only)
    { BRA, OpCodeInfo{
        "BRA",
        {
            { Op_Relative, 0x80 }
        },
        {
            { Op_Relative, 3 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch Always (65C02 only)"
    } },
    // BPL (Branch if Positive)
    { BPL, OpCodeInfo{
        "BPL",
        {
            { Op_Relative, 0x10 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Positive (N=0)"
    } },
    // BMI (Branch if Minus)
    { BMI, OpCodeInfo{
        "BMI",
        {
            { Op_Relative, 0x30 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Minus (N=1)"
    } },
    // BVC (Branch if Overflow Clear)
    { BVC, OpCodeInfo{
        "BVC",
        {
            { Op_Relative, 0x50 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Overflow Clear (V=0)"
    } },
    // BVS (Branch if Overflow Set)
    { BVS, OpCodeInfo{
        "BVS",
        {
            { Op_Relative, 0x70 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Overflow Set (V=1)"
    } },
    // BCC (Branch if Carry Clear)
    { BCC, OpCodeInfo{
        "BCC",
        {
            { Op_Relative, 0x90 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Carry Clear (C=0)"
    } },
    // BCS (Branch if Carry Set)
    { BCS, OpCodeInfo{
        "BCS",
        {
            { Op_Relative, 0xB0 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Carry Set (C=1)"
    } },
    // BNE (Branch if Not Equal)
    { BNE, OpCodeInfo{
        "BNE",
        {
            { Op_Relative, 0xD0 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Not Equal (Z=0)"
    } },
    // BEQ (Branch if Equal)
    { BEQ, OpCodeInfo{
        "BEQ",
        {
            { Op_Relative, 0xF0 }
        },
        {
            { Op_Relative, 2 } // +1 if page crossed, +2 if branch taken
        },
        false, // is_65c02
        false, // is_illegal
        "Branch if Equal (Z=1)"
    } },
    // BBR0 (Branch if Bit 0 Reset, 65C02 only)
    { BBR0, OpCodeInfo{
        "BBR0",
        {
            { Op_ZeroPageRelative, 0x0F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 0 Reset (65C02 only)"
    } },
    // BBR1 (Branch if Bit 1 Reset, 65C02 only)
    { BBR1, OpCodeInfo{
        "BBR1",
        {
            { Op_ZeroPageRelative, 0x1F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 1 Reset (65C02 only)"
    } },
    // BBR2 (Branch if Bit 2 Reset, 65C02 only)
    { BBR2, OpCodeInfo{
        "BBR2",
        {
            { Op_ZeroPageRelative, 0x2F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 2 Reset (65C02 only)"
    } },
    // BBR3 (Branch if Bit 3 Reset, 65C02 only)
    { BBR3, OpCodeInfo{
        "BBR3",
        {
            { Op_ZeroPageRelative, 0x3F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 3 Reset (65C02 only)"
    } },
    // BBR4 (Branch if Bit 4 Reset, 65C02 only)
    { BBR4, OpCodeInfo{
        "BBR4",
        {
            { Op_ZeroPageRelative, 0x4F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 4 Reset (65C02 only)"
    } },
    // BBR5 (Branch if Bit 5 Reset, 65C02 only)
    { BBR5, OpCodeInfo{
        "BBR5",
        {
            { Op_ZeroPageRelative, 0x5F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 5 Reset (65C02 only)"
    } },
    // BBR6 (Branch if Bit 6 Reset, 65C02 only)
    { BBR6, OpCodeInfo{
        "BBR6",
        {
            { Op_ZeroPageRelative, 0x6F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 6 Reset (65C02 only)"
    } },
    // BBR7 (Branch if Bit 7 Reset, 65C02 only)
    { BBR7, OpCodeInfo{
        "BBR7",
        {
            { Op_ZeroPageRelative, 0x7F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 7 Reset (65C02 only)"
    } },
    // BBS0 (Branch if Bit 0 Set, 65C02 only)
    { BBS0, OpCodeInfo{
        "BBS0",
        {
            { Op_ZeroPageRelative, 0x8F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 0 Set (65C02 only)"
    } },
    // BBS1 (Branch if Bit 1 Set, 65C02 only)
    { BBS1, OpCodeInfo{
        "BBS1",
        {
            { Op_ZeroPageRelative, 0x9F }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 1 Set (65C02 only)"
    } },
    // BBS2 (Branch if Bit 2 Set, 65C02 only)
    { BBS2, OpCodeInfo{
        "BBS2",
        {
            { Op_ZeroPageRelative, 0xAF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 2 Set (65C02 only)"
    } },
    // BBS3 (Branch if Bit 3 Set, 65C02 only)
    { BBS3, OpCodeInfo{
        "BBS3",
        {
            { Op_ZeroPageRelative, 0xBF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 3 Set (65C02 only)"
    } },
    // BBS4 (Branch if Bit 4 Set, 65C02 only)
    { BBS4, OpCodeInfo{
        "BBS4",
        {
            { Op_ZeroPageRelative, 0xCF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 4 Set (65C02 only)"
    } },
    // BBS5 (Branch if Bit 5 Set, 65C02 only)
    { BBS5, OpCodeInfo{
        "BBS5",
        {
            { Op_ZeroPageRelative, 0xDF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 5 Set (65C02 only)"
    } },
    // BBS6 (Branch if Bit 6 Set, 65C02 only)
    { BBS6, OpCodeInfo{
        "BBS6",
        {
            { Op_ZeroPageRelative, 0xEF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 6 Set (65C02 only)"
    } },
    // BBS7 (Branch if Bit 7 Set, 65C02 only)
    { BBS7, OpCodeInfo{
        "BBS7",
        {
            { Op_ZeroPageRelative, 0xFF }
        },
        {
            { Op_ZeroPageRelative, 5 } // +1 if page crossed, +2 if branch taken
        },
        true,  // is_65c02
        false, // is_illegal
        "Branch if Bit 7 Set (65C02 only)"
    } },
    
    // STP (Stop, WDC 65C02 only)
    { STP, OpCodeInfo{
        "STP",
        {
            { Op_Implied, 0xDB }
        },
        {
            { Op_Implied, 3 } // Actually stops the processor
        },
        true,  // is_65c02
        false, // is_illegal (though WDC-specific)
        "Stop the Processor (WDC 65C02 only)"
    } },
    // WAI (Wait for Interrupt, 65C02 only)
    { WAI, OpCodeInfo{
        "WAI",
        {
            { Op_Implied, 0xCB }
        },
        {
            { Op_Implied, 3 } // Plus interrupt latency
        },
        true,  // is_65c02
        false, // is_illegal
        "Wait for Interrupt (65C02 only)"
    } },
    // BRK (Break/Interrupt)
    { BRK, OpCodeInfo{
        "BRK",
        {
            { Op_Implied, 0x00 }
        },
        {
            { Op_Implied, 7 }
        },
        false, // is_65c02
        false, // is_illegal
        "Break/Interrupt"
    } },
    // RTI (Return from Interrupt)
    { RTI, OpCodeInfo{
        "RTI",
        {
            { Op_Implied, 0x40 }
        },
        {
            { Op_Implied, 6 }
        },
        false, // is_65c02
        false, // is_illegal
        "Return from Interrupt"
    } },
    // JSR (Jump to Subroutine)
    { JSR, OpCodeInfo{
        "JSR",
        {
            { Op_Absolute, 0x20 }
        },
        {
            { Op_Absolute, 6 }
        },
        false, // is_65c02
        false, // is_illegal
        "Jump to Subroutine"
    } },
    // RTS (Return from Subroutine)
    { RTS, OpCodeInfo{
        "RTS",
        {
            { Op_Implied, 0x60 }
        },
        {
            { Op_Implied, 6 }
        },
        false, // is_65c02
        false, // is_illegal
        "Return from Subroutine"
    } },
    // JMP (Jump)
    { JMP, OpCodeInfo{
        "JMP",
        {
            { Op_Absolute,  0x4C },
            { Op_Indirect,  0x6C },
            { Op_IndirectX, 0x7C } // 65C02 only
        },
        {
            { Op_Absolute,  3 },
            { Op_Indirect,  5 },
            { Op_IndirectX, 6 } // 65C02 only
        },
        false, // is_65c02 (partially)
        false, // is_illegal
        "Jump"
    } },
    // BIT (Test Bits in Memory with Accumulator)
    { BIT, OpCodeInfo{
        "BIT",
        {
            { Op_ZeroPage,  0x24 },
            { Op_Absolute,  0x2C },
            { Op_Immediate, 0x89 }, // 65C02 only
            { Op_ZeroPageX, 0x34 }, // 65C02 only
            { Op_AbsoluteX, 0x3C }  // 65C02 only
        },
        {
            { Op_ZeroPage,  3 },
            { Op_Absolute,  4 },
            { Op_Immediate, 2 }, // 65C02 only
            { Op_ZeroPageX, 4 }, // 65C02 only
            { Op_AbsoluteX, 4 }  // 65C02 only (+1 if page crossed)
        },
        false, // is_65c02 (partially)
        false, // is_illegal
        "Test Bits in Memory with Accumulator"
    } },
    // TRB (Test and Reset Bits, 65C02 only)
    { TRB, OpCodeInfo{
        "TRB",
        {
            { Op_ZeroPage, 0x14 },
            { Op_Absolute, 0x1C }
        },
        {
            { Op_ZeroPage, 5 },
            { Op_Absolute, 6 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Test and Reset Bits (65C02 only)"
    } },
    // TSB (Test and Set Bits, 65C02 only)
    { TSB, OpCodeInfo{
        "TSB",
        {
            { Op_ZeroPage, 0x04 },
            { Op_Absolute, 0x0C }
        },
        {
            { Op_ZeroPage, 5 },
            { Op_Absolute, 6 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Test and Set Bits (65C02 only)"
    } },

    // CLC (Clear Carry Flag)
    { CLC, OpCodeInfo{
        "CLC",
        {
            { Op_Implied, 0x18 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Clear Carry Flag"
    } },
    // SEC (Set Carry Flag)
    { SEC, OpCodeInfo{
        "SEC",
        {
            { Op_Implied, 0x38 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Set Carry Flag"
    } },
    // CLD (Clear Decimal Mode)
    { CLD, OpCodeInfo{
        "CLD",
        {
            { Op_Implied, 0xD8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Clear Decimal Mode"
    } },
    // SED (Set Decimal Mode)
    { SED, OpCodeInfo{
        "SED",
        {
            { Op_Implied, 0xF8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Set Decimal Mode"
    } },
    // CLI (Clear Interrupt Disable)
    { CLI, OpCodeInfo{
        "CLI",
        {
            { Op_Implied, 0x58 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Clear Interrupt Disable"
    } },
    // SEI (Set Interrupt Disable)
    { SEI, OpCodeInfo{
        "SEI",
        {
            { Op_Implied, 0x78 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Set Interrupt Disable"
    } },
    // CLV (Clear Overflow Flag)
    { CLV, OpCodeInfo{
        "CLV",
        {
            { Op_Implied, 0xB8 }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "Clear Overflow Flag"
    } },
    // NOP (No Operation)
    { NOP, OpCodeInfo{
        "NOP",
        {
            { Op_Implied, 0xEA }
        },
        {
            { Op_Implied, 2 }
        },
        false, // is_65c02
        false, // is_illegal
        "No Operation"
    } },
    // SLO (ASL + ORA - Illegal)
    { SLO, OpCodeInfo{
        "SLO",
        {
            { Op_ZeroPage,  0x07 },
            { Op_ZeroPageX, 0x17 },
            { Op_Absolute,  0x0F },
            { Op_AbsoluteX, 0x1F },
            { Op_AbsoluteY, 0x1B },
            { Op_IndirectX, 0x03 },
            { Op_IndirectY, 0x13 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "ASL then ORA (Illegal)"
    } },
    // RLA (ROL + AND - Illegal)
    { RLA, OpCodeInfo{
        "RLA",
        {
            { Op_ZeroPage,  0x27 },
            { Op_ZeroPageX, 0x37 },
            { Op_Absolute,  0x2F },
            { Op_AbsoluteX, 0x3F },
            { Op_AbsoluteY, 0x3B },
            { Op_IndirectX, 0x23 },
            { Op_IndirectY, 0x33 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "ROL then AND (Illegal)"
    } },
    // SRE (LSR + EOR - Illegal)
    { SRE, OpCodeInfo{
        "SRE",
        {
            { Op_ZeroPage,  0x47 },
            { Op_ZeroPageX, 0x57 },
            { Op_Absolute,  0x4F },
            { Op_AbsoluteX, 0x5F },
            { Op_AbsoluteY, 0x5B },
            { Op_IndirectX, 0x43 },
            { Op_IndirectY, 0x53 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "LSR then EOR (Illegal)"
    } },
    // RRA (ROR + ADC - Illegal)
    { RRA, OpCodeInfo{
        "RRA",
        {
            { Op_ZeroPage,  0x67 },
            { Op_ZeroPageX, 0x77 },
            { Op_Absolute,  0x6F },
            { Op_AbsoluteX, 0x7F },
            { Op_AbsoluteY, 0x7B },
            { Op_IndirectX, 0x63 },
            { Op_IndirectY, 0x73 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "ROR then ADC (Illegal)"
    } },
    // SAX (STA + STX - Illegal)
    { SAX, OpCodeInfo{
        "SAX",
        {
            { Op_ZeroPage,  0x87 },
            { Op_ZeroPageY, 0x97 },
            { Op_Absolute,  0x8F },
            { Op_IndirectX, 0x83 }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageY, 4 },
            { Op_Absolute,  4 },
            { Op_IndirectX, 6 }
        },
        false, // is_65c02
        true,  // is_illegal
        "STA AND STX (Illegal)"
    } },
    // LAX (LDA + LDX - Illegal)
    { LAX, OpCodeInfo{
        "LAX",
        {
            { Op_ZeroPage,  0xA7 },
            { Op_ZeroPageY, 0xB7 },
            { Op_Absolute,  0xAF },
            { Op_AbsoluteY, 0xBF },
            { Op_IndirectX, 0xA3 },
            { Op_IndirectY, 0xB3 }
        },
        {
            { Op_ZeroPage,  3 },
            { Op_ZeroPageY, 4 },
            { Op_Absolute,  4 },
            { Op_AbsoluteY, 4 }, // +1 if page crossed
            { Op_IndirectX, 6 },
            { Op_IndirectY, 5 }  // +1 if page crossed
        },
        false, // is_65c02
        true,  // is_illegal
        "LDA then LDX (Illegal)"
    } },
    // DCP (DEC + CMP - Illegal)
    { DCP, OpCodeInfo{
        "DCP",
        {
            { Op_ZeroPage,  0xC7 },
            { Op_ZeroPageX, 0xD7 },
            { Op_Absolute,  0xCF },
            { Op_AbsoluteX, 0xDF },
            { Op_AbsoluteY, 0xDB },
            { Op_IndirectX, 0xC3 },
            { Op_IndirectY, 0xD3 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "DEC then CMP (Illegal)"
    } },
    // ISC (INC + SBC - Illegal)
    { ISC, OpCodeInfo{
        "ISC",
        {
            { Op_ZeroPage,  0xE7 },
            { Op_ZeroPageX, 0xF7 },
            { Op_Absolute,  0xEF },
            { Op_AbsoluteX, 0xFF },
            { Op_AbsoluteY, 0xFB },
            { Op_IndirectX, 0xE3 },
            { Op_IndirectY, 0xF3 }
        },
        {
            { Op_ZeroPage,  5 },
            { Op_ZeroPageX, 6 },
            { Op_Absolute,  6 },
            { Op_AbsoluteX, 7 },
            { Op_AbsoluteY, 7 },
            { Op_IndirectX, 8 },
            { Op_IndirectY, 8 }
        },
        false, // is_65c02
        true,  // is_illegal
        "INC then SBC (Illegal)"
    } },
    // ANC (AND + CARRY - Illegal)
    { ANC, OpCodeInfo{
        "ANC",
        {
            { Op_Immediate, 0x0B },
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "AND then set CARRY (Illegal)"
    } },
    { ANC2, OpCodeInfo{
        "ANC2",
        {
            { Op_Immediate, 0x2B } // Alternate encoding
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "AND then set CARRY (Illegal)"
    } },
    // ALR (AND + LSR - Illegal)
    { ALR, OpCodeInfo{
        "ALR",
        {
            { Op_Immediate, 0x4B }
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "AND then LSR (Illegal)"
    } },
    // ARR (AND + ROR - Illegal)
    { ARR, OpCodeInfo{
        "ARR",
        {
            { Op_Immediate, 0x6B }
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "AND then ROR (Illegal)"
    } },
    // XAA (TXA + AND - Illegal)
    { XAA, OpCodeInfo{
        "XAA",
        {
            { Op_Immediate, 0x8B }
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "TXA then AND (Illegal)"
    } },
    // AXS (CMP + DEX - Illegal)
    { AXS, OpCodeInfo{
        "AXS",
        {
            { Op_Immediate, 0xCB }
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "CMP then DEX (Illegal)"
    } },
    // USBC (SBC with unstable carry - Illegal)
    { USBC, OpCodeInfo{
        "USBC",
        {
            { Op_Immediate, 0xEB }
        },
        {
            { Op_Immediate, 2 }
        },
        false, // is_65c02
        true,  // is_illegal
        "Unstable SBC (Illegal)"
    } },
    // AHX (STA + STX + STY - Illegal)
    { AHX, OpCodeInfo{
        "AHX",
        {
            { Op_AbsoluteY, 0x9F },
            { Op_IndirectY, 0x93 }
        },
        {
            { Op_AbsoluteY, 5 },
            { Op_IndirectY, 6 }
        },
        false, // is_65c02
        true,  // is_illegal
        "STA AND STX AND STY (Illegal)"
    } },
    // SHY (Store Y AND high byte of address+1 - Illegal)
    { SHY, OpCodeInfo{
        "SHY",
        {
            { Op_AbsoluteX, 0x9C }
        },
        {
            { Op_AbsoluteX, 5 }
        },
        false, // is_65c02
        true,  // is_illegal
        "Store Y AND high byte (Illegal)"
    } },
    // SHX (Store X AND high byte of address+1 - Illegal)
    { SHX, OpCodeInfo{
        "SHX",
        {
            { Op_AbsoluteY, 0x9E }
        },
        {
            { Op_AbsoluteY, 5 }
        },
        false, // is_65c02
        true,  // is_illegal
        "Store X AND high byte (Illegal)"
    } },
    // TAS (Transfer A AND X to SP - Illegal)
    { TAS, OpCodeInfo{
        "TAS",
        {
            { Op_AbsoluteY, 0x9B }
        },
        {
            { Op_AbsoluteY, 5 }
        },
        false, // is_65c02
        true,  // is_illegal
        "Transfer A AND X to SP (Illegal)"
    } },
    // LAS (LDA AND TSX - Illegal)
    { LAS, OpCodeInfo{
        "LAS",
        {
            { Op_AbsoluteY, 0xBB }
        },
        {
            { Op_AbsoluteY, 4 } // +1 if page crossed
        },
        false, // is_65c02
        true,  // is_illegal
        "LDA AND TSX (Illegal)"
    } },

    // PHX (Push X Register, 65C02 only)
    { PHX, OpCodeInfo{
        "PHX",
        {
            { Op_Implied, 0xDA }
        },
        {
            { Op_Implied, 3 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Push X Register (65C02 only)"
    } },

    // PHY (Push Y Register, 65C02 only)
    { PHY, OpCodeInfo{
        "PHY",
        {
            { Op_Implied, 0x5A }
        },
        {
            { Op_Implied, 3 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Push Y Register (65C02 only)"
    } },

    // PLX (Pull X Register, 65C02 only)
    { PLX, OpCodeInfo{
        "PLX",
        {
            { Op_Implied, 0xFA }
        },
        {
            { Op_Implied, 4 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Pull X Register (65C02 only)"
    } },

    // PLY (Pull Y Register, 65C02 only)
    { PLY, OpCodeInfo{
        "PLY",
        {
            { Op_Implied, 0x7A }
        },
        {
            { Op_Implied, 4 }
        },
        true,  // is_65c02
        false, // is_illegal
        "Pull Y Register (65C02 only)"
    } },
};