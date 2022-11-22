#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#define WIDTH 64
#define HEIGHT 32
struct Screen {
    bool pixels[HEIGHT][WIDTH];
};

void clearScreen(struct Screen *screen);
bool screenIsSet(struct Screen *screen, int x, int y);
bool drawSprite(struct Screen *screen, int x, int y, const char *sprite, int num);
#endif