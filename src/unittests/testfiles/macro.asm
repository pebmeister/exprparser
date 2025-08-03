    .macro paul
    ldx \1
    inx
    .endm    

top:
    lda middle    

middle:
    paul bottom - top
    
bottom:
    nop
    