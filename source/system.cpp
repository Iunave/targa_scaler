#include "system.hpp"

#if defined(__linux__)

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <csignal>
#include <sys/stat.h>

class syscheck_t
{
public:
    syscheck_t& operator=(__syscall_slong_t in_val)
    {
        val = in_val;
        if(val == -1)
        {
            perror(nullptr);
            abort();
        }
        return *this;
    }

    operator __syscall_slong_t() const {return val;}
    __syscall_slong_t val = 0;
};
inline constinit thread_local syscheck_t syscheck;

std::vector<uint8_t> read_file_binary(const char* filepath)
{
    syscheck = open(filepath, O_RDONLY);
    int fd = syscheck;

    struct stat64 statbuf;
    syscheck = fstat64(fd, &statbuf);

    std::vector<uint8_t> file_data(statbuf.st_size);

    size_t nread = read(fd, file_data.data(), file_data.size());
    if(nread != statbuf.st_size)
    {
        perror(nullptr);
        abort();
    }

    syscheck = close(fd);

    return file_data;
}

void write_file_binary(const char* filepath, std::span<uint8_t> data)
{
    syscheck = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    int fd = syscheck;

    size_t nwrite = write(fd, data.data(), data.size());
    if(nwrite != data.size())
    {
        perror(nullptr);
        abort();
    }

    syscheck = close(fd);

}

#elif defined(__WIN32__)

#include <fstream>

std::vector<uint8_t> read_file_binary(const char* filepath)
{
    std::ifstream filestream{filepath, std::ios_base::in | std::ios_base::binary};

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
    std::ifstream filestream{filepath, std::ios_base::out | std::ios_base::binary};
    for(uint8_t byte : data)
    {
        filestream.putback(byte);
    }
    filestream.close();
}

#else
#error "unknown platform"
#endif