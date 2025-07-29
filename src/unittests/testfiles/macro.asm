    
top:
    jmp bottom

    .macro paul
    ldx \1
    inx
    .endm
    

bottom:

    jmp bottom
    jmp top
    
    