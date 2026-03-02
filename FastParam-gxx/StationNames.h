//---------------------------------------------------------------------------

#ifndef StationNamesH
#define StationNamesH

#define StationsCount 256
//---------------------------------------------------------------------------
//�������
struct StStationName{
  unsigned char id;
  std::string name;
  std::string comment;
};
//�������
class StStations{
private:
  StStationName *StationName[StationsCount];
  std::string InitPath;
  bool oneMa;
  void ReadStationName();
public:
  StStations(std::string Path,bool one=false);
  ~StStations();
  std::string GetStationName(unsigned char id);
  bool GetStationNameById(int id,std::string &SName);
  int GetStationId(std::string SName);
  int CheckStationId(std::string SName,int id);
};
//
extern StStations *Stations;
#endif
