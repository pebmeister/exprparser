    .macro mac2
    nop
    iny
    nop
    iny
    nop
    iny
    .endm
 
    .macro mac
    ldx \1
    inx
    mac2 $123
    inx
    .endm
 
    .macro bad
    lda #0
    .endm
 
    mac $4545
    bad 6 
    