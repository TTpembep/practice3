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
        getline(ss, path, ' ');
        if (action == "POST" && path == "/user"){   // POST /user username=john1
            string requestBody;
            getline(ss, requestBody);

            Client cli("127.0.0.1", 7432);
            auto res = cli.Post(path, requestBody, "application/x-www-form-urlencoded");
            if (res) {
                cout << res->body << "\n";
            }else {
                cout << ">ERROR: Request failed. \n";
            }

        }else if (action == "POST" && path == "/order"){
            cout << "\nX-USER-KEY: ";
            string request;
            getline(cin, request);
            request += ",";
            cout << "\n{\n";
            cout << "\t\"pair_id\": ";
            string temp;
            getline(cin, temp); // int
            if (temp[temp.length() - 1] != ','){
                cout << ">DEBUG: syntax error\n";
                continue;
            }
            request += temp;
            cout << "\t\"quantity\": ";
            getline(cin, temp); // float
            if (temp[temp.length() - 1] != ','){
                cout << ">DEBUG: syntax error\n";
                continue;
            }
            request += temp;
            cout << "\t\"price\": ";
            getline(cin, temp); // float
            if (temp[temp.length() - 1] != ','){
                cout << ">DEBUG: syntax error\n";
                continue;
            }
            request += temp;
            cout << "\t\"type\": ";
            getline(cin, temp); //"sell" || "buy"
            if (temp != "sell" && temp != "buy"){
                cout << ">DEBUG: syntax error\n";
                continue;
            }
            request += temp;
            cout << "}\n";
            //cout << ">DEBUG:   \n\n" << request << "\n\n";
            cout << ">Response\n{\n\t\"order_id\":";
            Client cli("127.0.0.1", 7432);
            auto res = cli.Post(path, request, "application/x-www-form-urlencoded");
            if (res) {
                cout << res->body << "\n";
            }else {
                cout << ">ERROR: Request failed. \n";
            }
            cout << "}\n";
        }else if(action == "GET" && path == "/lot"){
            cout << ">Response\n[\n";
            Client cli("127.0.0.1", 7432);
            auto res = cli.Get(path);
            if (res) {
                cout << res->body << "\n";
            }else {
                cout << ">ERROR: Request failed. \n";
            }
            cout << "]\n";
        }else if (action == "EXIT"){
            cout << ">Bye, bye! <3\n";
            return 0;
        }else {cout << ">Unknown command. \n";}

    }
    return 0;
}