#include "inc/screen.h"
#include <assert.h>
#include <memory.h>

void checkBounds(int x, int y) {
    assert(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
}


void clearScreen(struct Screen *screen) {
    memset(screen->pixels, 0, sizeof(screen->pixels));
}

bool screenIsSet(struct Screen *screen, int x, int y) {
    checkBounds(x, y);
    return screen->pixels[y][x];
}

/*	hardest part of all!
	http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

	Dxyn - DRW Vx, Vy, nibble
    Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

    The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as
    sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If this causes any pixels to
    be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the
    coordinates of the display, it wraps around to the opposite side of the screen. See instruction 8xy3 for more
    information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.

    -----
    To simplfy this, the opcode DXYN is broken to
    D = draw,
    X = vaalue stored at register x, (coordinate x)
    Y = value stored at register y, (coordinate y)
    N = number of bytes to be drwan,
    those values are unknown to us, however, assuming the values are
    X = V1 (register 1) which holds the value 10
    Y = V2 (register 2) which holds the value 20
    N = 4 bytes
    the instruction then will be read as D124 which means
    draw 4 bytes of sprites at x coordinate 10 and y coordinate 20
*/
bool drawSprite(struct Screen *screen, int x, int y, const char *sprite, int num) {
    bool pixelCollison = false;
    int byte = 8;
    // num == N in main.c execOpcode()
    for (int y_cord = 0; y_cord < num; y_cord++) {
        unsigned short c = sprite[y_cord];
        for (int x_cord = 0; x_cord < byte; x_cord++) {
            if ((c & (0x80 >> x_cord)) != 0) {
                continue;
            }
            if (screen->pixels[(y_cord + y) % HEIGHT][(x_cord + x) % WIDTH]) {
                pixelCollison = true;
            }
            screen->pixels[(y_cord + y) % HEIGHT][(x_cord + x) % WIDTH] ^= true;
        }
    }
    return pixelCollison;
}
