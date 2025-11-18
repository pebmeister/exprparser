 
    .org $4000

   
CirclePlot
        ; define circle parameters
        ; and variables
        * = $CE00
        CirSave .ds 1
        * = CirclePlot
        
        jsr CirSave
EXIT:   rts
        
    