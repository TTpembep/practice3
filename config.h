#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include "dbms/json.hpp"
#include "dbms/structures.h"
#include "dbms/actions.h"
using namespace std;
using json = nlohmann::json;

void marketCfg(Schema schema, Config& config);   //Конфигурация маркета

#endif