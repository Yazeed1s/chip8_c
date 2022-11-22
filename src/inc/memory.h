#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_SIZE 4096
struct Memory {
    unsigned char memory[MEMORY_SIZE];
};
unsigned char getMemory(struct Memory *memory, int index);
unsigned short mergeBytes(struct Memory *memory, int index);

#endif