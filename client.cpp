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
        getline(ss, path, '?');
        // POST /user?username=john1&smth=value
        // POST /order?x-user-key=string&pair_id=int&quantity=float&price=float&type=sell
        if (action == "POST" && (path == "/user" || path == "/order")){ 
            string requestBody;
            getline(ss, requestBody);

            Client cli("127.0.0.1", 7432);
            auto res = cli.Post(path, requestBody, "application/x-www-form-urlencoded");
            if (res) {
                cout << res->body;
            }else {
                cout << ">ERROR: Request failed. \n";
            }
        }else if(action == "GET" && (path == "/order" || path == "/lot" || path == "/pair")){ // GET /balance - ???
            Client cli("127.0.0.1", 7432);
            auto res = cli.Get(path);
            if (res) {
                cout << res->body;
            }else {
                cout << ">ERROR: Request failed. \n";
            }
        }else if (action == "EXIT"){    // +DELETE /order - ???
            cout << ">Bye, bye! <3\n";
            return 0;
        }else {cout << ">Unknown command. \n";}
    }
    return 0;
}