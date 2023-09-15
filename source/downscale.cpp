#include <vector>
#include <iostream>
#include <cstring>

#include "system.hpp"
#include "algorithm.hpp"

void usage()
{
    std::cout << "Usage: downscale [INPUT_FILE] [OUTPUT_FILE] [ALGORITHM]\n"
    << "INPUT_FILE: path/to/input/targa/image\n"
    << "OUTPUT_FILE: path/to/output/targa/image\n"
    << "ALGORITHM: \"nearest\" \"average\" \"lanczos\"\n";
}

int main(int argc, const char** argv)
{
    if(argc != 4)
    {
        usage();
        return EXIT_FAILURE;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    const char* algorithm = argv[3];

    std::vector<uint8_t> input_data = read_file_binary(input_file);
    tga_header_t* header = reinterpret_cast<tga_header_t*>(input_data.data());

    if(header->color_map_type != 0)
    {
        std::cout << "color map is not supported\n";
        return EXIT_FAILURE;
    }

    if(header->image_type != 2)
    {
        std::cout << "only uncompressed true-color image is supported\n";
        return EXIT_FAILURE;
    }

    void* pixels = input_data.data() + sizeof(tga_header_t) + header->id_length;

    downscaling_algroithm_e downscaling{};

    if(std::strcmp(algorithm, "nearest") == 0)
    {
        downscaling = downscaling_algroithm_e::nearest;
    }
    else if(std::strcmp(algorithm, "average") == 0)
    {
        downscaling = downscaling_algroithm_e::average;
    }
    else if(std::strcmp(algorithm, "lanczos") == 0)
    {
        downscaling = downscaling_algroithm_e::lanczos;
    }
    else
    {
        std::cout << algorithm << "is not a valid algorithm\n";
        usage();
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> output_data = downscale(header, pixels, downscaling);
    write_file_binary(output_file, output_data);

    return EXIT_SUCCESS;
}