;File: fig0527.pep
;Computer Systems, Fifth edition
;Figure 5.27
;
         BR      main        
bonus:   .EQUATE 10          ;constant
exam1:   .word  75           ;global variable #2d
exam2:   .word   65            ;global variable #2d
score:   .BLOCK  2           ;global variable #2d
;
main:    LDWA    exam1,d     ;score = (exam1 + exam2) / 2 + bonus
         ADDA    exam2,d     
         ASRA                
         ADDA    bonus,i     
         STWA    score,d     
         STOP                
         .END                  
