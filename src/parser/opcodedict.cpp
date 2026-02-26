// written by Paul Baxter
#include  <map>

#include "expr_rules.h"
#include "opcodedict.h"
#include "token.h"

/// <summary>
/// A mapping of 6502 CPU instruction tokens to their opcode information, including mnemonic, addressing modes, opcodes, instruction lengths, and descriptions.
/// </summary>
std::map<TOKEN_TYPE, OpCodeInfo> opcodeDict = {
    {
        ORA,
        OpCodeInfo
        {
            "ORA",
            {
                { Op_Immediate, { 0x09, 2} },
                { Op_ZeroPage,  { 0x05, 3} },
                { Op_ZeroPageX, { 0x15, 4} },
                { Op_Absolute,  { 0x0D, 4} },
                { Op_AbsoluteX, { 0x1D, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0x19, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0x01, 6} },
                { Op_IndirectY, { 0x11, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0x12, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Logical Inclusive OR with Accumulator"
        }
    },
    {
        AND,
        OpCodeInfo
        {
            "AND",
            {
                { Op_Immediate, { 0x29, 2} },
                { Op_ZeroPage,  { 0x25, 3} },
                { Op_ZeroPageX, { 0x35, 4} },
                { Op_Absolute,  { 0x2D, 4} },
                { Op_AbsoluteX, { 0x3D, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0x39, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0x21, 6} },
                { Op_IndirectY, { 0x31, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0x32, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Logical AND with Accumulator"
        }
    },
    {
        PHA,
        OpCodeInfo
        {
            "PHA",
            {
                { Op_Implied, { 0x48, 3} }
            },

            false, // is_65c02
            false, // is_illegal
            "Push Accumulator"
        }
    },
    {
        PHP, OpCodeInfo
        {
            "PHP",
            {
                { Op_Implied, { 0x08, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Push Processor Status"
        }
    },
    { 
        PLA, 
        OpCodeInfo
        {
            "PLA",
            {
                { Op_Implied, { 0x68, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Pull Accumulator"
        }
    },
    { 
        PLP, 
        OpCodeInfo
        {
            "PLP",
            {
                { Op_Implied, { 0x28, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Pull Processor Status"
        }
    },
    { 
        TSX, 
        OpCodeInfo
        {
            "TSX",
            {
                { Op_Implied, {0xBA, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer Stack Pointer to X"
        }
    },
    { 
        TXS, 
        OpCodeInfo
        {
            "TXS",
            {
                { Op_Implied, { 0x9A, 2 }}
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer X to Stack Pointer"
        } 
    },
    { 
        EOR, 
        OpCodeInfo
        {
            "EOR",
            {
                { Op_Immediate, { 0x49, 2} },
                { Op_ZeroPage,  { 0x45, 3} },
                { Op_ZeroPageX, { 0x55, 4} },
                { Op_Absolute,  { 0x4D, 4} },
                { Op_AbsoluteX, { 0x5D, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0x59, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0x41, 6} },
                { Op_IndirectY, { 0x51, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0x52, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Exclusive OR with Accumulator"
        }
    },
    { 
        ADC, 
        OpCodeInfo
        {
            "ADC",
            {
                { Op_Immediate, { 0x69, 2} },
                { Op_ZeroPage,  { 0x65, 3} },
                { Op_ZeroPageX, { 0x75, 4} },
                { Op_Absolute,  { 0x6D, 4} },
                { Op_AbsoluteX, { 0x7D, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0x79, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0x61, 6} },
                { Op_IndirectY, { 0x71, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0x72, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Add with Carry"
        }
    },
    { 
        SBC, 
        OpCodeInfo
        {
            "SBC",
            {
                { Op_Immediate, { 0xE9, 2} },
                { Op_ZeroPage,  { 0xE5, 3} },
                { Op_ZeroPageX, { 0xF5, 4} },
                { Op_Absolute,  { 0xED, 4} },
                { Op_AbsoluteX, { 0xFD, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0xF9, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0xE1, 6} },
                { Op_IndirectY, { 0xF1, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0xF2, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Subtract with Carry"
        }
    },
    { 
        CMP, 
        OpCodeInfo
        {
            "CMP",
            {
                { Op_Immediate, { 0xC9, 2} },
                { Op_ZeroPage,  { 0xC5, 3} },
                { Op_ZeroPageX, { 0xD5, 4} },
                { Op_Absolute,  { 0xCD, 4} },
                { Op_AbsoluteX, { 0xDD, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0xD9, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0xC1, 6} },
                { Op_IndirectY, { 0xD1, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0xD2, 6} }
            },
            false, // is_65c02
            false, // is_illegal
            "Compare Accumulator"
        }
    },
    { 
        CPX, 
        OpCodeInfo
        {
            "CPX",
            {
                { Op_Immediate, { 0xE0, 2} },
                { Op_ZeroPage,  { 0xE4, 3} },
                { Op_Absolute,  { 0xEC, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Compare X Register"
        } 
    },
    { 
        CPY, 
        OpCodeInfo
        {
            "CPY",
            {
                { Op_Immediate, { 0xC0, 2} },
                { Op_ZeroPage,  { 0xC4, 3} },
                { Op_Absolute,  { 0xCC, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Compare Y Register"
        } 
    },
    { 
        DEC, 
        OpCodeInfo
        {
            "DEC",
            {
                { Op_ZeroPage,  { 0xC6, 5} },
                { Op_ZeroPageX, { 0xD6, 6} },
                { Op_Absolute,  { 0xCE, 6} },
                { Op_AbsoluteX, { 0xDE, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Decrement Memory"
        } 
    },
    { 
        DEX, 
        OpCodeInfo
        {
            "DEX",
            {
                { Op_Implied, { 0xCA, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Decrement X Register"
        }
    },
    { 
        DEY, 
        OpCodeInfo
        {
            "DEY",
            {
                { Op_Implied, { 0x88, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Decrement Y Register"
        } 
    },
    { 
        INC, 
        OpCodeInfo
        {
            "INC",
            {
                { Op_ZeroPage,  { 0xE6, 5} },
                { Op_ZeroPageX, { 0xF6, 6} },
                { Op_Absolute,  { 0xEE, 6} },
                { Op_AbsoluteX, { 0xFE, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Increment Memory"
        } 
    },
    { 
        INX, 
        OpCodeInfo
        {
            "INX",
            {
                { Op_Implied, { 0xE8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Increment X Register"
        } 
    },
    { 
        INY, 
        OpCodeInfo
        {
            "INY",
            {
                { Op_Implied, { 0xC8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Increment Y Register"
        } 
    },
    { 
        ASL, 
        OpCodeInfo
        {
            "ASL",
            {
                { Op_Accumulator, { 0x0A, 2} },
                { Op_ZeroPage,    { 0x06, 5} },
                { Op_ZeroPageX,   { 0x16, 6} },
                { Op_Absolute,    { 0x0E, 6} },
                { Op_AbsoluteX,   { 0x1E, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Arithmetic Shift Left"
        } 
    },
    { 
        ROL, 
        OpCodeInfo
        {
            "ROL",
            {
                { Op_Accumulator, { 0x2A, 2} },
                { Op_ZeroPage,    { 0x26, 5} },
                { Op_ZeroPageX,   { 0x36, 6} },
                { Op_Absolute,    { 0x2E, 6} },
                { Op_AbsoluteX,   { 0x3E, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Rotate Left"
        } 
    },
    { 
        LSR, 
        OpCodeInfo
        {
            "LSR",
            {
                { Op_Accumulator, { 0x4A, 2} },
                { Op_ZeroPage,    { 0x46, 5} },
                { Op_ZeroPageX,   { 0x56, 6} },
                { Op_Absolute,    { 0x4E, 6} },
                { Op_AbsoluteX,   { 0x5E, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Logical Shift Right"
        } 
    },
    { 
        ROR, 
        OpCodeInfo
        {
            "ROR",
            {
                { Op_Accumulator, { 0x6A, 2} },
                { Op_ZeroPage,    { 0x66, 5} },
                { Op_ZeroPageX,   { 0x76, 6} },
                { Op_Absolute,    { 0x6E, 6} },
                { Op_AbsoluteX,   { 0x7E, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Rotate Right"
        }
    },
    { 
        LDA, 
        OpCodeInfo
        {
            "LDA",
            {
                { Op_Immediate, { 0xA9, 2} },
                { Op_ZeroPage,  { 0xA5, 3} },
                { Op_ZeroPageX, { 0xB5, 4} },
                { Op_Absolute,  { 0xAD, 4} },
                { Op_AbsoluteX, { 0xBD, 4} }, // +1 if page crossed
                { Op_AbsoluteY, { 0xB9, 4} }, // +1 if page crossed
                { Op_IndirectX, { 0xA1, 6} },
                { Op_IndirectY, { 0xB1, 5} }, // +1 if page crossed
                { Op_Indirect,  { 0xB2, 5} }  // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Load Accumulator"
        } 
    },
    { 
        STA, 
        OpCodeInfo
        {
            "STA",
            {
                { Op_ZeroPage,  { 0x85, 3} },
                { Op_ZeroPageX, { 0x95, 4} },
                { Op_Absolute,  { 0x8D, 5} },
                { Op_AbsoluteX, { 0x9D, 5} },
                { Op_AbsoluteY, { 0x99, 6} },
                { Op_IndirectX, { 0x81, 6} },
                { Op_IndirectY, { 0x91, 6} },
                { Op_Indirect,  { 0x92, 6} }
            },
            false, // is_65c02
            false, // is_illegal
            "Store Accumulator"
        } 
    },
    { 
        LDX, 
        OpCodeInfo
        {
            "LDX",
            {
                { Op_Immediate, { 0xA2, 2} },
                { Op_ZeroPage,  { 0xA6, 3} },
                { Op_ZeroPageY, { 0xB6, 4} },
                { Op_Absolute,  { 0xAE, 4} },
                { Op_AbsoluteY, { 0xBE, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Load X Register"
        }
    },
    {
        STX, 
        OpCodeInfo
        {
            "STX",
            {
                { Op_ZeroPage,  { 0x86, 3} },
                { Op_ZeroPageY, { 0x96, 4} },
                { Op_Absolute,  { 0x8E, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Store X Register"
        } 
    },
    { 
        LDY, 
        OpCodeInfo
        {
            "LDY",
            {
                { Op_Immediate, { 0xA0, 2} },
                { Op_ZeroPage,  { 0xA4, 3} },
                { Op_ZeroPageX, { 0xB4, 4} },
                { Op_Absolute,  { 0xAC, 4} },
                { Op_AbsoluteX, { 0xBC, 4} } // +1 if page crossed
            },
            false, // is_65c02
            false, // is_illegal
            "Load Y Register"
        }
    },
    { 
        STY, 
        OpCodeInfo
        {
            "STY",
            {
                { Op_ZeroPage,  { 0x84, 3} },
                { Op_ZeroPageX, { 0x94, 4} },
                { Op_Absolute,  { 0x8C, 4} }
            },
            false, // is_65c02
            false, // is_illegal
            "Store Y Register"
        } 
    },
    { 
        RMB0, 
        OpCodeInfo
        {
            "RMB0",
            {
                { Op_ZeroPage, {0x07, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 0 (65C02 only)"
        } 
    },
    { 
        RMB1, 
        OpCodeInfo
        {
            "RMB1",
            {
                { Op_ZeroPage, { 0x17, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 1 (65C02 only)"
        }
    },
    { 
        RMB2, 
        OpCodeInfo
        {
            "RMB2",
            {
                { Op_ZeroPage, { 0x27, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 2 (65C02 only)"
        } 
    },
    { 
        RMB3, 
        OpCodeInfo
        {
            "RMB3",
            {
                { Op_ZeroPage, { 0x37, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 3 (65C02 only)"
        }
    },
    { 
        RMB4, 
        OpCodeInfo
        {
            "RMB4",
            {
                { Op_ZeroPage, { 0x47, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 4 (65C02 only)"
        }
    },
    { 
        RMB5, 
        OpCodeInfo
        {
            "RMB5",
            {
                { Op_ZeroPage, { 0x57, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 5 (65C02 only)"
        }
    },
    { 
        RMB6, 
        OpCodeInfo
        {
            "RMB6",
            {
                { Op_ZeroPage, { 0x67, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 6 (65C02 only)"
        }
    },
    { 
        RMB7, 
        OpCodeInfo
        {
            "RMB7",
            {
                { Op_ZeroPage, { 0x77, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Reset Memory Bit 7 (65C02 only)"
        }
    },
    { 
        SMB0, 
        OpCodeInfo
        {
            "SMB0",
            {
                { Op_ZeroPage, { 0x87, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 0 (65C02 only)"
        } 
    },
    { 
        SMB1, 
        OpCodeInfo
        {
            "SMB1",
            {
                { Op_ZeroPage, { 0x97, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 1 (65C02 only)"
        }
    },
    { 
        SMB2, 
        OpCodeInfo
            {
            "SMB2",
            {
                { Op_ZeroPage, { 0xA7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 2 (65C02 only)"
        } 
    },
    { 
        SMB3, 
        OpCodeInfo
        {
            "SMB3",
            {
                { Op_ZeroPage, { 0xB7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 3 (65C02 only)"
        } 
    },
    { 
        SMB4, 
        OpCodeInfo
        {
            "SMB4",
            {
                { Op_ZeroPage, { 0xC7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 4 (65C02 only)"
        } 
    },
    { 
        SMB5, 
        OpCodeInfo
        {
        "SMB5",
            {
                { Op_ZeroPage, { 0xD7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 5 (65C02 only)"
        }
    },
    { 
        SMB6, 
        OpCodeInfo
        {
            "SMB6",
            {
                { Op_ZeroPage, { 0xE7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 6 (65C02 only)"
        }
    },
    { 
        SMB7, 
        OpCodeInfo
        {
            "SMB7",
            {
                { Op_ZeroPage, { 0xF7, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Set Memory Bit 7 (65C02 only)"
        } 
    },
    { 
        STZ, 
        OpCodeInfo
        {
            "STZ",
            {
                { Op_ZeroPage,  { 0x64, 3} },
                { Op_ZeroPageX, { 0x74, 4} },
                { Op_Absolute,  { 0x9C, 4} },
                { Op_AbsoluteX, { 0x9E, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Store Zero (65C02 only)"
        }
    },
    {
        TRB, 
        OpCodeInfo
        {
            "TRB",
            {
                { Op_ZeroPage, { 0x14, 5} },
                { Op_Absolute, { 0x1C, 6} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Test and Reset Bits (65C02 only)"
        } 
    },
    { 
        TSB, 
        OpCodeInfo
        {
            "TSB",
            {
                { Op_ZeroPage, { 0x40, 5} },
                { Op_Absolute, { 0xC0, 6} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Test and Set Bits (65C02 only)"
        } 
    },
    { 
        TAX, 
        OpCodeInfo
        {
            "TAX",
            {
                { Op_Implied, { 0xAA, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer Accumulator to X"
        }
    },
    {
        TXA, 
        OpCodeInfo
        {
            "TXA",
            {
                { Op_Implied, { 0x8A, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer X to Accumulator"
        }
    },
    { 
        TAY, 
        OpCodeInfo
        {
            "TAY",
            {
                { Op_Implied, { 0xA8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer Accumulator to Y"
        } 
    },
    { 
        TYA, 
        OpCodeInfo
        {
            "TYA",
            {
                { Op_Implied, { 0x98, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Transfer Y to Accumulator"
        } 
    },
    { 
        BRA, 
        OpCodeInfo
        {
            "BRA",
            {
                { Op_Relative, { 0x80, 3} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch Always (65C02 only)"
        } 
    },
    { 
        BPL, 
        OpCodeInfo
        {
            "BPL",
            {
                { Op_Relative, { 0x10, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Positive (N=0)"
        } 
    },
    { 
        BMI, 
        OpCodeInfo
        {
            "BMI",
            {
                { Op_Relative, { 0x30, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Minus (N=1)"
        } 
    },
    { 
        BVC, 
        OpCodeInfo
        {
            "BVC",
            {
                { Op_Relative, {0x50,  2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Overflow Clear (V=0)"
        } 
    },
    { 
        BVS, 
        OpCodeInfo
        {
            "BVS",
            {
                { Op_Relative, { 0x70, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Overflow Set (V=1)"
        } 
    },
    { 
        BCC, 
        OpCodeInfo
        {
            "BCC",
            {
                { Op_Relative, { 0x90, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Carry Clear (C=0)"
        }
    },
    { 
        BCS, 
        OpCodeInfo
        {
            "BCS",
            {
                { Op_Relative, { 0xB0, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Carry Set (C=1)"
        } 
    },
    { 
        BNE, 
        OpCodeInfo
        {
            "BNE",
            {
                { Op_Relative, { 0xD0, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Not Equal (Z=0)"
        } 
    },
    { 
        BEQ, 
        OpCodeInfo
        {
            "BEQ",
            {
                { Op_Relative, { 0xF0, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Branch if Equal (Z=1)"
        } 
    },
    { 
        BBR0, 
        OpCodeInfo
        {
            "BBR0",
            {
                { Op_ZeroPageRelative, { 0x0F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 0 Reset (65C02 only)"
        }
    },
    { 
        BBR1, 
        OpCodeInfo
        {
            "BBR1",
            {
                { Op_ZeroPageRelative, { 0x1F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 1 Reset (65C02 only)"
        } 
    },
    { 
        BBR2,
        OpCodeInfo
            {
            "BBR2",
            {
                { Op_ZeroPageRelative, { 0x2F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 2 Reset (65C02 only)"
        }
    },
    { 
        BBR3, 
        OpCodeInfo
        {
            "BBR3",
            {
                { Op_ZeroPageRelative, { 0x3F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 3 Reset (65C02 only)"
        }
    },
    { 
        BBR4, 
        OpCodeInfo
        {
            "BBR4",
            {
                { Op_ZeroPageRelative, { 0x4F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 4 Reset (65C02 only)"
        } 
    },
    { 
        BBR5,
        OpCodeInfo
        {
            "BBR5",
            {
                { Op_ZeroPageRelative, { 0x5F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 5 Reset (65C02 only)"
        }
    },
    { 
        BBR6, 
        OpCodeInfo
        {
            "BBR6",
            {
                { Op_ZeroPageRelative, { 0x6F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 6 Reset (65C02 only)"
        } 
    },
    { 
        BBR7, 
        OpCodeInfo
        {
            "BBR7",
            {
                { Op_ZeroPageRelative, { 0x7F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 7 Reset (65C02 only)"
        }
    },       
    { 
        BBS0, 
        OpCodeInfo
        {
            "BBS0",
            {
                { Op_ZeroPageRelative, { 0x8F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 0 Se t (65C02 only)"
        } 
    },
    { 
        BBS1, 
        OpCodeInfo
        {
            "BBS1",
            {
                { Op_ZeroPageRelative, { 0x9F, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 1 Set (65C02 only)"
        } 
    },
    { 
        BBS2, 
        OpCodeInfo
        {
            "BBS2",
            {
                { Op_ZeroPageRelative, { 0xAF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 2 Set (65C02 only)"
        } 
    },
    { 
        BBS3, 
        OpCodeInfo
        {
            "BBS3",
            {
                { Op_ZeroPageRelative, { 0xBF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 3 Set (65C02 only)"
        } 
    },
    { 
        BBS4, 
        OpCodeInfo
        {
            "BBS4",
            {
                { Op_ZeroPageRelative, { 0xCF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 4 Set (65C02 only)"
        }
    },
    { 
        BBS5, 
        OpCodeInfo
        {
            "BBS5",
            {
                { Op_ZeroPageRelative, { 0xDF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 5 Set (65C02 only)"
        }
    },
    { 
        BBS6, 
        OpCodeInfo
        {
            "BBS6",
            {
                { Op_ZeroPageRelative, { 0xEF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 6 Set (65C02 only)"
        }
    },
    {
        BBS7, 
        OpCodeInfo
        {
            "BBS7",
            {
                { Op_ZeroPageRelative, { 0xFF, 5} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Branch if Bit 7 Set (65C02 only)"
        }
    },
    { 
        STP, 
        OpCodeInfo
        {
            "STP",
            {
                { Op_Implied, { 0xDB, 3} }
            },
            true,  // is_65c02
            false, // is_illegal (though WDC-specific)
            "Stop the Processor (WDC 65C02 only)"
        }
    },
    { 
        WAI, 
        OpCodeInfo
        {
            "WAI",
            {
                { Op_Implied, { 0xCB, 3} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Wait for Interrupt (65C02 only)"
        } 
    },
    { 
        BRK, 
        OpCodeInfo
        {
            "BRK",
            {
                { Op_Implied, { 0x00, 7} }
            },
            false, // is_65c02
            false, // is_illegal
            "Break/Interrupt"
        }
    },
    { 
        RTI, 
        OpCodeInfo
        {
            "RTI",
            {
                { Op_Implied, { 0x40, 6} }
            },
            false, // is_65c02
            false, // is_illegal
            "Return from Interrupt"
        }
    },
    { 
        JSR, 
        OpCodeInfo
        {
            "JSR",
            {
                { Op_Absolute, { 0x20, 6} }
            },
            false, // is_65c02
            false, // is_illegal
            "Jump to Subroutine"
        }
    },
    // RTS (Return from Subroutine)
    { 
        RTS, 
        OpCodeInfo
        {
            "RTS",
            {
                { Op_Implied, { 0x60, 6} }
            },
            false, // is_65c02
            false, // is_illegal
            "Return from Subroutine"
        }
    },
    { 
        JMP, 
        OpCodeInfo
        {
            "JMP",
            {
                { Op_Absolute,  { 0x4C, 3} },
                { Op_Indirect,  { 0x6C, 5} },
                { Op_IndirectX, { 0x7C, 6} } // 65C02 only
            },
            false, // is_65c02 (partially)
            false, // is_illegal
            "Jump"
        }
    },
    // BIT (Test Bits in Memory with Accumulator)
    { 
        BIT, 
        OpCodeInfo
        {
            "BIT",
            {
                { Op_ZeroPage,  { 0x24, 3} },
                { Op_Absolute,  { 0x2C, 4} },
                { Op_Immediate, { 0x89, 2} }, // 65C02 only
                { Op_ZeroPageX, { 0x34, 4} }, // 65C02 only
                { Op_AbsoluteX, { 0x3C, 4} }  // 65C02 only (+1 if page crossed)
            },
            false, // is_65c02 (partially)
            false, // is_illegal
            "Test Bits in Memory with Accumulator"
        }
    },
    { 
        TRB, 
        OpCodeInfo
        {
            "TRB",
            {
                { Op_ZeroPage, { 0x14, 5} },
                { Op_Absolute, { 0x1C, 6} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Test and Reset Bits (65C02 only)"
        }
    },
    { 
        TSB, 
        OpCodeInfo
        {
            "TSB",
            {
                { Op_ZeroPage, { 0x04, 5} },
                { Op_Absolute, { 0x0C, 6} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Test and Set Bits (65C02 only)"
        }
    },
    { 
        CLC, 
        OpCodeInfo
        {
            "CLC",
            {
                { Op_Implied, { 0x18, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Clear Carry Flag"
        }
    },
    {
        SEC, 
        OpCodeInfo
        {
            "SEC",
            {
                { Op_Implied, { 0x38, 3} }
            },
            false, // is_65c02
            false, // is_illegal
            "Set Carry Flag"
        }
    },
    { 
        CLD, 
        OpCodeInfo
        {
            "CLD",
            {
                { Op_Implied, { 0xD8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Clear Decimal Mode"
        }
    },
    {
        SED,
        OpCodeInfo
        {
            "SED",
            {
                { Op_Implied, { 0xF8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Set Decimal Mode"
        }
    },
    { 
        CLI, 
        OpCodeInfo
        {
            "CLI",
            {
                { Op_Implied, { 0x58, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Clear Interrupt Disable"
        } 
    },
    { 
        SEI,
        OpCodeInfo
        {
            "SEI",
            {
                { Op_Implied, { 0x78, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Set Interrupt Disable"
        }
    },
    { 
        CLV, 
        OpCodeInfo
        {
            "CLV",
            {
                { Op_Implied, { 0xB8, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "Clear Overflow Flag"
        }
    },
    { 
        NOP, 
        OpCodeInfo
        {
            "NOP",
            {
                { Op_Implied, { 0xEA, 2} }
            },
            false, // is_65c02
            false, // is_illegal
            "No Operation"
        } 
    },
    { 
        SLO,
        OpCodeInfo
        {
            "SLO",
            {
                { Op_ZeroPage,  { 0x07, 5} },
                { Op_ZeroPageX, { 0x17, 6} },
                { Op_Absolute,  { 0x0F, 6} },
                { Op_AbsoluteX, { 0x1F, 7} },
                { Op_AbsoluteY, { 0x1B, 7} },
                { Op_IndirectX, { 0x03, 8} },
                { Op_IndirectY, { 0x13, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "ASL then ORA (Illegal)"
        }
    },
    { 
        RLA, 
        OpCodeInfo
        {
            "RLA",
            {
                { Op_ZeroPage,  { 0x27, 5} },
                { Op_ZeroPageX, { 0x37, 6} },
                { Op_Absolute,  { 0x2F, 6} },
                { Op_AbsoluteX, { 0x3F, 7} },
                { Op_AbsoluteY, { 0x3B, 7} },
                { Op_IndirectX, { 0x23, 8} },
                { Op_IndirectY, { 0x33, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "ROL then AND (Illegal)"
        }
    },
    { 
        SRE, 
        OpCodeInfo
        {
            "SRE",
            {
                { Op_ZeroPage,  { 0x47, 5} },
                { Op_ZeroPageX, { 0x57, 6} },
                { Op_Absolute,  { 0x4F, 6} },
                { Op_AbsoluteX, { 0x5F, 7} },
                { Op_AbsoluteY, { 0x5B, 7} },
                { Op_IndirectX, { 0x43, 8} },
                { Op_IndirectY, { 0x53, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "LSR then EOR (Illegal)"
        } 
    },
    { 
        RRA, 
        OpCodeInfo
        {
            "RRA",
            {
                { Op_ZeroPage,  { 0x67, 5} },
                { Op_ZeroPageX, { 0x77, 6} },
                { Op_Absolute,  { 0x6F, 6} },
                { Op_AbsoluteX, { 0x7F, 7} },
                { Op_AbsoluteY, { 0x7B, 7} },
                { Op_IndirectX, { 0x63, 8} },
                { Op_IndirectY, { 0x73, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "ROR then ADC (Illegal)"
        } 
    },
    { 
        SAX, 
        OpCodeInfo
        {
            "SAX",
            {
                { Op_ZeroPage,  { 0x87, 3} },
                { Op_ZeroPageY, { 0x97, 4} },
                { Op_Absolute,  { 0x8F, 4} },
                { Op_IndirectX, { 0x83, 6} }
            },
            false, // is_65c02
            true,  // is_illegal
            "STA AND STX (Illegal)"
        } 
    },
    { 
        LAX, 
        OpCodeInfo
        {
            "LAX",
            {
                { Op_ZeroPage,  { 0xA7, 3} },
                { Op_ZeroPageY, { 0xB7, 4} },
                { Op_Absolute,  { 0xAF, 4} },
                { Op_AbsoluteY, { 0xBF, 4} },
                { Op_IndirectX, { 0xA3, 6} },
                { Op_IndirectY, { 0xB3, 5} },
                { Op_Immediate, { 0xAB, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "LDA then LDX (Illegal)"
        }
    },
    { 
        DCP, 
        OpCodeInfo
        {
            "DCP",
            {
                { Op_ZeroPage,  { 0xC7, 5} },
                { Op_ZeroPageX, { 0xD7, 6} },
                { Op_Absolute,  { 0xCF, 6} }, 
                { Op_AbsoluteX, { 0xDF, 7} },
                { Op_AbsoluteY, { 0xDB, 7} },
                { Op_IndirectX, { 0xC3, 8} },
                { Op_IndirectY, { 0xD3, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "DEC then CMP (Illegal)"
        } 
    },
    { 
        ISC, 
        OpCodeInfo
        {
            "ISC",
            {
                { Op_ZeroPage,  { 0xE7, 5} },
                { Op_ZeroPageX, { 0xF7, 6} },
                { Op_Absolute,  { 0xEF, 6} },
                { Op_AbsoluteX, { 0xFF, 7} },
                { Op_AbsoluteY, { 0xFB, 7} },
                { Op_IndirectX, { 0xE3, 8} },
                { Op_IndirectY, { 0xF3, 8} }
            },
            false, // is_65c02
            true,  // is_illegal
            "INC then SBC (Illegal)"
        }
    },
    { 
        ANC, 
        OpCodeInfo
        {
        "ANC",
        {
            { Op_Immediate, { 0x0B, 2} },
        },
        false, // is_65c02
        true,  // is_illegal
        "AND then set CARRY (Illegal)"
        }
    },
    { 
        ANC2, 
        OpCodeInfo
        {
            "ANC2",
            {
                { Op_Immediate, { 0x2B, 2} } // Alternate encoding
            },
            false, // is_65c02
            true,  // is_illegal
            "AND then set CARRY (Illegal)"
        } 
    },
    { 
        ALR, 
        OpCodeInfo
        {
            "ALR",
            {
                { Op_Immediate, { 0x4B, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "AND then LSR (Illegal)"
        } 
    },
    { 
        ARR,
        OpCodeInfo
        {
            "ARR",
            {
                { Op_Immediate, { 0x6B, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "AND then ROR (Illegal)"
        } 
    },
    { 
        XAA, 
        OpCodeInfo
        {
            "XAA",
            {
                { Op_Immediate, { 0x8B, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "TXA then AND (Illegal)"
        } 
    },
    { 
        AXS, 
        OpCodeInfo
        {
            "AXS",
            {
                { Op_Immediate, { 0xCB, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "CMP then DEX (Illegal)"
        } 
    },
    { 
        USBC, 
        OpCodeInfo
        {
            "USBC",
            {
                { Op_Immediate, { 0xEB, 2} }
            },
            false, // is_65c02
            true,  // is_illegal
            "Unstable SBC (Illegal)"
        } 
    },
    { 
        AHX, 
        OpCodeInfo
        {
            "AHX",
            {
                { Op_AbsoluteY, { 0x9F, 5} },
                { Op_IndirectY, { 0x93, 6} }
            },
            false, // is_65c02
            true,  // is_illegal
            "STA AND STX AND STY (Illegal)"
        }
    },
    {
        SHY, 
        OpCodeInfo
        {
            "SHY",
            {
                { Op_AbsoluteX, { 0x9C, 5} }
            },
            false, // is_65c02
            true,  // is_illegal
            "Store Y AND high byte (Illegal)"
        }
    },
    { 
        SHX, 
        OpCodeInfo
        {
            "SHX",
            {
                { Op_AbsoluteY, { 0x9E, 5} }
            },
            false, // is_65c02
            true,  // is_illegal
            "Store X AND high byte (Illegal)"
        } 
    },
    { 
        TAS, 
        OpCodeInfo{
        "TAS",
        {
            { Op_AbsoluteY, { 0x9B, 5} }
        },
        false, // is_65c02
        true,  // is_illegal
        "Transfer A AND X to SP (Illegal)"
        } 
    },
    { 
        LAS, 
        OpCodeInfo
        {
            "LAS",
            {
                { Op_AbsoluteY, { 0xBB, 4} }
            },
            false, // is_65c02
            true,  // is_illegal
            "LDA AND TSX (Illegal)"
        } 
    },
    { 
        PHX, 
        OpCodeInfo
        {
            "PHX",
            {
                { Op_Implied, { 0xDA, 3} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Push X Register (65C02 only)"
        } 
    },
    { 
        PHY, 
        OpCodeInfo
        {
            "PHY",
            {
                { Op_Implied, { 0x5A, 3} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Push Y Register (65C02 only)"
        } 
    },
    { 
        PLX, 
        OpCodeInfo
        {
            "PLX",
            {
                { Op_Implied, { 0xFA, 4} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Pull X Register (65C02 only)"
        } 
    },
    { 
        PLY, 
        OpCodeInfo
        {
            "PLY",
            {
                { Op_Implied, {0x7A, 4} }
            },
            true,  // is_65c02
            false, // is_illegal
            "Pull Y Register (65C02 only)"
        } 
    }
};
