// Token.h
#pragma once
#include <string>

enum TOKEN_TYPE {
    ORA, AND, EOR, ADC, SBC,
    CMP, CPX, CPY, DEC, DEX,
    DEY, INC, INX, INY, ASL,
    ROL, LSR, ROR, LDA, STA,
    LDX, STX, LDY, STY, RMB0,
    RMB1, RMB2, RMB3, RMB4, RMB5,
    RMB6, RMB7, SMB0, SMB1, SMB2,
    SMB3, SMB4, SMB5, SMB6, SMB7,
    STZ, TAX, TXA, TAY, TYA,
    TSX, TXS, PLA, PHA, PLP,
    PHP, PHX, PHY, PLX, PLY,
    BRA, BPL, BMI, BVC, BVS,
    BCC, BCS, BNE, BEQ, BBR0,
    BBR1, BBR2, BBR3, BBR4, BBR5,
    BBR6, BBR7, BBS0, BBS1, BBS2,
    BBS3, BBS4, BBS5, BBS6, BBS7,
    STP, WAI, BRK, RTI, JSR, 
    RTS, JMP, BIT, TRB, TSB,
    CLC, SEC, CLD, SED, CLI,
    SEI, CLV, NOP, SLO, RLA,
    SRE, RRA, SAX, LAX, DCP,
    ISC, ANC, ANC2, ALR, ARR,
    XAA, AXS, USBC, AHX, SHY,
    SHX, TAS, LAS,

    DECNUM,     HEXNUM,     BINNUM,     PLUS,       MINUS, 
    MUL,        DIV,        BIT_AND,    BIT_OR,     LPAREN, 
    RPAREN,     WS,         SLEFT,      SRIGHT,     COMMA,
    POUND,      X,          Y,          A,          COMMENT,
    SYM,        LOCALSYM,   AT,         EQUAL,      BIT_XOR,
    EOL,        CHAR,       MOD,        ONESCOMP,   TEXT,
    ORG,        MACRO_DIR,  ENDMACRO_DIR, MACRO_PARAM, BYTE,
    COLAN
};

struct Token {
    TOKEN_TYPE type;
    std::string value;
    size_t line;
    size_t line_pos;
    bool start;
};
