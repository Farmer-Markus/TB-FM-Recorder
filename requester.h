#pragma once
#include <cstddef>
#include <string>

#include <openssl/ssl.h>


// Http/s request (default port 443)
class Requester {
public:
    ~Requester();

    // Throws exception.                Defaults to port 443 (https)
    void prepare(const char* hostName, const char* port = "443");
    bool sendRequest(const char* req) const;

    // Do not run this in the main thread! It will block and waste cpu time
    bool receive(int* bytesReceived, char* buffer, const size_t bufferSize);
    static void splitUrl(std::string* hostUrl, std::string* subUrl, std::string url);

private:
    int m_sock = -1;
    char* m_buffer = nullptr;
    SSL_CTX* m_ctx = nullptr;
    SSL* m_ssl = nullptr;


};