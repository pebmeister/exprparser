; Test file exercising common 6502/65C02 addressing modes.
; Each line is commented with the expected machine code bytes (hex).

    .org $1000              ; Set assembly origin to $1000

; Immediate
LDA_IMM:    LDA #$01        ; A9 $01

; Zero Page
LDA_ZP:     LDA $10         ; A5 10

; Zero Page,X
LDA_ZPX:    LDA $10,X       ; B5 10

; Absolute
LDA_ABS:    LDA $1234       ; AD 34 12

; Absolute,X
LDA_ABSX:   LDA $1234,X     ; BD 34 12

; Absolute,Y
LDA_ABSY:   LDA $1234,Y     ; B9 34 12

; (Indirect,X) — indexed indirect
LDA_INDX:   LDA ($20,X)     ; A1 20

; (Indirect),Y — indirect indexed
LDA_INDY:   LDA ($20),Y     ; B1 20

; Store (zero page)
STA_ZP:     STA $10         ; 85 10

; Store (zero page,X)
STA_ZPX:    STA $10,X       ; 95 10

; Store (absolute)
STA_ABS:    STA $1234       ; 8D 34 12

; Store (absolute,X)
STA_ABSX:   STA $1234,X     ; 9D 34 12

; Store (absolute,Y)
STA_ABSY:   STA $1234,Y     ; 99 34 12

; Store (indexed indirect)
STA_INDX:   STA ($20,X)     ; 81 20

; Store (indirect indexed)
STA_INDY:   STA ($20),Y     ; 91 20

; LDX immediate
LDX_IMM:    LDX #$01        ; A2 01

; LDX zero page
LDX_ZP:     LDX $10         ; A6 10

; LDX absolute
LDX_ABS:    LDX $1234       ; AE 34 12

; LDX zero page,Y
LDX_ZPY:    LDX $10,Y       ; B6 10

; LDY immediate
LDY_IMM:    LDY #$02        ; A0 02

; LDY zero page
LDY_ZP:     LDY $10         ; A4 10

; LDY absolute
LDY_ABS:    LDY $1234       ; AC 34 12

; LDY zero page,X
LDY_ZPX:    LDY $10,X       ; B4 10

; Arithmetic / logical immediate
ADC_IMM:    ADC #$05        ; 69 05
SBC_IMM:    SBC #$03        ; E9 03
AND_IMM:    AND #$FF        ; 29 FF
ORA_IMM:    ORA #$AA        ; 09 AA
EOR_IMM:    EOR #$55        ; 49 55

; Accumulator addressing (single-byte)
ASL_A:      ASL A           ; 0A
LSR_A:      LSR A           ; 4A
ROL_A:      ROL A           ; 2A
ROR_A:      ROR A           ; 6A

; ASL (zero page / zero page,X / absolute / absolute,X)
ASL_ZP:     ASL $10         ; 06 10
ASL_ZPX:    ASL $10,X       ; 16 10
ASL_ABS:    ASL $1234       ; 0E 34 12
ASL_ABSX:   ASL $1234,X     ; 1E 34 12

; Jumps / subroutine / return
JMP_ABS:    JMP $1234       ; 4C 34 12
JMP_IND:    JMP ($1234)     ; 6C 34 12
JSR_ABS:    JSR $1234       ; 20 34 12
RTS_INS:    RTS             ; 60
RTI_INS:    RTI             ; 40
BRK_INS:    BRK             ; 00

; Relative branch (forward). Offset chosen to land at label `BR_TARGET` below.
BEQ_FWD:    BEQ BR_TARGET   ; F0 04

; Four NOPs to make BEQ target predictable (4 bytes)
NOP1:       NOP             ; EA
NOP2:       NOP             ; EA
NOP3:       NOP             ; EA
NOP4:       NOP             ; EA

; Branch target (should be at PC = origin + computed offset)
BR_TARGET:  LDA #$99        ; A9 99

; Transfer instructions (implied)
TAX_INS:    TAX             ; AA
TXA_INS:    TXA             ; 8A
TAY_INS:    TAY             ; A8
TYA_INS:    TYA             ; 98
TSX_INS:    TSX             ; BA
TXS_INS:    TXS             ; 9A

; Stack operations
PHA_INS:    PHA             ; 48
PLA_INS:    PLA             ; 68
PHP_INS:    PHP             ; 08
PLP_INS:    PLP             ; 28
; 65C02/65816 stack ops (if supported)
PHX_INS:    PHX             ; DA
PLX_INS:    PLX             ; FA
PHY_INS:    PHY             ; 5A
PLY_INS:    PLY             ; 7A

; Processor flags (implied)
CLC_INS:    CLC             ; 18
SEC_INS:    SEC             ; 38
CLD_INS:    CLD             ; D8
SED_INS:    SED             ; F8
CLI_INS:    CLI             ; 58
SEI_INS:    SEI             ; 78
NOP_AGAIN:  NOP             ; EA

; Bit test (zp / abs)
BIT_ABS:    BIT $1234       ; 2C 34 12
BIT_ZP:     BIT $10         ; 24 10

; TRB / TSB (65C02) — zero page and absolute variants
TRB_ZP:     TRB $10         ; 14 10
TRB_ABS:    TRB $1234       ; 1C 34 12
TSB_ZP:     TSB $10         ; 04 10
TSB_ABS:    TSB $1234       ; 0C 34 12

; Data directives (little-endian for .word)
BYTE_DATA:  .byte $01,$02   ; 01 02
WORD_DATA:  .word $1234     ; 34 12
  