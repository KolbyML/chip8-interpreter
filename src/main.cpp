#include <iostream>
#include <vector>
#include <SDL.h>
#include <cstdint>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>
#include "utils.h"
#include "main.h"

bool DEBUG = false;
bool CHIP48_MODE = true;

void init_SDL2() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64 * scale, 32 * scale, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, 64, 32);
    SDL_SetWindowTitle(window, "Kolby's Chip-8 Emulator");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
}

int main(int argc, char* argv[]) {
    int PC = 0x200;
    
    char *file_dir;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (i == 1) {
            file_dir = argv[i];
        } else if ((arg == "-d") || (arg == "--debug")) {
            DEBUG = true;
        } else if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }  else if (arg == "-chip48") {
            CHIP48_MODE = false;
        }
    }

    // 4Kb of memory
    std::vector<std::uint8_t> memory(4096, 0);
    std::fill(memory.begin(), memory.end(), 0);

    // general-purpose variable registers
    std::vector<std::uint8_t> gpv_registers(16, 0);
    std::fill(gpv_registers.begin(), gpv_registers.end(), 0);

    // index register
    std::uint16_t index_register = 0;

    // delay timer
    std::uint8_t delay_timer = 0;

    // sound timer
    std::uint16_t sound_timer = 0;

    // stack
    std::vector<std::uint16_t> stack(0, 0);

    /// Load game
    std::ifstream input(file_dir, std::ios::binary);
    // copies all data into buffer
    std::vector<std::uint8_t> buffer(std::istreambuf_iterator<char>(input), {});

    for (int i = 0; i < buffer.size(); i++) {
        memory[i + 0x200] = buffer[i];
    }
    /// End Load game

    // Load font
    std::uint8_t font[80] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                            0x20, 0x60, 0x20, 0x20, 0x70, // 1
                            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++) {
        memory[i + 0x050] = font[i];
    }

    bool screen[64][32];
    for (int i = 0; i < 64; i++) {
        for (int k = 0; k < 32; k++) {
            screen[i][k] = 0;
        }
    }

    init_SDL2();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1/700));

        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;

        std::uint8_t byte_one = memory[PC];
        std::uint8_t byte_two = memory[PC + 1];

        PC += 2;

        if (DEBUG) {
            bool quit = false;
            while (!quit)
            {
                if (SDL_WaitEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            goto finish;
                        case SDL_KEYUP:
                            quit = true;
                            break;
                    }
                }
            }
            std::cout << "index register " << index_register << "\n";
            std::cout << "opcode 0x" << std::uppercase << std::hex << nibble_1(byte_one) << nibble_2(byte_one) << nibble_1(byte_two) << nibble_2(byte_two) << "\n";
        }

        // Decode
        std::uint8_t    X = nibble_2(byte_one);
        std::uint8_t    Y = nibble_1(byte_two);
        std::uint8_t    N = nibble_2(byte_two);
        std::uint8_t   NN = byte_two;
        std::uint16_t NNN = (X << 8) + byte_two;

        switch (nibble_1(byte_one)) {
            case 0x00: {
                if (byte_two == 0xE0) {
                    // turn all pixels to 0

                    memset(screen, 0, sizeof(screen[0][0]) * 64 * 32);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);
                } else if (byte_two == 0xEE) {
                    std::uint16_t saved_PC = stack.back();
                    stack.pop_back();
                    PC = saved_PC;
                }
                break;
            } case 0x01: {
                PC = NNN;
                break;
            } case 0x02: {
                stack.emplace_back(PC);
                PC = NNN;
                break;
            } case 0x03: {
                if (gpv_registers[X] == byte_two) PC += 2;
                break;
            } case 0x04: {
                if (gpv_registers[X] != byte_two) PC += 2;
                break;
            } case 0x05: {
                if (gpv_registers[X] == gpv_registers[Y]) PC += 2;
                break;
            } case 0x06: {
                gpv_registers[X] = byte_two;
                break;
            } case 0x07: {
                // todo:: read doc later may have done this one wrong
                gpv_registers[X] += byte_two;
                break;
            } case 0x08: {
                switch (nibble_2(byte_two)) {
                    case 0x00: {
                        gpv_registers[X] = gpv_registers[Y];
                        break;
                    } case 0x01: {
                        gpv_registers[X] = gpv_registers[X] | gpv_registers[Y];
                        break;
                    } case 0x02: {
                        gpv_registers[X] = gpv_registers[X] & gpv_registers[Y];
                        break;
                    } case 0x03: {
                        gpv_registers[X] = gpv_registers[X] ^ gpv_registers[Y];
                        break;
                    } case 0x04: {
                        if ((int)gpv_registers[X] + (int)gpv_registers[Y] > 255) gpv_registers[0xF] = 1;
                        else gpv_registers[0xF] = 0;
                        gpv_registers[X] = gpv_registers[X] + gpv_registers[Y];
                        break;
                    } case 0x05: {
                        if (gpv_registers[X] > gpv_registers[Y]) gpv_registers[0xF] = 1;
                        else gpv_registers[0xF] = 0;
                        gpv_registers[X] = gpv_registers[X] - gpv_registers[Y];
                        break;
                    } case 0x06: {
                        if (!CHIP48_MODE) {
                            gpv_registers[X] = gpv_registers[Y];
                        }
                        if (get_bit(gpv_registers[X], 7) == 1) gpv_registers[0xF] = 1;
                        else gpv_registers[0xF] = 0;
                        gpv_registers[X] = gpv_registers[X] >> 1;
                        break;
                    } case 0x07: {
                        if (gpv_registers[Y] > gpv_registers[X]) gpv_registers[0xF] = 1;
                        else gpv_registers[0xF] = 0;
                        gpv_registers[X] = gpv_registers[Y] - gpv_registers[X];
                        break;
                    } case 0x0E: {
                        if (!CHIP48_MODE) {
                            gpv_registers[X] = gpv_registers[Y];
                        }
                        if (get_bit(gpv_registers[X], 0) == 1) gpv_registers[0xF] = 1;
                        else gpv_registers[0xF] = 0;
                        gpv_registers[X] = gpv_registers[X] << 1;
                        break;
                    }
                }
                break;
            } case 0x09: {
                if (gpv_registers[X] != gpv_registers[Y]) PC += 2;
                break;
            } case 0x0A: {
                index_register = NNN;
                break;
            } case 0x0B: {
                if (!CHIP48_MODE) {
                    NNN = NNN + gpv_registers[0x0];
                    PC = NNN;
                } else {
                    std::uint16_t XNN = NN + gpv_registers[X];
                    PC = XNN;
                }
                break;
            } case 0x0C: {
                std::uint8_t random_number = rand() % UINT8_MAX;
                gpv_registers[X] = random_number & byte_two;
                break;
            } case 0x0D: {
                std::uint8_t x = gpv_registers[X];
                std::uint8_t y = gpv_registers[Y];
                gpv_registers[0x0F] = 0;

                for (int i = 0; i < N; i++) {
                    std::uint8_t byte = memory[index_register + i];
                    for (int k = 0; k < 8; k++) {
                        std::uint8_t bit = get_bit(byte, k);
                        if (bit != 0) {
                            if (screen[(x + k)][y + i] == 1) {
                                gpv_registers[0x0F] = 1;
                            }
                            screen[(x + k)][y + i] ^= 1;
                        }
                    }
                }

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                for (int i = 0; i < 64; i++) {
                    for (int k = 0; k < 32; k++) {
                        if (screen[i][k] == 1) {
                            SDL_RenderDrawPoint(renderer, i, k);
                        }
                    }
                }
                SDL_RenderPresent(renderer);

                break;
            } case 0x0E: {
                switch (byte_two) {
                    case 0x9E: {
                        const std::uint8_t *key_states = SDL_GetKeyboardState(nullptr);
                        if (key_states[SDL_SCANCODE_1] && gpv_registers[X] == 0x1) PC += 2;
                        if (key_states[SDL_SCANCODE_2] && gpv_registers[X] == 0x2) PC += 2;
                        if (key_states[SDL_SCANCODE_3] && gpv_registers[X] == 0x3) PC += 2;
                        if (key_states[SDL_SCANCODE_4] && gpv_registers[X] == 0xC) PC += 2;

                        if (key_states[SDL_SCANCODE_Q] && gpv_registers[X] == 0x4) PC += 2;
                        if (key_states[SDL_SCANCODE_W] && gpv_registers[X] == 0x5) PC += 2;
                        if (key_states[SDL_SCANCODE_E] && gpv_registers[X] == 0x6) PC += 2;
                        if (key_states[SDL_SCANCODE_R] && gpv_registers[X] == 0xD) PC += 2;

                        if (key_states[SDL_SCANCODE_A] && gpv_registers[X] == 0x7) PC += 2;
                        if (key_states[SDL_SCANCODE_S] && gpv_registers[X] == 0x8) PC += 2;
                        if (key_states[SDL_SCANCODE_D] && gpv_registers[X] == 0x9) PC += 2;
                        if (key_states[SDL_SCANCODE_F] && gpv_registers[X] == 0xE) PC += 2;

                        if (key_states[SDL_SCANCODE_Z] && gpv_registers[X] == 0xA) PC += 2;
                        if (key_states[SDL_SCANCODE_X] && gpv_registers[X] == 0x0) PC += 2;
                        if (key_states[SDL_SCANCODE_C] && gpv_registers[X] == 0xB) PC += 2;
                        if (key_states[SDL_SCANCODE_V] && gpv_registers[X] == 0xF) PC += 2;
                        break;
                    } case 0xA1: {
                        const std::uint8_t *key_states = SDL_GetKeyboardState(nullptr);
                        if (!key_states[SDL_SCANCODE_1] && gpv_registers[X] == 0x1) PC += 2;
                        if (!key_states[SDL_SCANCODE_2] && gpv_registers[X] == 0x2) PC += 2;
                        if (!key_states[SDL_SCANCODE_3] && gpv_registers[X] == 0x3) PC += 2;
                        if (!key_states[SDL_SCANCODE_4] && gpv_registers[X] == 0xC) PC += 2;

                        if (!key_states[SDL_SCANCODE_Q] && gpv_registers[X] == 0x4) PC += 2;
                        if (!key_states[SDL_SCANCODE_W] && gpv_registers[X] == 0x5) PC += 2;
                        if (!key_states[SDL_SCANCODE_E] && gpv_registers[X] == 0x6) PC += 2;
                        if (!key_states[SDL_SCANCODE_R] && gpv_registers[X] == 0xD) PC += 2;

                        if (!key_states[SDL_SCANCODE_A] && gpv_registers[X] == 0x7) PC += 2;
                        if (!key_states[SDL_SCANCODE_S] && gpv_registers[X] == 0x8) PC += 2;
                        if (!key_states[SDL_SCANCODE_D] && gpv_registers[X] == 0x9) PC += 2;
                        if (!key_states[SDL_SCANCODE_F] && gpv_registers[X] == 0xE) PC += 2;

                        if (!key_states[SDL_SCANCODE_Z] && gpv_registers[X] == 0xA) PC += 2;
                        if (!key_states[SDL_SCANCODE_X] && gpv_registers[X] == 0x0) PC += 2;
                        if (!key_states[SDL_SCANCODE_C] && gpv_registers[X] == 0xB) PC += 2;
                        if (!key_states[SDL_SCANCODE_V] && gpv_registers[X] == 0xF) PC += 2;
                        break;
                    }
                }
                break;
            } case 0x0F: {
                switch (byte_two) {
                    case 0x07: {
                        gpv_registers[X] = delay_timer;
                        break;
                    } case 0x15: {
                        delay_timer = gpv_registers[X];
                        break;
                    } case 0x18: {
                        sound_timer = gpv_registers[X];
                        break;
                    } case 0x1E: {
                        index_register += gpv_registers[X];
                        break;
                    } case 0x0A: {
                        while (true) {
                            const std::uint8_t *key_states = SDL_GetKeyboardState(nullptr);
                            if (key_states[SDL_SCANCODE_1]) {gpv_registers[X] = 0x1; break;}
                            if (key_states[SDL_SCANCODE_2]) {gpv_registers[X] = 0x2; break;}
                            if (key_states[SDL_SCANCODE_3]) {gpv_registers[X] = 0x3; break;}
                            if (key_states[SDL_SCANCODE_4]) {gpv_registers[X] = 0xC; break;}

                            if (key_states[SDL_SCANCODE_Q]) {gpv_registers[X] = 0x4; break;}
                            if (key_states[SDL_SCANCODE_W]) {gpv_registers[X] = 0x5; break;}
                            if (key_states[SDL_SCANCODE_E]) {gpv_registers[X] = 0x6; break;}
                            if (key_states[SDL_SCANCODE_R]) {gpv_registers[X] = 0xD; break;}

                            if (key_states[SDL_SCANCODE_A]) {gpv_registers[X] = 0x7; break;}
                            if (key_states[SDL_SCANCODE_S]) {gpv_registers[X] = 0x8; break;}
                            if (key_states[SDL_SCANCODE_D]) {gpv_registers[X] = 0x9; break;}
                            if (key_states[SDL_SCANCODE_F]) {gpv_registers[X] = 0xE; break;}

                            if (key_states[SDL_SCANCODE_Z]) {gpv_registers[X] = 0xA; break;}
                            if (key_states[SDL_SCANCODE_X]) {gpv_registers[X] = 0x0; break;}
                            if (key_states[SDL_SCANCODE_C]) {gpv_registers[X] = 0xB; break;}
                            if (key_states[SDL_SCANCODE_V]) {gpv_registers[X] = 0xF; break;}
                        }
                        break;
                    } case 0x29: {
                        index_register = 0x050 + gpv_registers[X] * 5;
                        break;
                    } case 0x33: {
                        uint8_t number = gpv_registers[X];
                        std::vector<std::uint8_t> numbers(0, 0);
                        collect_digits(numbers, number);

                        for (int i = 0; i < numbers.size(); i++) {
                            memory[index_register + i] = numbers[i];
                        }
                        break;
                    } case 0x55: {
                        for (int i = 0; i <= X; i++) {
                            memory[index_register + i] = gpv_registers[i];
                        }
                        break;
                    } case 0x65: {
                        for (int i = 0; i <= X; i++) {
                            gpv_registers[i] = memory[index_register + i];
                        }
                        break;
                    }
                }
                break;
            }
        }

        // decrement timers
        if (delay_timer > 0) {
            delay_timer -= 1;
        }
        if (sound_timer > 0) {
            // This is the beep, but honestly I find it annoying on windows where it just plays the error sound now
            //std::cout << '\a';
            sound_timer -= 1;
        }

    }

    finish:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
