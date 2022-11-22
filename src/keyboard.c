#include "inc/keyboard.h"
#include <assert.h>


void setMap(struct Keyboard *keyboard, const char *map) {
    keyboard->keyboard_map = map;
}

int mapKey(struct Keyboard *keyboard, char key) {

    for (int i = 0; i < TOTAL_KEYS; i++) {
        if (keyboard->keyboard_map[i] == key) {
            return i;
        }
    }
    return -1;
}

void keyDown(struct Keyboard *keyboard, int key) {
    keyboard->keyboard[key] = true;
}

void keyUp(struct Keyboard *keyboard, int key) {
    keyboard->keyboard[key] = false;
}

bool keyIsDown(struct Keyboard *keyboard, int key) {
    return keyboard->keyboard[key];
}