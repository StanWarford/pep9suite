         BR      main        
         .ALIGN  2           
;#####################
gDepth:  .WORD   0x0002      ;#2d Change the value of this word to change which fib number is calculated 
;######################
gRes:    .BLOCK  2           ;#2d
retVal:  .EQUATE 8           ;#2d
depth:   .EQUATE 6           ;#2d
;rAddr:	.EQUATE	4
it:      .EQUATE 2           ;#2d
arr:     .EQUATE 0           ;#2d
fib:     SUBSP   4,i         ;push #it #arr 
         LDWA    2,i         
         STWA    it,s        
         LDWA    depth,s     
         CALL    new         
         STWX    arr,s       
;The address of the first byte of the arr is already in X, so use a simpler addr mode
         LDWA    1,i         
         STWA    0,x         
         STWA    2,x         
;Prepare for main loop
         LDWX    it,s        
loop:    CPWX    depth,s     
         BRGE    eLoop       ;End iter if it >= depth
         ASLX                ;Convert idx number to address of word
;The current value of the accumulator is mem[it-1], so all we need to do
;is add to the accumulator mem[it-2]
         SUBX    4,i         ;2 places back is 4 bytes
         ADDA    arr,sfx     
;Store to computer address mem[it] and store next value to mem[it]
         ADDX    4,i         
         STWA    arr,sfx     
;Must load iterator again, can't use right shift, otherwise sign extension on shift causes problems.
         LDWX    it,s        
         ADDX    1,i         
         STWX    it,s        
         BR      loop        
         NOP0                
eLoop:   STWA    retVal,s    
         LDWX    arr,s       
         CALL    del         
         ADDSP   4,i         ;pop #it #arr
         RET                 
         .ALIGN  2           
main:    LDWA    gDepth,d    
         STWA    -4,s        
         SUBSP   4,i         ;push #retVal #depth
         CALL    fib         
         ADDSP   4,i         ;pop ##retVal #depth
         LDWA    -2,s        
         STWA    gRes,d      
         HEXO    gRes,d      ;implemented as nop in MAL
         LDWA    gRes,d      
         STOP                
;Assuming delete actually did something
del:     NOP0                
         RET                 
;Assume size to alloc is passed in Acc, address of first byte return in X
new:     LDWX    hPtr,d      
         ADDA    hPtr,d      
         STWA    hPtr,d      
         RET                 
         .ALIGN  2           
hPtr:    .ADDRSS heap        ;#2d
heap:    .BLOCK  2           ;#2d
         .END                  