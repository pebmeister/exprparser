; 6502 / 65C02 Instruction Set Test File (Corrected)

.org $8000

; ============================================================
; ================== NMOS 6502 LEGAL =========================
; ============================================================

; --- ORA ---
ORA_IMMEDIATE:      ORA #$12        ; $09 $12
ORA_ZEROPAGE:       ORA $12         ; $05 $12
ORA_ZEROPAGEX:      ORA $12,X       ; $15 $12
ORA_ABSOLUTE:       ORA $1234       ; $0d $34 $12
ORA_ABSOLUTEX:      ORA $1234,X     ; $1d $34 $12
ORA_ABSOLUTEY:      ORA $1234,Y     ; $19 $34 $12
ORA_INDIRECTX:      ORA ($12,X)     ; $01 $12
ORA_INDIRECTY:      ORA ($12),Y     ; $11 $12

; --- AND ---
AND_IMMEDIATE:      AND #$12        ; $29 $12
AND_ZEROPAGE:       AND $12         ; $25 $12
AND_ZEROPAGEX:      AND $12,X       ; $35 $12
AND_ABSOLUTE:       AND $1234       ; $2d $34 $12
AND_ABSOLUTEX:      AND $1234,X     ; $3d $34 $12
AND_ABSOLUTEY:      AND $1234,Y     ; $39 $34 $12
AND_INDIRECTX:      AND ($12,X)     ; $21 $12
AND_INDIRECTY:      AND ($12),Y     ; $31 $12

; --- EOR ---
EOR_IMMEDIATE:      EOR #$12        ; $49 $12
EOR_ZEROPAGE:       EOR $12         ; $45 $12
EOR_ZEROPAGEX:      EOR $12,X       ; $55 $12
EOR_ABSOLUTE:       EOR $1234       ; $4d $34 $12
EOR_ABSOLUTEX:      EOR $1234,X     ; $5d $34 $12
EOR_ABSOLUTEY:      EOR $1234,Y     ; $59 $34 $12
EOR_INDIRECTX:      EOR ($12,X)     ; $41 $12
EOR_INDIRECTY:      EOR ($12),Y     ; $51 $12

; --- ADC ---
ADC_IMMEDIATE:      ADC #$12        ; $69 $12
ADC_ZEROPAGE:       ADC $12         ; $65 $12
ADC_ZEROPAGEX:      ADC $12,X       ; $75 $12
ADC_ABSOLUTE:       ADC $1234       ; $6d $34 $12
ADC_ABSOLUTEX:      ADC $1234,X     ; $7d $34 $12
ADC_ABSOLUTEY:      ADC $1234,Y     ; $79 $34 $12
ADC_INDIRECTX:      ADC ($12,X)     ; $61 $12
ADC_INDIRECTY:      ADC ($12),Y     ; $71 $12

; --- SBC ---
SBC_IMMEDIATE:      SBC #$12        ; $e9 $12
SBC_ZEROPAGE:       SBC $12         ; $e5 $12
SBC_ZEROPAGEX:      SBC $12,X       ; $f5 $12
SBC_ABSOLUTE:       SBC $1234       ; $ed $34 $12
SBC_ABSOLUTEX:      SBC $1234,X     ; $fd $34 $12
SBC_ABSOLUTEY:      SBC $1234,Y     ; $f9 $34 $12
SBC_INDIRECTX:      SBC ($12,X)     ; $e1 $12
SBC_INDIRECTY:      SBC ($12),Y     ; $f1 $12

; --- CMP ---
CMP_IMMEDIATE:      CMP #$12        ; $c9 $12
CMP_ZEROPAGE:       CMP $12         ; $c5 $12
CMP_ZEROPAGEX:      CMP $12,X       ; $d5 $12
CMP_ABSOLUTE:       CMP $1234       ; $cd $34 $12
CMP_ABSOLUTEX:      CMP $1234,X     ; $dd $34 $12
CMP_ABSOLUTEY:      CMP $1234,Y     ; $d9 $34 $12
CMP_INDIRECTX:      CMP ($12,X)     ; $c1 $12
CMP_INDIRECTY:      CMP ($12),Y     ; $d1 $12

; --- Shifts ---
ASL_ACC:            ASL A           ; $0a
ASL_ZEROPAGE:       ASL $12         ; $06 $12
ASL_ZEROPAGEX:      ASL $12,X       ; $16 $12
ASL_ABSOLUTE:       ASL $1234       ; $0e $34 $12
ASL_ABSOLUTEX:      ASL $1234,X     ; $1e $34 $12

LSR_ACC:            LSR A           ; $4a
LSR_ZEROPAGE:       LSR $12         ; $46 $12
LSR_ZEROPAGEX:      LSR $12,X       ; $56 $12
LSR_ABSOLUTE:       LSR $1234       ; $4e $34 $12
LSR_ABSOLUTEX:      LSR $1234,X     ; $5e $34 $12

ROL_ACC:            ROL A           ; $2a
ROR_ACC:            ROR A           ; $6a

; --- Stack ---
PHA_IMPLIED:        PHA             ; $48
PLA_IMPLIED:        PLA             ; $68
PHP_IMPLIED:        PHP             ; $08
PLP_IMPLIED:        PLP             ; $28

; --- Transfers ---
TAX_IMPLIED:        TAX             ; $aa
TXA_IMPLIED:        TXA             ; $8a
TAY_IMPLIED:        TAY             ; $a8
TYA_IMPLIED:        TYA             ; $98
TSX_IMPLIED:        TSX             ; $ba
TXS_IMPLIED:        TXS             ; $9a

; --- Branches ---
BPL_RELATIVE:       BPL *+2         ; $10 $00
BMI_RELATIVE:       BMI *+2         ; $30 $00
BVC_RELATIVE:       BVC *+2         ; $50 $00
BVS_RELATIVE:       BVS *+2         ; $70 $00
BCC_RELATIVE:       BCC *+2         ; $90 $00
BCS_RELATIVE:       BCS *+2         ; $b0 $00
BNE_RELATIVE:       BNE *+2         ; $d0 $00
BEQ_RELATIVE:       BEQ *+2         ; $f0 $00

; --- Control ---
BRK_IMPLIED:        BRK             ; $00
RTI_IMPLIED:        RTI             ; $40
RTS_IMPLIED:        RTS             ; $60
JSR_ABSOLUTE:       JSR $1234       ; $20 $34 $12
JMP_ABSOLUTE:       JMP $1234       ; $4c $34 $12
JMP_INDIRECT:       JMP ($1234)     ; $6c $34 $12

; ============================================================
; ================ NMOS 6502 UNDOCUMENTED ====================
; ============================================================

SLO_ZP:             SLO $12         ; $07 $12
RLA_ZP:             RLA $12         ; $27 $12
SRE_ZP:             SRE $12         ; $47 $12
RRA_ZP:             RRA $12         ; $67 $12
SAX_ZP:             SAX $12         ; $87 $12
LAX_ZP:             LAX $12         ; $a7 $12
DCP_ZP:             DCP $12         ; $c7 $12
ISC_ZP:             ISC $12         ; $e7 $12

ANC_IMM:            ANC #$12        ; $0b $12
ALR_IMM:            ALR #$12        ; $4b $12
ARR_IMM:            ARR #$12        ; $6b $12
XAA_IMM:            XAA #$12        ; $8b $12
AXS_IMM:            AXS #$12        ; $cb $12
USBC_IMM:           SBC #$12        ; $eb $12

; ============================================================
; ===================== 65C02 EXTENSIONS =====================
; ============================================================

BRA_RELATIVE:       BRA *+2         ; $80 $00
STZ_ZP:             STZ $12         ; $64 $12
STZ_ABS:            STZ $1234       ; $9c $34 $12
PHX_IMPLIED:        PHX             ; $da
PLX_IMPLIED:        PLX             ; $fa
PHY_IMPLIED:        PHY             ; $5a
PLY_IMPLIED:        PLY             ; $7a
WAI_IMPLIED:        WAI             ; $cb
STP_IMPLIED:        STP             ; $db

TSB_ZP:             TSB $12         ; $04 $12
TSB_ABS:            TSB $1234       ; $0c $34 $12
TRB_ZP:             TRB $12         ; $14 $12
TRB_ABS:            TRB $1234       ; $1c $34 $12
