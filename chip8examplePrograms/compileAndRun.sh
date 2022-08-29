assemblerPath="/home/scott/Dropbox/Programming/chip8-master"
$assemblerPath/./c8asm ./$1.asm -o ./$1.ch8;
if [ "$?" -ne 0 ]
then
	echo "ASSEMBLY ERROR, PROGRAM NOT RUN"
	exit 1
fi
$assemblerPath/./chip8 $1.ch8
