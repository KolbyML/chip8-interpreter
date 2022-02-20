#include <cstdint>
#include <vector>
#include <assert.h>
#include <iostream>

std::uint8_t nibble_1(std::uint8_t byte) {
    return ((byte & 0xF0) >> 4);
}

std::uint8_t nibble_2(std::uint8_t byte) {
    return (byte & 0x0F);
}

bool get_bit(std::uint8_t byte, std::uint8_t index) {
    assert(0 <= index <= 7);
    int i = 7 - index;
    return (byte & 1 << i) >> i;
}

void collect_digits(std::vector<std::uint8_t>& digits, std::uint8_t num) {
    if (num > 9) {
        collect_digits(digits, num / 10);
    }
    digits.push_back(num % 10);
}

void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-d,--debug print debug messages into the console and go opcode by opcode on keyboard input"
              << std::endl;
}