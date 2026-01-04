#include "ftp_client.h"

using namespace std;

FTPClient::FTPClient(const string& host_name, int port_number, const string& user_name, const string& password) {
    cout << "\nCp.FTP-Client Started\n" << endl;
    host = host_name;
    user = user_name;
    passwd = password;
    port = port_number;
    control_socket = NULL;
    data_socket = NULL;
}

FTPClient::~FTPClient() {
    if (control_socket) delete control_socket;
    if (data_socket) delete data_socket;
}

void FTPClient::start() {
    cout << "Connecting to Host: " << host << " Port: " << port << endl;

    try {
        control_socket = new Socket(host, port);

        // Receive welcome message
        FTPProtocol::Response resp = receiveResponse();
        cout << resp.message;

        // Send USER command
        sendRequest("USER", user);
        resp = receiveResponse();

        // Send PASS command
        sendRequest("PASS", passwd);
        resp = receiveResponse();
        cout << resp.message;

        if (resp.code != 230) {
            cout << "Re-enter User Name: ";
            cin >> user;
            cout << "Re-enter Password: ";
            passwd = getPassword();
            delete control_socket;
            start();
        }
    } catch (SocketException& e) {
        cout << "Exception occurred: " << e.description() << endl;
        return;
    }
}

void FTPClient::communicate() {
    string command, cmd;
    vector<string> flags, args;

    while (true) {
        flags.clear();
        args.clear();

        cout << "Cp.FTP > ";
        getline(cin, command);

        if (!parseCommand(command, cmd, flags, args)) {
            continue;
        }

        // Server-side commands
        if (cmd == "get" && args.size() >= 1 && args.size() <= 2 && flags.size() == 0) {
            string curr_server = pwd(false);
            string curr_local = _pwd(false);

            // Handle destination directory
            if (args.size() == 2) {
                if (_cd(args[1], false) != 1) {
                    cout << "Destination doesn't exist. File Transfer couldn't be done." << endl;
                    continue;
                }
            }

            // Handle source file path
            string filePath = getFilePath(args[0]);
            if (!filePath.empty()) {
                if (cd(filePath, false) != 250) {
                    _cd(curr_local, false);
                    cout << "Source doesn't exist. File Transfer couldn't be done." << endl;
                    continue;
                }
            }

            get(getFileName(args[0]));
            cd(curr_server, false);
            _cd(curr_local, false);
        }
        else if (cmd == "put" && args.size() >= 1 && args.size() <= 2 && flags.size() == 0) {
            string curr_server = pwd(false);

            // Handle destination directory
            if (args.size() == 2) {
                if (cd(args[1], false) != 250) {
                    cout << "Destination doesn't exist. File Transfer couldn't be done." << endl;
                    continue;
                }
            }

            put(args[0]);
            cd(curr_server, false);
        }
        else if (cmd == "pwd" && args.size() == 0 && flags.size() == 0) {
            pwd();
        }
        else if (cmd == "cd" && flags.size() == 0 && args.size() == 1) {
            cd(args[0]);
        }
        else if (cmd == "ls") {
            if (pasv() != 227) {
                cout << "Couldn't get file listing." << endl;
                continue;
            }
            ls(flags, args);
        }
        else if (cmd == "mkdir" && args.size() == 1 && flags.size() == 0) {
            bool success = true;
            string curr_loc = pwd(false);
            vector<string> dirs = tokenize(args[0], "/");

            for (int i = 0; i < dirs.size(); i++) {
                if (mkd(dirs[i], false) != 257 && cd(dirs[i], false) != 250) {
                    cout << "Couldn't create the required directory structure." << endl;
                    success = false;
                    break;
                }
            }

            cd(curr_loc, false);
            if (success) {
                cout << "Directory structure " << args[0] << " successfully created." << endl;
            }
        }
        // Client-side commands (prefixed with !)
        else if (cmd == "!pwd" && args.size() == 0 && flags.size() == 0) {
            _pwd();
        }
        else if (cmd == "!cd" && flags.size() == 0 && args.size() == 1) {
            _cd(args[0]);
        }
        else if (cmd == "!ls") {
            _ls(flags, args);
        }
        else if (cmd == "!mkdir" && args.size() == 1 && flags.size() == 0) {
            bool success = true;
            string curr_loc = _pwd(false);
            vector<string> dirs = tokenize(args[0], "/");

            for (int i = 0; i < dirs.size(); i++) {
                if (_mkd(dirs[i], false) != 1 && _cd(dirs[i], false) != 1) {
                    cout << "Couldn't create the required directory structure." << endl;
                    success = false;
                    break;
                }
            }

            _cd(curr_loc, false);
            if (success) {
                cout << "Directory structure " << args[0] << " successfully created." << endl;
            }
        }
        else if (cmd == "quit") {
            if (quit()) {
                control_socket->close();
                return;
            } else {
                cout << "Couldn't terminate the session." << endl;
            }
        }
        else if (cmd == "help") {
            help();
        }
        else {
            cout << "Command improperly formatted. Type \"help\" for reference." << endl;
        }
    }
}

void FTPClient::get(const string& filename) {
    ofstream out(filename.c_str(), ios::out | ios::binary);
    if (!out) {
        cout << "Couldn't create local file." << endl;
        return;
    }

    // Switch to binary mode
    sendRequest("TYPE", "I");
    FTPProtocol::Response resp = receiveResponse();
    cout << resp.message;
    if (resp.code != 200) {
        return;
    }

    // Enter passive mode
    if (pasv() != 227) {
        cout << "File Transfer couldn't be initiated." << endl;
        return;
    }

    // Request file
    sendRequest("RETR", filename);
    resp = receiveResponse();
    cout << resp.message;
    if (resp.code != 150) {
        return;
    }

    cout << "Receiving File: " << filename << " ...." << endl;

    // Receive file data
    double length = 0;
    while (true) {
        string data = data_socket->receive();
        if (data.length() == 0) {
            break;
        }
        out << data;
        length += data.length();
    }

    data_socket->close();
    delete data_socket;
    data_socket = NULL;

    resp = receiveResponse();
    out.close();
    cout << resp.message;

    // Print file size
    cout << "File Size: ";
    if (length < 1024) {
        cout << length << " bytes" << endl;
    } else if (length < 1024 * 1024) {
        cout << (length / 1024) << " KB" << endl;
    } else if (length < 1024 * 1024 * 1024) {
        cout << (length / (1024 * 1024)) << " MB" << endl;
    } else {
        cout << (length / (1024 * 1024 * 1024)) << " GB" << endl;
    }
}

void FTPClient::put(const string& filename) {
    ifstream in(filename.c_str(), ios::in | ios::binary | ios::ate);
    if (!in) {
        cout << "Couldn't open local file." << endl;
        return;
    }

    long length = in.tellg();
    in.seekg(0, in.beg);

    // Switch to binary mode
    sendRequest("TYPE", "I");
    FTPProtocol::Response resp = receiveResponse();
    cout << resp.message;
    if (resp.code != 200) {
        return;
    }

    // Enter passive mode
    if (pasv() != 227) {
        cout << "File Transfer couldn't be initiated." << endl;
        return;
    }

    // Send STOR command
    sendRequest("STOR", getFileName(filename));
    resp = receiveResponse();
    cout << resp.message;
    if (resp.code != 150) {
        return;
    }

    cout << "Sending File: " << filename << " ...." << endl;

    // Send file data
    while (length > 0) {
        int read_sz = MAXRECV < length ? MAXRECV : length;
        char buf[MAXRECV + 1];
        in.read(buf, read_sz);
        string data(buf, read_sz);
        *data_socket << data;
        length -= read_sz;
    }

    in.close();
    data_socket->close();
    delete data_socket;
    data_socket = NULL;

    resp = receiveResponse();
    cout << resp.message;
}

string FTPClient::pwd(bool print) {
    sendRequest("PWD");
    FTPProtocol::Response resp = receiveResponse();

    if (print) {
        cout << resp.message;
    }

    // Extract directory from quotes
    size_t start = resp.message.find("\"");
    size_t end = resp.message.find("\"", start + 1);
    if (start != string::npos && end != string::npos) {
        return resp.message.substr(start + 1, end - start - 1);
    }

    return "";
}

int FTPClient::cd(const string& path, bool print) {
    sendRequest("CWD", path);
    FTPProtocol::Response resp = receiveResponse();

    if (print) {
        cout << resp.message;
    }

    return resp.code;
}

void FTPClient::ls(const vector<string>& flags, const vector<string>& args) {
    string arg_str;
    for (const auto& flag : flags) {
        arg_str += flag + " ";
    }
    for (const auto& arg : args) {
        arg_str += arg + " ";
    }

    sendRequest("LIST", arg_str);
    FTPProtocol::Response resp = receiveResponse();
    cout << resp.message;

    if (resp.code == 150) {
        // Receive directory listing
        while (true) {
            string data = data_socket->receive();
            if (data.length() == 0) {
                break;
            }
            cout << data;
        }

        data_socket->close();
        delete data_socket;
        data_socket = NULL;

        resp = receiveResponse();
        cout << resp.message;
    }
}

int FTPClient::mkd(const string& dirname, bool print) {
    sendRequest("MKD", dirname);
    FTPProtocol::Response resp = receiveResponse();

    if (print) {
        cout << resp.message;
    }

    return resp.code;
}

int FTPClient::pasv() {
    sendRequest("PASV");
    FTPProtocol::Response resp = receiveResponse();

    if (resp.code == 227) {
        int port = FTPProtocol::extractPort(resp.message);
        if (port != -1) {
            try {
                data_socket = new Socket(host, port);
                return 227;
            } catch (SocketException& e) {
                cout << "Exception occurred: " << e.description() << endl;
                return -1;
            }
        }
    }

    return resp.code;
}

bool FTPClient::quit() {
    sendRequest("QUIT");
    FTPProtocol::Response resp = receiveResponse();
    cout << resp.message;
    return resp.code == 221;
}

string FTPClient::_pwd(bool print) {
    int code;
    string response = exec_cmd("pwd", "", code);
    if (print) {
        cout << response;
    }
    // Remove quotes and newline
    if (response.length() >= 3) {
        return response.substr(1, response.length() - 3);
    }
    return response;
}

int FTPClient::_cd(const string& path, bool print) {
    int code;
    string response = exec_cmd("cd", path, code);
    if (print) {
        cout << response;
    }
    return code;
}

void FTPClient::_ls(const vector<string>& flags, const vector<string>& args) {
    string cmd = "ls";
    for (const auto& flag : flags) {
        cmd += " " + flag;
    }
    for (const auto& arg : args) {
        cmd += " " + arg;
    }

    int code;
    string response = exec_cmd("ls", cmd, code);
    cout << response;
}

int FTPClient::_mkd(const string& dirname, bool print) {
    int code;
    string response = exec_cmd("mkdir", dirname, code);
    if (print) {
        cout << response;
    }
    return code;
}

void FTPClient::help() {
    cout << "\n=== Cp.FTP Client Help ===" << endl;
    cout << "\nServer Commands:" << endl;
    cout << "  get <file> [dest]    - Download file from server" << endl;
    cout << "  put <file> [dest]    - Upload file to server" << endl;
    cout << "  ls [flags] [path]    - List server directory" << endl;
    cout << "  cd <path>            - Change server directory" << endl;
    cout << "  pwd                  - Show server working directory" << endl;
    cout << "  mkdir <path>         - Create server directory" << endl;
    cout << "\nClient Commands (prefix with !):" << endl;
    cout << "  !ls [flags] [path]   - List local directory" << endl;
    cout << "  !cd <path>           - Change local directory" << endl;
    cout << "  !pwd                 - Show local working directory" << endl;
    cout << "  !mkdir <path>        - Create local directory" << endl;
    cout << "\nOther Commands:" << endl;
    cout << "  help                 - Show this help message" << endl;
    cout << "  quit                 - Exit FTP client" << endl;
    cout << endl;
}

void FTPClient::sendRequest(const string& cmd, const string& args) {
    string request = FTPProtocol::formatRequest(cmd, args);
    *control_socket << request;
}

FTPProtocol::Response FTPClient::receiveResponse() {
    string data = control_socket->receive();
    return FTPProtocol::parseResponse(data);
}
