#ifndef INCLUDE_MARKET_H
#define INCLUDE_MARKET_H

#include <string>

#include <chrono>   //Для получения текущего времени
#include <iomanip>
#include <sstream>


#include <stdexcept>
#include <random>
#include "dbms/structures.h"
#include "dbms/dbms.h"

using namespace std;

float convertToFloat(const string& str);
string floatToStr(const float& value);  //Форматирование строки из float значения
string idFinder(string table, string object, string value, Schema& schema);
string valFinder(string table, string column, string idCol, string id_, Schema& schema);
string keyGen(string username, Schema& schema);
bool isUserExists(string username, Schema& schema);
string getCurrentTime();
string createOrder(string user_id, string pair_id, float quantity, float price, string type, Schema& schema);

#endif