#include <iostream>
#include <vector>
#include <SDL.h>
#include <cstdint>
#include <fstream>
#include <format>


const bool DEBUG = false;

std::uint8_t nibble_1(std::uint8_t byte) {
    return ((byte & 0xF0) >> 4);
}

std::uint8_t nibble_2(std::uint8_t byte) {
    return (byte & 0x0F);
}

bool get_bit(std::uint8_t byte, std::uint8_t index) {
    int i = 7 - index;
    return (byte & 1 << i) >> i;
}

int main(int argc, char* args[]) {
    std::cout << "Hello, World!" << std::endl;
    int PC = 0x200;
    
    int scale = 9;

    std::vector<std::uint8_t> memory(4096, 0);
    std::fill(memory.begin(), memory.end(), 0);

    // general-purpose variable registers
    std::vector<std::uint8_t> gpv_registers(16, 0);
    std::fill(gpv_registers.begin(), gpv_registers.end(), 0);

    // index register
    std::uint16_t index_register = 0;

    /// Load game
    std::ifstream input( "BC_test.ch8", std::ios::binary );
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

    bool screen[576][288];
    for (int i = 0; i < 64 * scale; i++) {
        for (int k = 0; k < 32 * scale; k++) {
            screen[i][k] = 0;
        }
    }

    /// SDL2
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(64 * scale, 32 * scale, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Kolby's Chip-8 pogcham");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderSetScale(renderer, scale, scale);

    /// SDL2 endl


    // Done loading font
    std::cout << "Hello, World!3" << std::endl;
    while (true) {
        /// SDL2
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        /// SDL2 endl
        //std::cout << PC << std::endl;
        // Fetch
        if (PC > 4096) {
            //std::cout << "PC goes out of memory range" << std::endl;
            continue;
        }

        std::uint8_t byte_one = memory[PC];
        std::uint8_t byte_two = memory[PC + 1];

        PC += 2;

        if (DEBUG) {
            bool quit = false;
            while (!quit)
            {
                if (SDL_PollEvent(&event) && event.type == SDL_KEYUP) {
                    quit = true;
                    break;
                }
            }
            std::cout << std::format("index register {}\n", index_register);
            std::cout << std::format("opcode 0x{:X}{:X}{:X}{:X}\n", nibble_1(byte_one), nibble_2(byte_one), nibble_1(byte_two), nibble_2(byte_two));
        }

        // Decode
        switch (nibble_1(byte_one)) {
            case 0x00: {
                if (byte_one == 0x00 && byte_two == 0xE0) {
                    // turn all pixels to 0

                    memset(screen, 0, sizeof(screen[0][0]) * 64 * scale * 32 * scale);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    for (int i = 0; i < 64 * scale; i++) {
                        for (int k = 0; k < 32 * scale; k++) {
                            SDL_RenderDrawPoint(renderer, i, k);
                        }
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            } case 0x01: {
                std::uint16_t second_nibble = nibble_2(byte_one);
                std::uint16_t NNN = (second_nibble << 8) + byte_two;
                PC = NNN;
                break;
            } case 0x02: {
                break;
            } case 0x03: {
                break;
            } case 0x04: {
                break;
            } case 0x05: {
                break;
            } case 0x06: {
                std::uint16_t X = nibble_2(byte_one);
                gpv_registers[X] = byte_two;
                break;
            } case 0x07: {
                // todo:: read doc later may have done this one wrong
                std::uint16_t X = nibble_2(byte_one);
                gpv_registers[X] += byte_two;
                break;
            } case 0x08: {
                break;
            } case 0x09: {
                break;
            } case 0x0A: {
                std::uint16_t second_nibble = nibble_2(byte_one);
                std::uint16_t NNN = (second_nibble << 8) + byte_two;
                index_register = NNN;
                break;
            } case 0x0B: {
                break;
            } case 0x0C: {
                break;
            } case 0x0D: {
                std::uint8_t x = gpv_registers[nibble_2(byte_one)];
                std::uint8_t y = gpv_registers[nibble_1(byte_two)];
                gpv_registers[0x0F] = 0;
                std::uint8_t N = nibble_2(byte_two);

                for (int i = 0; i < N; i++) {
                    std::cout << "momma " << index_register << std::endl;
                    std::uint8_t byte = memory[index_register + i];
                    for (int k = 0; k < 8; k++) {
                        //std::cout << x << " " << y << std::endl;
                        std::uint8_t bit = get_bit(byte, k);
                        if (bit != 0) {
                            if (screen[(x + k)][y + i] == 1) {
                                gpv_registers[0x0F] = 1;
                            }
                            screen[(x + k)][y + i] ^= 1;
                        }
                    }
                }

                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                for (int i = 0; i < 64 * scale; i++) {
                    for (int k = 0; k < 32 * scale; k++) {
                        if (screen[i][k] == 1) {
                            SDL_RenderDrawPoint(renderer, i, k);
                        }
                    }
                }
                SDL_RenderPresent(renderer);

                break;
            } case 0x0E: {
                break;
            } case 0x0F: {
                break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    // Excute
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
