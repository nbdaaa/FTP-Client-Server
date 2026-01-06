#ifndef UTILITY_H
#define UTILITY_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>

using namespace std;

// Kiểm tra số hợp lệ
bool is_number(string s);

// Lấy password ở dạng gõ ẩn (không hiện lên màn hình)
string getPassword();

// Lấy tên file và lấy tên source
string getFileName(string path);
string getFilePath(string path);

// Thực thi theo từng cmd
string exec_cmd(string cmd_type, string cmd, int& code);

// Phân tách tên path theo sep
vector<string> tokenize(string s, string sep);

// Command parsing
bool parseCommand(string command, string& cmd, string& args); // Server 
bool parseCommand(string command, string& cmd, vector<string>& flags, vector<string>& args); // Client

// Lấy thời gian hiện tại với format hh:ss:mm
string getCurrentTime();

#endif
