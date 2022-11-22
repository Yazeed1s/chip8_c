#include "inc/stack.h"
#include "inc/chip8.h"
#include <stdbool.h>
#include <stdio.h>

bool isFull(struct Chip8 *chip8) {
    return chip8->registers.SP == STACK_SIZE ? true : false;
}

bool isEmpty(struct Chip8 *chip8) {
    return chip8->registers.SP == 0 ? true : false;
}

void stackPush(struct Chip8 *chip8, unsigned short val) {
    if (isFull(chip8)) {
        printf("stack overflow!");
    }
    chip8->registers.SP += 1;
    chip8->stack.stack[chip8->registers.SP] = val;
}

unsigned short stackPop(struct Chip8 *chip8) {
    if (isEmpty(chip8)) {
		printf("stack is already empty!");
	}
    unsigned short result = chip8->stack.stack[chip8->registers.SP];
    chip8->registers.SP -= 1;
    return result;
}