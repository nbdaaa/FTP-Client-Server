#include "socket.h"
#include "socket_exception.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>

using namespace std;

// Default constructor
Socket::Socket() : _sockfd(-1), _is_server(false) {
    memset(&_addr, 0, sizeof(_addr));
}

// Server constructor
Socket::Socket(int port) : _sockfd(-1), _is_server(true) {
    memset(&_addr, 0, sizeof(_addr));

    createSocket();

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(port);

    // Bind socket
    if (::bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1) {
        throw SocketException("Failed to bind socket");
    }

    // Listen for connections
    if (::listen(_sockfd, BACKLOG) == -1) {
        throw SocketException("Failed to listen on socket");
    }
}

// Client constructor
Socket::Socket(const string& host, int port) : _sockfd(-1), _is_server(false) {
    memset(&_addr, 0, sizeof(_addr));

    createSocket();

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);

    // Resolve hostname
    struct hostent* he = gethostbyname(host.c_str());
    if (he == NULL) {
        throw SocketException("Failed to resolve hostname: " + host);
    }

    memcpy(&_addr.sin_addr, he->h_addr_list[0], he->h_length);

    // Connect to server
    if (::connect(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1) {
        throw SocketException("Failed to connect to " + host);
    }
}

Socket::~Socket() {
    if (isValid()) {
        ::close(_sockfd);
    }
}

// Move constructor
Socket::Socket(Socket&& other) : _sockfd(other._sockfd), _addr(other._addr), _is_server(other._is_server) {
    other._sockfd = -1;  // Invalidate the source socket
}

// Move assignment operator
Socket& Socket::operator=(Socket&& other) {
    if (this != &other) {
        // Close current socket if valid
        if (isValid()) {
            ::close(_sockfd);
        }

        // Move data from other
        _sockfd = other._sockfd;
        _addr = other._addr;
        _is_server = other._is_server;

        // Invalidate the source socket
        other._sockfd = -1;
    }
    return *this;
}

void Socket::createSocket() {
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (!isValid()) {
        throw SocketException("Failed to create socket");
    }

    // Set socket options
    int on = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        throw SocketException("Failed to set socket options");
    }
}

bool Socket::isValid() const {
    return _sockfd != -1;
}

Socket Socket::accept() {
    if (!_is_server) {
        throw SocketException("Cannot accept on client socket");
    }

    Socket new_socket;
    socklen_t addr_len = sizeof(new_socket._addr);

    new_socket._sockfd = ::accept(_sockfd, (struct sockaddr*)&new_socket._addr, &addr_len);

    if (!new_socket.isValid()) {
        throw SocketException("Failed to accept connection");
    }

    return new_socket;
}

void Socket::send(const string& data) {
    if (::send(_sockfd, data.c_str(), data.length(), 0) == -1) {
        throw SocketException("Failed to send data");
    }
}

string Socket::receive() {
    char buffer[MAXRECV + 1];
    memset(buffer, 0, MAXRECV + 1);

    int bytes_received = ::recv(_sockfd, buffer, MAXRECV, 0);

    if (bytes_received == -1) {
        throw SocketException("Failed to receive data");
    } else if (bytes_received == 0) {
        return "";
    }

    return string(buffer, bytes_received);
}

void Socket::close() {
    if (isValid()) {
        ::close(_sockfd);
        _sockfd = -1;
    }
}

Socket& Socket::operator<<(const string& data) {
    send(data);
    return *this;
}

Socket& Socket::operator>>(string& data) {
    data = receive();
    return *this;
}

int Socket::port() const {
    if (!isValid()) {
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getsockname(_sockfd, (struct sockaddr*)&addr, &addr_len) == -1) {
        return -1;
    }

    return ntohs(addr.sin_port);
}

string Socket::host() const {
    return string(inet_ntoa(_addr.sin_addr));
}
