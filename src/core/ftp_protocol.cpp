#include "ftp_protocol.h"
#include <sstream>
#include <cctype>
#include <cstdlib>

using namespace std;

namespace FTPProtocol {

string formatRequest(const string& cmd, const string& args) {
    if (args.empty()) {
        return cmd + "\r\n";
    }
    return cmd + " " + args + "\r\n";
}

string formatRequest(const string& cmd,
                    const vector<string>& flags,
                    const vector<string>& args) {
    string request = cmd;

    for (const auto& flag : flags) {
        request += " " + flag;
    }

    for (const auto& arg : args) {
        request += " " + arg;
    }

    return request + "\r\n";
}

Response parseResponse(const string& raw) {
    Response resp;

    // Find first non-space character
    size_t beginPos = raw.find_first_not_of(" ");
    if (beginPos == string::npos) {
        return resp;
    }

    // Check if response starts with a digit (status code)
    if (isdigit(raw[beginPos])) {
        size_t endPos = raw.find(" ", beginPos);
        if (endPos != string::npos) {
            string codeStr = raw.substr(beginPos, endPos - beginPos);
            resp.code = atoi(codeStr.c_str());
            beginPos = raw.find_first_not_of(" ", endPos);
            if (beginPos != string::npos) {
                resp.message = raw.substr(beginPos);
            }
        }
    } else {
        resp.message = raw.substr(beginPos);
    }

    return resp;
}

string formatResponse(int code, const string& msg) {
    stringstream ss;
    ss << code << " " << msg << "\r\n";
    return ss.str();
}

int extractPort(const string& pasvResponse) {
    // Find opening and closing parentheses
    size_t beginPos = pasvResponse.find("(");
    size_t endPos = pasvResponse.find(")", beginPos);

    if (beginPos == string::npos || endPos == string::npos) {
        return -1;
    }

    string portInfo = pasvResponse.substr(beginPos + 1, endPos - beginPos - 1);

    // Parse h1,h2,h3,h4,p1,p2 format
    int count = 0;
    size_t pos = 0;
    string p1Str, p2Str;

    while (count < 6 && pos < portInfo.length()) {
        size_t commaPos = portInfo.find(",", pos);

        if (count == 4) {
            // Extract p1
            if (commaPos != string::npos) {
                p1Str = portInfo.substr(pos, commaPos - pos);
            }
        } else if (count == 5) {
            // Extract p2
            p2Str = portInfo.substr(pos);
        }

        if (commaPos == string::npos) break;
        pos = commaPos + 1;
        count++;
    }

    if (!p1Str.empty() && !p2Str.empty()) {
        int p1 = atoi(p1Str.c_str());
        int p2 = atoi(p2Str.c_str());
        return p1 * 256 + p2;
    }

    return -1;
}

string formatPortCommand(const string& host, int port) {
    stringstream ss;

    // Replace dots with commas in IP address
    for (char c : host) {
        if (c == '.') {
            ss << ',';
        } else {
            ss << c;
        }
    }

    // Add port as p1,p2 where port = p1*256 + p2
    ss << ',' << (port / 256) << ',' << (port % 256);

    return ss.str();
}

} // namespace FTPProtocol
