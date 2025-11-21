 
    .org $4000


top       
    nop
 -
    inx
    
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
        ; mac2 
        nop
        iny
        nop
        iny
        nop
        iny
        sta \1
        ; end mac2
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
        ldx #24
    .endm

    @localMac()
    beq -
    
there:
-
    bpl -
    mac CX_MINUS_CURY, CX_MINUS_CURX
    bad top
    bad here
    bne --
    
    
Morestorage:
    .ds 2
    
