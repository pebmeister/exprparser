;    OUTLOOP = 1
    
    .org $4000
.ifdef OUTLOOP
    .var outer = 0
.endif
   
top       
    nop
-

do_top
.ifdef OUTLOOP
    .do
.endif

    inx
    .var count = 0
    .do
        .word count
        asl
        sta $1000 + count, x
        count = count + 1
        iny
    .while count < 10

.ifdef OUTLOOP
    outer = outer + 1
    .while outer < 1 
.endif

    bne do_top
    jmp top
    
    rts
 Exit:
