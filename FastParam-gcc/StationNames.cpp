//---------------------------------------------------------------------------
#include <cstring>
#include <glibmm/keyfile.h>
#include <string>

#include "StationNames.h"
//---------------------------------------------------------------------------
StStations *Stations = NULL;
//
StStations::StStations(std::string Path, bool one) {
    InitPath = Path;
    oneMa = one;
    memset(StationName, 0, sizeof(StationName));
    ReadStationName();
}
//
StStations::~StStations() {
    for (int i = 0; i < StationsCount; i++) {
        if (StationName[i])
            delete StationName[i];
    }
}
// ïðî÷èòàòü èìåíà ñòàíöèé
void StStations::ReadStationName() {
    if (!oneMa) {
        Glib::KeyFile key_file;
        try {
            key_file.load_from_file(InitPath + "Stations.ini");
            auto SSections = key_file.get_groups();
            for (const auto &section : SSections) {
                if (section.find("Station") == 0) {
                    int id = key_file.get_integer(section, "ID");
                    StationName[id] = new StStationName;
                    StationName[id]->id = id;
                    StationName[id]->name =
                        key_file.get_string(section, "Name");
                    StationName[id]->comment =
                        key_file.get_string(section, "Comments");
                }
            }
        } catch (...) {
        }
    } else {
        StationName[0] = new StStationName;
        StationName[0]->id = 0;
        StationName[0]->name = "Контроллеры";
        StationName[0]->comment = "Контроллеры";
    }
}
// âåðíóòü èìÿ ñòàíöèè
std::string StStations::GetStationName(unsigned char id) {
    if (StationName[id])
        return StationName[id]->name;
    return "Ñòàíöèÿ " + std::to_string(id);
}
// âåðíóòü èìåíà ñòàíöèè
bool StStations::GetStationNameById(int id, std::string &SName) {
    if (id >= 0 && id < StationsCount && StationName[id]) {
        SName = StationName[id]->name;
        return true;
    }
    return false;
}
// âåðíóòü èä ñòàíöèè
int StStations::GetStationId(std::string SName) {
    for (int i = 0; i < StationsCount; i++) {
        if (StationName[i] && StationName[i]->name == SName)
            return i;
    }
    return -1;
}
// ïðîâåðèòü èä ñòàíöèè
int StStations::CheckStationId(std::string SName, int id) {
    if (id >= 0 && id < StationsCount) {
        if (StationName[id] && StationName[id]->name == SName)
            return id;
        if (SName == "Ñòàíöèà " + std::to_string(id))
            return id;
        return GetStationId(SName);
    }
    return GetStationId(SName);
}
