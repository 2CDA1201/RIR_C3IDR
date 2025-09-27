#include "ip_utils.hpp"

// 事前に計算された2桁のLUT
static constexpr char DIGIT_PAIRS[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
    '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
    '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
    '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
    '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
    '7', '9', '8', '9', '9'};

// 1桁のLUT
static constexpr char SINGLE_DIGITS[10] = {'0', '1', '2', '3', '4',
                                           '5', '6', '7', '8', '9'};

void fast_ip_to_str(uint32_t ip, char *__restrict__ buf) noexcept {
  const uint32_t octets[4] = {(ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                              (ip >> 8) & 0xFF, ip & 0xFF};

  char *p = buf;
  for (const auto &[i, o] : std::views::enumerate(octets)) {
    if (o >= 100) {
      *p++ = '0' + (o / 100);
      const uint32_t rem = o % 100;
      *((uint16_t *)p) = *((uint16_t *)&DIGIT_PAIRS[rem * 2]);
      p += 2;
    } else if (o >= 10) {
      *((uint16_t *)p) = *((uint16_t *)&DIGIT_PAIRS[o * 2]);
      p += 2;
    } else {
      *p++ = SINGLE_DIGITS[o];
    }
    if (i != 3) *p++ = '.';
  }
  *p = '\0';
}

uint32_t fast_atoi(const char *__restrict__ str,
                   const char *__restrict__ end) noexcept {
  uint32_t result = 0;
  while (str < end) {
    const char c = *str++;  // 現在の文字を取得し、ポインタを進める
    if (c >= '0' && c <= '9') {
      result =
          (result << 3) + (result << 1) + (c - '0');  // result * 10 + digit
    } else {
      break;
    }
  }
  return result;
}

uint32_t fast_ip_to_int(const char *__restrict__ ip_str, size_t len) noexcept {
  const char *end = ip_str + len;
  const char *p = ip_str;
  uint32_t result = 0;
  int shift = 24;

  while (p < end && shift >= 0) {
    uint32_t octet = 0;
    while (p < end && *p != '.') {
      octet = (octet << 3) + (octet << 1) + (*p - '0');
      ++p;
    }
    result |= (octet << shift);
    shift -= 8;
    ++p;  // ピリオドをスキップ
  }
  return result;
}

size_t fast_parse_fields(const char *__restrict__ line, size_t line_len,
                         FieldPos *__restrict__ fields,
                         size_t max_fields) noexcept {
  const char *p = line;
  const char *end = line + line_len;
  const char *field_start = p;
  size_t field_count = 0;

  while (p < end && field_count < max_fields) {
    if (*p == '|') {
      fields[field_count].ptr = field_start;
      fields[field_count].len = p - field_start;
      ++field_count;
      field_start = ++p;
    } else {
      ++p;
    }
  }

  // Handle last field
  if (field_count < max_fields && field_start < end) {
    fields[field_count].ptr = field_start;
    fields[field_count].len = end - field_start;
    ++field_count;
  }

  return field_count;
}

void fast_cidr_decompose(std::ofstream &__restrict__ out,
                         const char *__restrict__ country, size_t country_len,
                         const char *__restrict__ ip_str, size_t ip_len,
                         uint32_t total_hosts) noexcept {
  uint32_t current_ip = fast_ip_to_int(ip_str, ip_len);
  uint32_t remaining = total_hosts;

  while (remaining > 0) {
    // Get max block size using bit manipulation
    const uint32_t trailing_zeros = current_ip ? __builtin_ctz(current_ip) : 32;
    const uint32_t max_aligned = 1U << trailing_zeros;

    const uint32_t leading_zeros = __builtin_clz(remaining);
    uint32_t max_size = 1U << (31 - leading_zeros);
    if (max_size > remaining) max_size >>= 1;

    const uint32_t block_size =
        (max_aligned < max_size) ? max_aligned : max_size;
    const uint8_t prefix = 32 - (31 - __builtin_clz(block_size));

    // Convert IP to string using fast method
    char ip_buffer[16];
    fast_ip_to_str(current_ip, ip_buffer);

    // Write directly to stream with minimal function calls
    out.write(country, country_len);
    out.put(',');
    out << ip_buffer << '/';

    // Fast prefix writing
    if (prefix >= 10) {
      out.put('0' + (prefix / 10));
      out.put('0' + (prefix % 10));
    } else {
      out.put('0' + prefix);
    }
    out.put('\n');

    current_ip += block_size;
    remaining -= block_size;
  }
}