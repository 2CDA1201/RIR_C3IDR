#pragma once
#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>

#ifdef _MSC_VER
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

// Field position structure
struct FieldPos {
    const char* ptr;
    size_t len;
};

// Fast IP to string conversion
void fast_ip_to_str(uint32_t ip, char* RESTRICT buf) noexcept;

// Fast string to integer conversion
uint32_t fast_atoi(const char* RESTRICT str, const char* RESTRICT end) noexcept;

// Fast IP string to integer conversion
uint32_t fast_ip_to_int(const char* RESTRICT ip_str, size_t len) noexcept;

// Fast field parsing
size_t fast_parse_record(const char* RESTRICT line, size_t line_len, FieldPos* RESTRICT fields, size_t max_fields) noexcept;

// Fast CIDR decomposition
void fast_cidr_decompose(std::ofstream& RESTRICT out, const char* RESTRICT country, size_t country_len, const char* RESTRICT ip_str, size_t ip_len, uint32_t total_hosts) noexcept;