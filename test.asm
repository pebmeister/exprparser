; renum.asm created by DisAsm6502 1.4.0.0
; 8/18/2021 12:20 PM

          R6510 = $01
          UNUSED1 = $02
          DIMFLG = $0C
          INDEX = $22
          TXTTAB = $2B
          VARTAB = $2D
          ARYTAB = $2F
          STREND = $31
          FRETOP = $33
          MEMSIZ = $37
          CURLIN = $39
          OLDLIN = $3B
          OLDTXT = $3D
          DATLIN = $3F
          DATPTR = $41
          INPPTR = $43
          VARNAM = $45
          FORPNT = $49
          OPPTR = $4B
          OPMASK = $4D
          DEFPNT = $4E
          UNUSED2 = $57
          FAC1 = $61
          FACHO = $62
          FACSSGN = $66
          FAC2 = $69
          ARGHO = $6A
          CHRGET = $73
          TXTPTR = $7A
          TMPDATA = $A3
          FREKZP = $FB
          IGONE = $0308
          ERROR = $A437
          BORDER = $D020
          CIAICR = $DC0D
          CI2ICR = $DD0D
          WARM = $E37B

          .ORG $C000

           CLI                  ; $C000: $58
           LDA IGONE            ; $C001: $AD $08 $03
           STA L_0820           ; $C004: $8D $31 $C6
           LDA IGONE + 1        ; $C007: $AD $09 $03
           STA L_0820 + 1       ; $C00A: $8D $32 $C6
           LDA #<L_0039         ; $C00D: $A9 $51
           STA IGONE            ; $C00F: $8D $08 $03
           LDA #>L_0039         ; $C012: $A9 $C0
           STA IGONE + 1        ; $C014: $8D $09 $03
           LDA TXTTAB           ; $C017: $A5 $2B
           STA FREKZP           ; $C019: $85 $FB
           STA STREND           ; $C01B: $85 $31
           LDA TXTTAB + 1       ; $C01D: $A5 $2C
           STA FREKZP + 1       ; $C01F: $85 $FC
           STA STREND + 1       ; $C021: $85 $32
           LDA #$00             ; $C023: $A9 $00
           LDY #$05             ; $C025: $A0 $05
L_0017     STA (FREKZP),y       ; $C027: $91 $FB
           DEY                  ; $C029: $88
           BNE L_0017           ; $C02A: $D0 $FB
           CLC                  ; $C02C: $18
           LDA STREND           ; $C02D: $A5 $31
           ADC #$02             ; $C02F: $69 $02
           STA STREND           ; $C031: $85 $31
           LDA STREND + 1       ; $C033: $A5 $32
           ADC #$00             ; $C035: $69 $00
           STA STREND + 1       ; $C037: $85 $32
           LDA MEMSIZ           ; $C039: $A5 $37
           STA FRETOP           ; $C03B: $85 $33
           LDA MEMSIZ + 1       ; $C03D: $A5 $38
           STA FRETOP + 1       ; $C03F: $85 $34
           LDA STREND           ; $C041: $A5 $31
           STA VARTAB           ; $C043: $85 $2D
           STA ARYTAB           ; $C045: $85 $2F
           LDA STREND + 1       ; $C047: $A5 $32
           STA VARTAB + 1       ; $C049: $85 $2E
           STA ARYTAB + 1       ; $C04B: $85 $30
           SEI                  ; $C04D: $78
           JMP WARM             ; $C04E: $4C $7B $E3
L_0039     LDA CURLIN + 1       ; $C051: $A5 $3A
           CMP #$FF             ; $C053: $C9 $FF
           BEQ L_0043           ; $C055: $F0 $03
           JMP (L_0820)         ; $C057: $6C $31 $C6
L_0043     LDY #$01             ; $C05A: $A0 $01
L_0044     LDA L_0113,y         ; $C05C: $B9 $F2 $C0
           BEQ L_0050           ; $C05F: $F0 $07
           CMP (TXTPTR),y       ; $C061: $D1 $7A
           BNE L_0106           ; $C063: $D0 $7D
           INY                  ; $C065: $C8
           BNE L_0044           ; $C066: $D0 $F4
L_0050     DEY                  ; $C068: $88
L_0051     JSR CHRGET           ; $C069: $20 $73 $00
           DEY                  ; $C06C: $88
           BNE L_0051           ; $C06D: $D0 $FA
           LDA #$64             ; $C06F: $A9 $64
           STA L_0819           ; $C071: $8D $2F $C6
           LDA #$00             ; $C074: $A9 $00
           STA L_0819 + 1       ; $C076: $8D $30 $C6
           LDA #$0A             ; $C079: $A9 $0A
           STA TMPDATA          ; $C07B: $85 $A3
           LDA #$00             ; $C07D: $A9 $00
           STA TMPDATA + 1      ; $C07F: $85 $A4
           LDY #$01             ; $C081: $A0 $01
           LDA (TXTPTR),y       ; $C083: $B1 $7A
           BEQ L_0102           ; $C085: $F0 $51
           JSR CHRGET           ; $C087: $20 $73 $00
           JSR L_0107           ; $C08A: $20 $E5 $C0
           LDY FACSSGN          ; $C08D: $A4 $66
           BEQ L_0106           ; $C08F: $F0 $51
           JSR L_0115           ; $C091: $20 $F9 $C0
           LDA $FD              ; $C094: $A5 $FD
           STA L_0819           ; $C096: $8D $2F $C6
           LDA $FE              ; $C099: $A5 $FE
           STA L_0819 + 1       ; $C09B: $8D $30 $C6
           DEY                  ; $C09E: $88
           BEQ L_0079           ; $C09F: $F0 $06
L_0076     JSR CHRGET           ; $C0A1: $20 $73 $00
           DEY                  ; $C0A4: $88
           BNE L_0076           ; $C0A5: $D0 $FA
L_0079     LDY #$01             ; $C0A7: $A0 $01
           LDA (TXTPTR),y       ; $C0A9: $B1 $7A
           BEQ L_0102           ; $C0AB: $F0 $2B
           JSR CHRGET           ; $C0AD: $20 $73 $00
           CMP #$2C             ; $C0B0: $C9 $2C
           BNE L_0106           ; $C0B2: $D0 $2E
           LDY #$01             ; $C0B4: $A0 $01
           LDA (TXTPTR),y       ; $C0B6: $B1 $7A
           BEQ L_0102           ; $C0B8: $F0 $1E
           JSR CHRGET           ; $C0BA: $20 $73 $00
           JSR L_0107           ; $C0BD: $20 $E5 $C0
           LDY FACSSGN          ; $C0C0: $A4 $66
           BEQ L_0106           ; $C0C2: $F0 $1E
           JSR L_0115           ; $C0C4: $20 $F9 $C0
           LDA $FD              ; $C0C7: $A5 $FD
           STA TMPDATA          ; $C0C9: $85 $A3
           LDA $FE              ; $C0CB: $A5 $FE
           STA TMPDATA + 1      ; $C0CD: $85 $A4
           DEY                  ; $C0CF: $88
           BEQ L_0102           ; $C0D0: $F0 $06
L_0099     JSR CHRGET           ; $C0D2: $20 $73 $00
           DEY                  ; $C0D5: $88
           BNE L_0099           ; $C0D6: $D0 $FA
L_0102     JSR L_0137           ; $C0D8: $20 $1E $C1
           BCC L_0106           ; $C0DB: $90 $05
           LDX #$0E             ; $C0DD: $A2 $0E
           JMP ERROR            ; $C0DF: $4C $37 $A4
L_0106     JMP (L_0820)         ; $C0E2: $6C $31 $C6
L_0107     LDA TXTPTR           ; $C0E5: $A5 $7A
           STA FREKZP           ; $C0E7: $85 $FB
           LDA TXTPTR + 1       ; $C0E9: $A5 $7B
           STA FREKZP + 1       ; $C0EB: $85 $FC
           LDY #$00             ; $C0ED: $A0 $00
           JMP L_0368           ; $C0EF: $4C $ED $C2
L_0113     .BYTE " RENUM"       ; $C0F2: $20 $52 $45 $4E $55 $4D
           .BYTE $00            ; $C0F8: $00
L_0115     PHA                  ; $C0F9: $48
           TYA                  ; $C0FA: $98
           PHA                  ; $C0FB: $48
           LDA FACSSGN          ; $C0FC: $A5 $66
           CMP #$06             ; $C0FE: $C9 $06
           BCS L_0135           ; $C100: $B0 $17
           CMP #$05             ; $C102: $C9 $05
           BCC L_0131           ; $C104: $90 $0F
           LDY FACHO            ; $C106: $A4 $62
           LDA (FREKZP),y       ; $C108: $B1 $FB
           CMP #$37             ; $C10A: $C9 $37
           BCS L_0135           ; $C10C: $B0 $0B
           INY                  ; $C10E: $C8
           LDA (FREKZP),y       ; $C10F: $B1 $FB
           CMP #$34             ; $C111: $C9 $34
           BCS L_0135           ; $C113: $B0 $04
L_0131     PLA                  ; $C115: $68
           TAY                  ; $C116: $A8
           PLA                  ; $C117: $68
           RTS                  ; $C118: $60
L_0135     LDX #$0E             ; $C119: $A2 $0E
           JMP ERROR            ; $C11B: $4C $37 $A4
L_0137     JSR L_0145           ; $C11E: $20 $2F $C1
           JSR L_0200           ; $C121: $20 $A7 $C1
           PHP                  ; $C124: $08
           BCS L_0142           ; $C125: $B0 $03
           JSR L_0300           ; $C127: $20 $64 $C2
L_0142     JSR L_0184           ; $C12A: $20 $85 $C1
           PLP                  ; $C12D: $28
           RTS                  ; $C12E: $60
L_0145     CLD                  ; $C12F: $D8
           LDA TMPDATA          ; $C130: $A5 $A3
           BNE L_0152           ; $C132: $D0 $09
           LDA TMPDATA + 1      ; $C134: $A5 $A4
           BNE L_0152           ; $C136: $D0 $05
           LDX #$0E             ; $C138: $A2 $0E
           JMP ERROR            ; $C13A: $4C $37 $A4
L_0152     LDA #$7F             ; $C13D: $A9 $7F
           STA CIAICR           ; $C13F: $8D $0D $DC
           STA CI2ICR           ; $C142: $8D $0D $DD
           LDA TXTTAB           ; $C145: $A5 $2B
           STA FREKZP           ; $C147: $85 $FB
           LDA TXTTAB + 1       ; $C149: $A5 $2C
           STA FREKZP + 1       ; $C14B: $85 $FC
           LDA L_0819           ; $C14D: $AD $2F $C6
           STA $FD              ; $C150: $85 $FD
           LDA L_0819 + 1       ; $C152: $AD $30 $C6
           STA $FE              ; $C155: $85 $FE
           LDA #$00             ; $C157: $A9 $00
           STA UNUSED2          ; $C159: $85 $57
           STA UNUSED2 + 1      ; $C15B: $85 $58
           LDA R6510            ; $C15D: $A5 $01
           STA L_0818           ; $C15F: $8D $2E $C6
           AND #$01             ; $C162: $29 $01
           BEQ L_0177           ; $C164: $F0 $10
           LDA #$00             ; $C166: $A9 $00
           STA L_0821           ; $C168: $8D $33 $C6
           LDA #$A0             ; $C16B: $A9 $A0
           STA L_0821 + 1       ; $C16D: $8D $34 $C6
           LDA R6510            ; $C170: $A5 $01
           AND #$FE             ; $C172: $29 $FE
           STA R6510            ; $C174: $85 $01
L_0177     LDA L_0821           ; $C176: $AD $33 $C6
           STA FORPNT           ; $C179: $85 $49
           STA DATPTR           ; $C17B: $85 $41
           LDA L_0821 + 1       ; $C17D: $AD $34 $C6
           STA FORPNT + 1       ; $C180: $85 $4A
           STA DATPTR + 1       ; $C182: $85 $42
           RTS                  ; $C184: $60
L_0184     LDA L_0818           ; $C185: $AD $2E $C6
           STA R6510            ; $C188: $85 $01
           LDA #$81             ; $C18A: $A9 $81
           STA CIAICR           ; $C18C: $8D $0D $DC
           STA CI2ICR           ; $C18F: $8D $0D $DD
           LDA MEMSIZ           ; $C192: $A5 $37
           STA FRETOP           ; $C194: $85 $33
           LDA MEMSIZ + 1       ; $C196: $A5 $38
           STA FRETOP + 1       ; $C198: $85 $34
           LDA STREND           ; $C19A: $A5 $31
           STA VARTAB           ; $C19C: $85 $2D
           STA ARYTAB           ; $C19E: $85 $2F
           LDA STREND + 1       ; $C1A0: $A5 $32
           STA VARTAB + 1       ; $C1A2: $85 $2E
           STA ARYTAB + 1       ; $C1A4: $85 $30
           RTS                  ; $C1A6: $60
L_0200     JSR L_0746           ; $C1A7: $20 $AE $C5
           BNE L_0203           ; $C1AA: $D0 $03
           JMP L_0277           ; $C1AC: $4C $3D $C2
L_0203     JSR L_0279           ; $C1AF: $20 $3F $C2
           LDY #$02             ; $C1B2: $A0 $02
           LDA $FD              ; $C1B4: $A5 $FD
           STA (FREKZP),y       ; $C1B6: $91 $FB
           LDA $FE              ; $C1B8: $A5 $FE
           INY                  ; $C1BA: $C8
           STA (FREKZP),y       ; $C1BB: $91 $FB
           CLC                  ; $C1BD: $18
           LDA $FD              ; $C1BE: $A5 $FD
           ADC TMPDATA          ; $C1C0: $65 $A3
           STA $FD              ; $C1C2: $85 $FD
           LDA $FE              ; $C1C4: $A5 $FE
           ADC TMPDATA + 1      ; $C1C6: $65 $A4
           STA $FE              ; $C1C8: $85 $FE
           BCC L_0262           ; $C1CA: $90 $55
           LDA TXTTAB           ; $C1CC: $A5 $2B
           STA FREKZP           ; $C1CE: $85 $FB
           LDA TXTTAB + 1       ; $C1D0: $A5 $2C
           STA FREKZP + 1       ; $C1D2: $85 $FC
           LDA L_0821           ; $C1D4: $AD $33 $C6
           STA DATPTR           ; $C1D7: $85 $41
           LDA L_0821 + 1       ; $C1D9: $AD $34 $C6
           STA DATPTR + 1       ; $C1DC: $85 $42
L_0226     LDY #$00             ; $C1DE: $A0 $00
           LDA (DATPTR),y       ; $C1E0: $B1 $41
           LDY #$02             ; $C1E2: $A0 $02
           STA (FREKZP),y       ; $C1E4: $91 $FB
           LDY #$01             ; $C1E6: $A0 $01
           LDA (DATPTR),y       ; $C1E8: $B1 $41
           LDY #$03             ; $C1EA: $A0 $03
           STA (FREKZP),y       ; $C1EC: $91 $FB
           LDA UNUSED2          ; $C1EE: $A5 $57
           BNE L_0240           ; $C1F0: $D0 $06
           LDA UNUSED2 + 1      ; $C1F2: $A5 $58
           BNE L_0240           ; $C1F4: $D0 $02
           SEC                  ; $C1F6: $38
           RTS                  ; $C1F7: $60
L_0240     SEC                  ; $C1F8: $38
           LDA UNUSED2          ; $C1F9: $A5 $57
           SBC #$01             ; $C1FB: $E9 $01
           STA UNUSED2          ; $C1FD: $85 $57
           LDA UNUSED2 + 1      ; $C1FF: $A5 $58
           SBC #$00             ; $C201: $E9 $00
           STA UNUSED2 + 1      ; $C203: $85 $58
           CLC                  ; $C205: $18
           LDA DATPTR           ; $C206: $A5 $41
           ADC #$02             ; $C208: $69 $02
           STA DATPTR           ; $C20A: $85 $41
           LDA DATPTR + 1       ; $C20C: $A5 $42
           ADC #$00             ; $C20E: $69 $00
           STA DATPTR + 1       ; $C210: $85 $42
           LDY #$00             ; $C212: $A0 $00
           LDA (FREKZP),y       ; $C214: $B1 $FB
           TAX                  ; $C216: $AA
           INY                  ; $C217: $C8
           LDA (FREKZP),y       ; $C218: $B1 $FB
           STX FREKZP           ; $C21A: $86 $FB
           STA FREKZP + 1       ; $C21C: $85 $FC
           JMP L_0226           ; $C21E: $4C $DE $C1
L_0262     LDY #$00             ; $C221: $A0 $00
           LDA (FREKZP),y       ; $C223: $B1 $FB
           TAX                  ; $C225: $AA
           INY                  ; $C226: $C8
           LDA (FREKZP),y       ; $C227: $B1 $FB
           STX FREKZP           ; $C229: $86 $FB
           STA FREKZP + 1       ; $C22B: $85 $FC
           CLC                  ; $C22D: $18
           LDA UNUSED2          ; $C22E: $A5 $57
           ADC #$01             ; $C230: $69 $01
           STA UNUSED2          ; $C232: $85 $57
           LDA UNUSED2 + 1      ; $C234: $A5 $58
           ADC #$00             ; $C236: $69 $00
           STA UNUSED2 + 1      ; $C238: $85 $58
           JMP L_0200           ; $C23A: $4C $A7 $C1
L_0277     CLC                  ; $C23D: $18
           RTS                  ; $C23E: $60
L_0279     LDY #$02             ; $C23F: $A0 $02
           LDA (FREKZP),y       ; $C241: $B1 $FB
           TAX                  ; $C243: $AA
           INY                  ; $C244: $C8
           LDA (FREKZP),y       ; $C245: $B1 $FB
           STX OPPTR            ; $C247: $86 $4B
           STA OPPTR + 1        ; $C249: $85 $4C
           LDY #$00             ; $C24B: $A0 $00
           LDA OPPTR            ; $C24D: $A5 $4B
           STA (DATPTR),y       ; $C24F: $91 $41
           LDA OPPTR + 1        ; $C251: $A5 $4C
           INY                  ; $C253: $C8
           STA (DATPTR),y       ; $C254: $91 $41
           CLC                  ; $C256: $18
           LDA DATPTR           ; $C257: $A5 $41
           ADC #$02             ; $C259: $69 $02
           STA DATPTR           ; $C25B: $85 $41
           LDA DATPTR + 1       ; $C25D: $A5 $42
           ADC #$00             ; $C25F: $69 $00
           STA DATPTR + 1       ; $C261: $85 $42
           RTS                  ; $C263: $60
L_0300     LDA BORDER           ; $C264: $AD $20 $D0
           PHA                  ; $C267: $48
           LDA TXTTAB           ; $C268: $A5 $2B
           STA FREKZP           ; $C26A: $85 $FB
           LDA TXTTAB + 1       ; $C26C: $A5 $2C
           STA FREKZP + 1       ; $C26E: $85 $FC
L_0306     LDA #$00             ; $C270: $A9 $00
           STA UNUSED1          ; $C272: $85 $02
           JSR L_0746           ; $C274: $20 $AE $C5
           BNE L_0313           ; $C277: $D0 $05
           PLA                  ; $C279: $68
           STA BORDER           ; $C27A: $8D $20 $D0
           RTS                  ; $C27D: $60
L_0313     LDY #$03             ; $C27E: $A0 $03
           LDA #$00             ; $C280: $A9 $00
L_0315     CMP #$20             ; $C282: $C9 $20
           BEQ L_0318           ; $C284: $F0 $02
           STA DIMFLG           ; $C286: $85 $0C
L_0318     INY                  ; $C288: $C8
           LDA (FREKZP),y       ; $C289: $B1 $FB
           BEQ L_0359           ; $C28B: $F0 $4E
           CMP #$22             ; $C28D: $C9 $22
           BEQ L_0355           ; $C28F: $F0 $41
           LDX UNUSED1          ; $C291: $A6 $02
           BNE L_0315           ; $C293: $D0 $ED
           CMP #$89             ; $C295: $C9 $89
           BEQ L_0336           ; $C297: $F0 $12
           CMP #$8D             ; $C299: $C9 $8D
           BEQ L_0336           ; $C29B: $F0 $0E
           CMP #$A7             ; $C29D: $C9 $A7
           BEQ L_0336           ; $C29F: $F0 $0A
           CMP #$A4             ; $C2A1: $C9 $A4
           BNE L_0315           ; $C2A3: $D0 $DD
           LDX DIMFLG           ; $C2A5: $A6 $0C
           CPX #$CB             ; $C2A7: $E0 $CB
           BNE L_0315           ; $C2A9: $D0 $D7
L_0336     INY                  ; $C2AB: $C8
           LDA (FREKZP),y       ; $C2AC: $B1 $FB
           BEQ L_0359           ; $C2AE: $F0 $2B
           CMP #$3A             ; $C2B0: $C9 $3A
           BEQ L_0315           ; $C2B2: $F0 $CE
           CMP #$20             ; $C2B4: $C9 $20
           BEQ L_0336           ; $C2B6: $F0 $F3
           CMP #$2C             ; $C2B8: $C9 $2C
           BEQ L_0336           ; $C2BA: $F0 $EF
           CMP #$30             ; $C2BC: $C9 $30
           BCC L_0315           ; $C2BE: $90 $C2
           CMP #$3A             ; $C2C0: $C9 $3A
           BCS L_0315           ; $C2C2: $B0 $BE
           JSR L_0368           ; $C2C4: $20 $ED $C2
           JSR L_0410           ; $C2C7: $20 $35 $C3
           JSR L_0479           ; $C2CA: $20 $BC $C3
           TXA                  ; $C2CD: $8A
           TAY                  ; $C2CE: $A8
           JMP L_0336           ; $C2CF: $4C $AB $C2
L_0355     LDA UNUSED1          ; $C2D2: $A5 $02
           EOR #$01             ; $C2D4: $49 $01
           STA UNUSED1          ; $C2D6: $85 $02
           JMP L_0315           ; $C2D8: $4C $82 $C2
L_0359     LDY #$00             ; $C2DB: $A0 $00
           LDA (FREKZP),y       ; $C2DD: $B1 $FB
           TAX                  ; $C2DF: $AA
           INY                  ; $C2E0: $C8
           LDA (FREKZP),y       ; $C2E1: $B1 $FB
           STX FREKZP           ; $C2E3: $86 $FB
           STA FREKZP + 1       ; $C2E5: $85 $FC
           INC BORDER           ; $C2E7: $EE $20 $D0
           JMP L_0306           ; $C2EA: $4C $70 $C2
L_0368     LDA #$00             ; $C2ED: $A9 $00
           STA $FD              ; $C2EF: $85 $FD
           STA $FE              ; $C2F1: $85 $FE
           STA FACSSGN          ; $C2F3: $85 $66
           STY FACHO            ; $C2F5: $84 $62
L_0373     LDA (FREKZP),y       ; $C2F7: $B1 $FB
           CMP #$30             ; $C2F9: $C9 $30
           BCC L_0409           ; $C2FB: $90 $37
           CMP #$3A             ; $C2FD: $C9 $3A
           BCS L_0409           ; $C2FF: $B0 $33
           INC FACSSGN          ; $C301: $E6 $66
           PHA                  ; $C303: $48
           TYA                  ; $C304: $98
           PHA                  ; $C305: $48
           ASL $FD              ; $C306: $06 $FD
           ROL $FE              ; $C308: $26 $FE
           LDX $FD              ; $C30A: $A6 $FD
           LDY $FE              ; $C30C: $A4 $FE
           ASL $FD              ; $C30E: $06 $FD
           ROL $FE              ; $C310: $26 $FE
           ASL $FD              ; $C312: $06 $FD
           ROL $FE              ; $C314: $26 $FE
           CLC                  ; $C316: $18
           TXA                  ; $C317: $8A
           ADC $FD              ; $C318: $65 $FD
           STA $FD              ; $C31A: $85 $FD
           TYA                  ; $C31C: $98
           ADC $FE              ; $C31D: $65 $FE
           STA $FE              ; $C31F: $85 $FE
           PLA                  ; $C321: $68
           TAY                  ; $C322: $A8
           PLA                  ; $C323: $68
           AND #$CF             ; $C324: $29 $CF
           CLC                  ; $C326: $18
           ADC $FD              ; $C327: $65 $FD
           STA $FD              ; $C329: $85 $FD
           LDA $FE              ; $C32B: $A5 $FE
           ADC #$00             ; $C32D: $69 $00
           STA $FE              ; $C32F: $85 $FE
           INY                  ; $C331: $C8
           BNE L_0373           ; $C332: $D0 $C3
L_0409     RTS                  ; $C334: $60
L_0410     LDA UNUSED2          ; $C335: $A5 $57
           STA INPPTR           ; $C337: $85 $43
           LDA UNUSED2 + 1      ; $C339: $A5 $58
           STA INPPTR + 1       ; $C33B: $85 $44
           LDA #$FF             ; $C33D: $A9 $FF
           STA $59              ; $C33F: $85 $59
           STA $5A              ; $C341: $85 $5A
           SEC                  ; $C343: $38
           ASL INPPTR           ; $C344: $06 $43
           ROL INPPTR + 1       ; $C346: $26 $44
           LDA #$00             ; $C348: $A9 $00
           STA VARNAM           ; $C34A: $85 $45
           STA VARNAM + 1       ; $C34C: $85 $46
L_0423     SEC                  ; $C34E: $38
           LDA INPPTR           ; $C34F: $A5 $43
           SBC VARNAM           ; $C351: $E5 $45
           STA INDEX            ; $C353: $85 $22
           LDA INPPTR + 1       ; $C355: $A5 $44
           SBC VARNAM + 1       ; $C357: $E5 $46
           STA INDEX + 1        ; $C359: $85 $23
           LSR INDEX + 1        ; $C35B: $46 $23
           ROR INDEX            ; $C35D: $66 $22
           CLC                  ; $C35F: $18
           LDA INDEX            ; $C360: $A5 $22
           ADC VARNAM           ; $C362: $65 $45
           STA INDEX            ; $C364: $85 $22
           LDA INDEX + 1        ; $C366: $A5 $23
           ADC VARNAM + 1       ; $C368: $65 $46
           STA INDEX + 1        ; $C36A: $85 $23
           LDA INDEX            ; $C36C: $A5 $22
           AND #$FE             ; $C36E: $29 $FE
           STA INDEX            ; $C370: $85 $22
           CMP $59              ; $C372: $C5 $59
           BNE L_0448           ; $C374: $D0 $07
           LDA INDEX + 1        ; $C376: $A5 $23
           CMP $5A              ; $C378: $C5 $5A
           BNE L_0448           ; $C37A: $D0 $01
           RTS                  ; $C37C: $60
L_0448     LDA INDEX            ; $C37D: $A5 $22
           STA $59              ; $C37F: $85 $59
           LDA INDEX + 1        ; $C381: $A5 $23
           STA $5A              ; $C383: $85 $5A
           CLC                  ; $C385: $18
           LDA L_0821           ; $C386: $AD $33 $C6
           ADC INDEX            ; $C389: $65 $22
           STA FORPNT           ; $C38B: $85 $49
           LDA L_0821 + 1       ; $C38D: $AD $34 $C6
           ADC INDEX + 1        ; $C390: $65 $23
           STA FORPNT + 1       ; $C392: $85 $4A
           LDY #$01             ; $C394: $A0 $01
           LDA (FORPNT),y       ; $C396: $B1 $49
           CMP $FE              ; $C398: $C5 $FE
           BNE L_0467           ; $C39A: $D0 $07
           DEY                  ; $C39C: $88
           LDA (FORPNT),y       ; $C39D: $B1 $49
           CMP $FD              ; $C39F: $C5 $FD
           BEQ L_0478           ; $C3A1: $F0 $18
L_0467     BCC L_0473           ; $C3A3: $90 $0B
           LDA INDEX            ; $C3A5: $A5 $22
           STA INPPTR           ; $C3A7: $85 $43
           LDA INDEX + 1        ; $C3A9: $A5 $23
           STA INPPTR + 1       ; $C3AB: $85 $44
           JMP L_0423           ; $C3AD: $4C $4E $C3
L_0473     LDA INDEX            ; $C3B0: $A5 $22
           STA VARNAM           ; $C3B2: $85 $45
           LDA INDEX + 1        ; $C3B4: $A5 $23
           STA VARNAM + 1       ; $C3B6: $85 $46
           JMP L_0423           ; $C3B8: $4C $4E $C3
L_0478     RTS                  ; $C3BB: $60
L_0479     PHA                  ; $C3BC: $48
           TYA                  ; $C3BD: $98
           PHA                  ; $C3BE: $48
           LSR INDEX + 1        ; $C3BF: $46 $23
           ROR INDEX            ; $C3C1: $66 $22
           LDA TMPDATA          ; $C3C3: $A5 $A3
           PHA                  ; $C3C5: $48
           LDA TMPDATA + 1      ; $C3C6: $A5 $A4
           PHA                  ; $C3C8: $48
           LDA #$00             ; $C3C9: $A9 $00
           STA ARGHO + 1        ; $C3CB: $85 $6B
           STA $6C              ; $C3CD: $85 $6C
           LDX #$10             ; $C3CF: $A2 $10
L_0492     LSR TMPDATA + 1      ; $C3D1: $46 $A4
           ROR TMPDATA          ; $C3D3: $66 $A3
           BCC L_0501           ; $C3D5: $90 $0B
           LDA ARGHO + 1        ; $C3D7: $A5 $6B
           CLC                  ; $C3D9: $18
           ADC INDEX            ; $C3DA: $65 $22
           STA ARGHO + 1        ; $C3DC: $85 $6B
           LDA $6C              ; $C3DE: $A5 $6C
           ADC INDEX + 1        ; $C3E0: $65 $23
L_0501     ROR                  ; $C3E2: $6A
           STA $6C              ; $C3E3: $85 $6C
           ROR ARGHO + 1        ; $C3E5: $66 $6B
           ROR ARGHO            ; $C3E7: $66 $6A
           ROR FAC2             ; $C3E9: $66 $69
           DEX                  ; $C3EB: $CA
           BNE L_0492           ; $C3EC: $D0 $E3
           PLA                  ; $C3EE: $68
           STA TMPDATA + 1      ; $C3EF: $85 $A4
           PLA                  ; $C3F1: $68
           STA TMPDATA          ; $C3F2: $85 $A3
           CLC                  ; $C3F4: $18
           LDA L_0819           ; $C3F5: $AD $2F $C6
           ADC FAC2             ; $C3F8: $65 $69
           STA OPPTR            ; $C3FA: $85 $4B
           LDA L_0819 + 1       ; $C3FC: $AD $30 $C6
           ADC ARGHO            ; $C3FF: $65 $6A
           STA OPPTR + 1        ; $C401: $85 $4C
           JSR L_0752           ; $C403: $20 $B8 $C5
           JSR L_0801           ; $C406: $20 $0E $C6
           LDA FACSSGN          ; $C409: $A5 $66
           CMP FAC1             ; $C40B: $C5 $61
           BNE L_0542           ; $C40D: $D0 $19
           LDY FACHO            ; $C40F: $A4 $62
L_0525     SEC                  ; $C411: $38
           LDA #$05             ; $C412: $A9 $05
           SBC FAC1             ; $C414: $E5 $61
           TAX                  ; $C416: $AA
L_0529     LDA OLDLIN,x         ; $C417: $B5 $3B
           STA (FREKZP),y       ; $C419: $91 $FB
           INY                  ; $C41B: $C8
           INX                  ; $C41C: $E8
           CPX #$05             ; $C41D: $E0 $05
           BNE L_0529           ; $C41F: $D0 $F6
           TYA                  ; $C421: $98
           TAX                  ; $C422: $AA
           DEX                  ; $C423: $CA
           PLA                  ; $C424: $68
           TAY                  ; $C425: $A8
           PLA                  ; $C426: $68
           RTS                  ; $C427: $60
L_0542     BCC L_0546           ; $C428: $90 $07
           JSR L_0548           ; $C42A: $20 $37 $C4
L_0544     LDY FACHO            ; $C42D: $A4 $62
           BNE L_0525           ; $C42F: $D0 $E0
L_0546     JSR L_0631           ; $C431: $20 $DA $C4
           JMP L_0544           ; $C434: $4C $2D $C4
L_0548     LDA FREKZP           ; $C437: $A5 $FB
           STA L_0630           ; $C439: $8D $D8 $C4
           LDA FREKZP + 1       ; $C43C: $A5 $FC
           STA L_0630 + 1       ; $C43E: $8D $D9 $C4
           LDA FREKZP           ; $C441: $A5 $FB
           STA VARNAM           ; $C443: $85 $45
           LDA FREKZP + 1       ; $C445: $A5 $FC
           STA VARNAM + 1       ; $C447: $85 $46
           CLC                  ; $C449: $18
           LDA FACHO            ; $C44A: $A5 $62
           ADC VARNAM           ; $C44C: $65 $45
           STA VARNAM           ; $C44E: $85 $45
           LDA #$00             ; $C450: $A9 $00
           ADC VARNAM + 1       ; $C452: $65 $46
           STA VARNAM + 1       ; $C454: $85 $46
           SEC                  ; $C456: $38
           LDA FACSSGN          ; $C457: $A5 $66
           SBC FAC1             ; $C459: $E5 $61
           STA $59              ; $C45B: $85 $59
           SEC                  ; $C45D: $38
           LDA STREND           ; $C45E: $A5 $31
           SBC $59              ; $C460: $E5 $59
           STA STREND           ; $C462: $85 $31
           LDA STREND + 1       ; $C464: $A5 $32
           SBC #$00             ; $C466: $E9 $00
           STA STREND + 1       ; $C468: $85 $32
L_0574     LDY #$00             ; $C46A: $A0 $00
           SEC                  ; $C46C: $38
           LDA (FREKZP),y       ; $C46D: $B1 $FB
           STA INPPTR           ; $C46F: $85 $43
           SBC $59              ; $C471: $E5 $59
           STA (FREKZP),y       ; $C473: $91 $FB
           INY                  ; $C475: $C8
           LDA (FREKZP),y       ; $C476: $B1 $FB
           STA INPPTR + 1       ; $C478: $85 $44
           SBC #$00             ; $C47A: $E9 $00
           STA (FREKZP),y       ; $C47C: $91 $FB
           LDA INPPTR           ; $C47E: $A5 $43
           STA FREKZP           ; $C480: $85 $FB
           LDA INPPTR + 1       ; $C482: $A5 $44
           STA FREKZP + 1       ; $C484: $85 $FC
           JSR L_0746           ; $C486: $20 $AE $C5
           BNE L_0574           ; $C489: $D0 $DF
           SEC                  ; $C48B: $38
           LDA INPPTR           ; $C48C: $A5 $43
           SBC VARNAM           ; $C48E: $E5 $45
           STA INDEX            ; $C490: $85 $22
           LDA INPPTR + 1       ; $C492: $A5 $44
           SBC VARNAM + 1       ; $C494: $E5 $46
           STA INDEX + 1        ; $C496: $85 $23
           CLC                  ; $C498: $18
           LDA INDEX            ; $C499: $A5 $22
           ADC #$04             ; $C49B: $69 $04
           STA INDEX            ; $C49D: $85 $22
           LDA INDEX + 1        ; $C49F: $A5 $23
           ADC #$00             ; $C4A1: $69 $00
           STA INDEX + 1        ; $C4A3: $85 $23
           CLC                  ; $C4A5: $18
           LDA VARNAM           ; $C4A6: $A5 $45
           ADC $59              ; $C4A8: $65 $59
           STA INPPTR           ; $C4AA: $85 $43
           LDA VARNAM + 1       ; $C4AC: $A5 $46
           ADC #$00             ; $C4AE: $69 $00
           STA INPPTR + 1       ; $C4B0: $85 $44
           LDA VARNAM           ; $C4B2: $A5 $45
           STA FREKZP           ; $C4B4: $85 $FB
           LDA VARNAM + 1       ; $C4B6: $A5 $46
           STA FREKZP + 1       ; $C4B8: $85 $FC
           LDA INPPTR           ; $C4BA: $A5 $43
           STA VARNAM           ; $C4BC: $85 $45
           LDA INPPTR + 1       ; $C4BE: $A5 $44
           STA VARNAM + 1       ; $C4C0: $85 $46
           LDA FREKZP           ; $C4C2: $A5 $FB
           STA INPPTR           ; $C4C4: $85 $43
           LDA FREKZP + 1       ; $C4C6: $A5 $FC
           STA INPPTR + 1       ; $C4C8: $85 $44
           JSR L_0727           ; $C4CA: $20 $8D $C5
           LDA L_0630           ; $C4CD: $AD $D8 $C4
           STA FREKZP           ; $C4D0: $85 $FB
           LDA L_0630 + 1       ; $C4D2: $AD $D9 $C4
           STA FREKZP + 1       ; $C4D5: $85 $FC
           RTS                  ; $C4D7: $60
L_0630     .WORD $0000          ; $C4D8: $00 $00
L_0631     LDA FREKZP           ; $C4DA: $A5 $FB
           STA L_0700           ; $C4DC: $8D $61 $C5
           LDA FREKZP + 1       ; $C4DF: $A5 $FC
           STA L_0700 + 1       ; $C4E1: $8D $62 $C5
           LDA FREKZP           ; $C4E4: $A5 $FB
           STA VARNAM           ; $C4E6: $85 $45
           LDA FREKZP + 1       ; $C4E8: $A5 $FC
           STA VARNAM + 1       ; $C4EA: $85 $46
           CLC                  ; $C4EC: $18
           LDA FACHO            ; $C4ED: $A5 $62
           ADC VARNAM           ; $C4EF: $65 $45
           STA VARNAM           ; $C4F1: $85 $45
           LDA #$00             ; $C4F3: $A9 $00
           ADC VARNAM + 1       ; $C4F5: $65 $46
           STA VARNAM + 1       ; $C4F7: $85 $46
           SEC                  ; $C4F9: $38
           LDA FAC1             ; $C4FA: $A5 $61
           SBC FACSSGN          ; $C4FC: $E5 $66
           STA $59              ; $C4FE: $85 $59
           CLC                  ; $C500: $18
           ADC STREND           ; $C501: $65 $31
           STA STREND           ; $C503: $85 $31
           LDA #$00             ; $C505: $A9 $00
           ADC STREND + 1       ; $C507: $65 $32
           STA STREND + 1       ; $C509: $85 $32
L_0656     LDY #$00             ; $C50B: $A0 $00
           CLC                  ; $C50D: $18
           LDA (FREKZP),y       ; $C50E: $B1 $FB
           STA INPPTR           ; $C510: $85 $43
           ADC $59              ; $C512: $65 $59
           STA (FREKZP),y       ; $C514: $91 $FB
           INY                  ; $C516: $C8
           LDA (FREKZP),y       ; $C517: $B1 $FB
           STA INPPTR + 1       ; $C519: $85 $44
           ADC #$00             ; $C51B: $69 $00
           STA (FREKZP),y       ; $C51D: $91 $FB
           LDA INPPTR           ; $C51F: $A5 $43
           STA FREKZP           ; $C521: $85 $FB
           LDA INPPTR + 1       ; $C523: $A5 $44
           STA FREKZP + 1       ; $C525: $85 $FC
           JSR L_0746           ; $C527: $20 $AE $C5
           BNE L_0656           ; $C52A: $D0 $DF
           SEC                  ; $C52C: $38
           LDA INPPTR           ; $C52D: $A5 $43
           SBC VARNAM           ; $C52F: $E5 $45
           STA INDEX            ; $C531: $85 $22
           LDA INPPTR + 1       ; $C533: $A5 $44
           SBC VARNAM + 1       ; $C535: $E5 $46
           STA INDEX + 1        ; $C537: $85 $23
           CLC                  ; $C539: $18
           LDA INDEX            ; $C53A: $A5 $22
           ADC #$04             ; $C53C: $69 $04
           STA INDEX            ; $C53E: $85 $22
           LDA INDEX + 1        ; $C540: $A5 $23
           ADC #$00             ; $C542: $69 $00
           STA INDEX + 1        ; $C544: $85 $23
           CLC                  ; $C546: $18
           LDA VARNAM           ; $C547: $A5 $45
           ADC $59              ; $C549: $65 $59
           STA INPPTR           ; $C54B: $85 $43
           LDA VARNAM + 1       ; $C54D: $A5 $46
           ADC #$00             ; $C54F: $69 $00
           STA INPPTR + 1       ; $C551: $85 $44
           JSR L_0701           ; $C553: $20 $63 $C5
           LDA L_0700           ; $C556: $AD $61 $C5
           STA FREKZP           ; $C559: $85 $FB
           LDA L_0700 + 1       ; $C55B: $AD $62 $C5
           STA FREKZP + 1       ; $C55E: $85 $FC
           RTS                  ; $C560: $60
L_0700     .WORD $0000          ; $C561: $00 $00
L_0701     LDX INDEX + 1        ; $C563: $A6 $23
           CLC                  ; $C565: $18
           TXA                  ; $C566: $8A
           ADC VARNAM + 1       ; $C567: $65 $46
           STA VARNAM + 1       ; $C569: $85 $46
           CLC                  ; $C56B: $18
           TXA                  ; $C56C: $8A
           ADC INPPTR + 1       ; $C56D: $65 $44
           STA INPPTR + 1       ; $C56F: $85 $44
           INX                  ; $C571: $E8
           LDY INDEX            ; $C572: $A4 $22
           BEQ L_0721           ; $C574: $F0 $0E
           DEY                  ; $C576: $88
           BEQ L_0719           ; $C577: $F0 $07
L_0715     LDA (VARNAM),y       ; $C579: $B1 $45
           STA (INPPTR),y       ; $C57B: $91 $43
           DEY                  ; $C57D: $88
           BNE L_0715           ; $C57E: $D0 $F9
L_0719     LDA (VARNAM),y       ; $C580: $B1 $45
           STA (INPPTR),y       ; $C582: $91 $43
L_0721     DEY                  ; $C584: $88
           DEC VARNAM + 1       ; $C585: $C6 $46
           DEC INPPTR + 1       ; $C587: $C6 $44
           DEX                  ; $C589: $CA
           BNE L_0715           ; $C58A: $D0 $ED
           RTS                  ; $C58C: $60
L_0727     LDY #$00             ; $C58D: $A0 $00
           LDX INDEX + 1        ; $C58F: $A6 $23
           BEQ L_0738           ; $C591: $F0 $0E
L_0730     LDA (VARNAM),y       ; $C593: $B1 $45
           STA (INPPTR),y       ; $C595: $91 $43
           INY                  ; $C597: $C8
           BNE L_0730           ; $C598: $D0 $F9
           INC VARNAM + 1       ; $C59A: $E6 $46
           INC INPPTR + 1       ; $C59C: $E6 $44
           DEX                  ; $C59E: $CA
           BNE L_0730           ; $C59F: $D0 $F2
L_0738     LDX INDEX            ; $C5A1: $A6 $22
           BEQ L_0745           ; $C5A3: $F0 $08
L_0740     LDA (VARNAM),y       ; $C5A5: $B1 $45
           STA (INPPTR),y       ; $C5A7: $91 $43
           INY                  ; $C5A9: $C8
           DEX                  ; $C5AA: $CA
           BNE L_0740           ; $C5AB: $D0 $F8
L_0745     RTS                  ; $C5AD: $60
L_0746     LDY #$00             ; $C5AE: $A0 $00
           LDA (FREKZP),y       ; $C5B0: $B1 $FB
           BNE L_0751           ; $C5B2: $D0 $03
           INY                  ; $C5B4: $C8
           LDA (FREKZP),y       ; $C5B5: $B1 $FB
L_0751     RTS                  ; $C5B7: $60
L_0752     SED                  ; $C5B8: $F8
           LDA #$00             ; $C5B9: $A9 $00
           STA OPMASK           ; $C5BB: $85 $4D
           STA DEFPNT           ; $C5BD: $85 $4E
           STA DEFPNT + 1       ; $C5BF: $85 $4F
           LDY #$10             ; $C5C1: $A0 $10
L_0758     ASL OPPTR            ; $C5C3: $06 $4B
           ROL OPPTR + 1        ; $C5C5: $26 $4C
           LDA OPMASK           ; $C5C7: $A5 $4D
           ADC OPMASK           ; $C5C9: $65 $4D
           STA OPMASK           ; $C5CB: $85 $4D
           LDA DEFPNT           ; $C5CD: $A5 $4E
           ADC DEFPNT           ; $C5CF: $65 $4E
           STA DEFPNT           ; $C5D1: $85 $4E
           LDA DEFPNT + 1       ; $C5D3: $A5 $4F
           ADC DEFPNT + 1       ; $C5D5: $65 $4F
           STA DEFPNT + 1       ; $C5D7: $85 $4F
           DEY                  ; $C5D9: $88
           BNE L_0758           ; $C5DA: $D0 $E7
           CLD                  ; $C5DC: $D8
           LDA OPMASK           ; $C5DD: $A5 $4D
           AND #$F0             ; $C5DF: $29 $F0
           LSR                  ; $C5E1: $4A
           LSR                  ; $C5E2: $4A
           LSR                  ; $C5E3: $4A
           LSR                  ; $C5E4: $4A
           ORA #$30             ; $C5E5: $09 $30
           STA OLDTXT + 1       ; $C5E7: $85 $3E
           LDA OPMASK           ; $C5E9: $A5 $4D
           AND #$0F             ; $C5EB: $29 $0F
           ORA #$30             ; $C5ED: $09 $30
           STA DATLIN           ; $C5EF: $85 $3F
           LDA DEFPNT           ; $C5F1: $A5 $4E
           AND #$F0             ; $C5F3: $29 $F0
           LSR                  ; $C5F5: $4A
           LSR                  ; $C5F6: $4A
           LSR                  ; $C5F7: $4A
           LSR                  ; $C5F8: $4A
           ORA #$30             ; $C5F9: $09 $30
           STA OLDLIN + 1       ; $C5FB: $85 $3C
           LDA DEFPNT           ; $C5FD: $A5 $4E
           AND #$0F             ; $C5FF: $29 $0F
           ORA #$30             ; $C601: $09 $30
           STA OLDTXT           ; $C603: $85 $3D
           LDA DEFPNT + 1       ; $C605: $A5 $4F
           AND #$0F             ; $C607: $29 $0F
           ORA #$30             ; $C609: $09 $30
           STA OLDLIN           ; $C60B: $85 $3B
           RTS                  ; $C60D: $60
L_0801     LDY #$00             ; $C60E: $A0 $00
           STY FAC1             ; $C610: $84 $61
L_0803     LDA OLDLIN,y         ; $C612: $B9 $3B $00
           CMP #$30             ; $C615: $C9 $30
           BNE L_0810           ; $C617: $D0 $08
           CPY #$04             ; $C619: $C0 $04
           BEQ L_0810           ; $C61B: $F0 $04
           LDX FAC1             ; $C61D: $A6 $61
           BEQ L_0816           ; $C61F: $F0 $0A
L_0810     STY FAC1             ; $C621: $84 $61
           LDA #$05             ; $C623: $A9 $05
           SEC                  ; $C625: $38
           SBC FAC1             ; $C626: $E5 $61
           STA FAC1             ; $C628: $85 $61
           RTS                  ; $C62A: $60
L_0816     INY                  ; $C62B: $C8
           BNE L_0803           ; $C62C: $D0 $E4
L_0818     .BYTE $00            ; $C62E: $00
L_0819     .WORD $0000          ; $C62F: $00 $00
L_0820     .WORD $0000          ; $C631: $00 $00
L_0821     .WORD L_0821         ; $C633: $33 $C6

