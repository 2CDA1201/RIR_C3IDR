#pragma once
#include <cstdint>
#include <fstream>


// Field position structure
struct FieldPos {
  const char *ptr;
  size_t len;
};

// Fast IP to string conversion
void fast_ip_to_str(uint32_t ip, char *__restrict__ buf) noexcept;

// Fast string to integer conversion
uint32_t fast_atoi(const char *__restrict__ str,
                   const char *__restrict__ end) noexcept;

// Fast IP string to integer conversion
uint32_t fast_ip_to_int(const char *__restrict__ ip_str, size_t len) noexcept;

// Fast field parsing
size_t fast_parse_fields(const char *__restrict__ line, size_t line_len,
                         FieldPos *__restrict__ fields,
                         size_t max_fields) noexcept;

// Fast CIDR decomposition
void fast_cidr_decompose(std::ofstream &__restrict__ out,
                         const char *__restrict__ country, size_t country_len,
                         const char *__restrict__ ip_str, size_t ip_len,
                         uint32_t total_hosts) noexcept;