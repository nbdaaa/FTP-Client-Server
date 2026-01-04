#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../net/socket.h"
#include "../net/socket_exception.h"
#include "../utils/utility.h"
#include "ftp_protocol.h"

using namespace std;

class FTPClient {
private:
    string host;
    string user;
    string passwd;
    int port;

    Socket* control_socket;
    Socket* data_socket;
    string last_command;  // Track last command for logging

    // Server commands
    void get(const string& filename);
    void put(const string& filename);
    string pwd(bool print = true);
    int cd(const string& path, bool print = true);
    void ls(const vector<string>& flags, const vector<string>& args);
    int mkd(const string& dirname, bool print = true);
    int pasv();
    bool quit();

    // Client-side commands (prefixed with !)
    string _pwd(bool print = true);
    int _cd(const string& path, bool print = true);
    void _ls(const vector<string>& flags, const vector<string>& args);
    int _mkd(const string& dirname, bool print = true);

    // Helper methods
    void help();
    void sendRequest(const string& cmd, const string& args = "");
    FTPProtocol::Response receiveResponse(bool log = true);

public:
    FTPClient(const string& host_name, int port_number, const string& user_name, const string& password);
    ~FTPClient();
    void start();
    void communicate();
};

#endif
