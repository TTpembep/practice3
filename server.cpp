#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> //sleep func
#include <future>
#include <mutex>
#include <thread>
#include <arpa/inet.h>
#include "ip.h"
#include "dbms/DBinit.h"
#include "dbms/structures.h"
#include "config.h"
#include "market.h"
#include <map>

using namespace std;

Schema schema;  //Глобальная структура для обработки бд
Config config; //Глобальная конфигурация сервера
mutex userMutex;  //Глобальный мьютекс для защиты доступа к schema

map<int, string> clientPorts;  // Хранилище для идентификации клиентов по портам

void requestProcessing(const int clientSocket, const sockaddr_in& clientAddress) {
    char receive[1024] = {};
    string sending;
    bool isExit = false;
    int clientPort = ntohs(clientAddress.sin_port);
    string clientId = inet_ntoa(clientAddress.sin_addr) + ':' + to_string(clientPort);
    clientPorts[clientSocket] = clientId;

    while (!isExit) {
        bzero(receive, 1024);
        const ssize_t userRead = read(clientSocket, receive, 1024);
        if (userRead <= 0) {
            cerr << "client[" << clientId << "] disconnected\n";
            isExit = true;
            continue;
        }
        if (strcmp(receive, "disconnect") == 0) {
            isExit = true;
            continue;
        }
        lock_guard<mutex> guard(userMutex);
        string result = market(receive, schema);
        send(clientSocket, result.c_str(), result.size(), 0);
    }
    close(clientSocket);
    clientPorts.erase(clientSocket);
}

void startServer() {
    const int server = socket(AF_INET, SOCK_STREAM, 0); // file descriptor
    if (server == -1) {
        cerr << "Socket creation error" << endl;
        return;
    }
    sockaddr_in address{}; // IPV4 protocol structure
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(IP); // any = 0.0.0.0
    address.sin_addr.s_addr = inet_addr(config.ip.c_str());
    address.sin_port = htons(config.port); // host to net short

    if (bind(server, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
        cerr << "Binding error" << endl;
        close(server);
        return;
    }
    if (listen(server, 10) == -1) {
        cerr << "Socket listening error" << endl;
        close(server);
        return;
    }
    cout << "Server started" << endl;

    sockaddr_in clientAddress{};
    socklen_t clientAddrLen = sizeof(clientAddress);
    while (true) {
        int clientSocket = accept(server, reinterpret_cast<struct sockaddr *>(&clientAddress), &clientAddrLen);
        if (clientSocket == -1) {
            cout << "connection fail" << endl;
            continue;
        }
        cout << "Client[" << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << "] was connected" << endl;
        thread(requestProcessing, clientSocket, clientAddress).detach();
    }
    close(server);
}

int main() {
    dbInit(schema);   // Функция создания и проверки наличия БД
    marketCfg(schema, config);
    cout << "Market is ready. \n";
    startServer();
    return 0;
}
/*
g++ client.cpp -o client
g++ server.cpp dbms/DBinit.cpp config.cpp market.cpp dbms/dbms.cpp  dbms/syntaxCheck.cpp dbms/actions.cpp -o server

*/