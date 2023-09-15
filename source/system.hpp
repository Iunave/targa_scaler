#ifndef TGA_DOWNSCALER_SYSTEM_HPP
#define TGA_DOWNSCALER_SYSTEM_HPP

#include <cstdint>
#include <span>
#include <vector>

std::vector<uint8_t> read_file_binary(const char* filepath);
void write_file_binary(const char* filepath, std::span<uint8_t> data);

#endif //TGA_DOWNSCALER_SYSTEM_HPP
