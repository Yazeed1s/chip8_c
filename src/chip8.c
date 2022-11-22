#include "inc/chip8.h"
#include "inc/SDL2/SDL.h"
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
    Main components of CHIP-8 :
        - Virtual RAM memory space: 4 kB (4096 Bytes)
        - Memory space for fonts (80 bytes)
        - Memory space for display (64x32)
        - The stack, a LIFO array of 16-bit values used for subroutines
        - The keyboard, which contains 16 keys used as input
        - 16 general purpose 8-bit registers
        - The delay timer DT register (8-bit)
        - The sound timer ST register (8-bit)
        - The index register I (16-bit), used to store memory addresses
        - The program counter PC, another pseudo-register (16-bit) that points to
            the address in memory of the current instruction
        - The stack pointer SP, a pseudo-register (8 or 16-bit, depending on the size of the stack)
            that points to the top of the stack
*/

/*
    CHIP-8 memory map:
        The Chip-8 emulator reuquires up to 4KB (4,096 bytes) of RAM, from location 0x000 (0) to 0xFFF (4095).
        The first 512 bytes(from 0x000 to 0x200) are where the original interpreter was located, and should not
        be used by programs (games).
        ADDRESS                                        		CONTENT
        ~~~~~~~                                        		~~~~~~~
        0x000(0)    --------------------------------  <--  Start of RAM
                    |                              |
                    |  Interpreter code, fonts     |
                    |                              |
        0x200(512)  --------------------------------  <--  Start of most CHIP-8 programs
                    |                              |
                    |       (CHIP-8 program)       |
                    |       programs and data      |
                    |          go here             |
                    |                              |
        0x600(1536) --------------------------------  <--  Start of most ETI 660 CHIP-8 programs
                    |                              |
                    |				   |
                    |    (ETI 660 CHIP-8 program)  |
                    |       programs and data      |
                    |          go here             |
                    |                              |
                    |                              |
        0xFFF(4095) --------------------------------  <--  End of RAM
*/

const unsigned char fontSet[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// -----------CHIP-8 implementation--------------

/**
 * @brief chInit(chip8) is used to load the fonts into the memory
 * it pupulates memory rooms from 0x00(0) to 0x4F(79)
 * @param struct Chip8 *chip8
 * @return void
 */
void chInit(struct Chip8 *chip8) {
    // pupulates memory rooms from 0x00(0)to 0x4F(79)with font[] elements
    // rooms 0x4F to 0x200 are reserved for VM operations
    memset(chip8, 0, sizeof(struct Chip8));
    int n = sizeof(fontSet);
    for (int i = 0; i < n; i++) {
        chip8->memory.memory[i] = fontSet[i];
    }
}

/**
 * @brief loadROM(chip8, buf) is used to load the ROM program to the memory
 * starting from 0x200(512) to 0xFFF(4095), it pupulates the needed memory rooms
 * depending on the size/needs of the program
 * @param chip8 chip8's memory
 * @param buf (read-only-memory) file to read from
 * @return void
 */
void chLoad(struct Chip8 *chip8, const char *buf) {

    FILE *ptr;
    int i, c;
    if ((ptr = fopen(buf, "r+")) == 0x00) {
        printf("Error opening file\n");
    }
    for (i = 0; (c = fgetc(ptr)) != EOF; i++) {
        chip8->memory.memory[i + 0x200] = c;
        // i + 0x200 guarantees that ROM is loaded beyound room 0x200
    }
    fclose(ptr);
    chip8->registers.PC = 0x200;
}

static char chip8_wait_for_key_press(struct Chip8 *chip8) {
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;
        char chip8_key = mapKey(&chip8->keyboard, c);
        if (chip8_key != -1) {
            return chip8_key;
        }
    }

    return -1;
}

/**
 * @brief execOpcode(chip8, opcode) is used to execute the chip8 instruction
 * uses long nested switch statements to get and match the full opcode
 * @param chip8 chip8's memory
 * @param opcode the opcode which defines the instruction
 * @return void
 */
void execOpcode(struct Chip8 *chip8, unsigned short opcode) {

    // https://en.wikipedia.org/wiki/CHIP-8
    // https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

    // X: The second nibble.
    // Used to look up one of the 16 registers (VX)
    // from V0 through VF.
    unsigned char X = (opcode >> 8) & 0x000F;
    // Y: The third nibble.
    // Also used to look up one of the 16 registers (VY)
    // from V0 through VF.
    unsigned char Y = (opcode >> 4) & 0x000F;
    // N: The fourth nibble. A 4-bit number.
    unsigned char N = opcode & 0x000F;
    // NN: The second byte (third and fourth nibbles).
    // An 8-bit immediate number.
    unsigned char NN = opcode & 0x00FF;
    // NNN: The second, third and fourth nibbles.
    // A 12-bit immediate memory address.
    unsigned short NNN = opcode & 0x0FFF;

    // bitwise AND the 1st nibble
    // then swicth and match all values in that F place
    switch (opcode & 0xF000) {
    case 0x0000: {
        // 0 is 1st nibble in more than one opcode
        // swaitch on the opcode with the last two nibbles
        switch (opcode & 0x00FF) {
        // 00E0: Clears the screen
        case 0x00E0: {
            printf("0x%X: 00E0\n", opcode);
            clearScreen(&chip8->screen);
        } break;
            // 00EE: Return from subroutine
        case 0x00EE: {
            printf("0x%X: 00EE\n", opcode);
            chip8->registers.PC = stackPop(chip8);
        } break;
        default:
            // printf("unknown opcode");
            break;
        }
    } break;
    // 1NNN: Jumps to address NNN
    case 0x1000: {
        printf("0x%X: 1NNN\n", opcode);
        chip8->registers.PC = NNN;
    } break;
    // 2NNN: Calls subroutine at NNN
    case 0x2000: {
        printf("0x%X: 2NNN\n", opcode);
        stackPush(chip8, chip8->registers.PC);
        chip8->registers.PC = NNN;

    } break;

    // 3XNN: Skips the next instruction if Vx equals NN
    case 0x3000: {
        printf("0x%X: 3XNN\n", opcode);
        if (chip8->registers.V[X] == NN) {
            chip8->registers.PC += 2;
        }
    } break;
    // 4XNN: Skips the next instruction if Vx !equal NN
    case 0x4000: {
        printf("0x%X: 4XNN\n", opcode);
        if (chip8->registers.V[X] != NN) {
            chip8->registers.PC += 2;
        }
    } break;

    // 5XY0: Skips the next instruction if Vx equals Vy
    case 0x5000: {
        printf("0x%X: 5XY0\n", opcode);
        if (chip8->registers.V[X] == chip8->registers.V[Y]) {
            chip8->registers.PC += 2;
        }
    } break;

    // 6XNN: Sets Vx to NN
    case 0x6000: {
        printf("0x%X: 6XNN\n", opcode);
        chip8->registers.V[X] = NN;
    } break;

    // 7XNN: Adds NN to Vx
    case 0x7000: {
        printf("0x%X: 7XNN\n", opcode);
        chip8->registers.V[X] += NN;
    } break;

    case 0x8000: {
        switch (opcode & 0x000F) {
        // 8XY0: Sets Vx to the value of Vy
        case 0x0000: {
            printf("0x%X: 8XY0\n", opcode);
            chip8->registers.V[X] = chip8->registers.V[Y];
        } break;

        // 8XY1: Sets VX to VX or VY. (bitwise OR operation)
        case 0x0001: {
            printf("0x%X: 8XY1\n", opcode);
            chip8->registers.V[X] = chip8->registers.V[X] | chip8->registers.V[Y];
        } break;

        // 8XY2: Sets VX to VX and VY. (bitwise AND operation)
        case 0x0002: {
            printf("0x%X: 8XY2\n", opcode);
            chip8->registers.V[X] = chip8->registers.V[X] & chip8->registers.V[Y];
        } break;

        // 8XY3: Sets VX to VX xor VY (bitwise OR operation)
        case 0x0003: {
            printf("0x%X: 8XY3\n", opcode);
            chip8->registers.V[X] = chip8->registers.V[X] ^ chip8->registers.V[Y];
        } break;

        // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry,
        // and to 0 when there is not.
        case 0x0004: {
            printf("0x%X: 8XY4\n", opcode);
            unsigned short tmp = 0;
            tmp = chip8->registers.V[X] + chip8->registers.V[Y];
            chip8->registers.V[0x0F] = false;
            if (tmp > 0xFF) {
                chip8->registers.V[0x0F] = true;
            }
            chip8->registers.V[X] = tmp;
        } break;

        // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow,
        // and 1 when there is not.
        case 0x0005: {
            printf("0x%X: 8XY5\n", opcode);
            chip8->registers.V[0x0F] = false;
            if (chip8->registers.V[X] > chip8->registers.V[Y]) {
                chip8->registers.V[0x0F] = true;
            }
            chip8->registers.V[X] = chip8->registers.V[X] - chip8->registers.V[Y];
        } break;

        // 8XY6: Stores the least significant bit of VX in VF
        // and then shifts VX to the right by 1
        case 0x0006: {
            printf("0x%X: 8XY6\n", opcode);
            chip8->registers.V[0x0F] = chip8->registers.V[X] & 0x01;
            chip8->registers.V[X] = chip8->registers.V[X] >> 1;
        } break;

        // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow,
        // and 1 when there is not.
        case 0x0007: {
            printf("0x%X: 8XY7\n", opcode);
            chip8->registers.V[0x0F] = chip8->registers.V[Y] > chip8->registers.V[X];
            chip8->registers.V[X] = chip8->registers.V[Y] - chip8->registers.V[X];
        } break;

        // 8XYE: Stores the most significant bit of VX in VF
        // and then shifts VX to the left by 1
        case 0x000E: {
            printf("0x%X: 8XYE\n", opcode);
            chip8->registers.V[0x0F] = chip8->registers.V[X] & 0x01;
            chip8->registers.V[X] = chip8->registers.V[X] << 1;
        } break;
        }
    } break;

    // 9XY0: Skips the next instruction if VX does not equal VY.
    // (Usually the next instruction is a jump to skip a code block);
    case 0x9000: {
        printf("0x%X: 9XY0\n", opcode);
        if (chip8->registers.V[X] != chip8->registers.V[Y]) {
            chip8->registers.PC += 2;
        }
    } break;

    // ANNN: Sets I to the address NNN.
    case 0xA000: {
        printf("0x%X: ANNN\n", opcode);
        chip8->registers.I = NNN;
    } break;

    // BNNN: Jumps to the address NNN plus V0.
    case 0xB000: {
        printf("0x%X: BNNN\n", opcode);
        chip8->registers.PC = NNN + chip8->registers.V[0x00];
    } break;
    // CXNN: Sets VX to the result of a bitwise and operation
    // on a random number (Typically: 0 to 255) and NN.
    // 0xFF == 255
    case 0xC000: {
        printf("0x%X: CXNN\n", opcode);
        srand(clock());
        chip8->registers.V[X] = (rand() % 0xFF) & NN;
    } break;

    // DXYN - DRW Vx, Vy, nibble. Draws sprite to the screen
    // bool drawSprite(struct Screen *screen, int x, int y, const char *sprite, int num)
    case 0xD000: {
        printf("0x%X: DXYN\n", opcode);
        const char *sprite = (const char *)&chip8->memory.memory[chip8->registers.I];
        chip8->registers.V[0x0F] = drawSprite(&chip8->screen, chip8->registers.V[X], chip8->registers.V[Y], sprite, N);
    } break;

    // Keyboard operations
    case 0xE000: {
        switch (opcode & 0x00FF) {
        // EX9E: Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump
        // to skip a code block).
        case 0x009E: {
            printf("0x%X: EX9E\n", opcode);
            if (keyIsDown(&chip8->keyboard, chip8->registers.V[X])) {
                chip8->registers.PC += 2;
            }
        } break;

        // EXA1: Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a
        // jump to skip a code block).
        case 0x00A1: {
            printf("0x%X: EXA1\n", opcode);
            if (!keyIsDown(&chip8->keyboard, chip8->registers.V[X])) {
                chip8->registers.PC += 2;
            }
        } break;
        }
    } break;

    case 0xF000: {
        switch (opcode & 0x00FF) {
        // FX07: Sets VX to the value of the delay timer.
        case 0x0007: {
            printf("0x%X: EX07\n", opcode);
            chip8->registers.V[X] = chip8->registers.delay_timer;
        } break;
        // FX0A: A key press is awaited, and then stored in VX
        // (blocking operation, all instruction halted until next key event).
        case 0x000A: {
            printf("0x%X: FX0A\n", opcode);
            char pressed_key = chip8_wait_for_key_press(chip8);
            chip8->registers.V[X] = pressed_key;
        } break;
        // FX15: Sets the delay timer to VX.
        case 0x0015: {
            printf("0x%X: FX15\n", opcode);
            chip8->registers.delay_timer = chip8->registers.V[X];
        } break;

        // FX18: Sets the sound timer to VX.
        case 0x0018: {
            printf("0x%X: FX18\n", opcode);
            chip8->registers.sound_timer = chip8->registers.V[X];
        } break;
        // FX1E: Adds VX to I. VF is not affected
        case 0x001E: {
            printf("0x%X: FX1E\n", opcode);
            chip8->registers.I += chip8->registers.V[X];
        } break;

        // FX29: Sets I to the location of the sprite for the character in VX.
        // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
        case 0x0029: {
            printf("0x%X: FX29\n", opcode);
            chip8->registers.I = chip8->registers.V[X] * 5;
        } break;

        // FX33: Stores the binary-coded decimal representation of VX,
        // with the hundreds digit in memory at location in I,
        // the tens digit at location I + 1,
        // and the ones digit at location I + 2.
        case 0x0033: {
            printf("0x%X: FX33\n", opcode);
            unsigned char hundreds = chip8->registers.V[X] / 100;
            unsigned char tens = chip8->registers.V[X] / 10 % 10;
            unsigned char units = chip8->registers.V[X] % 10;
            chip8->memory.memory[chip8->registers.I] = hundreds;
            chip8->memory.memory[chip8->registers.I + 1] = tens;
            chip8->memory.memory[chip8->registers.I + 2] = units;
        } break;

        // FX55: Stores from V0 to VX (including VX) in memory, starting at address I.
        // The offset from I is increased by 1 for each value written,
        // but I itself is left unmodified.
        case 0x0055: {
            printf("0x%X: FX55\n", opcode);
            for (int i = 0; i <= X; i++) {
                chip8->memory.memory[chip8->registers.I + i] = chip8->registers.V[i];
            }
        } break;

        // FX65: Fills from V0 to VX (including VX) with values from memory, starting at address I.
        // The offset from I is increased by 1 for each value read,
        // but I itself is left unmodified.
        case 0x0065: {
            printf("0x%X: FX65\n", opcode);
            for (int i = 0; i <= X; i++) {
                chip8->registers.V[i] = getMemory(&chip8->memory, chip8->registers.I + i);
            }
        } break;
        }
    } break;
    }
}
