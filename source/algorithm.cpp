#include "algorithm.hpp"
#include <cstdlib>
#include <cmath>
#include <array>
#include <numbers>
#include <tuple>

#include "glm/glm/glm.hpp"

namespace
{
    template<int32_t channels> requires(channels == 3 || channels == 4)
    struct five_bit_color_t
    {
        using expanded_int_t = glm::vec<channels, uint32_t>;
        using expanded_float_t = glm::vec<channels, float>;

        uint16_t r : 5;
        uint16_t g : 5;
        uint16_t b : 5;
        uint16_t a : 1;

        template<typename T>
        five_bit_color_t(glm::vec<channels, T> in)
        {
            r = in.r;
            g = in.g;
            b = in.b;
            if constexpr(channels == 4) {a = in.a;}
        }

        template<typename T>
        five_bit_color_t& operator=(glm::vec<channels, T> in)
        {
            r = in.r;
            g = in.g;
            b = in.b;
            if constexpr(channels == 4) {a = in.a;}
            return *this;
        }

        template<typename T>
        operator glm::vec<channels, T>() const
        {
            if constexpr(channels == 4)
            {
                return {r, g, b, a};
            }
            else
            {
                return {r, g, b};
            }
        }
    };

    using R5G5B5_t = five_bit_color_t<3>;
    using R5G5B5A1_t = five_bit_color_t<4>;

    template<int32_t N>
    five_bit_color_t<N> pack_normalized_5bit(glm::vec<N, float> val)
    {
        return glm::round(val * float(UINT8_MAX));
    }

    template<int32_t N>
    glm::vec<N, float> unpack_normalized_5bit(five_bit_color_t<N> val)
    {
        return glm::vec<N, float>{val} / float(UINT8_MAX);
    }

    template<int32_t N>
    glm::vec<N, uint8_t> pack_normalized_8bit(glm::vec<N, float> val)
    {
        return glm::round(val * float(UINT8_MAX));
    }

    template<int32_t N>
    glm::vec<N, float> unpack_normalized_8bit(glm::vec<N, uint8_t> val)
    {
        return glm::vec<N, float>{val} / float(UINT8_MAX);
    }

    template<typename color_t>
    void nearest(void* dest, void* source, uint64_t width, uint64_t height)
    {
        auto dest_pixels = static_cast<color_t*>(dest);
        auto source_pixels = static_cast<color_t*>(source);

        uint64_t downscaled_width = width / 2;
        uint64_t downscaled_height = height / 2;

        for(uint64_t x_index = 0; x_index < downscaled_width; ++x_index)
        {
            for(uint64_t y_index = 0; y_index < downscaled_height; ++y_index)
            {
                uint64_t x_sample = x_index * 2;
                uint64_t y_sample = y_index * 2;
                dest_pixels[x_index + (y_index * downscaled_width)] = source_pixels[x_sample + (y_sample * width)];
            }
        }
    }

    void nearest_R5G5B5(void* dest, void* source, uint64_t width, uint64_t height)
    {
        nearest<R5G5B5_t>(dest, source, width, height);
    }

    void nearest_R5G5B5A1(void* dest, void* source, uint64_t width, uint64_t height)
    {
        nearest<R5G5B5A1_t>(dest, source, width, height);
    }

    void nearest_R8G8B8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        nearest<glm::vec<3, uint8_t>>(dest, source, width, height);
    }

    void nearest_R8G8B8A8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        nearest<glm::vec<4, uint8_t>>(dest, source, width, height);
    }

    template<typename color_t, typename expanded_color_t>
    void average(void* dest, void* source, uint64_t width, uint64_t height)
    {
        auto dest_pixels = static_cast<color_t*>(dest);
        auto source_pixels = static_cast<color_t*>(source);

        uint64_t downscaled_width = width / 2;
        uint64_t downscaled_height = height / 2;

        for(uint64_t x_index = 0; x_index < downscaled_width; ++x_index)
        {
            for(uint64_t y_index = 0; y_index < downscaled_height; ++y_index)
            {
                uint64_t x_sample = x_index * 2;
                uint64_t y_sample = y_index * 2;

                expanded_color_t px0 = source_pixels[(x_sample + 0) + ((y_sample + 0) * width)];
                expanded_color_t px1 = source_pixels[(x_sample + 0) + ((y_sample + 1) * width)];
                expanded_color_t px2 = source_pixels[(x_sample + 1) + ((y_sample + 0) * width)];
                expanded_color_t px3 = source_pixels[(x_sample + 1) + ((y_sample + 1) * width)];

                expanded_color_t averaged = (px0 + px1 + px2 + px3) / 4u;
                dest_pixels[x_index + (y_index * downscaled_width)] = averaged;
            }
        }
    }

    template<int32_t channels>
    void average_5bit(void* dest, void* source, uint64_t width, uint64_t height)
    {
        using color_t = five_bit_color_t<channels>;
        using expanded_color_t = typename color_t::expanded_int_t;

        average<color_t, expanded_color_t>(dest, source, width, height);
    }

    template<int32_t channels>
    void average_8bit(void* dest, void* source, uint64_t width, uint64_t height)
    {
        using color_t = glm::vec<channels, uint8_t>;
        using expanded_color_t = glm::vec<channels, uint32_t>;

        average<color_t, expanded_color_t>(dest, source, width, height);
    }

    void average_R5G5B5(void* dest, void* source, uint64_t width, uint64_t height)
    {
        average_5bit<3>(dest, source, width, height);
    }

    void average_R5G5B5A1(void* dest, void* source, uint64_t width, uint64_t height)
    {
        average_5bit<4>(dest, source, width, height);
    }

    void average_R8G8B8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        average_8bit<3>(dest, source, width, height);
    }

    void average_R8G8B8A8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        average_8bit<4>(dest, source, width, height);
    }

#ifndef M_PI //mingw is being silly
#define M_PI 3.14159265358979323846
#endif

    float sinc(float x)
    {
        return std::sin(M_PI * x) / (M_PI * x);
    }

    float lanczos(float x, float a)
    {
        if(x <= -a || x >= a)
        {
            return 0.f;
        }

        if(std::abs(x) <= FLT_EPSILON)
        {
            return 1.f;
        }

        return sinc(x) * sinc(x / a);
    }

    template<typename color_t, typename expanded_color_t>
    color_t sample_image_lanczos(float x, float y, int64_t width, int64_t height, const color_t* image)
    {
        const auto[unpack_normalized, pack_normalized] = []()
        {
            if constexpr(std::is_same_v<color_t, R5G5B5_t> || std::is_same_v<color_t, R5G5B5A1_t>)
            {
                return std::make_tuple(&unpack_normalized_5bit<expanded_color_t::length()>, &pack_normalized_5bit<expanded_color_t::length()>);
            }
            else
            {
                return std::make_tuple(&unpack_normalized_8bit<expanded_color_t::length()>, &pack_normalized_8bit<expanded_color_t::length()>);
            }
        }();

        constexpr int64_t sample_size = 3; //36 sampled pixels

        int64_t x_start = int64_t(std::floor(x)) - sample_size + 1;
        int64_t x_end = int64_t(std::floor(x)) + sample_size;

        int64_t y_start = int64_t(std::floor(y)) - sample_size + 1;
        int64_t y_end = int64_t(std::floor(y)) + sample_size;

        std::array<expanded_color_t, sample_size * 2> horizontal_interpolations{};
        uint64_t horizontal = 0;

        for(int64_t y_sample = y_start; y_sample <= y_end; ++y_sample, ++horizontal)
        {
            if(y_sample < 0 || y_sample >= height)
            {
                continue;
            }

            for(int64_t x_sample = x_start; x_sample <= x_end; ++x_sample)
            {
                if(x_sample < 0 || x_sample >= width)
                {
                    continue;
                }

                expanded_color_t unpacked_pixel = unpack_normalized(image[x_sample + (y_sample * width)]);
                horizontal_interpolations[horizontal] += unpacked_pixel * lanczos(x - x_sample, sample_size);
            }

            horizontal_interpolations[horizontal] *= lanczos(y - y_sample,sample_size);
        }

        expanded_color_t vertical_interpolation{};
        for(expanded_color_t interpolated: horizontal_interpolations)
        {
            vertical_interpolation += interpolated;
        }

        vertical_interpolation = glm::clamp(vertical_interpolation, 0.0f, 1.0f);
        return pack_normalized(vertical_interpolation);
    }

    template<typename color_t, typename expanded_color_t>
    void lanczos(void* dest, void* source, uint64_t width, uint64_t height)
    {
        auto dest_pixels = static_cast<color_t*>(dest);
        auto source_pixels = static_cast<color_t*>(source);

        uint64_t downscaled_width = width / 2;
        uint64_t downscaled_height = height / 2;

        for(uint64_t y_index = 0; y_index < downscaled_height; ++y_index)
        {
            for(uint64_t x_index = 0; x_index < downscaled_width; ++x_index)
            {
                float x_sample = ((float(x_index) / float(downscaled_width)) * float(width)) + 0.5f;
                float y_sample = (float(y_index) / float(downscaled_height)) * float(height) + 0.5f;

                dest_pixels[x_index + (y_index * downscaled_width)] = sample_image_lanczos<color_t, expanded_color_t>(x_sample, y_sample, width, height, source_pixels);
            }
        }
    }

    template<int32_t channels>
    void lanczos_5bit(void* dest, void* source, uint64_t width, uint64_t height)
    {
        using color_t = five_bit_color_t<channels>;
        using expanded_color_t = typename color_t::expanded_float_t;

        lanczos<color_t, expanded_color_t>(dest, source, width, height);
    }

    template<int32_t channels>
    void lanczos_8bit(void* dest, void* source, uint64_t width, uint64_t height)
    {
        using color_t = glm::vec<channels, uint8_t>;
        using expanded_color_t = glm::vec<channels, float>;

        lanczos<color_t, expanded_color_t>(dest, source, width, height);
    }

    void lanczos_R5G5B5(void* dest, void* source, uint64_t width, uint64_t height)
    {
        lanczos_5bit<3>(dest, source, width, height);
    }

    void lanczos_R5G5B5A1(void* dest, void* source, uint64_t width, uint64_t height)
    {
        lanczos_5bit<4>(dest, source, width, height);
    }

    void lanczos_R8G8B8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        lanczos_8bit<3>(dest, source, width, height);
    }

    void lanczos_R8G8B8A8(void* dest, void* source, uint64_t width, uint64_t height)
    {
        lanczos_8bit<4>(dest, source, width, height);
    }
}

std::vector<uint8_t> downscale(tga_header_t* header, void* pixels, downscaling_algroithm_e algorithm)
{
    uint64_t downscaled_width = header->image_spec.width / 2;
    uint64_t downscaled_height = header->image_spec.height / 2;

    std::vector<uint8_t> downscaled_buffer(sizeof(tga_header_t) + (downscaled_width * downscaled_height * (header->image_spec.pixel_depth / 8)));

    auto downscaled_header = reinterpret_cast<tga_header_t*>(downscaled_buffer.data());
    *downscaled_header = *header;
    downscaled_header->image_spec.width = downscaled_width;
    downscaled_header->image_spec.height = downscaled_height;

    void* downscaled_pixels = downscaled_buffer.data() + sizeof(tga_header_t);

    switch(algorithm)
    {
        case downscaling_algroithm_e::nearest:
        {
            switch(header->image_spec.pixel_depth)
            {
                case 15: nearest_R5G5B5(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 16: nearest_R5G5B5A1(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 24: nearest_R8G8B8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 32: nearest_R8G8B8A8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                default: abort();
            }
            break;
        }
        case downscaling_algroithm_e::average:
        {
            switch(header->image_spec.pixel_depth)
            {
                case 15: average_R5G5B5(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 16: average_R5G5B5A1(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 24: average_R8G8B8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 32: average_R8G8B8A8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                default: abort();
            }
            break;
        }
        case downscaling_algroithm_e::lanczos:
        {
            switch(header->image_spec.pixel_depth)
            {
                case 15: lanczos_R5G5B5(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 16: lanczos_R5G5B5A1(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 24: lanczos_R8G8B8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                case 32: lanczos_R8G8B8A8(downscaled_pixels, pixels, header->image_spec.width, header->image_spec.height); break;
                default: abort();
            }
            break;
        }
    }

    return downscaled_buffer;
}
