#include "utility.h"
#include "../net/socket_exception.h"

using namespace std;

// Check xem có phải số không
bool is_number(string s) {
    for (int i = 0; i < s.length(); i++)
        if (!isdigit(s[i]))
            return false;
    return true;
}

// Validate IP address format (xxx.xxx.xxx.xxx)
bool validateIPAddress(const string& ip) {
    return count(ip.begin(), ip.end(), '.') == 3;
}

// Validate port number (1-65535)
bool validatePort(const string& port_str) {
    if (!is_number(port_str)) {
        return false;
    }
    int port = atoi(port_str.c_str());
    return port > 0 && port < 65536;
}

// Hàm đọc vào password người dùng nhập, có cơ chế thiết lập cấu hình cho terminal để gõ không hiện lên màn hình --> bảo mật
string getPassword() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    string pass;
    getline(cin, pass);
    getline(cin, pass);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;
    return pass;
}

// Lấy ra tên file từ vị trí cuối cùng trong đường dẫn path
string getFileName(string str) {
    string::size_type pos = str.find_last_of("/\\");
    return str.substr(pos + 1);
}

// Lấy source path của filename không chứa filename
string getFilePath(string str) {
    string::size_type pos = str.find_last_of("/\\");
    if (pos == string::npos) {
        return "";
    }
    return str.substr(0, pos);
}

// Xử lý từng lại cmd như cd, pwd, ls,...
string exec_cmd(string cmd_type, string cmd, int& code) {
    // in dùng để lưu lại output của lệnh chạy trực tiếp bằng terminal
    FILE *in;

    char buff[2048];

    // Tác dụng của stringstream data: gom toàn bộ nội dung output/thông báo trước khi trả về output cuối cùng
    stringstream data;
    
    // code = 1 ~ thành công, code = 0 ~ thất bại
    code = 0;

    // Sử dụng hàm getcwd để lấy ra thư mục làm việc hiện tại
    if (cmd_type == "pwd") {
        if (getcwd(buff, sizeof(buff)) != NULL) {
            code = 1;
            data << "\"" << buff << "\"" << endl; // data nhận vào "path". Ví dụ: "/mnt/c/User"
        } else {
            data << "\"Error : " << strerror(errno) << "\"" << endl; // data nhận vào "Error: ..."
        }
    }
    // Sử dụng hàm chdir để thay đổi thư mục làm việc hiện tại
    else if (cmd_type == "cd") {
        if (chdir(cmd.c_str()) == 0) { // cmd chứa thư mục làm việc target, .c_str() để lấy ra tên dạng string
            code = 1;
            data << "Succesfully changed to directory : " << cmd << "." << endl;
        } else {
            data << "Error : " << strerror(errno) << endl;
        }
    }
    // Sử dụng hàm mkdir để tạo thư mục mới
    else if (cmd_type == "mkdir") {
        if (mkdir(cmd.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) {
            code = 1;
            data << "Succesfully created directory : \"" << cmd << "\"." << endl;
        } else {
            data << "Error : " << strerror(errno) << endl;
        }
    }
    // Đối với các lệnh khác (bao gồm cả lệnh ls), sử dụng hàm popen để ngầm chạy câu lệnh trực tiếp trên terminal, lấy ra output và in ra màn hình
    else {
        if (!(in = popen(cmd.c_str(), "r"))) {
            data << "Couldn't execute the command : " << cmd << endl;
        } else {
            code = 1;
            if (cmd_type == "ls") {
                char *line = NULL;
                size_t len = 0;
                getline(&line, &len, in);
                if (string(line).find("total") == string::npos) {
                    code = 0;
                }
            }
            while (fgets(buff, sizeof(buff), in) != NULL) {
                data << buff;
            }
            pclose(in);
        }
    }

    return data.str();
}

// Hàm này để phân giải tên path theo sep, ví dụ nhận vào /usr/mnt/lib với sep là "/" --> output: <"usr", "mnt", "lib">
vector<string> tokenize(string s, string sep) {
    // Bỏ qua sep đầu tiên
    string::size_type lastPos = s.find_first_not_of(sep, 0);
    // Tìm ký tự đầu tiên không phải sep
    string::size_type pos = s.find_first_of(sep, lastPos);
    vector<string> tokens;
    while (pos != string::npos || lastPos != string::npos) {
        tokens.push_back(s.substr(lastPos, (pos - lastPos)));

        lastPos = s.find_first_not_of(sep, pos);

        pos = s.find_first_of(sep, lastPos);
    }
    return tokens;
}

// Hàm parse command cho FTP Client, phân tách command name (cmd), flags và arguments (args)
bool parseCommand(string command, string& cmd, vector<string>& flags, vector<string>& args) {
    string::size_type beginPos = command.find_first_not_of(" \r\n", 0);
    string::size_type endPos = command.find_first_of(" \r\n", beginPos);
    cmd = command.substr(beginPos, endPos - beginPos);
    beginPos = endPos;

    // split command into arugments, flags and opcode.
    while (beginPos < command.length()) {
        beginPos = command.find_first_not_of(" \r\n", endPos);
        if (beginPos != string::npos) {
            if (command[beginPos] == '\"') {
                endPos = command.find_first_of('\"', beginPos + 1);
                if (endPos == string::npos) {
                    cout << "Error : Missing \" at the end." << command.substr(beginPos) << endl;
                    return false;
                }
                args.push_back(command.substr(beginPos + 1, endPos - beginPos - 1));
                endPos = endPos + 1;
            } else if (command[beginPos] == '-') {
                endPos = command.find_first_of(" \r\n", beginPos);
                if (endPos == string::npos)
                    endPos = command.length();
                flags.push_back(command.substr(beginPos, endPos - beginPos));
            } else {
                endPos = command.find_first_of(" \r\n", beginPos);
                while (command[endPos - 1] == '\\' && string::npos != endPos) {
                    endPos = command.find_first_of(" \r\n", endPos + 1);
                }
                if (endPos == string::npos) {
                    endPos = command.length();
                }
                args.push_back(command.substr(beginPos, endPos - beginPos));
            }
        }
    }
    return true;
}

// Hàm parse command cho FTP Server, phân tách command name (cmd) và arguments (args)
bool parseCommand(string command, string& cmd, string& args) {
    string::size_type beginPos = command.find_first_not_of(" \r\n", 0);
    string::size_type endPos = command.find_first_of(" \r\n", beginPos);
    cmd = command.substr(beginPos, endPos - beginPos);
    beginPos = command.find_first_not_of(" \r\n", endPos);
    if (beginPos != string::npos) {
        endPos = command.find_first_of("\r\n", beginPos);
        if (endPos == string::npos) {
            return false;
        }
        args = command.substr(beginPos, endPos - beginPos);
    }
    return true;
}

// Lấy thời gian hiện tại với format hh:ss:mm
string getCurrentTime() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return string(buffer);
}

// Lấy thông tin đăng nhập từ user (username và password)
void getUserCredentials(string& user, string& pass) {
    cout << "Enter User Name: ";
    cin >> user;
    cout << "Enter Password: ";
    pass = getPassword();
}
