#include "inc/memory.h"
#include <assert.h>

void inBounds(int index) {
    assert(index >= 0 && index < MEMORY_SIZE);
}

unsigned char getMemory(struct Memory *memory, int index) {
    inBounds(index);
    return memory->memory[index];
}

/**
 * @brief mergeBytes(chip8, index) is used to merge and build the opcode
 * from two indices in the memory
 * @param chip8 chip8's memory
 * @param index the index from which to merge the bytes
 * @return merged opcode 
 */
unsigned short mergeBytes(struct Memory *memory, int index) {
    // the issue here is that the memory stores only one byte in a single index,
    // we need to merge two indices together in order to get the full opcode
    // opcode DXYN is stored as memory[i] = DX and memory[i+1] = YN
    unsigned char byte1 = getMemory(memory, index); // memory->memory[index];
    unsigned char byte2 = getMemory(memory, index + 1); // memory->memory[index + 1];
    return byte1 << 8 | byte2;
}