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
    stringstream ss (dbmsResult);
    string id;
    getline(ss, id, '\n');
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    getline(ss, id, ' ');
    string name;
    getline(ss, name);
    name.erase(0,5);
    name.erase(name.length()-1, 2);
    if (name != ""){
        return name;
    }else{return "false";}
}
string keyGen(string username, Schema& schema){
    // INSRET INTO user VALUES ('username', 'key')
    random_device rd;   // Генерация случайного ключа
    mt19937 gen(rd());
    uniform_int_distribution<> dis(10000000, 99999999); // Задан диапазон
    string key = to_string(dis(gen));

    string message = "INSERT INTO user VALUES (\'"+ username + "\', \'" + key + "\')";
    //cout << message << "\n";
    string dbmsResult;
    
    dbmsResult = dbms(message, schema);
    //cout << dbmsResult << "\n";
    // user_id lot_id quantity
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
    // INSRET INTO user VALUES ('user_id', 'lot_id', 'quantity')
    cout << "New user created succesfully! \n";
    return key;
}
bool isUserExists(string username, Schema& schema){
    string id = idFinder("user", "username", username, schema);
    if (id != "false"){
        return true;
    }else {return false;}
}
string createOrder(string user_id, string pair_id, float quantity, float price, string type, Schema& schema){
    // 16, 21, 300, 0.015, buy
    
    //string first_lot_id = valFinder("pair", "first_lot_id", "pair_id", pair_id, schema);
    //cout << first_lot_id << " " << second_lot_id << " DEBUG \n";
    //string first_lot = valFinder("lot", "name", "lot_id", first_lot_id, schema);
    //string second_lot = valFinder("lot", "name", "lot_id", second_lot_id, schema);
    //cout << first_lot << " " << second_lot << " DEBUG \n";
    //string message = "SELECT user_lot.quantity FROM user_lot WHERE user_lot.user_id = \'" + user_id + "\' AND user_lot.lot_id = \'" + second_lot_id + "\'";
    //string dbmsResult = dbms(message, schema);

    float reqCrncy;  // Required amount of currency
    string req_lot_id;
    if (type == "buy"){
        reqCrncy = price * quantity;
        req_lot_id = valFinder("pair", "second_lot_id", "pair_id", pair_id, schema);
    }else if (type == "sell"){
        reqCrncy = quantity;
        req_lot_id = valFinder("pair", "first_lot_id", "pair_id", pair_id, schema);
    }
    string inject = user_id + "\' AND user_lot.lot_id = \'" + req_lot_id;
    float user_quantity = stof(valFinder("user_lot", "quantity", "user_id", inject, schema));

    if (user_quantity < reqCrncy){  //Проверка что активов пользователя достаточно создания ордера
        cout << "ERROR: Lot balance is not enough\n";
        return "\tERROR: Lot balance is not enough\n";
    }

    //UPDATE user_lot SET quantity = 'new_quantity' WHERE user_lot.user_id = 'user_id' AND user_lot.lot_id = 'req_lot_id'
    //UPDATE order SET quantity = 'new_quantity', price = 'new_price', closed = 'time' WHERE order.order_id = 'that_order_id'

    //string message = "UPDATE user_lot SET quantity = '666' WHERE user_lot.user_id = '18' AND user_lot.lot_id = '11'";
    string message = "UPDATE order SET quantity = '200', price = '0.01', closed = '420' WHERE order.order_id = '0'";
    string dbmsResult = dbms(message, schema);
    cout << dbmsResult << endl;
    //order_id,user_id,pair_id,quantity,price,type,closed
    //0,12,21,300,0.015,buy,-



    return "This isnt complete yet :(";
}