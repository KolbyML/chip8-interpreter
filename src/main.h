#ifndef CHIP8_EMULATOR_MAIN_H
#define CHIP8_EMULATOR_MAIN_H
SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;
int scale = 14;

void init_SDL2();
#endif //CHIP8_EMULATOR_MAIN_H
