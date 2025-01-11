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
        string user_id, pair_id, type, tmpQnt, tmpPrc, syntaxError;
        float quantity, price;

        string result = "Response\n{\n";

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
                tmpQnt = p.second;
                quantity = convertToFloat(tmpQnt);
                if (quantity != 0.0f || p.second == "0" || p.second == "0.0") {
                    continue;
                }else{
                    result += "\tERROR: Invalid quantity variable type.\n";
                    syntaxError = "ERROR: Invalid quantity variable type.\n";
                    break;
                }
            }else if(p.first == "price"){
                tmpPrc = p.second;
                price = convertToFloat(tmpPrc);
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
            if (user_id != "" && pair_id != "" && type != "" && tmpQnt != "" && tmpPrc != ""){
                string order_id;
                order_id = createOrder(user_id, pair_id, quantity, price, type, schema);
                result += "\t\"order_id\": " + order_id + "\n";
            }else{
                result += "\tERROR: Wrong syntax.\n";
                syntaxError = "ERROR: Wrong syntax.\n";
            }
        }else {
            cout << syntaxError << "\n";
        }
        result += "}\n";
        res.set_content(result, "text/plain");
    });
    svr.Get("/order", [](const Request& req, Response& res){
        cout << ">Recieved GET /order request [" << req.remote_port << "];\n";
        string result = "Response\n[\n";

        ifstream inFile(schema.name+"/order/1.csv");
        string row;
        getline(inFile, row);
        while (getline(inFile, row)){
            stringstream ss(row);
            string temp;
            getline(ss, temp, ',');
            result += "\t{\n\t\t\"order_id\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"user_id\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"lot_id\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"quantity\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"price\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"type\": " + temp + ",\n";
            getline(ss,temp);
            result += "\t\t\"closed\": " + temp + "\n\t},\n";
        }
        inFile.close();
        result += "]\n";
        res.set_content(result, "text/plain");
    });
    svr.Delete("/order", [](const Request& req, Response& res){
        cout << ">Recieved DELETE /order request [" << req.remote_port << "];\n";
        string idVerification, order_id, user_id, pair_id, type, closed, syntaxError;
        float quantity, price;
        string result = "Response\n[\n";

        for (auto p : req.params) {
            if (p.first == "x-user-key"){
                idVerification = idFinder("user", "key", p.second, schema);
                if (idVerification != "false"){
                    continue;
                }else{
                    result += "\tERROR: Wrong x-user-key.\n";
                    syntaxError = "ERROR: Wrong x-user-key.\n";
                    break;
                }
            }else if(p.first == "order_id"){
                order_id = p.second;    //Узнаем какой пользователь создал ордер
                user_id = valFinder("order", "user_id", "order_id", order_id, schema);
                if (user_id != "false"){
                    continue;
                }else{
                    result += "\tERROR: Wrong order_id.\n";
                    syntaxError = "ERROR: Wrong order_id.\n";
                    break;
                }
            }else{
                result += "\tERROR: Wrong syntax.\n";
                syntaxError = "ERROR: Wrong syntax.\n";
                break;
            }
        }
        if (syntaxError == ""){
            if (idVerification != "" && order_id != ""){
                closed = valFinder("order", "closed", "order_id", order_id, schema);    
                if (user_id == idVerification && closed == "-"){    //Проверяем что пользователь указан верно
                    //Получаем данные ордера для последующей их обработки
                    pair_id = valFinder("order", "pair_id", "order_id", order_id, schema);
                    quantity = convertToFloat(valFinder("order", "quantity", "order_id", order_id, schema));
                    price = convertToFloat(valFinder("order", "price", "order_id", order_id, schema));
                    type = valFinder("order", "type", "order_id", order_id, schema);
                    
                    //Возвращаем лот на баланс пользователя
                    float reqCrncy; // Required amount of currency
                    float userQuantity, newQuantity;  
                    string reqLotId;
                    if (type == "buy"){
                        reqCrncy = price * quantity;
                        reqLotId = valFinder("pair", "second_lot_id", "pair_id", pair_id, schema);
                        string inject = user_id + "\' AND user_lot.lot_id = \'" + reqLotId;
                        userQuantity = stof(valFinder("user_lot", "quantity", "user_id", inject, schema));
                    }else if (type == "sell"){
                        reqCrncy = quantity;
                        reqLotId = valFinder("pair", "first_lot_id", "pair_id", pair_id, schema);
                        string inject = user_id + "\' AND user_lot.lot_id = \'" + reqLotId;
                        userQuantity = stof(valFinder("user_lot", "quantity", "user_id", inject, schema));
                    }
                    newQuantity = userQuantity + reqCrncy;
                    string nwQntty = floatToStr(newQuantity);    //Новое значение количества лота пользователя в string
                    //Возвращение требуемого лота на баланс пользователя
                    string message = "UPDATE user_lot SET quantity = '"+nwQntty+"' WHERE user_lot.user_id = '"+user_id+"' AND user_lot.lot_id = '"+reqLotId+"'";
                    string dbmsResult = dbms(message, schema);
                    //Удаляем ордер
                    message = "DELETE FROM order WHERE order.order_id = '"+order_id+"\'";
                    dbmsResult = dbms(message, schema);
                    cout << "Result: Order deleted succesfully.\n";
                    result += "\tResult: Order deleted succesfully.\n";
                }else if(user_id != idVerification){
                    result +=   "\tERROR: No permission for this user.\n";
                    syntaxError = "ERROR: No permission for this user.\n";
                }else if(closed != "-"){
                    result +=   "\tERROR: Order already closed.\n";
                    syntaxError = "ERROR: Order already closed.\n";
                }
            }else{
                result += "\tERROR: Wrong syntax.\n";
                syntaxError = "ERROR: Wrong syntax.\n";
            }
        }

        if (syntaxError!=""){cout << syntaxError;}
        result += "]\n";
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
    svr.Get("/pair", [](const Request& req, Response& res){
        cout << ">Recieved GET /pair request [" << req.remote_port << "];\n";
        string result = "Response\n[\n";

        ifstream inFile(schema.name+"/pair/1.csv");
        string row;
        getline(inFile, row);
        while (getline(inFile, row)){
            stringstream ss(row);
            string temp;
            getline(ss, temp, ',');
            result += "\t{\n\t\t\"pair_id\": " + temp + ",\n";
            getline(ss,temp, ',');
            result += "\t\t\"sale_lot_id\": " + temp + ",\n";
            getline(ss,temp);
            result += "\t\t\"buy_lot_id\": " + temp + "\n\t},\n";
        }
        inFile.close();
        result += "]\n";
        res.set_content(result, "text/plain");
    });
    svr.Get("/balance", [](const Request& req, Response& res){
        cout << ">Recieved GET /balance request [" << req.remote_port << "];\n";

        string user_id, syntaxError;
        string result = "Response\n[\n";

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
            }else{
                result += "\tERROR: Wrong syntax.\n";
                syntaxError = "ERROR: Wrong syntax.\n";
                break;
            }
        }
        if (syntaxError == ""){
            ifstream inFile(schema.name+"/user_lot/1.csv");
            string row;
            getline(inFile, row);
            while (getline(inFile, row)){
                stringstream ss(row);
                string idFromFile, temp;
                getline(ss, idFromFile, ',');
                if (idFromFile == user_id){
                    getline(ss,temp, ',');
                    result += "\t{\n\t\t\"lot_id\": " + temp + ",\n";
                    getline(ss,temp);
                    result += "\t\t\"quantity\": " + temp + "\n\t},\n";
                }
            }
            inFile.close();
        }
        if (result == "Response\n[\n"){
            result += "\tERROR: Wrong syntax.\n";;
        }

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
curl -d 'x-user-key=62938913&pair_id=21&quantity=300&price=0.015&type=buy' http://127.0.0.1:7432/order

curl -d 'x-user-key=62938913&pair_id=21&quantity=300&price=0.015&type=sell' http://127.0.0.1:7432/order
curl -d 'x-user-key=62938913&pair_id=21&price=0.015&type=buy' http://127.0.0.1:7432/order

DELETE /order?x-user-key=62938913&order_id=2
DELETE /order?x-user-key=52053858&order_id=2
*/