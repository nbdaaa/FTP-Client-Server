#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "../net/socket.h"
#include "../net/socket_exception.h"
#include "../utils/utility.h"
#include "ftp_protocol.h"

using namespace std;

class FTPServer {
private:
    int port;

    // User management
    struct User {
        string password;
        string homeDir;
    };
    map<string, User> users; // Cấu trúc gồm username, password và homeDir. Bản chất sử dụng map là vì username là unique identifier nên dùng làm key tra cứu theo kiểu dictionary

    // Session state
    struct ClientSession {
        string username;
        bool logged_in;
        bool binary_mode;
        Socket data_socket;

        ClientSession() : logged_in(false), binary_mode(false), data_socket() {}
    };

    // Command handlers
    void handleUser(Socket& client, ClientSession& session, const string& args);
    void handlePass(Socket& client, ClientSession& session, const string& args);
    void handleSyst(Socket& client, ClientSession& session, const string& args);
    void handlePwd(Socket& client, ClientSession& session, const string& args);
    void handleCwd(Socket& client, ClientSession& session, const string& args);
    void handleMkd(Socket& client, ClientSession& session, const string& args);
    void handleList(Socket& client, ClientSession& session, const string& args);
    void handleType(Socket& client, ClientSession& session, const string& args);
    void handlePasv(Socket& client, ClientSession& session, const string& args);
    void handleStor(Socket& client, ClientSession& session, const string& args);
    void handleRetr(Socket& client, ClientSession& session, const string& args);
    void handleQuit(Socket& client, ClientSession& session, const string& args);

    // Helper methods
    void sendResponse(Socket& client, int code, const string& message);
    string getCurrentDir();
    bool changeDir(const string& path);
    string getSystemInfo();
    void loadUsers(const string& loginFile);

    // Client communication
    void communicate(Socket& client);

public:
    FTPServer(int port_number, const string& loginFile = "data/login.info");
    ~FTPServer();
    void start();
};

#endif
