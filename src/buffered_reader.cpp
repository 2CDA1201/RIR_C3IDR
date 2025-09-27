#include "buffered_reader.hpp"

void BufferedReader::fill_buffer() noexcept
{
    stream.read(buffer, BUFFER_SIZE);
    buffer_end = stream.gcount();
    buffer_pos = 0;
    if (buffer_end < BUFFER_SIZE)
        eof_reached = true;
}

BufferedReader::BufferedReader(std::ifstream &s) : stream(s)
{
    fill_buffer();
}

bool BufferedReader::getline(const char *&line_start, size_t &line_len) noexcept
{
    if (buffer_pos >= buffer_end && eof_reached)
        return false;

    line_start = buffer + buffer_pos;
    const char *line_end = line_start;

    // Fast line scanning
    while (buffer_pos < buffer_end)
    {
        if (buffer[buffer_pos] == '\n')
        {
            line_len = line_end - line_start;
            ++buffer_pos; // skip newline
            return true;
        }
        line_end = buffer + (++buffer_pos);
    }

    if (!eof_reached)
    {
        // Handle line spanning buffer boundary (rare case)
        fill_buffer();
        if (buffer_end > 0)
        {
            while (buffer_pos < buffer_end && buffer[buffer_pos] != '\n')
            {
                ++buffer_pos;
            }
            if (buffer_pos < buffer_end)
            {
                ++buffer_pos; // skip newline
            }
        }
    }

    line_len = line_end - line_start;
    return line_len > 0;
}