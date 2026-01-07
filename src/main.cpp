#include "core/ftp_client.h"
#include "core/ftp_server.h"
#include "utils/utility.h"

using namespace std;

int main(int argc, char* argv[]) {
    // Run as FTP server
    if (argc == 2) {
        int port = atoi(argv[1]);
        FTPServer server(port);
        server.start();
    }
    // Run as FTP client
    else if (argc == 3) {
        string host(argv[1]);
        string port_str(argv[2]);

        // Validate IP address format and port
        if (validateIPAddress(host) && validatePort(port_str)) {
            int port = atoi(port_str.c_str());
            string user, pass;

            getUserCredentials(user, pass);

            FTPClient client(host, port, user, pass);
            client.start();
            client.communicate();
        } else {
            cout << "Input incorrectly formatted." << endl;
            cout << argv[0] << " [int].[int].[int].[int] [int(1-65535)]" << endl;
        }
    }
    // Incorrect arguments
    else {
        cout << "Incorrect usage." << endl;
        cout << argv[0] << " [port] : to use as an ftp-server hosted on [port]" << endl;
        cout << argv[0] << " [host] [port] : to connect to ftp-server hosted at host:port" << endl;
    }

    return 0;
}
