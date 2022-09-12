#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "simpleEmulator.h"

#define TOTAL_RAM 4096
#define STARTING_ADRESS 0x200
#define STACK_SIZE 12
#define START_PC 0x200
#define DISPLAY_MEMORY_SIZE 2048
#define DISPLAY_X 64
#define DISPLAY_Y 32
#define VERBOSE true

// ram
static uint16_t ram[TOTAL_RAM];

// registers
static uint8_t registers[16];

// stack (space for 12 calls)
static uint16_t stack[STACK_SIZE];

// stack pointer
static uint8_t stackPointer;

// delay register
static uint8_t delayRegister;

// sound register
static uint8_t soundRegister;

// Program counter
static uint16_t programCounter;

// Display memory
static uint8_t displayMemory[2048];

// 0 for automatic execution
// 1 for step through execution
static int executionMode = 0; 

static char singlePixelLeft[8] = {1, 0, 0, 0, 0, 0, 0, 0};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("ERROR, not enough arguments given.\n");
        return 1;
    }
    // memset(singlePixelLeft, 1, 1);
    char* filename = argv[1];
    int i = 0;
    // while (true) {
    //     char currentChar = filename[i];
    //     if (currentChar == 0) { break ; }
    //     putc(currentChar, stdout);
    //     i++;
    // }
    // putc('\n', stdout);
    resetState();
    // printf("Size int is %lu\n", sizeof(int));
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: File opening failed.\n");
        return 1;
    }
    loadProgram(fp);
    // uint8_t* tempDisplayStart = displayMemory + 72;
    // memset(tempDisplayStart, 1, 10);
    stepLoop(fp);
    int closeCode = fclose(fp);
    if (closeCode != 0) {
        printf("ERROR: File closing failed.\n");
        return 1;
    }
    return 0;
}

void resetState() {
    for (int i = 0; i < TOTAL_RAM; i++) {
        ram[i] = 0x0000;
    }
    memset(registers, 0x00, 16);
    memset(stack, 0, STACK_SIZE * 2);
    stackPointer = 0;
    delayRegister = 0;
    soundRegister = 0;
    programCounter = START_PC;
    memset(displayMemory, 0, DISPLAY_MEMORY_SIZE);
}

void loadProgram(FILE *filePointer) {
    // Read in two bytes from file
    // Check for EOF
    // Store bytes 
    char buffer[2];
    char firstByte[1];
    char secondByte[1];
    int startRamIndex = STARTING_ADRESS / 2;
    while(fread(firstByte, 1, 1, filePointer) == 1) {
        fread(secondByte, 1, 1, filePointer);
        *buffer = 0;
        // NOTE: Order of bytes reversed because of endianess mismatch
        memcpy(buffer, secondByte, 1);
        memcpy(&buffer[1], firstByte, 1);
        printDoubleByteInBinary(buffer);
        printf("%c", '\n');
        memcpy(ram + startRamIndex, buffer, 2);
        startRamIndex++;
    }
}

void printDoubleByteInBinary(char* doubleByte) {
    char byte0 = (doubleByte)[0];
    char byte1 = (doubleByte)[1];
    for (int j = 0; j < 8; j++) {
        printf("%d", !!((byte0 << j) & 0x80));
    }
    printf(" ");
    for (int j = 0; j < 8; j++) {
        printf("%d", !!((byte1 << j) & 0x80));
    }
    printf("%c", '\n');
}

void stepLoop(FILE* fp) {
    // read instruction
    int ramIndex = programCounter / 2;
    uint16_t hex12;
    uint8_t lowByte;
    uint8_t xRegister;
    uint8_t yRegister;
    uint8_t smallestNibble;
    uint16_t instruction;
    char opcode;
    bool verboseOpcode = false;
    bool verboseInstruction = true;
    bool manualMemoryChange;

    while (true) {
        manualMemoryChange = false;
        memcpy(&instruction, &ram[ramIndex], 2);
        if (instruction == 0x0000) { break ;}
        opcode = (instruction >> 12);
        hex12 = (instruction & 0x0fff);
        lowByte = (instruction & 0x00ff);
        xRegister = (instruction & 0x0f00) >> 8;
        yRegister = (instruction & 0x00f0) >> 4;
        smallestNibble = (instruction & 0x000f);
        switch(opcode) {
            case 0:
                if (verboseOpcode) { printf("Opcode is 0\n");}
                if (instruction == 0x00E0) {
                    if (verboseInstruction) { printf("CLEAR\n"); }
                } else if (instruction == 0x00EE) {
                    if (verboseInstruction) { printf("RETURN FROM SUBROUTINE\n"); }
                } else {
                    if (verboseInstruction) { printf("Execute machine language subroutine (?) at address %04x\n)", hex12); }
                }
                break;
            case 1:
                if (verboseOpcode) { printf("Opcode is 1\n");}
                if (verboseInstruction) { printf("Jump to address %d\n", hex12); }
                break;
            case 2:
                if (verboseOpcode) { printf("Opcode is 2\n");}
                if (verboseInstruction) { printf("Execute subroutine at address %d\n", hex12); }
                break;
            case 3:
                if (verboseOpcode) { printf("Opcode is 3\n");}
                if (verboseInstruction) { printf("Skip next instruction if register %d equals %d\n", xRegister, lowByte); }
                break;
            case 4:
                if (verboseOpcode) { printf("Opcode is 4\n");}
                if (verboseInstruction) { printf("Skip next instruction if register %d is not equal to %d\n", xRegister, lowByte); }
                break;
            case 5:
                if (verboseOpcode) { printf("Opcode is 5\n");}
                if (verboseInstruction) { printf("Skip next instruction if register %d and %d are equal\n", xRegister, yRegister); }
                break;
            case 6:
                if (verboseOpcode) { printf("Opcode is 6\n");}
                if (verboseInstruction) { printf("Store number %u in register %d\n", lowByte, xRegister); }
                break;
            case 7:
                if (verboseOpcode) { printf("Opcode is 7\n");}
                if (verboseInstruction) { printf("Add value %d to register %d\n", lowByte, xRegister); }
                break;
            case 8:
                if (verboseOpcode) { printf("Opcode is 8\n");}
                if (smallestNibble == 0) {
                    if (verboseInstruction) { printf("Register %d value stored in register %d\n", yRegister, xRegister); }
                } else if (smallestNibble == 1) {
                    if (verboseInstruction) { printf("OR register %d to register %d\n", xRegister, yRegister); }
                } else if (smallestNibble == 2) {
                    if (verboseInstruction) { printf("AND register %d to register %d\n", xRegister, yRegister); }
                } else if (smallestNibble == 3) {
                    if (verboseInstruction) { printf("XOR register %d to register %d\n", xRegister, yRegister); }
                } else if (smallestNibble == 4) {
                    if (verboseInstruction) { printf("Add register %d to register %d and set carry flag\n", yRegister, xRegister); }
                } else if (smallestNibble == 5) {
                    if (verboseInstruction) { printf("Subtract register %d to register %d and set borrow flag\n", yRegister, xRegister); }
                } else if (smallestNibble == 6) {
                    if (verboseInstruction) { printf("Set register %d to register %d right shifted by one and set VF to old least significant digit.\n", xRegister, yRegister); }
                } else if (smallestNibble == 7) {
                    if (verboseInstruction) { printf("Set register %d to register %d minus register %d and set VF 00 if borrow occurs\n", xRegister, yRegister, xRegister); }
                } else if (smallestNibble == 14) {
                    if (verboseInstruction) { printf("Set register %d to register %d left shifted by one and set VF to old most significant digit.\n", xRegister, yRegister); }
                }
                break;
            case 9:
                if (verboseOpcode) { printf("Opcode is 9\n");}
                if (verboseInstruction) { printf("Skip next instruction if register %d and %d are NOT equal\n", xRegister, yRegister); }
                break;
            case 10:
                if (verboseOpcode) { printf("Opcode is 10\n");}
                if (verboseInstruction) { printf("Store memory adress %d in I register\n", hex12); }
                break;
            case 11:
                if (verboseOpcode) { printf("Opcode is 11\n");}
                if (verboseInstruction) { printf("Jump to address %d + value of register 0\n", hex12); }
                break;
            case 12:
                if (verboseOpcode) { printf("Opcode is 12\n");}
                if (verboseInstruction) { printf("Set register %d to a random number with mask %d\n", xRegister, lowByte); }
                break;
            case 13:
                if (verboseOpcode) { printf("Opcode is 13\n");}
                if (verboseInstruction) { printf("Draw sprite at position in register %d and %d\n", xRegister, yRegister); }
                break;
            case 14:
                if (verboseOpcode) { printf("Opcode is 14\n");}
                if (lowByte == 0x9E) {
                    if (verboseInstruction) { printf("Skip instruction if key in register %d is pressed\n", xRegister); }
                } else if (lowByte == 0xA1) {
                    if (verboseInstruction) { printf("Skip instruction if key in register %d is NOT pressed\n", xRegister); }
                }
                break;
            case 15:
                if (verboseOpcode) { printf("Opcode is 15\n");}
                if (lowByte == 0x07) {
                    if (verboseInstruction) { printf("Delay timer value to register %d\n", xRegister); }
                } else if (lowByte == 0x0A) {
                    if (verboseInstruction) { printf("Wait for key press and store in %d\n", xRegister); }
                } else if (lowByte == 0x15) {
                    if (verboseInstruction) { printf("Set delay timer to value in register %d\n", xRegister); }
                } else if (lowByte == 0x18) {
                    if (verboseInstruction) { printf("Set sound timer to value in register %d\n", xRegister); }
                } else if (lowByte == 0x1E) {
                    if (verboseInstruction) { printf("Add value in register %d to I\n", xRegister); }
                } else if (lowByte == 0x29) {
                    if (verboseInstruction) { printf("Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register %d\n", xRegister); }
                } else if (lowByte == 0x33) {
                    if (verboseInstruction) { printf("Store the binary-coded decimal equivalent of the value stored in register %d at addresses I, I + 1, and I + 2\n", xRegister); }
                } else if (lowByte == 0x55) {
                    if (verboseInstruction) { printf("Store the values of registers V0 to V%d inclusive in memory starting at address I is set to I + X + 1 after operation\n", xRegister); }
                } else if (lowByte == 0x65) {
                    if (verboseInstruction) { printf("Fill registers V0 to V%d inclusive with the values stored in memory starting at address I is set to I + X + 1 after operation\n", xRegister); }
                }
                break;
            default:
                printf("ERROR, opcode unknown\n");
        }

        if (VERBOSE) {
            // printState();
        }
        // printDisplay();
        if (!manualMemoryChange) {
            ramIndex += 1;
        }
    }
}

void printDisplay() {
    char emptyPixel = '.';
    char fullPixel = '#';
    if (displayMemory[0] == 0) {
        putc(emptyPixel, stdout);
    } else {
        putc(fullPixel, stdout);
    }
    putc(displayMemory[0], stdout);
    for (int i = 1; i < DISPLAY_MEMORY_SIZE; i++) {
        if (displayMemory[i] == 0) {
            putc(emptyPixel, stdout);
        } else {
            putc(fullPixel, stdout);
        }
        if ((i+1) % 64 == 0) {
            putc('\n', stdout);
        }
    }
}

void printState() {
    printRegisters();
    //printLinesStartingMemory(10);
    fullPrintNonZeroMemory();
    printStack();
}

void printRegisters() {
    for (int i = 0; i < 16; i++) {
        printf("V%d %u\n", i, registers[i]);
    }
}

void printLinesStartingMemory(int linesToPrint) {
    uint16_t currentLine = STARTING_ADRESS;
    uint16_t maxLine = currentLine + linesToPrint;
    while (currentLine < maxLine) {
        printf("Line %u is    %u\n",currentLine, ram[currentLine]);
        currentLine++;
    }
}

void printLinesAfterPC(int linesToPrint) {
    uint16_t currentLine = programCounter;
    uint16_t maxLine = programCounter + linesToPrint;
    while (currentLine < maxLine) {
        printf("Line %u is    %u\n",currentLine, ram[currentLine]);
        currentLine++;
    }
}

void fullPrintNonZeroMemory() {
    int currentLine = 0;
    printf("Printing any non-zero memory values\n");
    while (currentLine < TOTAL_RAM) {
        if (ram[currentLine] != 0) {
            printf("Line %d is     %04x\n", currentLine, ram[currentLine]);
        }
        currentLine++;
        // if (ram[currentLine] != 0) {
        //     printf("Line %d is    %hu\n",currentLine, ram[currentLine]);
        // }
        // currentLine++;
    }
}

void printStack() {
    int currentIndex = 0;
    while (currentIndex < STACK_SIZE) {
        printf("Stack index %d is   %hu\n", currentIndex, stack[currentIndex]);
        currentIndex++;
    }
}