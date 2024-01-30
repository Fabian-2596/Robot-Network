#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

const string $POSTDATAPATH = "DB/POSTData.txt";

// TODO: Struktur anpassen/verbessern
struct Player
{
  int id;
  string name;
  string role;
  bool isActive;
};
struct Captain
{
  Player player;
};
struct Team
{
  Captain captain;
  vector<Player> playerList;
};

class DB
{

private:
  Captain captain;
  Team team;
  string controllerStatus;
  int countPlayer;
  vector<string> POSTData;

  bool txt_editable = true;

public:
  DB();
  ~DB();

  Captain getCaptain() { return captain; }
  Team getTeam() { return team; }
  string getConStatus() { return controllerStatus; };
  int getTeamSize() { return team.playerList.size(); }
  vector<string> getPOSTData() { return POSTData; }

  void setCaptain(Captain capt) { this->captain = capt; }
  void setTeam(Team t) { this->team = t; }
  void setConStatus(string stat) { this->controllerStatus = stat; }
  void setCountPlayer(int count) { this->countPlayer = count; }
  bool setPlayerStatus(int id, bool status);
  void addPOSTData(string postData) { this->POSTData.push_back(postData); }
  void addPlayer(Player p);

  // gibt 0 bei Erfolg, 1 bei Misserfolg zurück
  bool removePlayer(Player p);

  void persistPOST();
  vector<string> readPOSTtxt();
};

DB::DB()
{
  Player pl{};
  pl.name = "Tony Kroos";

  Captain cp{};
  this->captain = cp;

  Team t{};
  this->team = t;

  this->controllerStatus = "Von Constructor erstellt";

  this->countPlayer = 1;

  this->POSTData = readPOSTtxt();
}

DB::~DB()
{

  persistPOST();
}

void DB::persistPOST()
{
  while (txt_editable = false)
  {
  }
  txt_editable = false;

  ofstream DBFile($POSTDATAPATH);

  // Persiste alle POSTData Einträge
  for (int i{}; i < this->POSTData.size(); ++i)
  {
    DBFile << this->POSTData[i] << endl;
  }

  DBFile.close();
  txt_editable = true;

  cout << "POST-Daten wurden gesichert" << endl;
}

vector<string> DB::readPOSTtxt()
{

  vector<string> POSTData{};
  string temp{};

  ifstream DBFile($POSTDATAPATH);

  while (getline(DBFile, temp))
  {
    POSTData.push_back(temp);
  }

  return POSTData;
}

void DB::addPlayer(Player p)
{
  this->team.playerList.push_back(p);
  ++countPlayer;
}

bool DB::removePlayer(Player p)
{
  vector<Player> *pList{&this->team.playerList};
  for (int i{}; i < pList->size(); ++i)
  {
    if (pList->at(i).id == p.id)
    {
      pList->erase(next(pList->begin(), i));
      --countPlayer;
      return 0;
    }
  }
  return 1;
}

bool DB::setPlayerStatus(int id, bool status)
{
  for (int i{}; i < getTeamSize(); i++)
  {
    if (team.playerList.at(i).id == id)
    {
      team.playerList.at(i).isActive = status;
      return true;
    }
  }
  return false;
}