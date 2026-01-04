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

        // Validate IP address format and port
        if (count(host.begin(), host.end(), '.') == 3 && is_number(argv[2]) && atoi(argv[2]) < 65536) {
            int port = atoi(argv[2]);
            string user, pass;

            cout << "Enter User Name: ";
            cin >> user;
            cout << "Enter Password: ";
            pass = getPassword();

            FTPClient client(host, port, user, pass);
            client.start();
            client.communicate();
        } else {
            cout << "Input incorrectly formatted." << endl;
            cout << argv[0] << " [int].[int].[int].[int] [int(1-65536)]" << endl;
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
