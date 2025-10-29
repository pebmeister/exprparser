    * = $2000
    
Mystorage
    .ds 2
        
    .org $4000
        
top
    nop
    
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
    mac Mystorage, Morestorage
    bad top
    bad here
    
Morestorage:
    .ds 2
    