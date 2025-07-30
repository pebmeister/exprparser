    
top:
    lda bottom

    .macro paul
    ldx \1
    inx
    inx
    .endm
    
bottom:
    jmp top
    paul bottom
 