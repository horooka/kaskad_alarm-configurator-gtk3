#include <gtkmm.h>
#include <atomic>

const unsigned char SITUATIONPARAMTYPE_NONE = 0;
const unsigned char SITUATIONPARAMTYPE_JUMPFROM = 1;
const unsigned char SITUATIONPARAMTYPE_LESSTHAN = 2;
const unsigned char SITUATIONPARAMTYPE_GREATERTHAN = 3;
const unsigned char SITUATIONPARAMTYPE_DISCRETE = 4;
const unsigned char SITUATIONPARAMTYPE_ERRORCODE = 5;

const unsigned char ALARMOBJTYPE_OTHER = 0;
const unsigned char ALARMOBJTYPE_KLOGICPARAM = 1;
const unsigned char ALARMOBJTYPE_GROUPALARM = 2;
const unsigned char ALARMOBJTYPE_SERVICEPARAMS = 3;

const unsigned char KLOGICTYPE_FLOAT = 0;
const unsigned char KLOGICTYPE_BOOL = 1;
const unsigned char KLOGICTYPE_INT = 2;

struct AlarmRef {
    short unsigned int alarm_id;
    std::string userpath;
    short unsigned int passport_id;
    std::string alarm_string_id;
};

struct Passport {
    bool selected = false;
    short unsigned int station_id;
    unsigned char passport_type;
    short unsigned int group_id;
    short unsigned int passport_id;
    unsigned char value_type;
    std::string station_name;
    std::string passport_val_type_str;
    std::string chipher;
    std::string fullname;
    std::string passport_group;
    std::string measure_units;
    unsigned char type;
    unsigned char out;
};

struct SituationParams {
    std::string string1;
    std::string string2;
    std::string string3;
    bool bool1 = false;
    bool bool2 = false;
    bool bool3 = false;
    unsigned char type1;
    short unsigned int int1;
    short unsigned int int2;
    Passport passport;
    Glib::RefPtr<Gtk::ListStore> contacts;
};

// | Тип данных | PassportConfigType | KlogicConfigType
// | float | 1 | 0 |
// | bool | 2 | 1 |
// | int | 1 | 2 |

// Используется для станций и групп
class GroupCols : public Gtk::TreeModel::ColumnRecord {
    public:
        GroupCols() {
            add(icon);
            add(group_id);
            add(station_id);
            add(next_group_id);
            add(next_alarm_id);
            add(name);
            add(confname);
            add(use_own_settings);
            add(disable_alarm);
            add(checkback_on_startup);
            add(borders_from_passport);
            add(crashprecrash_passports_usage);
            add(crashprecrash_borders);
            add(device_mask);
            add(priority);
            add(block_by_passport_val);
            add(checkback_val);
            add(write_on_checkback);
            add(use_own_sound_in_group);
            add(passports);
            add(alarms);
            add(unique_situations_used);
            add(situations);
        }

        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<short unsigned int> group_id;
        Gtk::TreeModelColumn<short unsigned int> station_id;
        Gtk::TreeModelColumn<short unsigned int> next_group_id;
        Gtk::TreeModelColumn<short unsigned int> next_alarm_id;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> confname;
        Gtk::TreeModelColumn<bool> use_own_settings;
        Gtk::TreeModelColumn<bool> disable_alarm;
        Gtk::TreeModelColumn<bool> checkback_on_startup;
        Gtk::TreeModelColumn<bool> borders_from_passport;
        Gtk::TreeModelColumn<std::array<bool, 4>> crashprecrash_passports_usage;
        Gtk::TreeModelColumn<std::array<int, 4>> crashprecrash_borders;
        Gtk::TreeModelColumn<Glib::ustring> device_mask;
        Gtk::TreeModelColumn<unsigned char> priority;
        Gtk::TreeModelColumn<unsigned char> block_by_passport_val;
        Gtk::TreeModelColumn<unsigned char> checkback_val;
        Gtk::TreeModelColumn<bool> write_on_checkback;
        Gtk::TreeModelColumn<bool> use_own_sound_in_group;
        Gtk::TreeModelColumn<std::array<Passport, 7>> passports;
        Gtk::TreeModelColumn<Glib::RefPtr<Gtk::ListStore>> alarms;
        Gtk::TreeModelColumn<std::array<bool, 4>> unique_situations_used;
        Gtk::TreeModelColumn<Glib::RefPtr<Gtk::ListStore>> situations;
};

class AlarmObjCols : public Gtk::TreeModel::ColumnRecord {
    public:
        AlarmObjCols() {
            add(icon);
            add(alarmobj_type);
            add(userpath);
            add(passport_group);
            add(id);
            add(name);
            add(chipher);
            add(fullname);
            add(zone_name);
            add(station_id);
            add(station_name);
            add(group_id);
            add(passport_priority);
            add(passport_type);
            add(passport_group_id);
            add(passport_id);
            add(passport_val_type_str);
            add(measure_units);
            add(type);
            add(out);
            add(alarm_string_id);
            add(klogic_string_id);
        }

        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<unsigned char> alarmobj_type;
        Gtk::TreeModelColumn<short unsigned int> id;
        Gtk::TreeModelColumn<Glib::ustring> userpath;
        Gtk::TreeModelColumn<Glib::ustring> passport_group;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> chipher;
        Gtk::TreeModelColumn<Glib::ustring> fullname;
        Gtk::TreeModelColumn<Glib::ustring> zone_name;
        Gtk::TreeModelColumn<short unsigned int> station_id;
        Gtk::TreeModelColumn<Glib::ustring> station_name;
        Gtk::TreeModelColumn<short unsigned int> group_id;
        Gtk::TreeModelColumn<unsigned char> passport_priority;
        Gtk::TreeModelColumn<short unsigned int> passport_station_id;
        Gtk::TreeModelColumn<unsigned char> passport_type;
        Gtk::TreeModelColumn<short unsigned int> passport_group_id;
        Gtk::TreeModelColumn<short unsigned int> passport_id;
        Gtk::TreeModelColumn<Glib::ustring> passport_val_type_str;
        Gtk::TreeModelColumn<Glib::ustring> measure_units;
        Gtk::TreeModelColumn<unsigned char> type;
        Gtk::TreeModelColumn<bool> out;
        Gtk::TreeModelColumn<Glib::ustring> alarm_string_id;
        Gtk::TreeModelColumn<Glib::ustring> klogic_string_id;
};

class SoundCols : public Gtk::TreeModel::ColumnRecord {
    public:
        SoundCols() {
            add(icon);
            add(loop_start);
            add(name);
            add(filepath_or_pause_duration);
        }

        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<bool> loop_start;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> filepath_or_pause_duration;
};

class SituationCols : public Gtk::TreeModel::ColumnRecord {
    public:
        SituationCols() {
            add(is_enabled);
            add(situation_code);
            add(alarm_confirm_period);
            add(alarm_confirm_interval);
            add(name);
            add(alarm_kind);
            add(alarm_message);
            add(write_to_usergroup);
            add(usergroup_name);
            add(params_types);
            add(params_vals);
            add(sounds);
            add(checkboxes);
            add(params);
            add(intense_absolute);
            add(intense_up);
            add(intense_down);
        }

        Gtk::TreeModelColumn<bool> is_enabled;
        Gtk::TreeModelColumn<unsigned char> situation_code;
        Gtk::TreeModelColumn<int> alarm_confirm_period;
        Gtk::TreeModelColumn<unsigned char> alarm_confirm_interval;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<unsigned char> alarm_kind;
        Gtk::TreeModelColumn<Glib::ustring> alarm_message;
        Gtk::TreeModelColumn<bool> write_to_usergroup;
        Gtk::TreeModelColumn<Glib::ustring> usergroup_name;
        Gtk::TreeModelColumn<std::vector<unsigned char>> params_types;
        Gtk::TreeModelColumn<std::vector<float>> params_vals;
        Gtk::TreeModelColumn<Glib::RefPtr<Gtk::ListStore>> sounds;
        // Защелкивать ситуацию, писать события, гистерезис, SMS, e-mail, отслеживать ситуацию по паспорту, запись в паспорт при срабатывании
        Gtk::TreeModelColumn<std::array<bool, 7>> checkboxes;
        Gtk::TreeModelColumn<std::array<SituationParams, 4>> params;
        // Гистерезис
        Gtk::TreeModelColumn<bool> intense_absolute;
        Gtk::TreeModelColumn<double> intense_up;
        Gtk::TreeModelColumn<double> intense_down;
};

class UsergroupCols : public Gtk::TreeModel::ColumnRecord {
public:
    UsergroupCols() { add(description); }

    Gtk::TreeModelColumn<Glib::ustring> description;
};

class ContactCols : public Gtk::TreeModel::ColumnRecord {
    public:
        ContactCols() {
            add(icon);
            add(is_selected);
            add(is_group);
            add(name);
            add(comment);
            add(email);
            add(phone);
        }

        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<bool> is_selected;
        Gtk::TreeModelColumn<bool> is_group;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> comment;
        Gtk::TreeModelColumn<Glib::ustring> email;
        Gtk::TreeModelColumn<Glib::ustring> phone;
};

struct APSModuleSettings {
    unsigned int polling_period_msec = 1000;
    short unsigned int tcp_max_buffer_size_bytes = 2048;
    unsigned int connect_port = 22346;
    unsigned int receive_port = 22348;
    unsigned char receive_port_max = 255;
    bool fix_or_auto_checkback = true;
    unsigned int fix_timeout_sec = 0;
    unsigned int deblock_on_err_timeout_sec = 60;
    bool warn_by_default_sound = true;
    std::string warn_sound_file_path;
    bool cfg_one_copy = false;
    bool cfg_confirm_alarms_drag = false;
    bool cfg_save_alarms_xml = false;
};

struct MainSettings {
    int local_station_id;
    short unsigned int stations_amount;
    std::vector<APSModuleSettings> stations_aps_settings;
};

std::string cp1251_to_utf8(const std::string &s);
void render_masks(std::string &device_masks_text);
std::string format_float(float val);
void change_zone_params_view(Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
                             const AlarmObjCols &alarmobj_cols, bool chipher_or_description);
Gtk::TreeModel::Row get_klogicparam_row_by_string_id(
    const std::string &string_id,
    const Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
    const AlarmObjCols &alarmobj_cols);
bool copy_sound_to_sounds_dir(const std::string &sound_path,
                              const std::string &sound_dir,
                              std::string &errors);
void fill_passport(Passport &passport,
                   const Glib::RefPtr<Gtk::TreeStore> &treestore_zones, const Glib::RefPtr<Gtk::TreeStore> &treestore_groups,
                   const AlarmObjCols &alarmobj_cols, const GroupCols &group_cols);

bool parse_stations_config(const std::string &config_path, MainSettings &main_settings, Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const GroupCols &groups_cols, std::string &errors);
bool parse_alarms_configs(const std::string &alarms_dir_path, std::unordered_map<std::string, AlarmRef> &used_params, Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const std::array<Glib::RefPtr<Gdk::Pixbuf>, 5> &pixbufs_loop, const GroupCols &groups_cols, const AlarmObjCols &alarmobj_cols, const SituationCols &situation_cols, const SoundCols &sound_cols, const ContactCols &contact_cols, std::string &errors);
bool parse_alarms_config(const std::string &alarms_dir_path, const Gtk::TreeModel::Row &station_row, std::unordered_map<std::string, AlarmRef> &used_params, Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const std::array<Glib::RefPtr<Gdk::Pixbuf>, 5> &pixbufs_loop, const GroupCols &groups_cols, const AlarmObjCols &alarmobj_cols, const SituationCols &situation_cols, const SoundCols &sound_cols, const ContactCols &contact_cols, std::string &errors);
bool parse_stations_aps_settings(const std::string &alarms_dir_path, MainSettings &main_settings, const Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const GroupCols &group_cols, std::string &errors);
bool parse_controller_config(int cntrl_id, const std::string &controller_config_path, const Gtk::TreeModel::Children &zones_station_children, const Gtk::TreeModel::Row &station_row, Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const std::unordered_map<std::string, AlarmRef> &used_params,
                          Glib::RefPtr<Gtk::TreeStore> &treestore_zones, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_klogic,
                          const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_task, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_fb, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_service_params,
                          const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_discrete, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_analog, const std::array<Glib::RefPtr<Gdk::Pixbuf>, 6> &pixbufs_params,
                          const AlarmObjCols &alarmobj_cols, const GroupCols &group_cols, std::string &errors);
bool parse_klogic_configs(const std::string &alarms_dir_path, Glib::RefPtr<Gtk::TreeStore> &treestore_stations, const std::unordered_map<std::string, AlarmRef> &used_params,
                          Glib::RefPtr<Gtk::TreeStore> &treestore_zones, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_station, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_klogic,
                          const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_task, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_fb, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_service_params,
                          const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_discrete, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_analog, const std::array<Glib::RefPtr<Gdk::Pixbuf>, 6> &pixbufs_params,
                          const AlarmObjCols &alarmobj_cols, const GroupCols &group_cols, std::string &errors);
bool parse_contacts_config(const std::string &stations_config_path, Glib::RefPtr<Gtk::TreeStore> &treestore_contacts, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_contact, const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_contact_group, const ContactCols &contact_cols, std::string &errors);

int write_station_aps_settings(const std::string &alarms_dir_path, const APSModuleSettings &station_aps_settings, const Gtk::TreeModel::Row &station_row, const GroupCols &group_cols, std::string &errors);
int write_alarms_config(const std::string &alarms_dir_path, const Gtk::TreeModel::Row &station_row, const GroupCols &groups_cols, const AlarmObjCols &alarmobj_cols, const SituationCols &situation_cols, const SoundCols &sound_cols, const ContactCols &contact_cols, std::string &errors);
int write_contacts_config(const std::string &stations_config_path, const Glib::RefPtr<Gtk::TreeStore> &treestore_contacts, const ContactCols &contact_cols, std::string &errors);

std::string usergroup_description(const Glib::ustring &name);
std::vector<std::string> usergroup_names();
