;definitions
define posX V0
define posY V1
define oldX V2
define oldY V3
define key V4
define tmp V5


; initalizations
CLS
LD V0, 3
LD DT, V0
LD posX, 1
LD posY, 1
LD I, sprite1
DRW posX, posY, 8

;main loop
loop:
LD oldX, posX
LD oldY, posY

; draw subroutine
draw:
LD tmp, 1
SKNP TMP
JP pause
normalDraw:
LD tmp, DT
SE tmp, 0 
JP draw
LD I, sprite1
DRW oldX, oldY, 8
ADD posX, 1
DRW posX, posY, 8
LD tmp, 3
LD DT, tmp
JP loop

pause:
SE key, 0
JP secondPause
LD key, 1
JP normalDraw
secondPause:
LD key, 0
JP normalDraw

notQuitLoop:
JP notQuitLoop

EXIT

sprite1:
db  %01111110,
    %10000001,
    %10100101,
    %10111101,
    %10111101,
    %10011001,
    %10000001,
    %01111110,