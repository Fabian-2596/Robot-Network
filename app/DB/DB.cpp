
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

const string $POSTDATAPATH = "DB/POSTData.txt"; 

//TODO: Struktur anpassen/verbessern
struct Player{
  string name;
};
struct Captain
{
  Player player;
};
struct Team
{
  Captain captain;
  vector<Player> team;
};

class DB
{

private:
  Captain captain;
  Team mannschaft;
  string controllerStatus;
  int countSpieler;
  vector<string> POSTData;

public:
  DB();
  ~DB();

  Captain getCaptain() { return captain; }
  Team getMannschaft() { return mannschaft; }
  string getConStatus() {return controllerStatus;};
  int getCountSpieler() { return countSpieler; }
  vector<string> getPOSTData() { return POSTData; }

  void setCaptain(Captain capt) { this->captain = capt; }
  void setMannschaft(Team mann) { this->mannschaft = mann; }
  void setConStatus(string stat) { this->controllerStatus = stat; }
  void setCountSpieler(int count) { this->countSpieler = count; }

  void addPOSTData(string postData) { this->POSTData.push_back(postData); }

  void persistPOST();
  vector<string> readPOSTtxt();
};

DB::DB(){
  Player pl{};
  pl.name = "Tony Kroos";

  Captain cp{};
  cp.player = pl;
  this->captain = cp;

  Team mann{};
  this->mannschaft = mann;

  this->controllerStatus = "Von Contructor erstellt";

  this->countSpieler = 1;

  this->POSTData = readPOSTtxt();
}

DB::~DB(){

  persistPOST();

}

void DB::persistPOST()
{

  ofstream DBFile($POSTDATAPATH);

  //Persiste alle POSTData Einträge
  for (int i{}; i < this->POSTData.size(); ++i)
  {
    DBFile << this->POSTData[i] << endl;
  }

  DBFile.close();

  cout << "POST-Daten wurden gesichert" << endl;
}


vector<string> DB::readPOSTtxt(){

  vector<string> POSTData{};
  string temp{};

  ifstream DBFile($POSTDATAPATH);

  while (getline(DBFile,temp)){
    POSTData.push_back(temp);
  }

  return POSTData;

}