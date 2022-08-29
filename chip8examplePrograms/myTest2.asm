define key V0
define posY V1

LD posY, 0
loop:
LD key, K
CLS
LD I, sprite1
DRW key, posY, 1
JP loop

sprite1:
db  %10000000
