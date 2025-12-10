 
    .org $4000

    temp = *
  
    .var aa = $01
    .var bb = $1000
    
    .do
        .byte aa
        .do 
            .word bb
            bb = bb + 2
        .while bb < $1010
        aa = aa + 1
    .while aa < 5
    
 
  
   .var fib1 = 0
   .var fib2 = 1
   .var nextTerm;

    .do
        nextTerm = fib1 + fib2
        .if nextTerm <= 255
            .byte nextTerm
        .else
            .word nextTerm
        .endif
        fib1 = fib2;
        fib2 = nextTerm;
    .while (nextTerm < 512) ; up to 512

top       
    nop
    .fill 23, 29
 -
    inx
    sta temp
  
    .if 0
    THIS IS GARBAGE : # @ not valid
    .else
    lda #$45
    .endif

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
    
