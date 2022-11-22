#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#define TOTAL_KEYS 16
struct Keyboard {
    bool keyboard[TOTAL_KEYS];
    const char *keyboard_map;
};

void setMap(struct Keyboard *keyboard, const char *map);
int mapKey(struct Keyboard *keyboard, char key);
void keyDown(struct Keyboard *keyboard, int key);
void keyUp(struct Keyboard *keyboard, int key);
bool keyIsDown(struct Keyboard *keyboard, int key);

#endif