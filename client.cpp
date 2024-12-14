#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>

using namespace std;
using namespace httplib;

int main() {    //Переписать клиента, чтобы можно было задавать выполняемые команды
    Client cli("127.0.0.1", 7432);
    auto res = cli.Post("/user", "username=john1", "application/x-www-form-urlencoded");
    if (res) {
        cout << "Status: " << res->status << endl;
        cout << "Body: " << res->body << endl;
    } else {
        cout << "Request failed" << endl;
    }

    return 0;
}