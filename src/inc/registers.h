#ifndef REGISTERS_H
#define REGISTERS_H

#define DATA_REGISTERS 16
struct Registers {
    unsigned char V[DATA_REGISTERS];
    unsigned short I;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short PC;
    unsigned char SP;
};

#endif