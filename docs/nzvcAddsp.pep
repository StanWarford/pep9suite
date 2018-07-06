subsp 0xf000,i ;Make the SP a positive integer
ldwa 0x9fff,i
adda 0xd002,i
addsp 1,i ;Adding 1 should neither have a carry or an overflow, since I'm adding 2 small positives.
ldwa 0,i
addsp 1,i ; The SP should be nonzero, yet the Z flag is set
LDWA 0xffff,i
addsp 1,i ;The SP is certainly positive, yet the N flag is set
stop
.end