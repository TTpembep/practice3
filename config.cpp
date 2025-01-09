#include "config.h"

string lotAdd(string lot, string schemaName){
  int lotPrimaryKey = getPrimaryKey(schemaName+"/lot/lot");
  string result = to_string(lotPrimaryKey);
  ofstream lotFile(schemaName + "/lot/1.csv", ios::app);   // Открываем файл для добавления
  lotFile << lotPrimaryKey << "," <<  lot << endl;
  lotFile.close();  // Закрываем файл
  updatePrimaryKey(schemaName+"/lot/lot", lotPrimaryKey + 1);  // Обновляем первичный ключ в файле  
  return result;
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

  fList lot_ids;
  while (lotsNames.head->next != nullptr){
    string first_lot = lotsNames.head->data;
    //string lot_id = lotAdd(first_lot, schema.name);
    lot_ids.push_back(lotAdd(first_lot, schema.name));
    lotsNames.remove_index(0);
  }
  string last_lot = lotsNames.head->data;
  lot_ids.push_back(lotAdd(last_lot, schema.name));


  while (lot_ids.head->next != nullptr){
    string first_lot_id = lot_ids.head->data;
    //cout << first_lot_id << endl;
    //lotAdd(first_lot, schema.name);

    Node* current = lot_ids.head;
    current = current->next;
    while (current != nullptr) {
      string second_lot_id = current->data;
      int primaryKey = getPrimaryKey(schema.name+"/pair/pair");
      ofstream outfile(schema.name + "/pair/1.csv", ios::app);   //Открываем файл для добавления
      outfile << primaryKey << "," << first_lot_id << "," << second_lot_id << endl;
      outfile.close();
      updatePrimaryKey(schema.name+"/pair/pair", primaryKey + 1);
      current = current->next;
    }
    lot_ids.remove_index(0);
    delete current;
  }
  //string last_lot = lotsNames.head->data;
  //lotAdd(last_lot, schema.name);


  ofstream outputFile(schema.name+"/pair/pair_cfgStatus");
  outputFile << "1";
  outputFile.close();
  cout << "Market config complete. \n";
  return;
}