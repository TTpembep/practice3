#include "httplib.h"
#include <mutex>
#include <thread>
#include <map>
#include "dbms/DBinit.h"
#include "dbms/structures.h"
#include "config.h"
#include "market.h"

using namespace std;
using namespace httplib;

Schema schema;  //Глобальная структура для обработки бд
Config config; //Глобальная конфигурация сервера
mutex userMutex;  //Глобальный мьютекс для защиты доступа к schema

map<int, string> clientPorts;  // Хранилище для идентификации клиентов по портам

void requestProcessing(const Request& req, Response& res) {
    string clientId = req.remote_addr + ":" + to_string(req.remote_port);
    clientPorts[req.remote_port] = clientId;
    cout << "Client [" << req.remote_port << "] visited server. \n";

    // Вывод какой user совершает действие, или он выполняет это как гость

    lock_guard<mutex> guard(userMutex);
    string result = "Processed request: " + req.body;  // Замените на вашу логику обработки
    res.set_content(result, "text/plain");
}

// Функция для запуска сервера
void startServer() {
    Server svr; // HTTP-server

    // Обработка GET-запроса на маршруте "/hi"
    svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // Обработка POST-запроса на маршруте "/user"
    svr.Post("/user", requestProcessing);

    // Запуск сервера на указанном порту
    cout << "Starting server on http://" << config.ip << ":" << config.port << endl;
    svr.listen(config.ip.c_str(), config.port);
}

int main() {
    // Инициализация БД и конфигурации
    dbInit(schema);   // Функция создания и проверки наличия БД
    marketCfg(schema, config);
    cout << "Market is ready.\n";

    // Запуск сервера в отдельном потоке
    thread serverThread(startServer);
    serverThread.join();

    return 0;
}
/*
g++ -o client client.cpp -lssl -lcrypto
g++ -o server server.cpp dbms/DBinit.cpp config.cpp market.cpp dbms/dbms.cpp  dbms/syntaxCheck.cpp dbms/actions.cpp

*/