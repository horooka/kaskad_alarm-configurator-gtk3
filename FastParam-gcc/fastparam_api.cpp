#include "include/fastparam_api.hpp"
#include "BinDataStruct.h"
#include "StationNames.h"
#include <cstring>

namespace fastparam {

static inline StStations* S(Stations* s) { return reinterpret_cast<StStations*>(s); }
static inline StMachins* M(Machins* m) { return reinterpret_cast<StMachins*>(m); }

Stations* stations_get() {
    return reinterpret_cast<Stations*>(::Stations);
}
void stations_create(const std::string& path, bool one) {
    if (::Stations)
        delete ::Stations;
    ::Stations = new StStations(path, one);
}
void stations_destroy() {
    if (::Stations) {
        delete ::Stations;
        ::Stations = nullptr;
    }
}

std::string station_name(Stations* s, unsigned char id) {
    return S(s) ? S(s)->GetStationName(id) : std::string();
}
bool station_name_by_id(Stations* s, int id, std::string& out_name) {
    return S(s) && S(s)->GetStationNameById(id, out_name);
}
int station_id(Stations* s, const std::string& name) {
    return S(s) ? S(s)->GetStationId(name) : -1;
}
int station_check_id(Stations* s, const std::string& name, int id) {
    return S(s) ? S(s)->CheckStationId(name, id) : -1;
}

Machins* machins_get() {
    return reinterpret_cast<Machins*>(::Machins);
}
void machins_create(const std::string& path, bool one) {
    if (::Machins)
        delete ::Machins;
    ::Machins = new StMachins(path, one);
}
void machins_destroy() {
    if (::Machins) {
        delete ::Machins;
        ::Machins = nullptr;
    }
}

char machins_init_machine(Machins* m, unsigned char ma_id) {
    return M(m) ? M(m)->InitMachine(ma_id) : 0;
}
void machins_init_all(Machins* m) {
    if (M(m))
        M(m)->InitAllMachine();
}
int machins_get_use_count(Machins* m) {
    return M(m) ? M(m)->GetUseCount() : 0;
}
int machins_get_ma_id(Machins* m, unsigned char id) {
    return M(m) ? M(m)->GetMaId(id) : -1;
}

Controllers* machins_get_controllers(Machins* m, unsigned char ma_id) {
    return reinterpret_cast<Controllers*>(M(m) ? M(m)->GetControllers(ma_id) : nullptr);
}
Tree* machins_get_tree(Machins* m, unsigned char ma_id, short unsigned int cnt_id) {
    return reinterpret_cast<Tree*>(M(m) ? M(m)->GetTree(ma_id, cnt_id) : nullptr);
}
Cntrl* machins_get_cntrl(Machins* m, unsigned char ma_id, short unsigned int cnt_id) {
    return reinterpret_cast<Cntrl*>(M(m) ? M(m)->GetCntrl(ma_id, cnt_id) : nullptr);
}
CntrlIP* machins_get_cntrl_ip(Machins* m, unsigned char ma_id, short unsigned int cnt_id) {
    return reinterpret_cast<CntrlIP*>(M(m) ? M(m)->GetCntrlIP(ma_id, cnt_id) : nullptr);
}
char* machins_get_cntrl_ma_name(Machins* m, unsigned char ma_id, short unsigned int cnt_id) {
    return M(m) ? M(m)->GetCntrlMaName(ma_id, cnt_id) : nullptr;
}
int machins_get_cntrl_id_by_name(Machins* m, unsigned char ma_id, char* cname) {
    return M(m) ? M(m)->GetCntrlIdByName(ma_id, cname) : -1;
}
void* machins_get_ref_data(Machins* m, unsigned char ma_id, short unsigned int cnt_id) {
    return M(m) ? M(m)->GetRefData(ma_id, cnt_id) : nullptr;
}

static inline StControllers* C(Controllers* c) { return reinterpret_cast<StControllers*>(c); }
static inline StController* Ctrl(Cntrl* c) { return reinterpret_cast<StController*>(c); }

int controllers_count(Controllers* c) {
    return C(c) ? static_cast<int>(C(c)->CntrlHead.controlcount) : 0;
}
int controllers_id_by_index(Controllers* c, int index) {
    return C(c) ? C(c)->GetCntrlIdxId(index) : -1;
}
std::string cntrl_config_guid_str(Cntrl* c) {
    return Ctrl(c) ? Ctrl(c)->configid.GetStr() : std::string();
}

}
