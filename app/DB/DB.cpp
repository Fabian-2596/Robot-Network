
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

struct structCaptain
{
};
struct structMannschaft
{
};

enum status
{
  OK,
  Building,
  Crashed,
  ShutDown
};
class DB
{

private:
  structCaptain captain;
  structMannschaft mannschaft;
  status controllerStatus;
  int countSpieler;
  vector<string> POSTData;

public:
  DB(){};
  ~DB(){persist();};

  structCaptain getCaptain() { return captain; }
  structMannschaft getMannschaft() { return mannschaft; }
  status getConStatus() { return controllerStatus; }
  int getCountSpieler() { return countSpieler; }
  vector<string> getPOSTData() { return POSTData; }

  void setCaptain(structCaptain capt) { this->captain = capt; }
  void setMannschaft(structMannschaft mann) { this->mannschaft = mann; }
  void setConStatus(int stat) { this->controllerStatus = status(stat); }
  void setCountSpieler(int count) { this->countSpieler = count; }

  void addPOSTData(string postData) { this->POSTData.push_back(postData); }

  void persist();
};

void DB::persist()
{

  ofstream DBFile("DB/POSTData.txt");

  //Persist each POSTData Entry
  for (int i{}; i < this->POSTData.size(); ++i)
  {
    DBFile << this->POSTData[i] << endl;
  }

  DBFile.close();
}