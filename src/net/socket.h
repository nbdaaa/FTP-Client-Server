#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

#define BACKLOG 20
#define MAXRECV 2048

class Socket {
private:
    int _sockfd;
    struct sockaddr_in _addr;
    bool _is_server;

    bool isValid() const;
    void createSocket();

public:
    // Server constructor
    explicit Socket(int port);

    // Client constructor
    Socket(const string& host, int port);

    // Default constructor (for accept)
    Socket();

    ~Socket();

    // Disable copying
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Enable moving (C++11)
    Socket(Socket&& other);
    Socket& operator=(Socket&& other);

    // Operations
    Socket accept();
    void send(const string& data);
    string receive();
    void close();

    // Operator overloads
    Socket& operator<<(const string& data);
    Socket& operator>>(string& data);

    // Getters
    int fd() const { return _sockfd; }
    int port() const;
    string host() const;
};

#endif
