#include <stdio.h>

void stepLoop(FILE* fp);
void resetState();
void printState();
void printRegisters(); 
void printLinesStartingMemory();
void fullPrintNonZeroMemory();
void printStack();
void printLinesAfterPC(int linesToPrint);
void loadProgram(FILE *filePointer);
void printDisplay();
void printDoubleByteInBinary(char* doubleByte);