#include "market.h"

float convertToFloat(const string& str) {
    try {
        return stof(str);
    } catch (const invalid_argument& e) {
        cerr << "Invalid argument: " << e.what() << endl;
    } catch (const out_of_range& e) {
        cerr << "Out of range: " << e.what() << endl;
    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    return 0.0f; // Возвращаем 0.0f в случае ошибки
}
string floatToStr(const float& value){  //Форматирование строки из float значения
    string result = to_string(value);
    for (size_t i = result.length() - 1; result[i] == '0'; i--){
        result.erase(i,1);
        if (result[i-1] == '.'){
            result.erase(i-1, 1);
            break;
        }
    }
    return result;
}
string idFinder(string table, string column, string value, Schema& schema){
    string message = "SELECT "+table +"."+column+" FROM "+table+" WHERE "+table+"."+column+" = \'"+value+"\'";
    string dbmsResult = dbms(message, schema);

    stringstream ss (dbmsResult);
    string id;
    getline(ss, id, '\n');
    //cout << id << "\n";
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    //cout << id << "\n";
    //string name;
    //getline(ss, name);
    //name.erase(0,5);
    //cout << name << "\n";
    if (id != ""){
        return id;
    }else{return "false";}
}
string valFinder(string table, string column, string idCol, string id_, Schema& schema){
    string message = "SELECT "+table +"."+column+" FROM "+table+" WHERE "+table+"."+idCol+" = \'"+id_+"\'";
    string dbmsResult = dbms(message, schema);
    //cout << dbmsResult << "\n";
    stringstream ss (dbmsResult);
    string id;
    getline(ss, id, '\n');
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    if (id == ""){return "false";}
    string name;
    getline(ss, name);
    name.erase(0,5);
    name.erase(name.length()-1, 2);
    if (name != ""){
        return name;
    }else{return "false";}
}
string keyGen(string username, Schema& schema){
    random_device rd;   // Генерация случайного ключа
    mt19937 gen(rd());
    uniform_int_distribution<> dis(10000000, 99999999); // Задан диапазон
    string key = to_string(dis(gen));

    string message = "INSERT INTO user VALUES (\'"+ username + "\', \'" + key + "\')";
    string dbmsResult = dbms(message, schema);

    string user_id = idFinder("user", "username", username, schema);
    ifstream lotFile (schema.name + "/lot/1.csv");
    string row;
    getline(lotFile, row);
    while (getline(lotFile, row)){
        stringstream ss(row);
        string lot_id;
        getline(ss, lot_id, ',');
        message = "INSERT INTO user_lot VALUES (\'"+user_id+"\', \'"+lot_id+"\', \'1000\')";
        dbmsResult = dbms(message, schema);
    }
    lotFile.close();
    cout << "New user created succesfully! \n";
    return key;
}
bool isUserExists(string username, Schema& schema){
    string id = idFinder("user", "username", username, schema);
    if (id != "false"){
        return true;
    }else {return false;}
}
string getCurrentTime() {
    auto now = chrono::system_clock::now(); //Получаем текущее время
    time_t now_time = chrono::system_clock::to_time_t(now);
    tm* now_tm = localtime(&now_time);  //Преобразуем время в структуру tm
    ostringstream oss;  //Используем ostringstream для форматирования времени в строку
    oss << put_time(now_tm, "%Y-%m-%d|%H:%M:%S");
    return oss.str();
}
string createOrder(string user_id, string pair_id, float quantity, float price, string type, Schema& schema){  
    string first_lot_id = valFinder("pair", "first_lot_id", "pair_id", pair_id, schema);
    string second_lot_id = valFinder("pair", "second_lot_id", "pair_id", pair_id, schema);

    float reqCrncy; // Required amount of currency
    float userQuantity;  
    string reqLotId;
    if (type == "buy"){
        reqCrncy = price * quantity;
        reqLotId = second_lot_id;
        
    }else if (type == "sell"){
        reqCrncy = quantity;
        reqLotId = first_lot_id;

    }
    string inject = user_id + "\' AND user_lot.lot_id = \'" + reqLotId;
    userQuantity = stof(valFinder("user_lot", "quantity", "user_id", inject, schema));

    if (userQuantity < reqCrncy){  //Проверка что активов пользователя достаточно создания ордера
        cout << "ERROR: Lot balance is not enough\n";
        return "\tERROR: Lot balance is not enough\n";
    }
    //Снятие требуемого лота с баланса пользователя
    string message = "UPDATE user_lot SET quantity = '"+floatToStr(userQuantity - reqCrncy)+"' WHERE user_lot.user_id = '"+user_id+"' AND user_lot.lot_id = '"+reqLotId+"'";
    string dbmsResult = dbms(message, schema);
    inject = "";    //На всякий случай очищаю переменную
    //Если существует ордер с тем же pair_id но другого типа - проверяем цену и если что удовлетворяем частично либо полностью
    string torder_id, tuser_id, byrPrft, slrPrft, result, time;     //temporary values
    float tquantity, tprice, toBuyer, toSeller;
    if (type == "buy"){
        inject = pair_id + "\' AND order.type = \'sell\' AND order.closed = \'-";
        torder_id = idFinder("order", "pair_id", inject, schema);   //Пока находятся такие torder_id -> продолжаем проверять
        tprice = convertToFloat(valFinder("order", "price", "order_id", torder_id, schema));
        while (torder_id != "false" && tprice <= price && quantity != float(0)){
            tuser_id = valFinder("order", "user_id", "order_id", torder_id, schema);
            tquantity = convertToFloat(valFinder("order", "quantity", "order_id", torder_id, schema));

            float qntSwitch = float(0);
            if (quantity > tquantity){
                qntSwitch = quantity;
                quantity = tquantity;
            }

            time = getCurrentTime();    //Получаем текущее время
            toBuyer = quantity;
            toSeller = quantity * tprice;
            //Добавляем лот на счёт продавца
            inject = tuser_id + "\' AND user_lot.lot_id = \'" + second_lot_id;
            slrPrft = floatToStr(stof(valFinder("user_lot", "quantity", "user_id", inject, schema)) + toSeller);
            message = "UPDATE user_lot SET quantity = '"+slrPrft+"' WHERE user_lot.user_id = '"+tuser_id+"' AND user_lot.lot_id = '"+second_lot_id+"'";
            dbmsResult = dbms(message, schema);
            //Закрываем ордер продавца
            message = "UPDATE order SET quantity = '"+floatToStr(quantity)+"', price = '"+floatToStr(tprice)+"', closed = '"+time+"' WHERE order.order_id = '"+torder_id+"'";
            dbmsResult = dbms(message, schema);
            cout << "Closed order with id: " + torder_id + "\n";
            //Добавляем лот на счёт покупателя
            inject = user_id + "\' AND user_lot.lot_id = \'" + first_lot_id;
            byrPrft = floatToStr(stof(valFinder("user_lot", "quantity", "user_id", inject, schema)) + toBuyer);
            message = "UPDATE user_lot SET quantity = '"+byrPrft+"' WHERE user_lot.user_id = '"+user_id+"' AND user_lot.lot_id = '"+first_lot_id+"'";
            dbmsResult = dbms(message, schema);
            //Создаём закрытый ордер покупателя
            message = "INSERT INTO order VALUES (\'"+user_id+"\', \'"+pair_id+"\', \'"+floatToStr(quantity)+"\',\'"+floatToStr(tprice)+"\',\'"+type+"\', \'"+time+"\')";
            dbmsResult = dbms(message, schema);
            cout << "Created closed order with id: " + dbmsResult + "\n";
            result += dbmsResult + " ";
            
            if (qntSwitch != float(0)){
                quantity = qntSwitch - tquantity;
            }
            else if (quantity < tquantity){
                //Создаём новый ордер продавца
                message = "INSERT INTO order VALUES (\'"+tuser_id+"\', \'"+pair_id+"\', \'"+floatToStr(tquantity - quantity)+"\',\'"+floatToStr(tprice)+"\',\'sell\', \'-\')";
                dbmsResult = dbms(message, schema);
                cout << "Created order with id: " + dbmsResult + "\n";
                quantity = float(0);    //Обнуляем количество торгуемых лотов
            }else if (quantity == tquantity){
                quantity = float(0); //Обнуляем количество торгуемых лотов
            }
            inject = pair_id + "\' AND order.type = \'buy\' AND order.closed = \'-";
            torder_id = idFinder("order", "pair_id", inject, schema);
            tprice = convertToFloat(valFinder("order", "price", "order_id", torder_id, schema));
        }
    }else if (type == "sell"){
        inject = pair_id + "\' AND order.type = \'buy\' AND order.closed = \'-";
        torder_id = idFinder("order", "pair_id", inject, schema);   //Пока находятся такие torder_id -> продолжаем проверять
        tprice = convertToFloat(valFinder("order", "price", "order_id", torder_id, schema));
        while (torder_id != "false" && price <= tprice && quantity != float(0)){
            tuser_id = valFinder("order", "user_id", "order_id", torder_id, schema);
            tquantity = convertToFloat(valFinder("order", "quantity", "order_id", torder_id, schema));

            float qntSwitch = float(0);
            if (quantity > tquantity){
                qntSwitch = quantity;
                quantity = tquantity;
            }

            time = getCurrentTime();    //Получаем текущее время
            toBuyer = quantity;
            toSeller = quantity * price;
            //Добавляем лот на счёт покупателя
            inject = tuser_id + "\' AND user_lot.lot_id = \'" + first_lot_id;
            byrPrft = floatToStr(stof(valFinder("user_lot", "quantity", "user_id", inject, schema)) + toBuyer);
            message = "UPDATE user_lot SET quantity = '"+byrPrft+"' WHERE user_lot.user_id = '"+tuser_id+"' AND user_lot.lot_id = '"+first_lot_id+"'";
            dbmsResult = dbms(message, schema);
            //Закрываем ордер покупателя
            message = "UPDATE order SET quantity = '"+floatToStr(quantity)+"', price = '"+floatToStr(price)+"', closed = '"+time+"' WHERE order.order_id = '"+torder_id+"'";
            dbmsResult = dbms(message, schema);
            cout << "Closed order with id: " + torder_id + "\n";

            //Добавляем лот на счёт продавца
            inject = user_id + "\' AND user_lot.lot_id = \'" + second_lot_id;
            slrPrft = floatToStr(stof(valFinder("user_lot", "quantity", "user_id", inject, schema)) + toSeller);
            message = "UPDATE user_lot SET quantity = '"+slrPrft+"' WHERE user_lot.user_id = '"+user_id+"' AND user_lot.lot_id = '"+second_lot_id+"'";
            dbmsResult = dbms(message, schema);
            //Создаём закрытый ордер продавца
            message = "INSERT INTO order VALUES (\'"+user_id+"\', \'"+pair_id+"\', \'"+floatToStr(quantity)+"\',\'"+floatToStr(price)+"\',\'"+type+"\', \'"+time+"\')";
            dbmsResult = dbms(message, schema);
            cout << "Created closed order with id: " + dbmsResult + "\n";
            result += dbmsResult + " ";
            
            if (qntSwitch != float(0)){
                quantity = qntSwitch - tquantity;
            }
            else if (quantity < tquantity){
                //Создаём новый ордер покупателя
                message = "INSERT INTO order VALUES (\'"+tuser_id+"\', \'"+pair_id+"\', \'"+floatToStr(tquantity - quantity)+"\',\'"+floatToStr(tprice)+"\',\'buy\', \'-\')";
                dbmsResult = dbms(message, schema);
                cout << "Created order with id: " + dbmsResult + "\n";
                quantity = float(0);    //Обнуляем количество торгуемых лотов
            }else if (quantity == tquantity){
                quantity = float(0); //Обнуляем количество торгуемых лотов
            }
            inject = pair_id + "\' AND order.type = \'buy\' AND order.closed = \'-";
            torder_id = idFinder("order", "pair_id", inject, schema);
            tprice = convertToFloat(valFinder("order", "price", "order_id", torder_id, schema));
        }
    }
    if (quantity != float(0)){
        //Создаём новый ордер
        message = "INSERT INTO order VALUES (\'"+user_id+"\', \'"+pair_id+"\', \'"+floatToStr(quantity)+"\',\'"+floatToStr(price)+"\',\'"+type+"\', \'-\')";
        dbmsResult = dbms(message, schema);
        cout << "Created order with id: " + dbmsResult + "\n";
        result += dbmsResult + " ";
    }
    return result;
}