
    .org $4000


top       
    nop
-
    inx
    bne -
    
do_top

    .var val_first = 0
    .var val_second = 1
    .var val_next;
    .var fib_count = 0
    
    .do
        val_next = val_first + val_second
       .word val_next

        
        val_first = val_second
        val_second = val_next


        fib_count = fib_count + 1

        .var do_two = $1000

        .do
            .word do_two 
            do_two = do_two + $10
       .while do_two < $1050 


    .while fib_count < 10
    
    bne do_top
    jmp top
    
    rts
 Exit:
