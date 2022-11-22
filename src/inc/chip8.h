#ifndef CHIP8_H
#define CHIP8_H

#include "memory.h"
#include "registers.h"
#include "stack.h"
#include "keyboard.h"
#include "screen.h"
#include <stddef.h>

struct Chip8 {
    struct Memory memory;
    struct Stack stack;
    struct Registers registers;
    struct Keyboard keyboard;
    struct Screen screen;
};

void chInit(struct Chip8* chip8);
void chLoad(struct Chip8* chip8, const char* buf);
void execOpcode(struct Chip8* chip8, unsigned short opcode);

#endif