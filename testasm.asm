        .macro ADD16
        clc
        lda \1
        adc \2
        sta \3
        lda \1 + 1
        adc \2 + 1
        sta \3 + 1
        .endm
        
        Paul = $123             ; ok
        Mike = Paul + Fred + 2  ; perfect
        
        .org $2000
        .byte $20, $30, $40, $50, $60
                
        jsr Paul + 456
        lda #5 % 2
        ldx # 10 %3
        ldy # %1111 % %100
        lda ~$F0F0
        
@here
        asl a
        iny
        bne @here
        
        ADD16 $1000 + 2 - 1 + 4 * 4, $2000, $3000
        anc  #%1010 / %10 ; wow
        anc2 #$76 + %1010
        
        lda #(Mike & $FF) +44 / 2

        ADD16 Mike, Paul, Fred 
        
        jsr  Fred
        nop
Fred
        jsr Paul
        
        and $46

        ; ********************
        ; set s pointers
        ; ********************
        ldx #192 + 4 * (9 + 2)
        stx 2040                ; set sprite 0's pointer
        stx 2042                ; set sprite 2's pointer
        stx 2044                ; set sprite 4's pointer
        inx
        asl
        stx 2041                ; set sprite 1's pointer
        stx 2043                ; set sprite 3's pointer
        stx 2045                ; set sprite 5's pointer
 