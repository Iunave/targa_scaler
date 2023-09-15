#ifndef TGA_DOWNSCALER_ALGORITHM_HPP
#define TGA_DOWNSCALER_ALGORITHM_HPP

#include <vector>
#include <span>
#include <cstdint>

struct __attribute__((packed)) image_spec_t
{
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t pixel_depth;
    uint8_t alpha_channel_depth : 4;
    uint8_t pixel_ordering_right2left : 1;
    uint8_t pixel_ordering_top2bottom : 1;
};

struct __attribute__((packed)) tga_header_t
{
    uint8_t id_length;
    uint8_t color_map_type;
    uint8_t image_type;
    uint8_t color_map_spec[5];
    image_spec_t image_spec;
};

enum class downscaling_algroithm_e
{
    nearest,
    average,
    lanczos
};

std::vector<uint8_t> downscale(tga_header_t* header, void* pixels, downscaling_algroithm_e algorithm);

#endif //TGA_DOWNSCALER_ALGORITHM_HPP
