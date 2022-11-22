#include "inc/SDL2/SDL.h"
#include "inc/chip8.h"
#include "inc/keyboard.h"
#include "inc/screen.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char keyboard_map[TOTAL_KEYS] = {SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
                                       SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f};
SDL_Window *window;
SDL_Renderer *renderer;
struct Chip8 chip8;

void initWindow() {
    // https://wiki.libsdl.org/SDL_CreateWindow
    // SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
    SDL_Init(SDL_INIT_EVERYTHING);
    int x, y, w, h;
    x = SDL_WINDOWPOS_UNDEFINED;
    y = SDL_WINDOWPOS_UNDEFINED;
    w = WIDTH * 10;  // multiply each pixel by 10
    h = HEIGHT * 10; // multiply each pixel by 10
    Uint32 flag = SDL_WINDOW_SHOWN;
    window = SDL_CreateWindow("CHIP-8", x, y, w, h, flag);
    if (window == 0x00) {
        printf("Could not create window: %s\n", SDL_GetError());
        exit(0);
    }
    // initRenderer();
}
void initRenderer() {
    // https://wiki.libsdl.org/SDL_CreateRenderer
    // SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags);
    int index = -1;
    Uint32 flag = SDL_TEXTUREACCESS_TARGET;
    renderer = SDL_CreateRenderer(window, index, flag);
    if (renderer == 0x00) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        exit(0);
    }
}

void setRendererColors() {
    // https://wiki.libsdl.org/SDL_SetRenderDrawColor

    // set colors to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    // paint to balck
    SDL_RenderClear(renderer);
    // set the draw colors (drawn on the black color)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
}

void drawDisplay(struct Chip8 *chip8) {
    setRendererColors();
    // iterating thru the display (64*32)
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            if (screenIsSet(&chip8->screen, x, y)) {
                SDL_Rect rect;
                rect.x = x * 10;
                rect.y = y * 10;
                rect.w = 10;
                rect.h = 10;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    // update the screen
    // SDL_RenderPresent(renderer);
}

int handleEvent(struct Chip8 *chip8) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            SDL_DestroyWindow(window);
            return -1;
            break;
        case SDL_KEYDOWN: {
            char key = event.key.keysym.sym;
            int vkey = mapKey(&chip8->keyboard, key);
            if (vkey != -1) {
                keyDown(&chip8->keyboard, vkey);
            }
        } break;

        case SDL_KEYUP: {
            char key = event.key.keysym.sym;
            int vkey = mapKey(&chip8->keyboard, key);
            if (vkey != -1) {
                keyUp(&chip8->keyboard, vkey);
            }
        } break;
        }
    }
    return 0;
}
int main(int argc, char **argv) {

    if (argc < 2) {
        printf("You must provide a file to load\n");
        return -1;
    }
    switch (argc) {
    case 1:
        printf("[Error] usage: ./chip8 <rom file>\n");
        return -1;
        break;
    case 2: {
        printf("\nloading font into memory....");
        chInit(&chip8);
        printf("\n[OK] font is loaded successfully");
        const char *buf = argv[1];
        printf("\nloading file: %s....\n", buf);
        chLoad(&chip8, buf);
        printf("\n[OK] file is loaded successfully");
        setMap(&chip8.keyboard, keyboard_map);
        printf("\nstarting the emulator....");
        SDL_Init(SDL_INIT_EVERYTHING);
        initWindow();
        initRenderer();
        while (1) {
            int e = handleEvent(&chip8);
            setRendererColors();
            drawDisplay(&chip8);
            // update the screen
            SDL_RenderPresent(renderer);
            unsigned short opcode = mergeBytes(&chip8.memory, chip8.registers.PC);
            chip8.registers.PC += 2;
            execOpcode(&chip8, opcode);
            if (chip8.registers.delay_timer > 0) {
                sleep(10);
                chip8.registers.delay_timer -= 1;
            }
            if (chip8.registers.sound_timer > 0) {
                chip8.registers.sound_timer = 0;
            }
            if (e == -1) {
                break;
            }
        }
    } break;
    }
    return 0;
}
