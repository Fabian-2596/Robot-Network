
#include <string>
#include <vector>
using namespace std;

struct structCaptain{};
struct structMannschaft{};

enum status {OK,Building,Crashed,ShutDown};
class DB {

  private:
    structCaptain captain;
    structMannschaft mannschaft;
    status controllerStatus;
    int countSpieler;
    vector<string> lastPOSTData;

  public:

    DB() {};

    structCaptain getCaptain(){return captain;}
    structMannschaft getMannschaft(){return mannschaft;}
    status getConStatus(){return controllerStatus;}
    int getCountSpieler(){return countSpieler;}
    vector<string> getPOSTData(){return lastPOSTData;}

    void setCaptain(structCaptain capt){this->captain = capt;}
    void setMannschaft(structMannschaft mann){this->mannschaft = mann;}
    void setConStatus(int stat){this->controllerStatus = status(stat) ;}
    void setCountSpieler(int count){this->countSpieler = count;}
    void setLastPOSTData(vector<string> postData){this->lastPOSTData = postData;}
};