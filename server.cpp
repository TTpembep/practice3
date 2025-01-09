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
        res.set_content("Hello World!\n", "text/plain");
    });

    // Обработка POST-запроса на маршруте "/user"
    svr.Post("/user", [](const Request& req, Response& res){    // Создание нового пользователя
        // Вывод идентификатора запроса и его содержания
        cout << ">Recieved POST /user request [" << req.remote_port << "]; " << req.body <<"\n";

        string result = "Response\n{\n";
        for (auto p : req.params) {
            if (p.first == "username"){
                if(!isUserExists(p.second, schema)){
                    result += "\t\"key\": \"" + keyGen(p.second, schema) + "\"\n";
                }else {
                    result += "\tERROR: User already exists.\n";
                    cout << "ERROR: User already exists.\n" ;
                }
            }else{
                result += "\tERROR: Wrong syntax.\n";
                cout << "ERROR: Wrong syntax.\n";
                break;
            }
        }
        result += "}\n";
        res.set_content(result, "text/plain");
    });

    svr.Post("/order", [](const Request& req, Response& res){
        // Вывод идентификатора запроса и его содержания
        cout << ">Recieved POST /order request [" << req.remote_port << "]; " << req.body <<"\n";
        // POST /order?x-user-key=string&pair_id=int&quantity=float&price=float&type=sell
        string user_id, pair_id, type;
        float quantity, price;

        string result = "Response\n{\n";
        string syntaxError = "";

        for (auto p : req.params) {
            if (p.first == "x-user-key"){
                user_id = idFinder("user", "key", p.second, schema);
                if (user_id != "false"){
                    continue;
                }else{
                    result += "\tERROR: Wrong x-user-key.\n";
                    syntaxError = "ERROR: Wrong x-user-key.\n";
                    break;
                }
            }else if(p.first == "pair_id"){
                pair_id = idFinder("pair", "pair_id", p.second, schema);
                if (user_id != "false"){
                    continue;
                }else{
                    result += "\tERROR: Wrong pair_id.\n";
                    syntaxError = "ERROR: Wrong pair_id.\n";
                    break;
                }
            }else if(p.first == "quantity"){
                quantity = convertToFloat(p.second);
                if (quantity != 0.0f || p.second == "0" || p.second == "0.0") {
                    continue;
                }else{
                    result += "\tERROR: Invalid quantity variable type.\n";
                    syntaxError = "ERROR: Invalid quantity variable type.\n";
                    break;
                }
            }else if(p.first == "price"){
                price = convertToFloat(p.second);
                if (price != 0.0f || p.second == "0" || p.second == "0.0") {
                    continue;
                }else{
                    result += "\tERROR: Invalid price variable type.\n";
                    syntaxError = "ERROR: Invalid price variable type.\n";
                    break;
                }
            }else if(p.first == "type"){
                type = p.second;
                if (type == "sell" || type == "buy"){
                    continue;
                }
                else{
                    result += "\tERROR: Invalid order type.\n";
                    syntaxError = "ERROR: Invalid order type.\n";
                    break;
                }
            }else{
                result += "\tERROR: Wrong syntax.\n";
                syntaxError = "ERROR: Wrong syntax.\n";
                break;
            }
        }
        if (syntaxError == ""){
            string order_id;
            order_id = createOrder(user_id, pair_id, quantity, price, type, schema);
            result += "\t\"order_id\": " + order_id + "\n";
        }
        result += "}\n";
        res.set_content(result, "text/plain");
    });

    svr.Get("/lot", [](const Request& req, Response& res){
        cout << ">Recieved GET /lot request [" << req.remote_port << "];\n";
        string result = "Response\n[\n";

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
        result += "]\n";
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

http://127.0.0.1:7432/show?john=1&aboba=dir

curl http://127.0.0.1:7432/hi
curl http://127.0.0.1:7432/lot
curl -d 'username=john1' http://127.0.0.1:7432/user
curl -d 'x-user-key=62938913&pair_id=21&quantity=300&price=0.015&type=sell' http://127.0.0.1:7432/order
*/