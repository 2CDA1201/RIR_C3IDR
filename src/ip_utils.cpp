#include "ip_utils.hpp"
#include "bit_utils.hpp"

// n, n+1でペアの2桁数字リテラルのLUT // リトルエンディアン専用
#if defined(__LITTLE_ENDIAN__) || defined(_WIN32)
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
    '7', '9', '8', '9', '9'
};
#endif

void fast_ip_to_str(uint32_t ip, char* RESTRICT buf) noexcept
{
    const uint8_t octets[4] = {
        static_cast<uint8_t>((ip >> 24) & 0xFF),
        static_cast<uint8_t>((ip >> 16) & 0xFF),
        static_cast<uint8_t>((ip >> 8) & 0xFF),
        static_cast<uint8_t>(ip & 0xFF)
    };

    char* p = buf;
    for (const auto& [i, o] : std::views::enumerate(octets)) {
        if (o >= 100) {
            // 3桁数字処理: 百の位を直接計算
            *p++ = '0' + (o / 100);
            const uint8_t rem = o % 100;

            // LUT: 2桁数字ペアを事前計算済みテーブルから取得
            // 例(96): DIGIT_PAIRS[96*2] = '9', DIGIT_PAIRS[96*2+1] = '6' → "96"
            // 16bit一括書き込み: 2文字を1回のメモリ操作で高速処理
            *((uint16_t*)p) = *((uint16_t*)&DIGIT_PAIRS[rem * 2]);
            p += 2;
        } else if (o >= 10) {
            // 2桁数字処理: 3桁の時の2桁目以降と同様にLUT参照
            *((uint16_t*)p) = *((uint16_t*)&DIGIT_PAIRS[o * 2]);
            p += 2;
        } else {
            *p++ = '0' + o;
        }

        if (i != 3)
            *p++ = '.';
    }
    *p = '\0'; // 文字列終端
}

uint32_t fast_atoi(const char* RESTRICT str, const char* RESTRICT end) noexcept
{
    uint32_t result = 0;
    while (str < end) {
        const char c = *str++; // 現在の文字を取得し、ポインタを進める
        if (c >= '0' && c <= '9') {
            result = (result << 3) + (result << 1) + (c - '0'); // result * 10 + digit
        } else {
            break;
        }
    }
    return result;
}

uint32_t fast_ip_to_int(const char* RESTRICT ip_str, size_t len) noexcept
{
    const char* end = ip_str + len;
    const char* p = ip_str;
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
        ++p; // ピリオドをスキップ
    }
    return result;
}

size_t fast_parse_record(const char* RESTRICT line, size_t line_len, FieldPos* RESTRICT fields, size_t max_fields) noexcept
{
    const char* p = line;
    const char* end = line + line_len;
    const char* field_start = p;
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

    // 最後のフィールドを追加
    if (field_count < max_fields && field_start < end) {
        fields[field_count].ptr = field_start;
        fields[field_count].len = end - field_start;
        ++field_count;
    }

    return field_count;
}

void fast_cidr_decompose(std::ofstream& RESTRICT out, const char* RESTRICT country, size_t country_len, const char* RESTRICT ip_str, size_t ip_len, uint32_t total_hosts) noexcept
{
    uint32_t current_ip = fast_ip_to_int(ip_str, ip_len);
    uint32_t remaining = total_hosts;

    while (remaining > 0) {
        // Get max block size using bit manipulation
        const uint32_t trailing_zeros = current_ip ? bit::count_trailing_zeros(current_ip) : 32;
        const uint32_t max_aligned = 1U << trailing_zeros;

        const uint32_t leading_zeros = bit::count_leading_zeros(remaining);
        uint32_t max_size = 1U << (31 - leading_zeros);
        if (max_size > remaining)
            max_size >>= 1;

        const uint32_t block_size = (max_aligned < max_size) ? max_aligned : max_size;
        const uint8_t prefix = static_cast<uint8_t>(32 - (31 - bit::count_leading_zeros(block_size)));

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