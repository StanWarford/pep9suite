call a
xy: .equate 0 ;#2d3a
a:call b 
b:subsp 6,i ;#xy
ret
addsp 6,i ;#xy
stop
.end