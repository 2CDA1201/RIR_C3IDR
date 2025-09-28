#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
#include <curl/curl.h>
#endif

class HttpClient {
private:
#ifdef _WIN32
    HINTERNET hInternet;
    HINTERNET hConnect;
#endif

public:
    HttpClient();
    ~HttpClient();

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* data);
    std::vector<char> fetch(const std::string& url);
};