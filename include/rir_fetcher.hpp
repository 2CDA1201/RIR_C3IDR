#pragma once

#include <filesystem>
#include <future>
#include <map>
#include <string>
#include <vector>

#include "http_client.hpp"

struct RIRInfo {
    std::string region;
    std::string base_url;
    std::string file;
};

class RIRFetcher {
private:
    std::map<std::string, RIRInfo> rir_dict;
    HttpClient http_client;

    std::vector<char> read_file(const std::string& filepath);
    void write_file(const std::string& filepath, const std::vector<char>& data);
    bool fetch_rir(const RIRInfo& rir_info);

public:
    RIRFetcher();
    void fetch_all();
};