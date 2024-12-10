#include "market.h"

string market(string message, Schema schema){ //Получает строку 
    
    string parsedQuery;
    //Делает что-то 

    string request = dbms(parsedQuery, schema);  //Вызывает dbms

    //Формирует обратную строку
    return request; //Либо от dbms либо формирует что-то новое
}