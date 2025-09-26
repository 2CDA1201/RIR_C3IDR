#include <iostream>
#include <chrono>
#include <filesystem>

#include "ultra_buffered_reader.hpp"
#include "ip_utils.hpp"

namespace chrono = std::chrono;

int main() {
    // Ultra-large output buffers
    static constexpr size_t OUT_BUFFER_SIZE = 2 * 1024 * 1024; // 2MB
    alignas(64) static char out4_buffer[OUT_BUFFER_SIZE];
    alignas(64) static char out6_buffer[OUT_BUFFER_SIZE];
    
    std::ofstream fout4("cidr/ipv4_cidr.csv", std::ios::binary | std::ios::trunc);
    std::ofstream fout6("cidr/ipv6_cidr.csv", std::ios::binary | std::ios::trunc);
    
    if (!fout4 || !fout6) {
        std::cerr << "Failed to open output files" << std::endl;
        return 1;
    }
    
    fout4.rdbuf()->pubsetbuf(out4_buffer, OUT_BUFFER_SIZE);
    fout6.rdbuf()->pubsetbuf(out6_buffer, OUT_BUFFER_SIZE);
    
    const auto start = chrono::high_resolution_clock::now();
    
    // Process files sequentially for better cache behavior
    for (const auto& entry : std::filesystem::directory_iterator("data")) {
        std::ifstream fin(entry.path(), std::ios::binary);
        if (!fin) continue;
        
        UltraBufferedReader reader(fin);
        const char* line;
        size_t line_len;
        size_t region_len = 0;
        int header_count = 0;
        
        // Ultra-fast header processing
        while (reader.getline(line, line_len)) {
            if (line_len == 0) continue;
            
            const char first = line[0];
            if (first == '#') {
                ++header_count;
                continue;
            }
            
            if (first == '2') {
                // Extract region length quickly
                for (size_t i = 0; i < line_len - 1; ++i) {
                    if (line[i] == '|') {
                        for (size_t j = i + 1; j < line_len; ++j) {
                            if (line[j] == '|') {
                                region_len = j - i - 1;
                                break;
                            }
                        }
                        break;
                    }
                }
                ++header_count;
                continue;
            }
            
            if (line_len > 7 && std::string_view(line + line_len - 7, 7) == "summary") {
                ++header_count;
                continue;
            }
            
            break;
        }
        
        // Process data records with ultra-fast parsing
        do {
            if (line_len == 0) continue;
            
            // Ultra-fast type detection
            const size_t type_pos = region_len + 4;
            if (type_pos >= line_len || line[type_pos] == 'a') continue;
            
            const size_t version_pos = region_len + 7;
            if (version_pos >= line_len) continue;
            
            // Parse fields ultra-fast
            FieldPos fields[8];
            const size_t field_count = ultra_fast_parse_fields(line, line_len, fields, 8);
            if (field_count < 5) continue;
            
            if (line[version_pos] == '4') {
                // IPv4 ultra-fast processing
                const uint32_t block_count = ultra_fast_atoi(fields[4].ptr, fields[4].ptr + fields[4].len);
                
                ultra_fast_cidr_decompose(fout4,
                    fields[1].ptr, fields[1].len,
                    fields[3].ptr, fields[3].len,
                    block_count);
                    
            } else if (line[version_pos] == '6') {
                // IPv6 direct write (no decomposition needed)
                fout6.write(fields[1].ptr, fields[1].len);
                fout6.put(',');
                fout6.write(fields[3].ptr, fields[3].len);
                fout6.put('/');
                fout6.write(fields[4].ptr, fields[4].len);
                fout6.put('\n');
            }
            
        } while (reader.getline(line, line_len));
    }
    
    const auto end = chrono::high_resolution_clock::now();
    
    std::cout << "Elapsed: " 
              << chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0 
              << "ms" << std::endl;
    
    return 0;
}