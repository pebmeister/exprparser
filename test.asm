                              .org $1000
                            
BRK_Implied:                  BRK                 ; $00
ORA_IndirectX:                ORA ($13,X)         ; $01 $13
                              .BYTE $02           ; $02
SLO_IndirectX:                SLO ($14,X)         ; $03 $14
TSB_ZeroPage:                 TSB $15             ; $04 $15
ORA_ZeroPage:                 ORA $16             ; $05 $16
ASL_ZeroPage:                 ASL $17             ; $06 $17
RMB0_ZeroPage:                RMB0 $18            ; $07 $18
PHP_Implied:                  PHP                 ; $08
ORA_Immediate:                ORA #$1A            ; $09 $1A
ASL_Accumulator:              ASL A               ; $0A
ANC_Immediate:                ANC #$1C            ; $0B $1C
TSB_Absolute:                 TSB $1D3F           ; $0C $3F $1D
ORA_Absolute:                 ORA $1E40           ; $0D $40 $1E
ASL_Absolute:                 ASL $1F41           ; $0E $41 $1F
BBR0_ZeroPageRelative:        BBR0 $20, * + 3     ; $0F $20 $00
BPL_Relative:                 BPL * + 2           ; $10 $00
ORA_IndirectY:                ORA ($22),Y         ; $11 $22
ORA_Indirect:                 ORA ($23)           ; $12 $23
SLO_IndirectY:                SLO ($24),Y         ; $13 $24
TRB_ZeroPage:                 TRB $25             ; $14 $25
ORA_ZeroPageX:                ORA $26,X           ; $15 $26
ASL_ZeroPageX:                ASL $27,X           ; $16 $27
RMB1_ZeroPage:                RMB1 $28            ; $17 $28
CLC_Implied:                  CLC                 ; $18
ORA_AbsoluteY:                ORA $2A4C,Y         ; $19 $4C $2A
                              .BYTE $1A           ; $1A
SLO_AbsoluteY:                SLO $2B4D,Y         ; $1B $4D $2B
TRB_Absolute:                 TRB $2C4E           ; $1C $4E $2C
ORA_AbsoluteX:                ORA $2D4F,X         ; $1D $4F $2D
ASL_AbsoluteX:                ASL $2E50,X         ; $1E $50 $2E
BBR1_ZeroPageRelative:        BBR1 $2F, * + 3     ; $1F $2F $00
JSR_Absolute:                 JSR $3052           ; $20 $52 $30
AND_IndirectX:                AND ($31,X)         ; $21 $31
                              .BYTE $22           ; $22
RLA_IndirectX:                RLA ($32,X)         ; $23 $32
BIT_ZeroPage:                 BIT $33             ; $24 $33
AND_ZeroPage:                 AND $34             ; $25 $34
ROL_ZeroPage:                 ROL $35             ; $26 $35
RMB2_ZeroPage:                RMB2 $36            ; $27 $36
PLP_Implied:                  PLP                 ; $28
AND_Immediate:                AND #$38            ; $29 $38
ROL_Accumulator:              ROL A               ; $2A
ANC2_Immediate:               ANC2 #$3A           ; $2B $3A
BIT_Absolute:                 BIT $3B5D           ; $2C $5D $3B
AND_Absolute:                 AND $3C5E           ; $2D $5E $3C
ROL_Absolute:                 ROL $3D5F           ; $2E $5F $3D
BBR2_ZeroPageRelative:        BBR2 $3E, * + 3     ; $2F $3E $00
BMI_Relative:                 BMI * + 2           ; $30 $00
AND_IndirectY:                AND ($40),Y         ; $31 $40
AND_Indirect:                 AND ($41)           ; $32 $41
RLA_IndirectY:                RLA ($42),Y         ; $33 $42
BIT_ZeroPageX:                BIT $43,X           ; $34 $43
AND_ZeroPageX:                AND $44,X           ; $35 $44
ROL_ZeroPageX:                ROL $45,X           ; $36 $45
RMB3_ZeroPage:                RMB3 $46            ; $37 $46
SEC_Implied:                  SEC                 ; $38
AND_AbsoluteY:                AND $486A,Y         ; $39 $6A $48
                              .BYTE $3A           ; $3A
RLA_AbsoluteY:                RLA $496B,Y         ; $3B $6B $49
BIT_AbsoluteX:                BIT $4A6C,X         ; $3C $6C $4A
AND_AbsoluteX:                AND $4B6D,X         ; $3D $6D $4B
ROL_AbsoluteX:                ROL $4C6E,X         ; $3E $6E $4C
BBR3_ZeroPageRelative:        BBR3 $4D, * + 3     ; $3F $4D $00
RTI_Implied:                  RTI                 ; $40
EOR_IndirectX:                EOR ($4F,X)         ; $41 $4F
                              .BYTE $42           ; $42
SRE_IndirectX:                SRE ($50,X)         ; $43 $50
                              .BYTE $44           ; $44
EOR_ZeroPage:                 EOR $51             ; $45 $51
LSR_ZeroPage:                 LSR $52             ; $46 $52
RMB4_ZeroPage:                RMB4 $53            ; $47 $53
PHA_Implied:                  PHA                 ; $48
EOR_Immediate:                EOR #$55            ; $49 $55
LSR_Accumulator:              LSR A               ; $4A
ALR_Immediate:                ALR #$57            ; $4B $57
JMP_Absolute:                 JMP $587A           ; $4C $7A $58
EOR_Absolute:                 EOR $597B           ; $4D $7B $59
LSR_Absolute:                 LSR $5A7C           ; $4E $7C $5A
BBR4_ZeroPageRelative:        BBR4 $5B, * + 3     ; $4F $5B $00
BVC_Relative:                 BVC * + 2           ; $50 $00
EOR_IndirectY:                EOR ($5D),Y         ; $51 $5D
EOR_Indirect:                 EOR ($5E)           ; $52 $5E
SRE_IndirectY:                SRE ($5F),Y         ; $53 $5F
                              .BYTE $54           ; $54
EOR_ZeroPageX:                EOR $60,X           ; $55 $60
LSR_ZeroPageX:                LSR $61,X           ; $56 $61
RMB5_ZeroPage:                RMB5 $62            ; $57 $62
CLI_Implied:                  CLI                 ; $58
EOR_AbsoluteY:                EOR $6486,Y         ; $59 $86 $64
PHY_Implied:                  PHY                 ; $5A
SRE_AbsoluteY:                SRE $6688,Y         ; $5B $88 $66
                              .BYTE $5C           ; $5C
EOR_AbsoluteX:                EOR $6789,X         ; $5D $89 $67
LSR_AbsoluteX:                LSR $688A,X         ; $5E $8A $68
BBR5_ZeroPageRelative:        BBR5 $69, * + 3     ; $5F $69 $00
RTS_Implied:                  RTS                 ; $60
ADC_IndirectX:                ADC ($6B,X)         ; $61 $6B
                              .BYTE $62           ; $62
RRA_IndirectX:                RRA ($6C,X)         ; $63 $6C
STZ_ZeroPage:                 STZ $6D             ; $64 $6D
ADC_ZeroPage:                 ADC $6E             ; $65 $6E
ROR_ZeroPage:                 ROR $6F             ; $66 $6F
RMB6_ZeroPage:                RMB6 $70            ; $67 $70
PLA_Implied:                  PLA                 ; $68
ADC_Immediate:                ADC #$72            ; $69 $72
ROR_Accumulator:              ROR A               ; $6A
ARR_Immediate:                ARR #$74            ; $6B $74
JMP_Indirect:                 JMP ($7597)         ; $6C $97 $75
ADC_Absolute:                 ADC $7698           ; $6D $98 $76
ROR_Absolute:                 ROR $7799           ; $6E $99 $77
BBR6_ZeroPageRelative:        BBR6 $78, * + 3     ; $6F $78 $00
BVS_Relative:                 BVS * + 2           ; $70 $00
ADC_IndirectY:                ADC ($7A),Y         ; $71 $7A
ADC_Indirect:                 ADC ($7B)           ; $72 $7B
RRA_IndirectY:                RRA ($7C),Y         ; $73 $7C
STZ_ZeroPageX:                STZ $7D,X           ; $74 $7D
ADC_ZeroPageX:                ADC $7E,X           ; $75 $7E
ROR_ZeroPageX:                ROR $7F,X           ; $76 $7F
RMB7_ZeroPage:                RMB7 $80            ; $77 $80
SEI_Implied:                  SEI                 ; $78
ADC_AbsoluteY:                ADC $82A4,Y         ; $79 $A4 $82
PLY_Implied:                  PLY                 ; $7A
RRA_AbsoluteY:                RRA $84A6,Y         ; $7B $A6 $84
JMP_IndirectX:                JMP ($85A7,X)       ; $7C $A7 $85
ADC_AbsoluteX:                ADC $86A8,X         ; $7D $A8 $86
ROR_AbsoluteX:                ROR $87A9,X         ; $7E $A9 $87
BBR7_ZeroPageRelative:        BBR7 $88, * + 3     ; $7F $88 $00
BRA_Relative:                 BRA * + 2           ; $80 $00
STA_IndirectX:                STA ($8A,X)         ; $81 $8A
                              .BYTE $82           ; $82
SAX_IndirectX:                SAX ($8B,X)         ; $83 $8B
STY_ZeroPage:                 STY $8C             ; $84 $8C
STA_ZeroPage:                 STA $8D             ; $85 $8D
STX_ZeroPage:                 STX $8E             ; $86 $8E
SMB0_ZeroPage:                SMB0 $8F            ; $87 $8F
DEY_Implied:                  DEY                 ; $88
BIT_Immediate:                BIT #$91            ; $89 $91
TXA_Implied:                  TXA                 ; $8A
XAA_Immediate:                XAA #$93            ; $8B $93
STY_Absolute:                 STY $94B6           ; $8C $B6 $94
STA_Absolute:                 STA $95B7           ; $8D $B7 $95
STX_Absolute:                 STX $96B8           ; $8E $B8 $96
BBS0_ZeroPageRelative:        BBS0 $97, * + 3     ; $8F $97 $00
BCC_Relative:                 BCC * + 2           ; $90 $00
STA_IndirectY:                STA ($99),Y         ; $91 $99
STA_Indirect:                 STA ($9A)           ; $92 $9A
AHX_IndirectY:                AHX ($9B),Y         ; $93 $9B
STY_ZeroPageX:                STY $9C,X           ; $94 $9C
STA_ZeroPageX:                STA $9D,X           ; $95 $9D
STX_ZeroPageY:                STX $9E,Y           ; $96 $9E
SMB1_ZeroPage:                SMB1 $9F            ; $97 $9F
TYA_Implied:                  TYA                 ; $98
STA_AbsoluteY:                STA $A1C3,Y         ; $99 $C3 $A1
TXS_Implied:                  TXS                 ; $9A
TAS_AbsoluteY:                TAS $A3C5,Y         ; $9B $C5 $A3
STZ_Absolute:                 STZ $A4C6           ; $9C $C6 $A4
STA_AbsoluteX:                STA $A5C7,X         ; $9D $C7 $A5
STZ_AbsoluteX:                STZ $A6C8,X         ; $9E $C8 $A6
BBS1_ZeroPageRelative:        BBS1 $A7, * + 3     ; $9F $A7 $00
LDY_Immediate:                LDY #$A8            ; $A0 $A8
LDA_IndirectX:                LDA ($A9,X)         ; $A1 $A9
LDX_Immediate:                LDX #$AA            ; $A2 $AA
LAX_IndirectX:                LAX ($AB,X)         ; $A3 $AB
LDY_ZeroPage:                 LDY $AC             ; $A4 $AC
LDA_ZeroPage:                 LDA $AD             ; $A5 $AD
LDX_ZeroPage:                 LDX $AE             ; $A6 $AE
SMB2_ZeroPage:                SMB2 $AF            ; $A7 $AF
TAY_Implied:                  TAY                 ; $A8
LDA_Immediate:                LDA #$B1            ; $A9 $B1
TAX_Implied:                  TAX                 ; $AA
LAX_Immediate:                LAX #$B3            ; $AB $B3
LDY_Absolute:                 LDY $B4D6           ; $AC $D6 $B4
LDA_Absolute:                 LDA $B5D7           ; $AD $D7 $B5
LDX_Absolute:                 LDX $B6D8           ; $AE $D8 $B6
BBS2_ZeroPageRelative:        BBS2 $B7, * + 3     ; $AF $B7 $00
BCS_Relative:                 BCS * + 2           ; $B0 $00
LDA_IndirectY:                LDA ($B9),Y         ; $B1 $B9
LDA_Indirect:                 LDA ($BA)           ; $B2 $BA
LAX_IndirectY:                LAX ($BB),Y         ; $B3 $BB
LDY_ZeroPageX:                LDY $BC,X           ; $B4 $BC
LDA_ZeroPageX:                LDA $BD,X           ; $B5 $BD
LDX_ZeroPageY:                LDX $BE,Y           ; $B6 $BE
SMB3_ZeroPage:                SMB3 $BF            ; $B7 $BF
CLV_Implied:                  CLV                 ; $B8
LDA_AbsoluteY:                LDA $C1E3,Y         ; $B9 $E3 $C1
TSX_Implied:                  TSX                 ; $BA
LAS_AbsoluteY:                LAS $C3E5,Y         ; $BB $E5 $C3
LDY_AbsoluteX:                LDY $C4E6,X         ; $BC $E6 $C4
LDA_AbsoluteX:                LDA $C5E7,X         ; $BD $E7 $C5
LDX_AbsoluteY:                LDX $C6E8,Y         ; $BE $E8 $C6
BBS3_ZeroPageRelative:        BBS3 $C7, * + 3     ; $BF $C7 $00
CPY_Immediate:                CPY #$C8            ; $C0 $C8
CMP_IndirectX:                CMP ($C9,X)         ; $C1 $C9
                              .BYTE $C2           ; $C2
DCP_IndirectX:                DCP ($CA,X)         ; $C3 $CA
CPY_ZeroPage:                 CPY $CB             ; $C4 $CB
CMP_ZeroPage:                 CMP $CC             ; $C5 $CC
DEC_ZeroPage:                 DEC $CD             ; $C6 $CD
SMB4_ZeroPage:                SMB4 $CE            ; $C7 $CE
INY_Implied:                  INY                 ; $C8
CMP_Immediate:                CMP #$D0            ; $C9 $D0
DEX_Implied:                  DEX                 ; $CA
WAI_Implied:                  WAI                 ; $CB
CPY_Absolute:                 CPY $D3F5           ; $CC $F5 $D3
CMP_Absolute:                 CMP $D4F6           ; $CD $F6 $D4
DEC_Absolute:                 DEC $D5F7           ; $CE $F7 $D5
BBS4_ZeroPageRelative:        BBS4 $D6, * + 3     ; $CF $D6 $00
BNE_Relative:                 BNE * + 2           ; $D0 $00
CMP_IndirectY:                CMP ($D8),Y         ; $D1 $D8
CMP_Indirect:                 CMP ($D9)           ; $D2 $D9
DCP_IndirectY:                DCP ($DA),Y         ; $D3 $DA
                              .BYTE $D4           ; $D4
CMP_ZeroPageX:                CMP $DB,X           ; $D5 $DB
DEC_ZeroPageX:                DEC $DC,X           ; $D6 $DC
SMB5_ZeroPage:                SMB5 $DD            ; $D7 $DD
CLD_Implied:                  CLD                 ; $D8
CMP_AbsoluteY:                CMP $DF01,Y         ; $D9 $01 $DF
PHX_Implied:                  PHX                 ; $DA
STP_Implied:                  STP                 ; $DB
                              .BYTE $DC           ; $DC
CMP_AbsoluteX:                CMP $E204,X         ; $DD $04 $E2
DEC_AbsoluteX:                DEC $E305,X         ; $DE $05 $E3
BBS5_ZeroPageRelative:        BBS5 $E4, * + 3     ; $DF $E4 $00
CPX_Immediate:                CPX #$E5            ; $E0 $E5
SBC_IndirectX:                SBC ($E6,X)         ; $E1 $E6
                              .BYTE $E2           ; $E2
ISC_IndirectX:                ISC ($E7,X)         ; $E3 $E7
CPX_ZeroPage:                 CPX $E8             ; $E4 $E8
SBC_ZeroPage:                 SBC $E9             ; $E5 $E9
INC_ZeroPage:                 INC $EA             ; $E6 $EA
SMB6_ZeroPage:                SMB6 $EB            ; $E7 $EB
INX_Implied:                  INX                 ; $E8
SBC_Immediate:                SBC #$ED            ; $E9 $ED
NOP_Implied:                  NOP                 ; $EA
USBC_Immediate:               USBC #$EF           ; $EB $EF
CPX_Absolute:                 CPX $F012           ; $EC $12 $F0
SBC_Absolute:                 SBC $F113           ; $ED $13 $F1
INC_Absolute:                 INC $F214           ; $EE $14 $F2
BBS6_ZeroPageRelative:        BBS6 $F3, * + 3     ; $EF $F3 $00
BEQ_Relative:                 BEQ * + 2           ; $F0 $00
SBC_IndirectY:                SBC ($F5),Y         ; $F1 $F5
SBC_Indirect:                 SBC ($F6)           ; $F2 $F6
ISC_IndirectY:                ISC ($F7),Y         ; $F3 $F7
                              .BYTE $F4           ; $F4
SBC_ZeroPageX:                SBC $F8,X           ; $F5 $F8
INC_ZeroPageX:                INC $F9,X           ; $F6 $F9
SMB7_ZeroPage:                SMB7 $FA            ; $F7 $FA
SED_Implied:                  SED                 ; $F8
SBC_AbsoluteY:                SBC $FC1E,Y         ; $F9 $1E $FC
PLX_Implied:                  PLX                 ; $FA
ISC_AbsoluteY:                ISC $FE20,Y         ; $FB $20 $FE
                              .BYTE $FC           ; $FC
SBC_AbsoluteX:                SBC $FF21,X         ; $FD $21 $FF
INC_AbsoluteX:                INC $0122,X         ; $FE $22 $01
BBS7_ZeroPageRelative:        BBS7 $02, * + 3     ; $FF $02 $00