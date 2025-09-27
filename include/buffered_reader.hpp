#pragma once
#include <fstream>

class BufferedReader
{
private:
    static constexpr size_t BUFFER_SIZE = 1024 * 1024; // 1MB buffer
    alignas(64) char buffer[BUFFER_SIZE];              // Cache line aligned
    std::ifstream &stream;
    size_t buffer_pos = 0;
    size_t buffer_end = 0;
    bool eof_reached = false;

    void fill_buffer() noexcept;

public:
    explicit BufferedReader(std::ifstream &s);
    bool getline(const char *&line_start, size_t &line_len) noexcept;
};