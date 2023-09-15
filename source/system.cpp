#include "system.hpp"

#if defined(__linux__)
#warning "building for linux"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <csignal>
#include <sys/stat.h>

static void check_syscall(int ret, const char* info = nullptr)
{
    if(ret == -1)
    {
        perror(info);
        abort();
    }
}

std::vector<uint8_t> read_file_binary(const char* filepath)
{
    int fd = open(filepath, O_RDONLY);
    check_syscall(fd, filepath);

    struct stat64 statbuf;
    check_syscall(fstat64(fd, &statbuf), filepath);

    std::vector<uint8_t> file_data(statbuf.st_size);

    size_t nread = read(fd, file_data.data(), file_data.size());
    if(nread != statbuf.st_size)
    {
        perror(filepath);
        abort();
    }

    check_syscall(close(fd), filepath);

    return file_data;
}

void write_file_binary(const char* filepath, std::span<uint8_t> data)
{
    int fd = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    check_syscall(fd, filepath);

    size_t nwrite = write(fd, data.data(), data.size());
    if(nwrite != data.size())
    {
        perror(filepath);
        abort();
    }

    check_syscall(close(fd), filepath);
}

#elif defined(__WIN32__)
#warning "building for windows"

#include <fstream>

std::vector<uint8_t> read_file_binary(const char* filepath)
{
    std::ifstream filestream{filepath, std::ios_base::in | std::ios_base::binary};
    if(!filestream.is_open())
    {
        perror(filepath);
        abort();
    }

    std::vector<uint8_t> file_data{};
    while(!filestream.eof())
    {
        uint8_t& byte = file_data.emplace_back();
        filestream.get(reinterpret_cast<char&>(byte));
    }

    filestream.close();
    return file_data;
}

void write_file_binary(const char* filepath, std::span<uint8_t> data)
{
    std::ofstream filestream{filepath, std::ios_base::out | std::ios_base::binary};
    if(!filestream.is_open())
    {
        perror(filepath);
        abort();
    }

    for(uint8_t byte : data)
    {
        filestream.put(byte);
    }

    filestream.close();
}

#else
#error "unknown platform"
#endif