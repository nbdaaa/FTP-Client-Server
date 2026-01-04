#include "ftp_server.h"

using namespace std;

FTPServer::FTPServer(int port_number, const string& loginFile) {
    cout << "Cp.FTP-Server Started" << endl;
    port = port_number;
    loadUsers(loginFile);
}

FTPServer::~FTPServer() {
}

void FTPServer::start() {
    cout << "Starting server on port: " << port << endl;

    try {
        Socket control_socket(port);

        while (true) {
            try {
                Socket client_socket = control_socket.accept();

                if (!fork()) {
                    // Child process handles client
                    control_socket.close();
                    communicate(client_socket);
                    client_socket.close();
                    exit(0);
                }
            } catch (SocketException& e) {
                cout << "Exception occurred: " << e.description() << endl;
                continue;
            }
        }
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
        return;
    }
}

void FTPServer::communicate(Socket& client) {
    ClientSession session;
    string clientIP = client.host();

    try {
        // Send welcome message
        sendResponse(client, 200, "(Cp.FTP v1.0)");

        while (true) {
            string data = client.receive();
            string cmd, args;

            if (parseCommand(data, cmd, args)) {
                cout << getCurrentTime() << " " << cmd << " " << clientIP << endl;

                // Command dispatch
                if (cmd == "USER") {
                    handleUser(client, session, args);
                } else if (cmd == "PASS") {
                    handlePass(client, session, args);
                } else if (cmd == "SYST") {
                    handleSyst(client, session, args);
                } else if (cmd == "PWD" && session.logged_in) {
                    handlePwd(client, session, args);
                } else if (cmd == "CWD" && session.logged_in) {
                    handleCwd(client, session, args);
                } else if (cmd == "MKD" && session.logged_in) {
                    handleMkd(client, session, args);
                } else if (cmd == "LIST" && session.logged_in) {
                    handleList(client, session, args);
                } else if (cmd == "TYPE" && session.logged_in) {
                    handleType(client, session, args);
                } else if (cmd == "PASV" && session.logged_in) {
                    handlePasv(client, session, args);
                } else if (cmd == "STOR" && session.logged_in) {
                    handleStor(client, session, args);
                } else if (cmd == "RETR" && session.logged_in) {
                    handleRetr(client, session, args);
                } else if (cmd == "QUIT") {
                    handleQuit(client, session, args);
                    break;
                } else if (!session.logged_in) {
                    sendResponse(client, 332, "Need account for login.");
                } else {
                    sendResponse(client, 500, "UNKNOWN command");
                }
            } else {
                sendResponse(client, 500, "UNKNOWN command");
            }
        }
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
    }

    if (session.data_socket) {
        delete session.data_socket;
    }
}

void FTPServer::handleUser(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "USER: command requires a parameter");
        return;
    }

    if (users.find(args) != users.end()) {
        session.username = args;
        sendResponse(client, 331, "Please specify the password.");
    } else {
        sendResponse(client, 530, "Login incorrect.");
    }
}

void FTPServer::handlePass(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "PASS: command requires a parameter");
        return;
    }

    map<string, User>::iterator it = users.find(session.username);
    if (it != users.end() && it->second.password == args) {
        string homeDir = it->second.homeDir;
        if (changeDir(homeDir)) {
            session.logged_in = true;
            sendResponse(client, 230, "Login successful.");
        } else {
            sendResponse(client, 530, "Cannot change to home directory.");
        }
    } else {
        sendResponse(client, 530, "Login incorrect.");
    }
}

void FTPServer::handleSyst(Socket& client, ClientSession& session, const string& args) {
    sendResponse(client, 215, getSystemInfo());
}

void FTPServer::handlePwd(Socket& client, ClientSession& session, const string& args) {
    string pwd = getCurrentDir();
    sendResponse(client, 257, "\"" + pwd + "\"");
}

void FTPServer::handleCwd(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "CWD: command requires a parameter");
        return;
    }

    if (changeDir(args)) {
        sendResponse(client, 250, "Directory successfully changed.");
    } else {
        sendResponse(client, 550, "Failed to change directory.");
    }
}

void FTPServer::handleMkd(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "MKD: command requires a parameter");
        return;
    }

    int code;
    string response = exec_cmd("mkdir", args, code);

    if (code) {
        sendResponse(client, 257, response);
    } else {
        sendResponse(client, 550, response);
    }
}

void FTPServer::handleList(Socket& client, ClientSession& session, const string& args) {
    if (!session.data_socket) {
        sendResponse(client, 425, "Use PASV first.");
        return;
    }

    int code;
    string request = "ls -l " + args;
    string response = exec_cmd("ls", request, code);

    if (code) {
        sendResponse(client, 150, "Here comes the directory listing.");

        try {
            Socket data_client = session.data_socket->accept();

            // Send data in chunks
            int pos = 0;
            int len = response.length();
            while (pos < len) {
                int chunk_size = min(2048, len - pos);
                string chunk = response.substr(pos, chunk_size);
                data_client << chunk;
                pos += chunk_size;
            }

            data_client.close();
            sendResponse(client, 226, "Directory send OK.");
        } catch (SocketException& e) {
            cout << "Exception occurred: " << e.description() << endl;
            sendResponse(client, 450, "Directory NOT send.");
        }
    } else {
        sendResponse(client, 501, "Syntax error in parameters or arguments.");
    }

    session.data_socket->close();
    delete session.data_socket;
    session.data_socket = NULL;
}

void FTPServer::handleType(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "TYPE: command requires a parameter");
        return;
    }

    if (args == "I" || args == "A") {
        session.binary_mode = true;
        sendResponse(client, 200, "Switching to Binary mode.");
    } else {
        sendResponse(client, 400, "Mode not supported");
    }
}

void FTPServer::handlePasv(Socket& client, ClientSession& session, const string& args) {
    try {
        session.data_socket = new Socket(0);  // OS assigns random port

        string host = client.host();
        replace(host.begin(), host.end(), '.', ',');

        int data_port = session.data_socket->port();
        stringstream port_str;
        port_str << data_port / 256 << "," << data_port % 256;

        sendResponse(client, 227, "Entering Passive Mode (" + host + "," + port_str.str() + ").");
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
        sendResponse(client, 425, "Cannot open data connection");
    }
}

void FTPServer::handleStor(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "STOR: command requires a parameter");
        return;
    }

    if (!session.binary_mode) {
        sendResponse(client, 550, "Switch to Binary mode first.");
        return;
    }

    if (!session.data_socket) {
        sendResponse(client, 425, "Use PASV first.");
        return;
    }

    ofstream out(args.c_str(), ios::out | ios::binary);
    if (!out) {
        sendResponse(client, 550, "Create file operation failed.");
        return;
    }

    sendResponse(client, 150, "Ok to send data.");

    try {
        Socket data_client = session.data_socket->accept();

        string buff;
        while (true) {
            buff = data_client.receive();
            if (buff.length() == 0) {
                break;
            }
            out << buff;
        }

        out.close();
        data_client.close();
        sendResponse(client, 226, "Transfer complete.");
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
        sendResponse(client, 450, "Transfer failed.");
    }

    session.data_socket->close();
    delete session.data_socket;
    session.data_socket = NULL;
}

void FTPServer::handleRetr(Socket& client, ClientSession& session, const string& args) {
    if (args.empty()) {
        sendResponse(client, 500, "RETR: command requires a parameter");
        return;
    }

    if (!session.binary_mode) {
        sendResponse(client, 550, "Switch to Binary mode first.");
        return;
    }

    if (!session.data_socket) {
        sendResponse(client, 425, "Use PASV first.");
        return;
    }

    ifstream in(args.c_str(), ios::in | ios::binary | ios::ate);
    if (!in) {
        sendResponse(client, 550, "Couldn't retrieve file.");
        return;
    }

    long length = in.tellg();
    in.seekg(0, in.beg);

    stringstream msg;
    msg << "Opening BINARY mode data connection for " << args << " (" << length << " bytes).";
    sendResponse(client, 150, msg.str());

    try {
        Socket data_client = session.data_socket->accept();

        while (length > 0) {
            int read_sz = MAXRECV < length ? MAXRECV : length;
            char buf[MAXRECV + 1];
            in.read(buf, read_sz);
            string data(buf, read_sz);
            data_client << data;
            length -= read_sz;
        }

        in.close();
        data_client.close();
        sendResponse(client, 226, "Transfer complete.");
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
        sendResponse(client, 450, "Transfer failed.");
    }

    session.data_socket->close();
    delete session.data_socket;
    session.data_socket = NULL;
}

void FTPServer::handleQuit(Socket& client, ClientSession& session, const string& args) {
    sendResponse(client, 221, "Goodbye.");
}

void FTPServer::sendResponse(Socket& client, int code, const string& message) {
    string response = FTPProtocol::formatResponse(code, message);
    client << response;
}

string FTPServer::getCurrentDir() {
    int code;
    string response = exec_cmd("pwd", "", code);
    // Remove quotes and newline
    if (response.length() >= 3) {
        return response.substr(1, response.length() - 3);
    }
    return response;
}

bool FTPServer::changeDir(const string& path) {
    int code;
    exec_cmd("cd", path, code);
    return code == 1;
}

string FTPServer::getSystemInfo() {
    int code;
    string response = exec_cmd("uname", "uname", code);
    return response;
}

void FTPServer::loadUsers(const string& loginFile) {
    ifstream infile(loginFile.c_str());
    if (!infile) {
        cerr << "Warning: Could not open login file: " << loginFile << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        stringstream ss(line);
        string username, password, homeDir;

        if (ss >> username >> password >> homeDir) {
            User user;
            user.password = password;
            user.homeDir = homeDir;
            users[username] = user;
        }
    }

    infile.close();
    cout << "Loaded " << users.size() << " users from " << loginFile << endl;
}
