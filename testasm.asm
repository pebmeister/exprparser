
        Paul = $123   ; ok
        Mike = Paul + Fred + 2 ; perfect

        .org $2000
        
        jsr Paul + 456
@here
        asl a
        iny
        bne @here
        
        anc  #%1010 / %10 ; wow
        anc2 #$76 + %1010
        
        lda #(Mike & $FF) +44 / 2
        
        jsr  Fred
        nop
Fred
        jsr Paul
        
        and $46

        ; ********************
        ; set sprite pointers
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
        
        
