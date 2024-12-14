#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>
#include <cstdlib> // Для использования system
#include <sstream>

using namespace std;
using namespace httplib;

int main() {
    system("clear");    // Очистка терминала
    cout << "-= TTpembep`s Market Client =- \n";
    while (true){
        cout << ">Waiting for request.. \n>";
        string request;
        getline (cin, request);
        //cout << ">DEBUG:   " << request << "\n";

        string action, path;
        stringstream ss(request);
        getline(ss, action,  ' ');
        getline(ss, path);
        if (action == "POST" && path == "/user"){
            cout << "{\n\t\"username\": ";
            string username;
            getline(cin, username);
            cout << "}\n";
            //cout << ">DEBUG:   " << username << "\n";

            cout << ">Response\n{\n\t\"key\":";
            Client cli("127.0.0.1", 7432);
            auto res = cli.Post(path, username, "application/x-www-form-urlencoded");
            if (res) {
                //cout << ">Status: " << res->status << "\n";
                //cout << ">Body: " << res->body << "\n";
                cout << res->body << "\n";
            } else {
                cout << ">ERROR: Request failed. \n";
            }
            cout << "}\n";
        }else if (action == "EXIT"){
            cout << ">Bye, bye! <3\n";
            return 0;
        }else {cout << ">Unknown command. \n";}

    }
    return 0;
}