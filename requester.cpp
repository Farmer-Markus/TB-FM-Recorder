#include "requester.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/tls1.h>
#include <openssl/x509.h>


Requester::~Requester() {
    if(m_ssl) {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
    }
    if(m_ctx) {
        SSL_CTX_free(m_ctx);
    }
    if(m_sock > 0) {
        close(m_sock);
    }
}

void Requester::prepare(const char* hostName, const char* port) {
    if(m_sock > 0 || m_ctx || m_ssl)
        throw std::runtime_error("Requester already initialized!");

    // required, result
    addrinfo req, *res;
    memset(&req, 0, sizeof(req)); // Clear to 0

    // IPV4/6 : AF_INET/AF_INET6 | AF_UNSPEC : Both, does not matter
    req.ai_family = AF_UNSPEC;

    // SOCK_STREAM : TCP | SOCK_DGRAM = UDP
    req.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(hostName, port, &req, &res) != 0)
        throw std::runtime_error("Failed to get address info");

    m_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(m_sock < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("Failed to open socket");
    }

    if(connect(m_sock, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("Failed to connect to server");
    }

    freeaddrinfo(res);

    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    m_ctx = SSL_CTX_new(TLS_client_method());
    if(!m_ctx)
        throw std::runtime_error("Failed to SSL_CTX_new()");

    m_ssl = SSL_new(m_ctx);
    if(!m_ssl)
        throw std::runtime_error("Failed to SSL_new()");

    SSL_set_fd(m_ssl, m_sock);

    if(!SSL_set_tlsext_host_name(m_ssl, hostName))
        throw std::runtime_error("Failed to SSL_set_tlsext_host_name()");

    if(SSL_connect(m_ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Failed to SSL_connect()");
    }

    X509* cert = SSL_get_peer_certificate(m_ssl);
    if(!cert)
        throw std::runtime_error("No server certificates");

    X509_free(cert);
}

bool Requester::sendRequest(const char* req) const {
    if(!m_ssl)
        return false;

    //send(m_sock, req, strlen(req), 0);
    int send = SSL_write(m_ssl, req, strlen(req));
    return (send > 0);
}

bool Requester::receive(int* bytesReceived, char* buffer, const size_t bufferSize) {
    if(m_sock < 0)
        return false;

    //*bytesReceived = recv(m_sock, buffer, bufferSize, 0);
    *bytesReceived = SSL_read(m_ssl, buffer, bufferSize);
    if(*bytesReceived <= 0)
        return false;

    return true;
}

void Requester::splitUrl(std::string* hostUrl, std::string* subUrl, std::string url) {
    size_t i = url.find("http");
    if(i != std::string::npos)  {
        if(url[i + 4] == 's') {
            url.erase(i, 8);
        } else {
            url.erase(i, 7);
        }
    }

    i = url.find("/");
    if(i != std::string::npos) {
        // Skip '/' which is already defined below in the request
        *subUrl = url.substr(i + 1);
        url.erase(i, url.size() - i);
    }

    if(url.size() < 5)
        throw std::runtime_error("URL to short for a valid url");

    *hostUrl = std::move(url);
}