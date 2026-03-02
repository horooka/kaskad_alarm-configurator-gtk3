#include "FastParamMain.h"
#include "BinDataStruct.h"
#include "StationNames.h"
#include "ParamSelect.h"
#include "FastParamConst.h"
#include <cstring>
#include <sstream>

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
std::string cntrl_get_name(Cntrl* c) {
    StController* sc = Ctrl(c);
    return sc && sc->name[0] ? std::string(sc->name) : std::string();
}

// ----- обход дерева KLogic и параметры -----

static inline StTree* T(Tree* t) { return reinterpret_cast<StTree*>(t); }
static inline StTreeChild* TC(TreeChild* c) { return reinterpret_cast<StTreeChild*>(c); }
static inline TreeChild* TC(StTreeChild* c) { return reinterpret_cast<TreeChild*>(c); }
static inline const SParamData* PD(const ParamData* d) { return reinterpret_cast<const SParamData*>(d); }
static inline SParamData* PD(ParamData* d) { return reinterpret_cast<SParamData*>(d); }

static void split_path(const std::string& path, std::vector<std::string>& parts) {
    parts.clear();
    std::string s = path;
    size_t pos = 0;
    for (;;) {
        size_t next = s.find('\\', pos);
        if (next == std::string::npos) {
            if (pos < s.size())
                parts.push_back(s.substr(pos));
            break;
        }
        if (next > pos)
            parts.push_back(s.substr(pos, next - pos));
        pos = next + 1;
    }
}

TreeChild* tree_get_child_by_path(Machins* m, unsigned char ma_id, unsigned short cnt_id, const std::string& path) {
    StMachins* sm = M(m);
    if (!sm || path.empty())
        return nullptr;
    StTree* tree = sm->GetTree(ma_id, cnt_id);
    if (!tree || !tree->TreeChilds)
        return nullptr;
    std::vector<std::string> parts;
    split_path(path, parts);
    if (parts.empty())
        return nullptr;
    return TC(tree->GetChildByName(&parts, 0));
}

int tree_get_child_count(Tree* t) {
    StTree* st = T(t);
    return st && st->TreeChilds ? static_cast<int>(st->TreeHead.childcount) : 0;
}
TreeChild* tree_get_child_at(Tree* t, int index) {
    StTree* st = T(t);
    if (!st || !st->TreeChilds || index < 0 || static_cast<unsigned>(index) >= st->TreeHead.childcount)
        return nullptr;
    return TC(&st->TreeChilds[index]);
}
std::string tree_child_get_name(TreeChild* c) {
    StTreeChild* sc = TC(c);
    return sc ? std::string(sc->TreeChHead.name) : std::string();
}
int tree_child_get_imgid(TreeChild* c) {
    StTreeChild* sc = TC(c);
    return sc ? static_cast<int>(sc->TreeChHead.imgid) : 0;
}
int tree_child_get_params_count(TreeChild* c) {
    StTreeChild* sc = TC(c);
    return sc ? static_cast<int>(sc->TreeChHead.paramcount) : 0;
}
int tree_child_get_children_count(TreeChild* c) {
    StTreeChild* sc = TC(c);
    return sc ? static_cast<int>(sc->TreeChHead.childcount) : 0;
}
TreeChild* tree_child_get_child_at(TreeChild* c, int index) {
    StTreeChild* sc = TC(c);
    if (!sc || !sc->TreeChilds || index < 0 || static_cast<unsigned>(index) >= sc->TreeChHead.childcount)
        return nullptr;
    return TC(&sc->TreeChilds[index]);
}

void params_list_control(Machins* m, unsigned char ma_id, unsigned short cnt_id, std::vector<ParamData>& out) {
    out.clear();
    if (!M(m))
        return;
    std::vector<SParamData> tmp;
    ::params_list_control(M(m), ma_id, cnt_id, tmp);
    out.resize(tmp.size());
    for (size_t i = 0; i < tmp.size(); i++)
        out[i] = *reinterpret_cast<ParamData*>(&tmp[i]);
}

void params_list_from_tree_child(Machins* m, unsigned char ma_id, unsigned short cnt_id, TreeChild* child, std::vector<ParamData>& out) {
    out.clear();
    if (!M(m) || !child)
        return;
    std::vector<SParamData> tmp;
    ::params_list_from_tree_child(M(m), ma_id, cnt_id, TC(child), tmp);
    out.resize(tmp.size());
    for (size_t i = 0; i < tmp.size(); i++)
        out[i] = *reinterpret_cast<ParamData*>(&tmp[i]);
}

void params_list_by_path(Machins* m, unsigned char ma_id, unsigned short cnt_id, const std::string& path, std::vector<ParamData>& out) {
    out.clear();
    if (!M(m))
        return;
    if (path.empty() || path == KlogicControlName) {
        params_list_control(m, ma_id, cnt_id, out);
        return;
    }
    TreeChild* child = tree_get_child_by_path(m, ma_id, cnt_id, path);
    if (child) {
        params_list_from_tree_child(m, ma_id, cnt_id, child, out);
    }
}

int param_get_value_type(Machins* m, const ParamData* d) {
    return ::param_get_value_type(M(m), PD(d));
}

int param_get_imgid(Machins* m, const ParamData* d) {
    if (!M(m) || !d)
        return 0;
    StMachins* sm = M(m);
    if (d->type == 0) {
        StDynParam dp;
        if (sm->GetDynParam(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp))
            return static_cast<int>(dp.imgid);
    } else if (d->type == 1) {
        StDynParamControl dp;
        if (sm->GetDynParamControl(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp))
            return static_cast<int>(dp.imgid);
    }
    return 0;
}

std::string param_get_name(Machins* m, const ParamData* d) {
    if (!M(m) || !d)
        return std::string();
    StMachins* sm = M(m);
    if (d->type == 0) {
        StDynParam dp;
        if (sm->GetDynParam(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp) && dp.name)
            return std::string(dp.name);
    } else if (d->type == 1) {
        StDynParamControl dp;
        if (sm->GetDynParamControl(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp) && dp.name)
            return std::string(dp.name);
    }
    return std::string();
}
std::string param_get_cipher(Machins* m, const ParamData* d) {
    if (!M(m) || !d)
        return std::string();
    StMachins* sm = M(m);
    if (d->type == 0) {
        StDynParam dp;
        if (sm->GetDynParam(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp) && dp.cipher)
            return std::string(dp.cipher);
    } else if (d->type == 1) {
        StDynParamControl dp;
        if (sm->GetDynParamControl(static_cast<unsigned char>(d->MaId), static_cast<unsigned short>(d->CntId), d->ParamId, &dp) && dp.cipher)
            return std::string(dp.cipher);
    }
    return std::string();
}

void param_data_to_passport(const ParamData* d, int value_type, void* passport_out) {
    ::param_data_to_passport(PD(d), value_type, static_cast<TPassport*>(passport_out));
}

bool passport_to_param_data(const void* passport_in, ParamData* d) {
    return ::passport_to_param_data(static_cast<const TPassport*>(passport_in), PD(d));
}

}
