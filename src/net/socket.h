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
    explicit Socket(int port); // Đặt thêm explicit vào để tránh ép kiểu ngầm định. Ví dụ Socket s = 8080 --> Lỗi. Socket s(8080) --> Oke

    // Client constructor
    Socket(const string& host, int port); // Không cần đặt thêm explicit vì không xuất hiện hiện tượng ép kiểu ngầm định do đầu vào của Client Socket yêu cầu 2 tham số (thay vì 1)

    Socket();

    ~Socket();

    // Disable copying
    Socket(const Socket&) = delete; // Không cho phép hiện tượng: Socket a(8080); Socket b = a (lỗi); Socket c(a); (lỗi)
    Socket& operator=(const Socket&) = delete; // Không cho phép hiện tượng: Socket a(8080); Socket b(9090); b = a; (lỗi gán)

    // Enable moving (C++11)
    Socket(Socket&& other);
    Socket& operator=(Socket&& other); // Tạo ra move constructor. Ví dụ: Socket a(8080); Socket b = move(a) (khi đó, a sẽ rỗng và b sẽ nhận fd cũng như địa chỉ của a)

    // Operations
    Socket accept();
    void send(const string& data);
    string receive();
    void close();

    // Getters
    int fd() const { return _sockfd; }
    int port() const;
    string host() const;
};

#endif
