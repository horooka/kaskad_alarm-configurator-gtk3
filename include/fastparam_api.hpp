#ifndef KASKAD_FASTPARAM_API_HPP
#define KASKAD_FASTPARAM_API_HPP

#include <string>

namespace fastparam {

struct Machins;
struct Stations;

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

}

#endif
