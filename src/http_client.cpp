#include "http_client.hpp"

HttpClient::HttpClient()
{
#ifdef _WIN32
    hInternet = InternetOpenA("RIR-Fetcher/1.0", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) {
        throw std::runtime_error("Failed to initialize WinINet");
    }
#else
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
}

HttpClient::~HttpClient()
{
#ifdef _WIN32
    if (hInternet) {
        InternetCloseHandle(hInternet);
    }
#else
    curl_global_cleanup();
#endif
}

#ifndef _WIN32
size_t HttpClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* data)
{
    size_t total_size = size * nmemb;
    data->insert(data->end(), static_cast<char*>(contents), static_cast<char*>(contents) + total_size);
    return total_size;
}
#endif

std::vector<char> HttpClient::fetch(const std::string& url)
{
    std::vector<char> data;

#ifdef _WIN32
    HINTERNET hRequest = InternetOpenUrlA(hInternet, url.c_str(), nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    if (!hRequest) {
        throw std::runtime_error("Failed to open URL: " + url);
    }

    char buffer[4096];
    DWORD bytes_read;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytes_read) && bytes_read > 0) {
        data.insert(data.end(), buffer, buffer + bytes_read);
    }

    InternetCloseHandle(hRequest);
#else
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RIR-Fetcher/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("CURL failed: " + std::string(curl_easy_strerror(res)));
    }
#endif

    return data;
}