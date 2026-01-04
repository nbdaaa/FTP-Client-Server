#ifndef SOCKET_EXCEPTION_H
#define SOCKET_EXCEPTION_H

#include <string>
#include <exception>
#include <cstring>
#include <errno.h>

using namespace std;

class SocketException : public exception {
private:
    string _msg;

public:
    SocketException(const string& msg)
        : _msg(msg + " (" + string(strerror(errno)) + ")") {}

    virtual ~SocketException() throw() {}

    virtual const char* what() const throw() {
        return _msg.c_str();
    }

    string description() const { return _msg; }
};

#endif
