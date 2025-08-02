 top:
    lda bottom    
    .macro paul
    ldx \1
    inx
    .endm    
bottom:
    jmp top
    paul bottom
 