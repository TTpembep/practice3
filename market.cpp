#include "market.h"

string idFinder(string object, Schema& schema){


}
string keyGen(string username, Schema& schema){
    // INSRET INTO user VALUES ('username', 'key')
    size_t hash = 0;
    for (char letter : username) {   //Вычисляет значение ключа в таблице
         hash = hash * 32 + letter;
    }
    string key = to_string(hash % 100000000);
    string message = "INSERT INTO user VALUES (\'"+ username + "\', \'" + key + "\')";
    //cout << message << "\n";
    string dbmsResult;
    
    dbmsResult = dbms(message, schema);
    cout << dbmsResult << "\n";

    // user_id lot_id quantity

    // INSRET INTO user VALUES ('user_id', 'lot_id', 'quantity')
    return key;
}
bool isUserExists(string username, Schema& schema){
    // SELECT user.username FROM user WHERE username = john1
    string message = "SELECT user.username FROM user WHERE user.username = \'" + username +"\'";
    //cout << message << "\n";
    string dbmsResult;
    
    dbmsResult = dbms(message, schema);
    //cout << dbmsResult << "\n";

    if (dbmsResult.find(username) != string::npos){
        return true;
    }else {return false;}
}