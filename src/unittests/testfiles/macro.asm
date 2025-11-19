 
    .org $4000


top        
    nop
   
CirclePlot
        ; define circle parameters
        ; and variables
        * = $CE00
        CirSave         .ds 1
        CX_MINUS_CURY   .ds 2
        CX_MINUS_CURX   .ds 2
        CX_PLUS_CURY    .ds 2
        CX_PLUS_CURX    .ds 2

        CY_MINUS_CURY   .ds 1
        CY_MINUS_CURX   .ds 1
        CY_PLUS_CURY    .ds 1
        CY_PLUS_CURX    .ds 1
        * = CirclePlot
        
    .macro mac2
    nop
    iny
    nop
    iny
    nop
    iny
    sta \1
    .endm
 
    .macro mac
    ldx \1
    inx
    mac2 \2
    inx
    .endm
 
    .macro bad
    lda \1
    .endm




 here:
     .macro @localMac
        ldx #\1
    .endm

    @localMac here
    
    mac CX_MINUS_CURY, CX_MINUS_CURX
    bad top
    bad here
    
Morestorage:
    .ds 2
    