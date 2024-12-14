#ifndef INCLUDE_MARKET_H
#define INCLUDE_MARKET_H

#include <string>
#include "dbms/structures.h"
#include "dbms/dbms.h"

using namespace std;

string idFinder(string object, Schema& schema);
string keyGen(string username, Schema& schema);
bool isUserExists(string username, Schema& schema);

#endif