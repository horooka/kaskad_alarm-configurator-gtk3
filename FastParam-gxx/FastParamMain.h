#ifndef FastParamMainH
#define FastParamMainH

#include <string>
#include <vector>

namespace fastparam {

struct Machins;
struct Stations;
struct TreeChild;

// дескриптор параметра KLogic (дерево или служебный)
struct ParamData {
    unsigned char DirType;
    unsigned int MaId;
    unsigned int CntId;
    unsigned int ParamId;
    unsigned char type;  // 0 = параметр дерева, 1 = служебный
};

Stations* stations_get();
void stations_create(const std::string& path, bool one = false);
void stations_destroy();

std::string station_name(Stations* s, unsigned char id);
bool station_name_by_id(Stations* s, int id, std::string& out_name);
int station_id(Stations* s, const std::string& name);
int station_check_id(Stations* s, const std::string& name, int id);

Machins* machins_get();
void machins_create(const std::string& path, bool one = false);
void machins_destroy();

char machins_init_machine(Machins* m, unsigned char ma_id);
void machins_init_all(Machins* m);
int machins_get_use_count(Machins* m);
int machins_get_ma_id(Machins* m, unsigned char id);

struct Controllers;
struct Tree;
struct Cntrl;
struct CntrlIP;

Controllers* machins_get_controllers(Machins* m, unsigned char ma_id);
Tree* machins_get_tree(Machins* m, unsigned char ma_id, short unsigned int cnt_id);
Cntrl* machins_get_cntrl(Machins* m, unsigned char ma_id, short unsigned int cnt_id);
CntrlIP* machins_get_cntrl_ip(Machins* m, unsigned char ma_id, short unsigned int cnt_id);
char* machins_get_cntrl_ma_name(Machins* m, unsigned char ma_id, short unsigned int cnt_id);
int machins_get_cntrl_id_by_name(Machins* m, unsigned char ma_id, char* cname);
void* machins_get_ref_data(Machins* m, unsigned char ma_id, short unsigned int cnt_id);

int controllers_count(Controllers* c);
int controllers_id_by_index(Controllers* c, int index);
std::string cntrl_config_guid_str(Cntrl* c);
std::string cntrl_get_name(Cntrl* c);

// ----- обход дерева KLogic и список параметров -----

TreeChild* tree_get_child_by_path(Machins* m, unsigned char ma_id, unsigned short cnt_id, const std::string& path);

int tree_get_child_count(Tree* t);
TreeChild* tree_get_child_at(Tree* t, int index);

std::string tree_child_get_name(TreeChild* c);
int tree_child_get_imgid(TreeChild* c);
int tree_child_get_params_count(TreeChild* c);
int tree_child_get_children_count(TreeChild* c);
TreeChild* tree_child_get_child_at(TreeChild* c, int index);

void params_list_control(Machins* m, unsigned char ma_id, unsigned short cnt_id, std::vector<ParamData>& out);
void params_list_from_tree_child(Machins* m, unsigned char ma_id, unsigned short cnt_id, TreeChild* child, std::vector<ParamData>& out);
void params_list_by_path(Machins* m, unsigned char ma_id, unsigned short cnt_id, const std::string& path, std::vector<ParamData>& out);

int param_get_value_type(Machins* m, const ParamData* d);
int param_get_imgid(Machins* m, const ParamData* d);
std::string param_get_name(Machins* m, const ParamData* d);
std::string param_get_cipher(Machins* m, const ParamData* d);

void param_data_to_passport(const ParamData* d, int value_type, void* passport_out);
bool passport_to_param_data(const void* passport_in, ParamData* d);

}

#endif
