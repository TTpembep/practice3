#ifndef INCLUDE_MARKET_H
#define INCLUDE_MARKET_H

#include <string>
#include <stdexcept>
#include <random>
#include "dbms/structures.h"
#include "dbms/dbms.h"

using namespace std;

float convertToFloat(const string& str);
string idFinder(string table, string object, string value, Schema& schema);
string keyGen(string username, Schema& schema);
bool isUserExists(string username, Schema& schema);

#endif