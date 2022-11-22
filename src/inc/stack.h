#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 16
struct Chip8;
struct Stack {
    unsigned short stack[STACK_SIZE];
};

void stackPush(struct Chip8 *chip8, unsigned short val);
unsigned short stackPop(struct Chip8 *chip8);

#endif
