#ifndef FTP_PROTOCOL_H
#define FTP_PROTOCOL_H

#include <string>
#include <vector>

using namespace std;

namespace FTPProtocol {
    // Response structure
    struct Response {
        int code;
        string message;

        Response() : code(0) {}
        Response(int c, const string& m) : code(c), message(m) {}
    };

    // Request formatting
    string formatRequest(const string& cmd, const string& args = "");

    string formatRequest(const string& cmd,
                        const vector<string>& flags,
                        const vector<string>& args);

    // Response parsing/formatting
    Response parseResponse(const string& raw);
    string formatResponse(int code, const string& msg);

    // Helper functions
    int extractPort(const string& pasvResponse);
}

#endif
