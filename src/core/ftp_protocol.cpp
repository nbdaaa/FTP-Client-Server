#include "ftp_protocol.h"
#include <sstream>
#include <cctype>
#include <cstdlib>

using namespace std;

namespace FTPProtocol {

// Format request trong trường hợp không có flags và có duy nhất 1 tham số. VD: USER john, PASS secret, QUIT
string formatRequest(const string& cmd, const string& args) {
    if (args.empty()) {
        return cmd + "\r\n";
    }
    return cmd + " " + args + "\r\n";
}

// Format request trong trường hợp có list flags và list các args. VD: LIST -la /home/user --> formatRequest("LIST", {"-la"}, {"/home/user"})
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


// Hàm parseResponse(raw) nhận vào chuỗi data "raw" và tách ra thành: 
// resp.code: mã response, resp.message: nội dung của mã
Response parseResponse(const string& raw) {
    Response resp;

    // Tìm ký tự đầu tiên khác " "
    size_t beginPos = raw.find_first_not_of(" ");
    if (beginPos == string::npos) { // Nếu cả chuỗi toàn dấu cách --> trả về npos (ko có dữ liệu), reps mặc định
        return resp; 
    }

    // Kiểm tra ký tự đầu tiên sau khi bỏ space có phải số không
    if (isdigit(raw[beginPos])) { // Nếu là số
        size_t endPos = raw.find(" ", beginPos); // Tìm dấu cách tiếp theo
        
        // Nếu tìm thấy dấu cách tiếp, cắt đoạn từ beginPos đến trước dấu cách, atoi() chuyển về thành số và gán vào resp.code
        if (endPos != string::npos) { 
            string codeStr = raw.substr(beginPos, endPos - beginPos);
            resp.code = atoi(codeStr.c_str());
            // Lấy nốt message đằng sau status code
            beginPos = raw.find_first_not_of(" ", endPos);
            if (beginPos != string::npos) {
                resp.message = raw.substr(beginPos);
            }
        }
    } else { // Nếu không phải số, đồng nghĩa với không có status code --> resp.message được gán từ beginPos đến hết
        resp.message = raw.substr(beginPos);
    }

    return resp;
}

// Format response trả về của server
string formatResponse(int code, const string& msg) {
    stringstream ss;
    ss << code << " " << msg << "\r\n";
    return ss.str();
}

// Hàm dùng để trích xuất số port từ response của lệnh PASV trong FTP protocol. VD: "227 Entering Passive Mode (192,168,1,100,195,10)" --> 49930 = 256*195+10
int extractPort(const string& pasvResponse) {
    // Tìm ngoặc ()
    size_t beginPos = pasvResponse.find("(");
    size_t endPos = pasvResponse.find(")", beginPos);

    if (beginPos == string::npos || endPos == string::npos) {
        return -1;
    }

    string portInfo = pasvResponse.substr(beginPos + 1, endPos - beginPos - 1);

    // Parse h1,h2,h3,h4,p1,p2
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

// // Hàm phục vụ cho việc implement Active Mode. Hiện tại chưa cần đến
// string formatPortCommand(const string& host, int port) {
//     stringstream ss;

//     for (char c : host) {
//         if (c == '.') {
//             ss << ',';
//         } else {
//             ss << c;
//         }
//     }

//     ss << ',' << (port / 256) << ',' << (port % 256);

//     return ss.str();
// }

} // namespace FTPProtocol
