; This project is every kind of wrong
; First, some basic definitons
a:       .EQUATE 0           ; #2d
b:       .EQUATE 2           ; #2d
c:       .EQUATE 4           ; #2d4a
; Let's try cyclically defind data structures
; The assembler should be unable to resolve either tag, and should warn
st1:     .EQUATE 0           ;#st2 
st2:     .EQUATE 0           ;#st1 
         SUBSP   2,i         ; #st1 
         ADDSP   2,i         ; #st1 

; Let's try define structs in terms of other structs, out of order
strGlob: .BLOCK  4           ;#1c8a should trigger an error in anything including it
badSt4:  .EQUATE 12          ;#badSt3#b 
badSt3:  .EQUATE 10          ;#strGlob#a 
         SUBSP   12,i        ;push #badSt4 
         ADDSP   12,i        ;pop #badSt4 
strLocal:.EQUATE 0           ;#1c8a should not trigger bad inclusion behavior
st4:     .EQUATE 0           ;#st3#b
st3:     .EQUATE 0           ;#strLocal#a
         SUBSP   12,i        ;push #st4
         ADDSP   12,i        ;pop #st4
         .END                  