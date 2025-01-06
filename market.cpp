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