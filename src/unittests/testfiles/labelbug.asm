    .inc 'basic.asm' ; include basic

    .org $4000


top       
    nop
-
    inx
    bne -
    
do_top

    .var val_first = 0
    .var val_second = 1
    .var val_next
    .var fib_count
    
    .do
        val_next = val_first + val_second
        .word val_next
        val_first = val_second
        val_second = val_next
        fib_count = fib_count + 1
        
        .var do_two = $1000
        .while do_two < $1050 
            .word do_two 
            do_two = do_two + $30
        .wend
    .while fib_count <= 5
    
    bne do_top
    jmp top
    
    rts
 Exit:
