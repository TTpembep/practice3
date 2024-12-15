#include "config.h"

void lotAdd(string lot, string schemaName){
  int lotPrimaryKey = getPrimaryKey(schemaName+"/lot/lot");
  ofstream lotFile(schemaName + "/lot/1.csv", ios::app);   // Открываем файл для добавления
  lotFile << lotPrimaryKey << "," <<  lot << endl;
  lotFile.close();  // Закрываем файл
  updatePrimaryKey(schemaName+"/lot/lot", lotPrimaryKey + 1);  // Обновляем первичный ключ в файле  
}
void marketCfg(Schema schema, Config& config){  //Конфигурация маркета
  ifstream file("config.json");
  json jConfig;
  file >> jConfig;
  fList lotsNames;
  for (const auto& lot : jConfig["lots"]) {
    lotsNames.push_back(lot.get<string>());
  }
  config.ip = jConfig["database_ip"].get<string>();
  config.port = jConfig["database_port"].get<int>();

  ifstream fin(schema.name+"/pair/pair_cfgStatus");
  if(!fin.is_open()) {    
    cout << "File "+ schema.name+"/pair/pair_cfgStatus not found. \n";
    return;
  }
  string lockStatus;
  getline(fin,lockStatus);
  fin.close();
  if (lockStatus != "0"){
    cout << "Market is already configured. \n";
    return;
  }

  while (lotsNames.head->next != nullptr){
    string first_lot = lotsNames.head->data;
    lotAdd(first_lot, schema.name);

    Node* current = lotsNames.head;
    current = current->next;
    while (current != nullptr) {
      string second_lot = current->data;
      int primaryKey = getPrimaryKey(schema.name+"/pair/pair");
      ofstream outfile(schema.name + "/pair/1.csv", ios::app);   //Открываем файл для добавления
      outfile << primaryKey << "," << first_lot << "," << second_lot << endl;
      outfile.close();
      updatePrimaryKey(schema.name+"/pair/pair", primaryKey + 1);
      current = current->next;
    }
    lotsNames.remove_index(0);
    delete current;
  }

  string last_lot = lotsNames.head->data;
  lotAdd(last_lot, schema.name);

  ofstream outputFile(schema.name+"/pair/pair_cfgStatus");
  outputFile << "1";
  outputFile.close();
  cout << "Market config complete. \n";
  return;
}