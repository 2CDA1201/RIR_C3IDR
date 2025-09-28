#include "rir_fetcher.hpp"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

RIRFetcher::RIRFetcher()
    : rir_dict {
        { "ARIN", { "ARIN", "https://ftp.arin.net/pub/stats/arin/", "delegated-arin-extended-latest" } },
        { "RIPE NCC", { "RIPE NCC", "https://ftp.ripe.net/pub/stats/ripencc/", "delegated-ripencc-latest" } },
        { "APNIC", { "APNIC", "https://ftp.apnic.net/pub/stats/apnic/", "delegated-apnic-latest" } },
        { "LACNIC", { "LACNIC", "https://ftp.lacnic.net/pub/stats/lacnic/", "delegated-lacnic-latest" } },
        { "AFRINIC", { "AFRINIC", "https://ftp.afrinic.net/pub/stats/afrinic/", "delegated-afrinic-latest" } }
    }
{
}

std::vector<char> RIRFetcher::read_file(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return buffer;
}

void RIRFetcher::write_file(const std::string& filepath, const std::vector<char>& data)
{
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    file.write(data.data(), data.size());
}

bool RIRFetcher::fetch_rir(const RIRInfo& rir_info)
{
    try {
        std::cout << "Checking updates for " << rir_info.region << " ..." << std::endl;

        // MD5の取得
        std::string md5_url = rir_info.base_url + rir_info.file + ".md5";
        std::vector<char> remote_md5 = http_client.fetch(md5_url);

        std::string local_md5_path = "data/" + rir_info.file + ".md5";
        std::string local_file_path = "data/" + rir_info.file + ".psv";

        // ローカルのMD5と比較
        if (fs::exists(local_md5_path)) {
            std::vector<char> local_md5 = read_file(local_md5_path);
            if (remote_md5 == local_md5) {
                std::cout << "No update for " << rir_info.file << ", skipping download." << std::endl;
                return true;
            }
        }

        // データ本体を取得
        std::string data_url = rir_info.base_url + rir_info.file;
        std::cout << "Fetching " << rir_info.file << " ..." << std::endl;
        std::vector<char> data = http_client.fetch(data_url);

        // データの保存
        write_file(local_file_path, data);

        // MD5の保存
        write_file(local_md5_path, remote_md5);

        std::cout << "Updated " << rir_info.file << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error accessing " << rir_info.base_url << ": " << e.what() << std::endl;
        return false;
    }
}

void RIRFetcher::fetch_all()
{
    // データディレクトリの作成
    fs::create_directories("data");

    // 並行処理でRIRデータを取得
    std::vector<std::future<bool>> futures;

    for (const auto& [key, rir_info] : rir_dict) {
        futures.push_back(
            std::async(std::launch::async, [this, rir_info]() {
                return this->fetch_rir(rir_info);
            }));
    }

    // 全てのタスクの完了を待機
    for (auto& future : futures) {
        future.get();
    }
}