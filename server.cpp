#include "httplib.h"
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

void startServer() {    // Функция для запуска сервера
    Server svr; // HTTP-server

    // Обработка GET-запроса на маршруте "/hi"
    svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    // Обработка POST-запроса на маршруте "/user"
    svr.Post("/user", [](const Request& req, Response& res){    // Создание нового пользователя
        // Вывод какой клиент совершает действие
        cout << ">Request [" << req.remote_port << "]; " << "User:"<< req.body <<"\n";

        string result;
        if (!isUserExists(req.body, schema)){
            result = keyGen(req.body, schema);  
        }else {
            result = "ERROR-user already exists";
        }
        res.set_content(result, "text/plain");
    });

    svr.Get("/lot", [](const Request& req, Response& res){
        cout << ">Request [" << req.remote_port << "]; /lots\n";
        string result;

        ifstream inFile(schema.name+"/lot/1.csv");
        string row;
        getline(inFile, row);
        while (getline(inFile, row)){
            stringstream ss(row);
            string temp;
            getline(ss, temp, ',');
            result += "\t{\n\t\t\"lot_id\": " + temp + ",\n";
            getline(ss,temp);
            result += "\t\t\"name\": " + temp + "\n\t},\n";
        }
        inFile.close();

        res.set_content(result, "text/plain");
    });

    // Запуск сервера на указанном порту
    cout << "-= Starting server on http://" << config.ip << ":" << config.port << " =-\n";
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