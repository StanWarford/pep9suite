br main
a:.equate 0 ;#2d5a 
test: subsp 10,i ;#a
ldwx 2,i
ldwa a, sx
addsp 10,i ;#a
ret
main: nop0
call mess
call test
mess: ldwx test,i
addx 1,i
ldwa 0,x
adda 2,i
stwa 0,x
ret
stop

.end