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

// Hostname lookup
int hostlookup(string host);

// String validation
bool is_number(string s);

// Password input (hidden)
string getPassword();

// File path utilities
string getFileName(string path);
string getFilePath(string path);

// System command execution
string exec_cmd(string cmd_type, string cmd, int& code);

// String tokenization
vector<string> tokenize(string s, string sep);

// String utilities
string replaceAllOccurences(string str, string search, const string replace);

// Command parsing
bool parseCommand(string command, string& cmd, string& args);
bool parseCommand(string command, string& cmd, vector<string>& flags, vector<string>& args);

// Get current time in hh:mm:ss format
string getCurrentTime();

#endif
