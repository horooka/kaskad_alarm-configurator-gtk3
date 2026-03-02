#include "kaskad_alarm-configurator-gtk3/utils.hpp"
#include "FastParam-gxx/FastParamMain.h"
#include "tinyxml2/tinyxml2.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <glibmm.h>
#include <gtkmm.h>
#include <iostream>
#include <optional>
#include <regex>
#include <string.h>
#include <unordered_map>
#include <vector>

namespace {
const std::unordered_map<std::string, std::string> &
usergroup_description_map() {
    static const std::unordered_map<std::string, std::string> m = {
        {"ALARM_ALARMING", "Аварийные"},
        {"ALARM_WARNINGS", "Предупредительные"},
        {"ALARM_EVENTS", "Информационные"},
        {"ALARM_BLOCKED", "Выведенные из работы"},
        {"ALARM_CHECKBACK", "Квитированные"},
        {"KASKAD_CONTROL", "Управление параметрами"},
        {"KASKAD_DCONTROL", "Дискретное управление"},
        {"KASKAD_ACONTROL", "Аналоговое управление"},
        {"STARTSTOPGROUP", "СДД - запуск/останов"},
        {"KLOGICMNGR-IEC", "Библиотека KLogicMngr - ПУ МЭК"},
        {"DASERVERGROUP", "СДД - лог работы"},
        {"KREPORTS_FORM", "Рапорта"},
        {"EVENTMAIL", "Рассылка"},
        {"KLAUNCHER", "Центр управления"},
        {"CLIENT_NTP", "Синхронизация времени"},
        {"CONTROLLERSEVENTS", "Журнал событий контроллеров"},
        {"SDDMAINGROUP", "Конфигуратор СДД"},
        {"KLOGGERCFG", "Настройка БД ТП"},
        {"KASKAD_START_STOP", "Визуализация: Запуск/закрытие программы"},
        {"KASKAD_ADJUST", "Визуализация: Изменение настроек"},
        {"KASKAD_DEBUG", "Визуализация: Отладочные сообщения"},
        {"ALARM_OPEN", "Настройка алармов: Запуск/закрытие программы"},
        {"ALARM_CHANGES", "Настройка алармов: Изменение настроек"},
        {"ALARM_DEBUG", "Настройка алармов: Отладочные сообщения"},
        {"HISTORY_OPEN", "Предыстория: Открытие/закрытие проекта и панелей"},
        {"HISTORY_CHANGES", "Предыстория: Изменение настроек"},
        {"HISTORY_DEBUG", "Предыстория: Отладочные сообщения"},
        {"PASSPORT_OPEN", "Настройка паспортов: Запуск/закрытие программы"},
        {"PASSPORT_CHANGES", "Настройка паспортов: Изменение настроек"},
        {"PASSPORT_DEBUG", "Настройка паспортов: Отладочные сообщения"},
        {"KREPORTS_START_STOP", "Рапорта: Запуск/закрытие программы"},
        {"KREPORTS_ADJUST", "Рапорта: Изменение настроек"},
        {"KREPORTS_DEBUG", "Рапорта: Отладочные сообщения"},
        {"DISPATCHER", "Справочники и журналы"},
        {"DASRVAPI_LOG", "МОД DASrvAPI.dll"},
        {"RTP_DLL", "МОД rtp.dll"},
        {"RTP_DLL_LOADING", "МОД rtp.dll - дочитка БД"},
        {"ALARM_LOG", "МОД Alarm.dll"},
        {"KLOGGER", "МОД KLogger.dll"},
        {"KLOGGERACSSRV", "МОД KLoggerAcsSrv.dll"},
        {"RTP_ARCHIVER_LOG", "МОД RtpArchiver.dll"},
        {"LIB_KLOGIC", "МДД KLogic"},
        {"TIMESERVER_LOG", "МДД TimeServer"},
        {"DATALOGGER", "МОД DataLogger.dll"},
        {"DATALOGGER_LOADING", "МОД DataLogger.dll - дочитка БД"},
    };
    return m;
}
} // namespace

std::string usergroup_description(const Glib::ustring &name) {
    std::string key(static_cast<std::string>(name));
    const auto &m = usergroup_description_map();
    auto it = m.find(key);
    return it != m.end() ? it->second : "--группа не существует--";
}

std::vector<std::string> usergroup_names() {
    const auto &m = usergroup_description_map();
    std::vector<std::string> names;
    names.reserve(m.size());
    for (const auto &p : m)
        names.push_back(p.first);
    std::sort(names.begin(), names.end());
    return names;
}

int cp1251_to_utf8(char *out, const char *in, int buflen) {
    static const int table[128] = {
        0x82D0,   0x83D0,   0x9A80E2, 0x93D1,   0x9E80E2, 0xA680E2, 0xA080E2,
        0xA180E2, 0xAC82E2, 0xB080E2, 0x89D0,   0xB980E2, 0x8AD0,   0x8CD0,
        0x8BD0,   0x8FD0,   0x92D1,   0x9880E2, 0x9980E2, 0x9C80E2, 0x9D80E2,
        0xA280E2, 0x9380E2, 0x9480E2, 0,        0xA284E2, 0x99D1,   0xBA80E2,
        0x9AD1,   0x9CD1,   0x9BD1,   0x9FD1,   0xA0C2,   0x8ED0,   0x9ED1,
        0x88D0,   0xA4C2,   0x90D2,   0xA6C2,   0xA7C2,   0x81D0,   0xA9C2,
        0x84D0,   0xABC2,   0xACC2,   0xADC2,   0xAEC2,   0x87D0,   0xB0C2,
        0xB1C2,   0x86D0,   0x96D1,   0x91D2,   0xB5C2,   0xB6C2,   0xB7C2,
        0x91D1,   0x9684E2, 0x94D1,   0xBBC2,   0x98D1,   0x85D0,   0x95D1,
        0x97D1,   0x90D0,   0x91D0,   0x92D0,   0x93D0,   0x94D0,   0x95D0,
        0x96D0,   0x97D0,   0x98D0,   0x99D0,   0x9AD0,   0x9BD0,   0x9CD0,
        0x9DD0,   0x9ED0,   0x9FD0,   0xA0D0,   0xA1D0,   0xA2D0,   0xA3D0,
        0xA4D0,   0xA5D0,   0xA6D0,   0xA7D0,   0xA8D0,   0xA9D0,   0xAAD0,
        0xABD0,   0xACD0,   0xADD0,   0xAED0,   0xAFD0,   0xB0D0,   0xB1D0,
        0xB2D0,   0xB3D0,   0xB4D0,   0xB5D0,   0xB6D0,   0xB7D0,   0xB8D0,
        0xB9D0,   0xBAD0,   0xBBD0,   0xBCD0,   0xBDD0,   0xBED0,   0xBFD0,
        0x80D1,   0x81D1,   0x82D1,   0x83D1,   0x84D1,   0x85D1,   0x86D1,
        0x87D1,   0x88D1,   0x89D1,   0x8AD1,   0x8BD1,   0x8CD1,   0x8DD1,
        0x8ED1,   0x8FD1};

    char *pout = out;
    for (; *in && ((out - pout) < buflen - 1);) {
        if (*in & 0x80) {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        } else {
            *out++ = *in++;
        }
    }
    *out = 0;
    return (out - pout);
}

void cp1251_to_utf8(const std::string &s, std::string &out) {

    out.resize(s.length() * 2);

    int sz = cp1251_to_utf8(out.data(), s.c_str(), out.length());

    out.resize(sz);
}

std::string cp1251_to_utf8(const std::string &s) {
    std::string out;
    cp1251_to_utf8(s, out);
    return out;
}

Gtk::TreeModel::Row get_klogicparam_row_by_alarm_row(
    const Gtk::TreeModel::Row &alarm_row,
    const Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
    const AlarmObjCols &alarmobj_cols) {
    if (!alarm_row)
        return Gtk::TreeModel::Row();

    auto get_variableparam_row_by_alarm_row_recursive =
        [&](const auto &self, const Gtk::TreeModel::Row &curr_group_row,
            const Gtk::TreeModel::Row &alarm_row) -> Gtk::TreeModel::Row {
        for (const Gtk::TreeModel::Row &child_row : curr_group_row.children()) {
            if (child_row[alarmobj_cols.alarmobj_type] &
                ALARMOBJTYPE_KLOGICVARPARAM) {
                if (child_row[alarmobj_cols.id] == alarm_row[alarmobj_cols.id])
                    return child_row;
            } else {
                return self(self, child_row, alarm_row);
            }
        };
        return Gtk::TreeModel::Row();
    };

    Gtk::TreeModel::Row klogic_row = *treestore_zones->children().begin();
    for (const Gtk::TreeModel::Row &station_row : klogic_row.children()) {
        if (station_row[alarmobj_cols.station_id] !=
            alarm_row[alarmobj_cols.station_id])
            continue;
        for (const Gtk::TreeModel::Row &controller_row :
             station_row.children()) {
            if (controller_row[alarmobj_cols.passport_group_id] !=
                alarm_row[alarmobj_cols.passport_group_id])
                continue;
            for (const Gtk::TreeModel::Row &controller_child_row :
                 controller_row.children()) {
                if ((alarm_row[alarmobj_cols.alarmobj_type] &
                     ALARMOBJTYPE_KLOGICVARPARAM) &&
                    (controller_child_row[alarmobj_cols.alarmobj_type] &
                     ALARMOBJTYPE_VARPARAMS)) {
                    return get_variableparam_row_by_alarm_row_recursive(
                        get_variableparam_row_by_alarm_row_recursive,
                        controller_child_row, alarm_row);
                }
                if (controller_child_row[alarmobj_cols.alarmobj_type] &
                    ALARMOBJTYPE_SERVICEPARAMS) {
                    for (const Gtk::TreeModel::Row &param_row :
                         controller_child_row.children()) {
                        if (param_row[alarmobj_cols.passport_id] ==
                            alarm_row[alarmobj_cols.passport_id])
                            return param_row;
                    }
                }
                for (const Gtk::TreeModel::Row &fb_row :
                     controller_child_row.children()) {
                    for (const Gtk::TreeModel::Row &param_row :
                         fb_row.children()) {
                        if (param_row[alarmobj_cols.passport_id] ==
                            alarm_row[alarmobj_cols.passport_id])
                            return param_row;
                    }
                }
            }
        }
    }
    return Gtk::TreeModel::Row();
}

std::optional<UsedAlarmRef>
get_used_param(const std::vector<UsedAlarmRef> &used_params,
               short unsigned int station_id, unsigned char passport_type,
               short unsigned int group_id, short unsigned int passport_id) {
    for (const UsedAlarmRef &used_param : used_params) {
        if (used_param.passport.station_id == station_id &&
            used_param.passport.passport_type == passport_type &&
            used_param.passport.group_id == group_id &&
            used_param.passport.passport_id == passport_id)
            return used_param;
    }
    return std::nullopt;
}

void render_masks(std::string &device_masks_text) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char datetime[64];
    std::strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", &tm);

    // %pg
    static const std::regex re_pg(R"(%pg(\d+))");
    device_masks_text =
        std::regex_replace(device_masks_text, re_pg, "Пасп.Груп.$1");
    static const std::regex re_pg_neg(R"(%pg(-?\d+))");
    device_masks_text =
        std::regex_replace(device_masks_text, re_pg_neg, "Пасп.груп.$1");
    static const std::regex re_pg_only(R"(%pg\b)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_pg_only, "Пасп.Груп.-1");

    static const std::regex re_s(R"(%s)");
    device_masks_text = std::regex_replace(device_masks_text, re_s, "Станция");
    static const std::regex re_t(R"(%t)");
    device_masks_text = std::regex_replace(device_masks_text, re_t, "Тип");
    static const std::regex re_g(R"(%g)");
    device_masks_text = std::regex_replace(device_masks_text, re_g, "Группа");
    static const std::regex re_u(R"(%u)");
    device_masks_text = std::regex_replace(device_masks_text, re_u, "Ед.изм.");
    static const std::regex re_d(R"(%d)");
    device_masks_text = std::regex_replace(device_masks_text, re_d, datetime);
    static const std::regex re_v(R"(%v)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_v, "Тек.Знач.");
    static const std::regex re_i(R"(%i)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_i, "Ид. паспорта");
    static const std::regex re_c(R"(%c)");
    device_masks_text = std::regex_replace(device_masks_text, re_c, "Шифр");
    static const std::regex re_n(R"(%n)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_n, "Наименование");
    static const std::regex re_q(R"(%q)");
    device_masks_text = std::regex_replace(device_masks_text, re_q, "Ошибка");
    static const std::regex re_z(R"(%z)");
    device_masks_text = std::regex_replace(device_masks_text, re_z, "Зона");
    static const std::regex re_p(R"(%p)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_p, "Пред.Знач.");
    static const std::regex re_o(R"(%o)");
    device_masks_text =
        std::regex_replace(device_masks_text, re_o, "Оборудование");
}

std::string format_float(float val) {
    std::string str = std::to_string(val);
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        str = str.substr(0, str.find_last_not_of('0') + 1);
        if (str.back() == '.') {
            str.pop_back();
        }
    }
    return str;
}

std::string convert_alarm_to_klogic_string_id(const std::string &string_id) {
    std::stringstream ss(string_id);
    std::string token;
    std::vector<std::string> parts;
    std::getline(ss, token, '\\');
    while (std::getline(ss, token, '\\')) {
        if (token.empty())
            continue;
        if (std::regex_match(token, std::regex("^St(\\d+)$"))) {
            std::smatch match;
            if (std::regex_match(token, match, std::regex("^St(\\d+)$"))) {
                token = "Station_" + match[1].str();
            }
        } else if (std::regex_match(token, std::regex("^Cn(\\d+)$"))) {
            std::smatch match;
            if (std::regex_match(token, match, std::regex("^Cn(\\d+)$"))) {
                token = "Контроллер " + match[1].str();
            }
        }
        parts.push_back(token);
    }
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0)
            result += ".";
        result += parts[i];
    }
    return result;
}

std::string convert_klogic_to_alarm_string_id(const std::string &string_id) {
    std::stringstream ss(string_id);
    std::string token;
    std::vector<std::string> parts;
    std::getline(ss, token, '\\');
    parts.push_back("Kl");
    while (std::getline(ss, token, '\\')) {
        if (token.empty())
            continue;
        if (std::regex_match(token, std::regex("^Station_(\\d+)$"))) {
            std::smatch match;
            if (std::regex_match(token, match,
                                 std::regex("^Station_(\\d+)$"))) {
                token = "St" + match[1].str();
            }
        } else if (std::regex_match(token, std::regex("^Контроллер (\\d+)$"))) {
            std::smatch match;
            if (std::regex_match(token, match,
                                 std::regex("^Контроллер (\\d+)$"))) {
                token = "Cn" + match[1].str();
            }
        }
        parts.push_back(token);
    }
    std::string result;
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (i > 0)
            result += '\\';
        result += parts[i];
    }
    result += '\\';
    result += parts.back();
    return result;
}

std::string get_passport_val_type_str(unsigned char type, unsigned char out) {
    std::string passport_val_type_str;
    if (type == 1)
        passport_val_type_str = "Дискретный ";
    else
        passport_val_type_str = "Аналоговый ";
    if (out == 0)
        passport_val_type_str += "вход ";
    else
        passport_val_type_str += "выход ";
    if (type == 1)
        passport_val_type_str += "(Логический)";
    else if (type == 2)
        passport_val_type_str += "(Целочисленный)";
    else
        passport_val_type_str += "(Вещественный)";
    return passport_val_type_str;
}

std::string passport_group_from_string_id(const std::string &string_id) {
    size_t pos1 = string_id.rfind('.', string_id.size());
    if (pos1 == std::string::npos)
        return "";

    std::string last = string_id.substr(pos1 + 1);
    size_t pos2 = string_id.rfind('.', pos1 - 1);
    return (pos2 == std::string::npos)
               ? string_id.substr(pos1 - 1, string_id.size())
               : string_id.substr(pos2 + 1, string_id.size());
}

void change_zone_params_view(Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
                             const AlarmObjCols &alarmobj_cols,
                             bool chipher_or_description) {
    auto iterate_row_recursive = [&](const auto &self, Gtk::TreeModel::Row &row,
                                     bool chipher_or_description) -> void {
        for (Gtk::TreeModel::Row child_row : row.children()) {
            if (child_row[alarmobj_cols.alarmobj_type] &
                ALARMOBJTYPE_KLOGICPARAM) {
                Glib::ustring buf = chipher_or_description
                                        ? child_row[alarmobj_cols.fullname]
                                        : child_row[alarmobj_cols.chipher];
                child_row[alarmobj_cols.name] = buf;
            }
            self(self, child_row, chipher_or_description);
        }
    };

    for (Gtk::TreeModel::Row row : treestore_zones->children()) {
        iterate_row_recursive(iterate_row_recursive, row,
                              chipher_or_description);
    }
}

bool copy_sound_to_sounds_dir(const std::string &sound_path,
                              const std::string &sound_dir,
                              std::string &errors) {
    std::error_code ec;
    std::filesystem::path dest(
        sound_dir + "/" +
        std::filesystem::path(sound_path).filename().string());
    if (!std::filesystem::exists(sound_dir)) {
        std::filesystem::create_directories(dest, ec);
        if (ec) {
            errors += "Не удалось создать директорию \"" + sound_dir +
                      "\": " + ec.message() + '\n';
            return false;
        }
    }
    std::filesystem::copy_file(
        sound_path, dest, std::filesystem::copy_options::overwrite_existing,
        ec);
    if (ec) {
        errors += "Не удалось переместить файл по пути назначения \"" +
                  dest.string() + "\": " + ec.message() + '\n';
        return false;
    }
    return true;
}

std::string extract_pause(const std::string &s) {
    std::regex pattern_1(R"(<Pause>(\d+))");
    std::smatch match;
    if (std::regex_search(s, match, pattern_1) && match.size() > 1) {
        return match[1].str();
    }
    std::regex pattern_2(R"(Pause(\d+))");
    if (std::regex_search(s, match, pattern_2) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

bool parse_stations_config(const std::string &config_path,
                           MainSettings &main_settings,
                           Glib::RefPtr<Gtk::TreeStore> &treestore_groups,
                           const GroupCols &group_cols, std::string &errors) {
    errno = 0;
    std::ifstream ifs(config_path, std::ios::binary);
    int err = errno;
    if (!ifs.is_open() || err != 0) {
        errors += std::string("- Не удалось открыть файл конфигурации: ") +
                  strerror(err) + "\n";
        return false;
    }
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
    ifs.close();
    std::string raw_data(buffer.data(), buffer.size());
    std::string utf8_data;
    Glib::KeyFile key_file;
    if (g_utf8_validate(raw_data.c_str(), static_cast<gssize>(raw_data.size()),
                        nullptr)) {
        utf8_data = raw_data;
    } else {
        utf8_data = cp1251_to_utf8(raw_data);
    }
    ifs.close();
    try {
        key_file.load_from_data(utf8_data);
        if (key_file.has_key("Main", "LocalStationID")) {
            main_settings.local_station_id =
                key_file.get_integer("Main", "LocalStationID");
        }
    } catch (const Glib::Error &ex) {
        errors += "- Не удалось прочитать файл конфигурации \"" + config_path +
                  "\": " + ex.what() + "\n";
        return false;
    }

    auto append_station = [&](const std::string &group_name,
                              std::string & /*errors*/) {
        main_settings.stations_aps_settings.emplace_back();
        Gtk::TreeModel::Row new_row = *treestore_groups->append();
        new_row[group_cols.station_id] =
            key_file.has_key(group_name, "ID")
                ? key_file.get_integer(group_name, "ID")
                : main_settings.stations_amount + 1;
        new_row[group_cols.group_id] = 0;
        main_settings.stations_amount++;
        new_row[group_cols.next_group_id] = 0;
        new_row[group_cols.name] =
            key_file.has_key(group_name, "Name")
                ? Glib::ustring(key_file.get_string(group_name, "Name"))
                : Glib::ustring(group_name);
        new_row[group_cols.confname] = group_name;
        new_row[group_cols.use_own_settings] = true;
        new_row[group_cols.crashprecrash_borders] = {100, 0, 100, 0};
    };

    try {
        auto groups = key_file.get_groups();
        for (const auto &group : groups) {
            if (group.find("Station") == 0) {
                append_station(group, errors);
            }
        }
        key_file.save_to_file(config_path);
    } catch (const Glib::Error &ex) {
        errors += "- Ошибки чтения файла конфигурации \"" + config_path +
                  "\": " + ex.what() + "\n ";
        return ex.code();
    }
    return true;
}

std::string trim(const std::string &str) {
    const char *ws = " \t\r\n";
    std::string::size_type first = str.find_first_not_of(ws);
    if (std::string::npos == first)
        return "";
    std::string::size_type last = str.find_last_not_of(ws);
    return str.substr(first, (last - first + 1));
}

bool GetChildBool(tinyxml2::XMLElement *parent, const char *child_name) {
    if (!parent)
        return false;
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return false;
    const char *text = child->GetText();
    if (!text)
        return false;
    return text == std::string("true") || text == std::string("1");
}
int GetChildInt(tinyxml2::XMLElement *parent, const char *child_name) {
    if (!parent)
        return 0;
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return 0;
    const char *text = child->GetText();
    if (!text)
        return 0;
    try {
        return std::stoi(text);
    } catch (...) {
        return 0;
    };
}
double GetChildDouble(tinyxml2::XMLElement *parent, const char *child_name) {
    if (!parent)
        return 0;
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return 0;
    const char *text = child->GetText();
    if (!text)
        return 0;
    try {
        return std::stod(text);
    } catch (...) {
        return 0;
    };
}
short unsigned int GetChildSUInt(tinyxml2::XMLElement *parent,
                                 const char *child_name) {
    if (!parent)
        return 0;
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return 0;
    const char *text = child->GetText();
    if (!text)
        return 0;
    try {
        return std::stoi(text);
    } catch (...) {
        return 0;
    };
}
std::optional<int> GetChildOptInt(tinyxml2::XMLElement *parent,
                                  const char *child_name) {
    if (!parent)
        return std::nullopt;
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return std::nullopt;
    const char *text = child->GetText();
    if (!text)
        return std::nullopt;
    try {
        return std::stoi(text);
    } catch (...) {
        return std::nullopt;
    };
}
Glib::ustring GetChildText(tinyxml2::XMLElement *parent,
                           const char *child_name) {
    if (!parent)
        return "";
    tinyxml2::XMLElement *child = parent->FirstChildElement(child_name);
    if (!child)
        return "";
    const char *text = child->GetText();
    if (!text)
        return "";
    return Glib::ustring(text);
}
float GetElementFloat(tinyxml2::XMLElement *parent) {
    if (!parent)
        return 0;
    const char *text = parent->GetText();
    if (!text)
        return 0;
    try {
        return std::stof(text);
    } catch (...) {
        return 0;
    };
}

void fill_alarms_passport_recursive(
    short unsigned int passport_id, Passport &passport,
    const Glib::ustring station_name, std::string &passport_group_trace,
    const Gtk::TreeModel::Row &curr_group_row,
    const Glib::RefPtr<Gtk::TreeStore> &treestore_group,
    const GroupCols &group_cols) {
    if (passport.group_id == curr_group_row[group_cols.group_id]) {
        passport_group_trace += "/" + curr_group_row[group_cols.name];
        passport.station_name = station_name;
        passport.passport_val_type_str = "Аналоговый";
        passport.passport_group = passport_group_trace;
        switch (passport_id) {
        case 0:
            passport.chipher = "Новая_группа";
            passport.fullname = passport_group_trace;
            break;
        case 3000:
            passport.chipher = "Алармы";
            passport.fullname = "Общее количество сработавших алармов "
                                "(аварийных и предупредительных)";
            break;
        case 6000:
            passport.chipher = "Алармы активные";
            passport.fullname = "Общее количество активных алармов (аварийных "
                                "и предупредительных)";
            break;
        case 9000:
            passport.chipher = "Алармы квитированные";
            passport.fullname = "Общее количество квитированных алармов "
                                "(аварийных и предупредительных)";
            break;
        case 12000:
            passport.chipher = "Аварии";
            passport.fullname = "Количество сработавших аварий";
            break;
        case 15000:
            passport.chipher = "Аварии активные";
            passport.fullname = "Количество активных аварий";
            break;
        case 18000:
            passport.chipher = "Аварии квитированные";
            passport.fullname = "Количество квитированных аварий";
            break;
        case 21000:
            passport.chipher = "Предупреждения";
            passport.fullname = "Количество сработанных предупреждений";
            break;
        case 24000:
            passport.chipher = "Предупреждения активные";
            passport.fullname = "Количество активных предупреждений";
            break;
        case 27000:
            passport.chipher = "Предупреждения квитированные";
            passport.fullname = "Количество квитированных предупреждений";
            break;
        case 33000:
            passport.chipher = "Сообщения";
            passport.fullname = "Количество сообщений";
            break;
        case 36000:
            passport.chipher = "Алармы (Группа)";
            passport.fullname = "Общее количество групп сработавших алармов "
                                "(аварийных и предупредительных)";
            break;
        case 39000:
            passport.chipher = "Алармы активные (Группа)";
            passport.fullname = "Общее количество групп активных алармов "
                                "(аварийных и предупредительных)";
            break;
        case 42000:
            passport.chipher = "Алармы квитированные (Группа)";
            passport.fullname = "Общее количество групп квитированных алармов "
                                "(аварийных и предупредительных)";
            break;
        case 45000:
            passport.chipher = "Аварии (Группа)";
            passport.fullname = "Количество групп сработавших аварий";
            break;
        case 48000:
            passport.chipher = "Аварии активные (Группа)";
            passport.fullname = "Количество групп активных аварий";
            break;
        case 51000:
            passport.chipher = "Аварии квитированные (Группа)";
            passport.fullname = "Количество групп квитированных аварий";
            break;
        case 54000:
            passport.chipher = "Предупреждения (Группа)";
            passport.fullname = "Количество групп сработанных предупреждений";
            break;
        case 57000:
            passport.chipher = "Предупреждения активные (Группа)";
            passport.fullname = "Количество групп активных предупреждений";
            break;
        case 60000:
            passport.chipher = "Предупреждения квитированные (Группа)";
            passport.fullname = "Количество групп квитированных предупреждений";
            break;
        case 63000:
            passport.chipher = "Сообщения (Группа)";
            passport.fullname = "Количество групп сообщений";
            break;
        }
        return;
    }
    for (const Gtk::TreeModel::Row &group_row : curr_group_row.children()) {
        fill_alarms_passport_recursive(passport_id, passport, station_name,
                                       passport_group_trace, group_row,
                                       treestore_group, group_cols);
    }
}

Passport prepare_passport(tinyxml2::XMLElement *parent,
                          const std::string &passport_elem_name) {
    if (!parent)
        return Passport();
    tinyxml2::XMLElement *passport_elem =
        parent->FirstChildElement(passport_elem_name.c_str());
    if (!passport_elem)
        return Passport();
    return Passport{
        true,
        GetChildSUInt(passport_elem, "StationID"),
        static_cast<unsigned char>(
            GetChildSUInt(passport_elem, "PassportType")),
        GetChildSUInt(passport_elem, "GroupID"),
        GetChildSUInt(passport_elem, "PassportID"),
        static_cast<unsigned char>(GetChildSUInt(passport_elem, "ValueType")),
        "",
        "",
        "",
        "",
        "",
        "",
        0,
        0,
    };
}

void fill_passport(Passport &passport,
                   const Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
                   const Glib::RefPtr<Gtk::TreeStore> &treestore_group,
                   const AlarmObjCols &alarmobj_cols,
                   const GroupCols &group_cols) {
    const Gtk::TreeModel::Row klogic_row = *treestore_zones->children().begin();
    if (!klogic_row)
        return;
    short unsigned int passport_type = passport.passport_type;
    for (const Gtk::TreeModel::Row &station_row : klogic_row.children()) {
        if (passport.station_id != station_row[alarmobj_cols.station_id] ||
            !station_row.children())
            continue;
        if (passport_type == PASSPORTTYPE_ALARM) {
            for (const Gtk::TreeModel::Row &group_station_row :
                 treestore_group->children()) {
                if (passport.station_id !=
                    group_station_row[group_cols.station_id])
                    continue;
                std::string passport_group_trace =
                    "Алармы/" + group_station_row[group_cols.name];
                fill_alarms_passport_recursive(
                    passport.passport_id - passport.group_id, passport,
                    group_station_row[group_cols.name], passport_group_trace,
                    group_station_row, treestore_group, group_cols);
            }
            return;
        }
        for (const Gtk::TreeModel::Row &controller_row :
             station_row.children()) {
            short unsigned int passport_id = passport.passport_id;
            if (passport.group_id !=
                    controller_row[alarmobj_cols.passport_group_id] ||
                !controller_row.children())
                continue;

            auto fill_from_param_row =
                [&](const Gtk::TreeModel::Row &param_row) {
                    passport.station_name = static_cast<Glib::ustring>(
                        station_row[alarmobj_cols.name]);
                    passport.passport_val_type_str = static_cast<Glib::ustring>(
                        param_row[alarmobj_cols.passport_val_type_str]);
                    passport.chipher = static_cast<Glib::ustring>(
                        param_row[alarmobj_cols.chipher]);
                    passport.fullname = static_cast<Glib::ustring>(
                        param_row[alarmobj_cols.fullname]);
                    passport.passport_group = static_cast<Glib::ustring>(
                        param_row[alarmobj_cols.passport_group]);
                    passport.measure_units = static_cast<Glib::ustring>(
                        param_row[alarmobj_cols.measure_units]);
                    passport.out = param_row[alarmobj_cols.out];
                    passport.type = param_row[alarmobj_cols.type];
                };

            auto get_varparam_row_by_passport_id_recursive =
                [&](const auto &self, const Gtk::TreeModel::Row &curr_group_row)
                -> Gtk::TreeModel::Row {
                for (const Gtk::TreeModel::Row &child_row :
                     curr_group_row.children()) {
                    if (child_row[alarmobj_cols.alarmobj_type] &
                        ALARMOBJTYPE_KLOGICVARPARAM) {
                        if (child_row[alarmobj_cols.passport_id] == passport_id)
                            return child_row;
                    } else {
                        Gtk::TreeModel::Row found = self(self, child_row);
                        if (found)
                            return found;
                    }
                }
                return Gtk::TreeModel::Row();
            };

            for (const Gtk::TreeModel::Row &controller_child_row :
                 controller_row.children()) {
                if (controller_child_row[alarmobj_cols.alarmobj_type] &
                    ALARMOBJTYPE_VARPARAMS) {
                    Gtk::TreeModel::Row param_row =
                        get_varparam_row_by_passport_id_recursive(
                            get_varparam_row_by_passport_id_recursive,
                            controller_child_row);
                    if (param_row) {
                        fill_from_param_row(param_row);
                        return;
                    }
                }
                if ((controller_child_row[alarmobj_cols.alarmobj_type] &
                     ALARMOBJTYPE_SERVICEPARAMS) &&
                    passport_type == PASSPORTTYPE_SERVICE &&
                    controller_child_row.children()) {
                    for (const Gtk::TreeModel::Row &param_row :
                         controller_child_row.children()) {
                        if (param_row[alarmobj_cols.passport_id] != passport_id)
                            continue;
                        fill_from_param_row(param_row);
                        return;
                    }
                }
                if (passport_type == PASSPORTTYPE_USERDEF &&
                    controller_child_row.children()) {
                    for (const Gtk::TreeModel::Row &param_row :
                         controller_child_row.children()) {
                        if (param_row[alarmobj_cols.passport_id] != passport_id)
                            continue;
                        fill_from_param_row(param_row);
                        return;
                    }
                }
            }
        }
    }
}

bool parse_alarms_config(
    const std::string &alarms_dir_path, const Gtk::TreeModel::Row &station_row,
    std::vector<UsedAlarmRef> &used_params,
    Glib::RefPtr<Gtk::TreeStore> &treestore_groups,
    const std::array<Glib::RefPtr<Gdk::Pixbuf>, 5> &pixbufs_loop,
    const GroupCols &group_cols, const AlarmObjCols &alarmobj_cols,
    const SituationCols &situation_cols, const SoundCols &sound_cols,
    const ContactCols &contact_cols, std::string &errors) {
    auto parse_group_recursive =
        [&](const auto &self, tinyxml2::XMLElement *curr_elem,
            const Gtk::TreeModel::Row &group_row, short unsigned int station_id,
            int max_group_id) -> int {
        int group_id = GetChildInt(curr_elem, "GroupID");
        group_row[group_cols.group_id] = group_id;
        if (group_id > max_group_id)
            max_group_id = group_id;
        group_row[group_cols.station_id] = station_id;
        group_row[group_cols.name] = GetChildText(curr_elem, "GroupName");

        tinyxml2::XMLElement *children =
            curr_elem->FirstChildElement("Children");
        if (children) {
            for (tinyxml2::XMLElement *group_elem =
                     children->FirstChildElement("GroupItem");
                 group_elem != nullptr;
                 group_elem = group_elem->NextSiblingElement("GroupItem")) {
                Gtk::TreeModel::Row new_row =
                    *treestore_groups->append(group_row.children());
                int children_max_group_id =
                    self(self, group_elem, new_row, station_id, max_group_id);
                if (children_max_group_id > max_group_id)
                    max_group_id = children_max_group_id;
            }
        }

        tinyxml2::XMLElement *alarms_elem =
            curr_elem->FirstChildElement("Alarms");
        if (alarms_elem) {
            Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                Gtk::ListStore::create(alarmobj_cols);
            group_row[group_cols.alarms] = liststore_alarms;
            for (tinyxml2::XMLElement *alarm_elem =
                     alarms_elem->FirstChildElement("Alarm");
                 alarm_elem != nullptr;
                 alarm_elem = alarm_elem->NextSiblingElement("Alarm")) {
                std::string alarm_string_id =
                    GetChildText(alarm_elem, "StringID");
                if (alarm_string_id.empty())
                    continue;
                tinyxml2::XMLElement *alarm_passport_elem =
                    alarm_elem->FirstChildElement("Passport");
                UsedAlarmRef alarmref = {
                    GetChildSUInt(alarm_elem, "ID"),
                    GetChildText(alarm_elem, "MeasureUnits"), alarm_string_id,
                    treestore_groups->get_path(*group_row).to_string(),
                    Passport{
                        true, GetChildSUInt(alarm_passport_elem, "StationID"),
                        static_cast<unsigned char>(
                            GetChildSUInt(alarm_passport_elem, "PassportType")),
                        GetChildSUInt(alarm_passport_elem, "GroupID"),
                        GetChildSUInt(alarm_passport_elem, "PassportID"),
                        static_cast<unsigned char>(
                            GetChildSUInt(alarm_passport_elem, "ValueType")),
                        "", "", "", "", "", "", 0, 0}};
                used_params.push_back(alarmref);
            }
        }

        tinyxml2::XMLElement *default_alarm_elem =
            curr_elem->FirstChildElement("DefaultAlarm");
        if (!default_alarm_elem) {
            group_row[group_cols.situations] =
                Gtk::ListStore::create(situation_cols);
        }
        tinyxml2::XMLElement *default_alarm_child_elem =
            default_alarm_elem->FirstChildElement("Alarm");
        group_row[group_cols.use_own_settings] =
            GetChildBool(default_alarm_child_elem, "UseOwnSettings");
        std::array<int, 4> crashprecrash_borders{100, 0, 100, 0};
        std::array<bool, 4> crashprecrash_passports_usage{false, false, false,
                                                          false};
        std::array<Passport, 7> passports = group_row[group_cols.passports];
        tinyxml2::XMLElement *up_crashing_border_elem =
            default_alarm_child_elem->FirstChildElement("UpCrashingBorder");
        if (up_crashing_border_elem) {
            tinyxml2::XMLElement *extavalue_elem =
                up_crashing_border_elem->FirstChildElement("ExtAValue");
            if (extavalue_elem) {
                auto up_precrashing_border =
                    GetChildOptInt(extavalue_elem, "Value");
                if (up_precrashing_border)
                    crashprecrash_borders[0] = *up_precrashing_border;
                passports[0] = prepare_passport(extavalue_elem, "Passport");
                crashprecrash_passports_usage[0] = passports[0].selected;
            }
        } else
            crashprecrash_borders[0] = 100;
        tinyxml2::XMLElement *down_crashing_border_elem =
            default_alarm_child_elem->FirstChildElement("DownCrashingBorder");
        if (down_crashing_border_elem) {
            tinyxml2::XMLElement *extavalue_elem =
                down_crashing_border_elem->FirstChildElement("ExtAValue");
            if (extavalue_elem) {
                auto down_crashing_border =
                    GetChildOptInt(extavalue_elem, "Value");
                if (down_crashing_border)
                    crashprecrash_borders[1] = *down_crashing_border;
                passports[1] = prepare_passport(extavalue_elem, "Passport");
                crashprecrash_passports_usage[1] = passports[1].selected;
            }
        }
        tinyxml2::XMLElement *up_precrashing_border_elem =
            default_alarm_child_elem->FirstChildElement("UpPreCrashingBorder");
        if (up_precrashing_border_elem) {
            tinyxml2::XMLElement *extavalue_elem =
                up_precrashing_border_elem->FirstChildElement("ExtAValue");
            if (extavalue_elem) {
                auto up_precrashing_border =
                    GetChildOptInt(extavalue_elem, "Value");
                if (up_precrashing_border)
                    crashprecrash_borders[2] = *up_precrashing_border;
                passports[2] = prepare_passport(extavalue_elem, "Passport");
                crashprecrash_passports_usage[2] = passports[2].selected;
            }
        }
        tinyxml2::XMLElement *down_precrashing_border_elem =
            default_alarm_child_elem->FirstChildElement(
                "DownPreCrashingBorder");
        if (down_precrashing_border_elem) {
            tinyxml2::XMLElement *extavalue_elem =
                down_precrashing_border_elem->FirstChildElement("ExtAValue");
            if (extavalue_elem) {
                auto down_precrashing_border =
                    GetChildOptInt(extavalue_elem, "Value");
                if (down_precrashing_border)
                    crashprecrash_borders[3] = *down_precrashing_border;
                passports[3] = prepare_passport(extavalue_elem, "Passport");
                crashprecrash_passports_usage[3] = passports[3].selected;
            }
        }
        group_row[group_cols.crashprecrash_passports_usage] =
            crashprecrash_passports_usage;
        group_row[group_cols.crashprecrash_borders] = crashprecrash_borders;
        group_row[group_cols.disable_alarm] =
            GetChildBool(default_alarm_child_elem, "DisableAlarm");
        group_row[group_cols.checkback_on_startup] =
            GetChildBool(default_alarm_child_elem, "CheckBackOnStartup");
        group_row[group_cols.borders_from_passport] =
            GetChildBool(default_alarm_child_elem, "BordersFromPassport");
        group_row[group_cols.device_mask] =
            GetChildText(default_alarm_child_elem, "DeviceMask");
        group_row[group_cols.priority] =
            GetChildInt(default_alarm_child_elem, "Priority");
        group_row[group_cols.block_by_passport_val] =
            GetChildInt(default_alarm_child_elem, "BlockByPasspValue");
        passports[4] =
            prepare_passport(default_alarm_child_elem, "BlockPassport");
        group_row[group_cols.checkback_val] =
            GetChildInt(default_alarm_child_elem, "CheckBackValue");
        passports[5] =
            prepare_passport(default_alarm_child_elem, "CheckBackPassport");
        group_row[group_cols.write_on_checkback] =
            GetChildBool(default_alarm_child_elem, "WriteOnCheckBack");
        passports[6] = prepare_passport(default_alarm_child_elem,
                                        "PassportToWriteOnCheckBack");
        group_row[group_cols.passports] = passports;

        group_row[group_cols.use_own_sound_in_group] =
            GetChildBool(default_alarm_child_elem, "UseOwnSoundInGroup");

        tinyxml2::XMLElement *situations_elem =
            default_alarm_child_elem->FirstChildElement("Situations");
        if (!situations_elem)
            return max_group_id;
        Glib::RefPtr<Gtk::ListStore> liststore_situations =
            Gtk::ListStore::create(situation_cols);
        for (tinyxml2::XMLElement *situation_elem =
                 situations_elem->FirstChildElement("Situation");
             situation_elem != nullptr;
             situation_elem = situation_elem->NextSiblingElement("Situation")) {
            Gtk::TreeModel::Row new_situation_row =
                *liststore_situations->append();
            new_situation_row[situation_cols.sounds] =
                Gtk::ListStore::create(sound_cols);
            // Параметры
            bool is_enabled = GetChildBool(situation_elem, "Enabled");
            new_situation_row[situation_cols.is_enabled] = is_enabled;
            int situation_code = GetChildInt(situation_elem, "SituationCode");
            new_situation_row[situation_cols.situation_code] = situation_code;
            int alarm_confirm_period =
                GetChildInt(situation_elem, "AlarmConfirmPeriod");
            new_situation_row[situation_cols.alarm_confirm_period] =
                alarm_confirm_period;
            int alarm_confirm_interval =
                GetChildInt(situation_elem, "AlarmConfirmInterval");
            new_situation_row[situation_cols.alarm_confirm_interval] =
                alarm_confirm_interval;
            std::string situation_name;
            bool disable_hysteresis = false;
            switch (situation_code) {
            case 0:
                situation_name = "Норма";
                disable_hysteresis = true;
                break;
            case 1:
                situation_name = "Верхняя аварийная уставка";
                break;
            case 2:
                situation_name = "Верхняя предаварийная уставка";
                break;
            case 3:
                situation_name = "Нижняя аварийная уставка";
                break;
            case 4:
                situation_name = "Нижняя предаварийная уставка";
                break;
            case 5:
                situation_name = "Меньше значения";
                break;
            case 6:
                situation_name = "Больше значения";
                break;
            case 7:
                situation_name = "Значение в диапазоне";
                break;
            case 8:
                situation_name = "Дискретное значение";
                disable_hysteresis = true;
                break;
            case 9:
                situation_name = "Ошибка";
                disable_hysteresis = true;
                break;
            case 10:
                situation_name = "Переход";
                break;
            default:
                errors += "Неизвестный тип ситуации: " +
                          std::to_string(situation_code) + "\n";
                continue;
            }
            new_situation_row[situation_cols.name] = situation_name;
            new_situation_row[situation_cols.alarm_message] =
                GetChildText(situation_elem, "AlarmMessage");
            new_situation_row[situation_cols.write_to_usergroup] =
                GetChildBool(situation_elem, "WriteToUserGroup");
            new_situation_row[situation_cols.usergroup_name] =
                GetChildText(situation_elem, "UserGroupName");
            std::optional<int> insense_definition =
                GetChildOptInt(situation_elem, "InsenseDefinition");
            if (!insense_definition)
                new_situation_row[situation_cols.insense_absolute] = true;
            new_situation_row[situation_cols.insense_up] =
                GetChildDouble(situation_elem, "UpInsense");
            new_situation_row[situation_cols.insense_down] =
                GetChildDouble(situation_elem, "DownInsense");
            new_situation_row[situation_cols.alarm_kind] =
                GetChildInt(situation_elem, "AlarmKind");
            std::array<bool, 8> new_checkboxes = {
                GetChildBool(situation_elem, "FixAlarming"),
                GetChildBool(situation_elem, "WriteToEvents"),
                !disable_hysteresis,
                false,
                false,
                false,
                false,
                false};
            tinyxml2::XMLElement *situation_params_elem =
                situation_elem->FirstChildElement("SituationParams");
            if (!situation_params_elem)
                continue;
            std::vector<unsigned char> params_types;
            std::vector<float> params_vals;
            for (tinyxml2::XMLElement *param_elem =
                     situation_params_elem->FirstChildElement();
                 param_elem != nullptr;
                 param_elem = param_elem->NextSiblingElement()) {
                std::string param_name = param_elem->Name();
                unsigned char param_type = 0;
                if (param_name == "JumpFrom")
                    param_type = SITUATIONPARAMTYPE_JUMPFROM;
                else if (param_name == "LessThen")
                    param_type = SITUATIONPARAMTYPE_LESSTHAN;
                else if (param_name == "MoreThen")
                    param_type = SITUATIONPARAMTYPE_GREATERTHAN;
                else if (param_name == "Discrete")
                    param_type = SITUATIONPARAMTYPE_DISCRETE;
                else if (param_name == "ErrorCode")
                    param_type = SITUATIONPARAMTYPE_ERRORCODE;
                else {
                    errors +=
                        std::string("- Неизвестный тип параметра станции: ") +
                        param_name + "\n";
                    continue;
                }
                params_types.push_back(param_type);
                float param_val = GetElementFloat(param_elem);
                params_vals.push_back(param_val);
            }
            new_situation_row[situation_cols.params_types] = params_types;
            new_situation_row[situation_cols.params_vals] = params_vals;
            // Звук
            tinyxml2::XMLElement *soundfiles_elem =
                situation_elem->FirstChildElement("SoundFiles");
            if (soundfiles_elem) {
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Gtk::ListStore::create(sound_cols);
                std::string soundfiles_dir_path =
                    std::filesystem::path(alarms_dir_path)
                        .parent_path()
                        .string() +
                    "/SoundFiles";
                std::string cycled_idx =
                    GetChildText(situation_elem, "CycledIndex");
                int loop_idx = INT_MAX;
                if (!cycled_idx.empty())
                    loop_idx = std::stoi(cycled_idx);
                int sound_idx = 0;
                for (tinyxml2::XMLElement *soundfile_elem =
                         soundfiles_elem->FirstChildElement("File");
                     soundfile_elem != nullptr;
                     soundfile_elem =
                         soundfile_elem->NextSiblingElement("File")) {
                    std::string soundfile_filename =
                        static_cast<Glib::ustring>(soundfile_elem->GetText());
                    if (soundfile_filename.empty())
                        continue;
                    std::string filepath_or_pause_duration = "";
                    std::string pause_duration =
                        extract_pause(soundfile_filename);
                    if (pause_duration.empty()) {
                        filepath_or_pause_duration =
                            soundfiles_dir_path + "/" + soundfile_filename;
                        if (!std::filesystem::exists(soundfiles_dir_path + "/" +
                                                     soundfile_filename)) {
                            errors += "Файл \"" + soundfile_filename +
                                      "\" не существует в директории: " +
                                      soundfiles_dir_path + "\n";
                        }
                    } else {
                        filepath_or_pause_duration = pause_duration;
                    }
                    Gtk::TreeModel::Row soundfile_row =
                        *liststore_sounds->append();
                    soundfile_row[sound_cols.loop_start] = false;
                    if (loop_idx == INT_MAX || sound_idx < loop_idx) {
                        soundfile_row[sound_cols.icon] = pixbufs_loop[0];
                    } else if (sound_idx == loop_idx) {
                        if (!soundfile_elem->NextSiblingElement("File"))
                            soundfile_row[sound_cols.icon] = pixbufs_loop[1];
                        else
                            soundfile_row[sound_cols.icon] = pixbufs_loop[2];
                        soundfile_row[sound_cols.loop_start] = true;
                    } else if (sound_idx > loop_idx) {
                        if (!soundfile_elem->NextSiblingElement("File"))
                            soundfile_row[sound_cols.icon] = pixbufs_loop[4];
                        else
                            soundfile_row[sound_cols.icon] = pixbufs_loop[3];
                    }
                    soundfile_row[sound_cols.name] = soundfile_filename;
                    soundfile_row[sound_cols.filepath_or_pause_duration] =
                        filepath_or_pause_duration;
                    sound_idx++;
                }
                new_situation_row[situation_cols.sounds] = liststore_sounds;
            }
            std::array<SituationParams, 4> situation_params;
            // Пасспорт
            tinyxml2::XMLElement *passport_elem =
                situation_elem->FirstChildElement("ForcePassport");
            if (passport_elem) {
                SituationParams passport_params;
                passport_params.passport.selected = true;
                passport_params.passport.station_id =
                    GetChildSUInt(passport_elem, "StationID");
                passport_params.passport.passport_type =
                    GetChildSUInt(passport_elem, "PassportType");
                passport_params.passport.group_id =
                    GetChildSUInt(passport_elem, "GroupID");
                passport_params.passport.passport_id =
                    GetChildSUInt(passport_elem, "PassportID");
                passport_params.passport.value_type =
                    GetChildSUInt(passport_elem, "ValueType");
                passport_params.bool1 =
                    GetChildBool(passport_elem, "FForceByPassport");
                new_checkboxes[5] = true;
                situation_params[2] = passport_params;
            }
            // Запись
            tinyxml2::XMLElement *write_elem =
                situation_elem->FirstChildElement("WritePassport");
            if (write_elem) {
                SituationParams write_params;
                write_params.passport.selected = true;
                write_params.passport.station_id =
                    GetChildSUInt(write_elem, "StationID");
                write_params.passport.passport_type =
                    GetChildInt(write_elem, "PassportType");
                write_params.passport.group_id =
                    GetChildSUInt(write_elem, "GroupID");
                write_params.passport.passport_id =
                    GetChildSUInt(write_elem, "PassportID");
                write_params.passport.value_type =
                    GetChildInt(write_elem, "ValueType");
                write_params.bool1 =
                    GetChildBool(write_elem, "FWriteByPassport");
                new_checkboxes[6] = true;
                situation_params[3] = write_params;
            }
            // SMS
            tinyxml2::XMLElement *sms_elem =
                situation_elem->FirstChildElement("SMSOptions");
            if (sms_elem) {
                SituationParams sms_params;
                Glib::RefPtr<Gtk::ListStore> contacts =
                    Gtk::ListStore::create(contact_cols);
                sms_params.type1 = GetChildInt(sms_elem, "Priority");
                sms_params.int1 = GetChildInt(sms_elem, "LifeTime");
                sms_params.int2 = GetChildInt(sms_elem, "NoSendTime");
                sms_params.bool2 = GetChildBool(sms_elem, "NoSend");
                sms_params.string1 = GetChildText(sms_elem, "TextSMS");
                sms_params.bool1 = GetChildBool(sms_elem, "UseDefaultText");
                tinyxml2::XMLElement *telephones_elem =
                    sms_elem->FirstChildElement("Telephones");
                tinyxml2::XMLElement *comments_elem =
                    sms_elem->FirstChildElement("Comments");
                tinyxml2::XMLElement *telephone_elem =
                    telephones_elem
                        ? telephones_elem->FirstChildElement("Telephone")
                        : nullptr;
                tinyxml2::XMLElement *comment_elem =
                    comments_elem ? comments_elem->FirstChildElement("Comment")
                                  : nullptr;
                while (telephone_elem != nullptr || comment_elem != nullptr) {
                    Gtk::TreeModel::Row contact_row = *contacts->append();
                    contact_row[contact_cols.is_group] = false;
                    if (telephone_elem) {
                        const char *text = telephone_elem->GetText();
                        contact_row[contact_cols.phone] =
                            text ? Glib::ustring(text) : Glib::ustring();
                        telephone_elem =
                            telephone_elem->NextSiblingElement("Telephone");
                    }
                    if (comment_elem) {
                        const char *text = comment_elem->GetText();
                        contact_row[contact_cols.name] =
                            text ? Glib::ustring(text) : Glib::ustring();
                        comment_elem =
                            comment_elem->NextSiblingElement("Comment");
                    }
                }
                sms_params.contacts = contacts;
                new_checkboxes[3] = true;
                situation_params[0] = sms_params;
            } else {
                SituationParams sms_params;
                sms_params.contacts = Gtk::ListStore::create(contact_cols);
                situation_params[0] = sms_params;
            }
            // e-mail
            tinyxml2::XMLElement *email_elem =
                situation_elem->FirstChildElement("EMailOptions");
            if (email_elem) {
                SituationParams email_params;
                Glib::RefPtr<Gtk::ListStore> contacts =
                    Gtk::ListStore::create(contact_cols);
                email_params.type1 = GetChildInt(email_elem, "Priority");
                email_params.int1 = GetChildInt(email_elem, "LifeTime");
                email_params.int2 = GetChildInt(email_elem, "NoSendTime");
                email_params.string1 = GetChildText(email_elem, "Subject");
                email_params.bool1 = GetChildBool(email_elem, "UseDefaultSubj");
                email_params.string2 = GetChildText(email_elem, "MessMail");
                email_params.bool2 = GetChildBool(email_elem, "UseDefaultText");
                email_params.bool3 = GetChildBool(email_elem, "NoSend");
                tinyxml2::XMLElement *addresses_elem =
                    email_elem->FirstChildElement("Addresses");
                tinyxml2::XMLElement *comments_elem =
                    email_elem->FirstChildElement("Comments");
                tinyxml2::XMLElement *address_elem =
                    addresses_elem
                        ? addresses_elem->FirstChildElement("Address")
                        : nullptr;
                tinyxml2::XMLElement *comment_elem =
                    comments_elem ? comments_elem->FirstChildElement("Comment")
                                  : nullptr;
                while (address_elem != nullptr || comment_elem != nullptr) {
                    Gtk::TreeModel::Row contact_row = *contacts->append();
                    contact_row[contact_cols.is_group] = false;
                    if (address_elem) {
                        const char *text = address_elem->GetText();
                        contact_row[contact_cols.email] =
                            text ? Glib::ustring(text) : Glib::ustring();
                        address_elem =
                            address_elem->NextSiblingElement("Address");
                    }
                    if (comment_elem) {
                        const char *text = comment_elem->GetText();
                        contact_row[contact_cols.name] =
                            text ? Glib::ustring(text) : Glib::ustring();
                        comment_elem =
                            comment_elem->NextSiblingElement("Comment");
                    }
                }
                email_params.contacts = contacts;
                new_checkboxes[4] = true;
                situation_params[1] = email_params;
            } else {
                SituationParams email_params;
                email_params.contacts = Gtk::ListStore::create(contact_cols);
                situation_params[1] = email_params;
            }
            // Запускать программу
            tinyxml2::XMLElement *cmd_elem =
                situation_elem->FirstChildElement("CommandLine");
            if (cmd_elem) {
                new_situation_row[situation_cols.launch_program_file] =
                    GetChildText(cmd_elem, "RunProgram");
                new_situation_row[situation_cols.launch_program_params] =
                    GetChildText(cmd_elem, "Params");
                new_checkboxes[7] = true;
            }
            new_situation_row[situation_cols.checkboxes] = new_checkboxes;
            new_situation_row[situation_cols.params] = situation_params;
        }
        group_row[group_cols.situations] = liststore_situations;
        return max_group_id;
    };

    std::string station_confname =
        static_cast<Glib::ustring>(station_row[group_cols.confname]);
    std::string station_alarms_path =
        alarms_dir_path + "/" + station_confname + "/Alarms.xml";

    errno = 0;
    std::ifstream ifs(station_alarms_path);
    int err = errno;
    if (!ifs.is_open() || err != 0) {
        errors += std::string("- Не удалось открыть файл конфигурации ") +
                  station_alarms_path + ": " + strerror(err) + "\n";
        return false;
    }
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
    ifs.close();
    std::string raw_data(buffer.data(), buffer.size());
    std::string utf8_data;
    if (g_utf8_validate(raw_data.c_str(), static_cast<gssize>(raw_data.size()),
                        nullptr)) {
        utf8_data = raw_data;
    } else {
        utf8_data = cp1251_to_utf8(raw_data);
    }
    tinyxml2::XMLDocument config;
    tinyxml2::XMLError result =
        config.Parse(utf8_data.c_str(), utf8_data.size());
    if (result != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось прочитать файл конфигурации ") +
                  station_alarms_path + ": " + config.ErrorStr() + "\n";
        return false;
    }

    tinyxml2::XMLElement *root = config.FirstChildElement("Alarms")
                                     ->FirstChildElement("Root")
                                     ->FirstChildElement("GroupItem");
    station_row[group_cols.next_group_id] =
        parse_group_recursive(parse_group_recursive, root, station_row,
                              station_row[group_cols.station_id], 0) +
        1;
    return true;
}

bool parse_alarms_configs(
    const std::string &alarms_dir_path, std::vector<UsedAlarmRef> &used_params,
    Glib::RefPtr<Gtk::TreeStore> &treestore_groups,
    const std::array<Glib::RefPtr<Gdk::Pixbuf>, 5> &pixbufs_loop,
    const GroupCols &group_cols, const AlarmObjCols &alarmobj_cols,
    const SituationCols &situation_cols, const SoundCols &sound_cols,
    const ContactCols &contact_cols, std::string &errors) {
    if (!std::filesystem::exists(alarms_dir_path)) {
        errors +=
            "- Не удалось найти директорию алармов: " + alarms_dir_path + "\n";
        return false;
    }

    for (const Gtk::TreeModel::Row &station_row :
         treestore_groups->children()) {
        (void)parse_alarms_config(alarms_dir_path, station_row, used_params,
                                  treestore_groups, pixbufs_loop, group_cols,
                                  alarmobj_cols, situation_cols, sound_cols,
                                  contact_cols, errors);
    }

    return errors.empty();
}

int write_alarms_config(const std::string &alarms_dir_path,
                        const Gtk::TreeModel::Row &station_row,
                        const GroupCols &group_cols,
                        const AlarmObjCols &alarmobj_cols,
                        const SituationCols &situation_cols,
                        const SoundCols &sound_cols,
                        const ContactCols &contact_cols, std::string &errors) {
    if (!std::filesystem::exists(alarms_dir_path)) {
        errors +=
            "- Не удалось найти директорию алармов: " + alarms_dir_path + "\n";
        return false;
    }

    std::string station_confname =
        static_cast<Glib::ustring>(station_row[group_cols.confname]);
    std::string station_alarms_path =
        alarms_dir_path + "/" + station_confname + "/Alarms.xml";

    std::filesystem::path config_p(station_alarms_path.c_str());
    if (!std::filesystem::exists(config_p.parent_path())) {
        std::error_code ec;
        std::filesystem::create_directory(config_p.parent_path(), ec);
        if (ec) {
            errors += "- " + ec.message() + "\n";
            return ec.value();
        }
    }
    errno = 0;
    std::ofstream ofs(station_alarms_path, std::ios::trunc);
    int err = errno;
    if (!ofs.is_open() || err != 0) {
        errors +=
            std::string("- Не удалось открыть файл конфигурации для записи: ") +
            station_alarms_path + ": " + strerror(err) + "\n";
    }

    tinyxml2::XMLDocument config;
    auto append_elem = [&config](tinyxml2::XMLElement *parent,
                                 const std::string &name) {
        tinyxml2::XMLElement *node = config.NewElement(name.c_str());
        parent->InsertEndChild(node);
        return node;
    };
    auto append_val_elem = [&config](tinyxml2::XMLElement *parent,
                                     const std::string &name,
                                     const std::string &val) {
        tinyxml2::XMLElement *node = config.NewElement(name.c_str());
        node->SetText(val.c_str());
        parent->InsertEndChild(node);
        return node;
    };
    auto append_passport_elem = [&config, append_elem, append_val_elem](
                                    tinyxml2::XMLElement *passport_parent,
                                    std::string element_name,
                                    Passport passport) {
        tinyxml2::XMLElement *passport_elem =
            append_elem(passport_parent, element_name);
        append_val_elem(passport_elem, "StationID",
                        std::to_string(passport.station_id));
        append_val_elem(passport_elem, "PassportType",
                        std::to_string(passport.passport_type));
        append_val_elem(passport_elem, "GroupID",
                        std::to_string(passport.group_id));
        append_val_elem(passport_elem, "PassportID",
                        std::to_string(passport.passport_id));
        append_val_elem(passport_elem, "ValueType",
                        passport.type == 1 ? "2" : "1");
    };

    auto write_group_recursive = [&](const auto &self,
                                     const Gtk::TreeModel::Row &group_row,
                                     tinyxml2::XMLElement *group_item) -> void {
        append_val_elem(group_item, "GroupID",
                        std::to_string(group_row[group_cols.group_id]).c_str());
        append_val_elem(
            group_item, "GroupName",
            static_cast<Glib::ustring>(group_row[group_cols.name]).c_str());
        tinyxml2::XMLElement *children_elem =
            append_elem(group_item, "Children");
        tinyxml2::XMLElement *alarms_elem = append_elem(group_item, "Alarms");
        Glib::RefPtr<Gtk::ListStore> alarms = group_row[group_cols.alarms];
        for (const Gtk::TreeModel::Row &alarm_row : alarms->children()) {
            tinyxml2::XMLElement *alarm_elem =
                append_elem(alarms_elem, "Alarm");
            append_val_elem(
                alarm_elem, "ID",
                std::to_string(alarm_row[alarmobj_cols.id]).c_str());
            append_val_elem(
                alarm_elem, "Name",
                static_cast<Glib::ustring>(alarm_row[alarmobj_cols.name])
                    .c_str());
            append_val_elem(
                alarm_elem, "FullName",
                static_cast<Glib::ustring>(alarm_row[alarmobj_cols.fullname])
                    .c_str());
            Glib::ustring measure_units =
                alarm_row[alarmobj_cols.measure_units];
            if (!measure_units.empty())
                append_val_elem(alarm_elem, "MeasureUnits",
                                measure_units.c_str());
            append_val_elem(
                alarm_elem, "ZoneName",
                static_cast<Glib::ustring>(alarm_row[alarmobj_cols.zone_name])
                    .c_str());
            append_val_elem(alarm_elem, "StationName",
                            static_cast<Glib::ustring>(
                                alarm_row[alarmobj_cols.station_name])
                                .c_str());
            tinyxml2::XMLElement *alarm_passport_elem =
                append_elem(alarm_elem, "Passport");
            append_val_elem(
                alarm_passport_elem, "StationID",
                std::to_string(alarm_row[alarmobj_cols.station_id]).c_str());
            append_val_elem(
                alarm_passport_elem, "PassportType",
                std::to_string(alarm_row[alarmobj_cols.passport_type]).c_str());
            append_val_elem(
                alarm_passport_elem, "GroupID",
                std::to_string(alarm_row[alarmobj_cols.passport_group_id])
                    .c_str());
            append_val_elem(
                alarm_passport_elem, "PassportID",
                std::to_string(alarm_row[alarmobj_cols.passport_id]).c_str());
            unsigned char type = alarm_row[alarmobj_cols.type];
            append_val_elem(alarm_passport_elem, "ValueType",
                            type == 1 ? "2" : "1");
            append_val_elem(alarm_elem, "StringID",
                            static_cast<Glib::ustring>(
                                alarm_row[alarmobj_cols.alarm_string_id])
                                .c_str());
        }

        tinyxml2::XMLElement *default_alarm_elem =
            append_elem(group_item, "DefaultAlarm");
        tinyxml2::XMLElement *default_alarm_child_elem =
            append_elem(default_alarm_elem, "Alarm");
        append_val_elem(default_alarm_child_elem, "ID", "-1");
        append_val_elem(default_alarm_child_elem, "UseOwnSettings",
                        group_row[group_cols.use_own_settings] ? "true"
                                                               : "false");
        std::array<Passport, 7> passports = group_row[group_cols.passports];
        if (group_row[group_cols.borders_from_passport])
            append_val_elem(default_alarm_child_elem, "BordersFromPassport",
                            "true");
        else {
            std::array<int, 4> crashprecrash_borders =
                group_row[group_cols.crashprecrash_borders];
            if (crashprecrash_borders[0] != 100 || passports[0].selected) {
                tinyxml2::XMLElement *up_crashing_border_elem =
                    append_elem(default_alarm_child_elem, "UpCrashingBorder");
                tinyxml2::XMLElement *extavalue_elem =
                    append_elem(up_crashing_border_elem, "ExtAValue");
                append_val_elem(
                    extavalue_elem, "Value",
                    std::to_string(crashprecrash_borders[0]).c_str());
                if (passports[0].selected)
                    append_passport_elem(extavalue_elem, "Passport",
                                         passports[0]);
            }
            if (crashprecrash_borders[1] != 0 || passports[1].selected) {
                tinyxml2::XMLElement *down_crashing_border_elem =
                    append_elem(default_alarm_child_elem, "DownCrashingBorder");
                tinyxml2::XMLElement *extavalue_elem =
                    append_elem(down_crashing_border_elem, "ExtAValue");
                append_val_elem(
                    extavalue_elem, "Value",
                    std::to_string(crashprecrash_borders[1]).c_str());
                if (passports[1].selected)
                    append_passport_elem(extavalue_elem, "Passport",
                                         passports[1]);
            }
            if (crashprecrash_borders[2] != 100 || passports[2].selected) {
                tinyxml2::XMLElement *up_precrashing_border_elem = append_elem(
                    default_alarm_child_elem, "UpPreCrashingBorder");
                tinyxml2::XMLElement *extavalue_elem =
                    append_elem(up_precrashing_border_elem, "ExtAValue");
                append_val_elem(
                    extavalue_elem, "Value",
                    std::to_string(crashprecrash_borders[2]).c_str());
                if (passports[2].selected)
                    append_passport_elem(extavalue_elem, "Passport",
                                         passports[2]);
            }
            if (crashprecrash_borders[3] != 0 || passports[3].selected) {
                tinyxml2::XMLElement *down_precrashing_border_elem =
                    append_elem(default_alarm_child_elem,
                                "DownPreCrashingBorder");
                tinyxml2::XMLElement *extavalue_elem =
                    append_elem(down_precrashing_border_elem, "ExtAValue");
                append_val_elem(
                    extavalue_elem, "Value",
                    std::to_string(crashprecrash_borders[3]).c_str());
                if (passports[3].selected)
                    append_passport_elem(extavalue_elem, "Passport",
                                         passports[3]);
            }
        }
        append_val_elem(
            default_alarm_child_elem, "DeviceMask",
            static_cast<Glib::ustring>(group_row[group_cols.device_mask])
                .c_str());
        if (group_row[group_cols.checkback_on_startup])
            append_val_elem(default_alarm_child_elem, "CheckBackOnStartup",
                            "true");
        if (group_row[group_cols.disable_alarm])
            append_val_elem(default_alarm_child_elem, "DisableAlarm", "true");
        append_val_elem(default_alarm_child_elem, "Priority",
                        std::to_string(group_row[group_cols.priority]).c_str());
        Passport block_passport = passports[4];
        if (block_passport.selected)
            append_passport_elem(default_alarm_child_elem, "BlockPassport",
                                 block_passport);
        unsigned char block_by_passport_val =
            group_row[group_cols.block_by_passport_val];
        if (block_by_passport_val != 0)
            append_val_elem(default_alarm_child_elem, "BlockByPasspValue",
                            std::to_string(block_by_passport_val).c_str());
        Passport checkback_passport = passports[5];
        if (checkback_passport.selected)
            append_passport_elem(default_alarm_child_elem, "CheckBackPassport",
                                 checkback_passport);
        unsigned char checkback_val = group_row[group_cols.checkback_val];
        if (checkback_val != 0)
            append_val_elem(default_alarm_child_elem, "CheckBackValue",
                            std::to_string(checkback_val).c_str());
        Passport write_on_checkback_passport = passports[6];
        if (write_on_checkback_passport.selected)
            append_passport_elem(default_alarm_child_elem,
                                 "PassportToWriteOnCheckBack",
                                 write_on_checkback_passport);
        append_val_elem(default_alarm_child_elem, "WriteOnCheckBack",
                        group_row[group_cols.write_on_checkback] ? "true"
                                                                 : "false");
        if (group_row[group_cols.use_own_sound_in_group])
            append_val_elem(default_alarm_child_elem, "UseOwnSoundInGroup",
                            "true");
        Glib::RefPtr<Gtk::ListStore> liststore_situations =
            group_row[group_cols.situations];
        if (liststore_situations->children().size() > 0) {
            tinyxml2::XMLElement *situations_elem =
                append_elem(default_alarm_child_elem, "Situations");
            for (const Gtk::TreeModel::Row &situation_row :
                 liststore_situations->children()) {
                tinyxml2::XMLElement *situation_elem =
                    append_elem(situations_elem, "Situation");
                append_val_elem(situation_elem, "Enabled",
                                situation_row[situation_cols.is_enabled]
                                    ? "true"
                                    : "false");
                append_val_elem(
                    situation_elem, "SituationCode",
                    std::to_string(situation_row[situation_cols.situation_code])
                        .c_str());
                int alarm_confirm_period =
                    situation_row[situation_cols.alarm_confirm_period];
                if (alarm_confirm_period > 0)
                    append_val_elem(
                        situation_elem, "AlarmConfirmPeriod",
                        std::to_string(
                            situation_row[situation_cols
                                              .alarm_confirm_period]));
                int alarm_confirm_interval =
                    situation_row[situation_cols.alarm_confirm_interval];
                if (alarm_confirm_interval > 0)
                    append_val_elem(
                        situation_elem, "AlarmConfirmInterval",
                        std::to_string(
                            situation_row[situation_cols
                                              .alarm_confirm_interval]));
                append_val_elem(
                    situation_elem, "AlarmMessage",
                    static_cast<Glib::ustring>(
                        situation_row[situation_cols.alarm_message]));
                if (situation_row[situation_cols.write_to_usergroup])
                    append_val_elem(situation_elem, "WriteToUserGroup", "true");
                append_val_elem(
                    situation_elem, "UserGroupName",
                    static_cast<Glib::ustring>(
                        situation_row[situation_cols.usergroup_name]));
                std::array<bool, 8> checkboxes =
                    situation_row[situation_cols.checkboxes];
                if (checkboxes[2]) {
                    if (!situation_row[situation_cols.insense_absolute])
                        append_val_elem(situation_elem, "InsenseDefinition",
                                        "0");
                    if (situation_row[situation_cols.insense_up] != 0)
                        append_val_elem(
                            situation_elem, "UpInsense",
                            std::to_string(
                                situation_row[situation_cols.insense_up]));
                    if (situation_row[situation_cols.insense_down] != 0)
                        append_val_elem(
                            situation_elem, "DownInsense",
                            std::to_string(
                                situation_row[situation_cols.insense_down]));
                }
                if (checkboxes[0])
                    append_val_elem(situation_elem, "FixAlarming", "true");
                append_val_elem(
                    situation_elem, "AlarmKind",
                    std::to_string(situation_row[situation_cols.alarm_kind]));
                if (checkboxes[1])
                    append_val_elem(situation_elem, "WriteToEvents", "true");
                std::array<SituationParams, 4> situation_params =
                    situation_row[situation_cols.params];
                // Пасспорт
                if (checkboxes[5])
                    append_val_elem(situation_elem, "FForceByPassport", "true");
                SituationParams passport_params = situation_params[2];
                if (passport_params.passport.selected)
                    append_passport_elem(situation_elem, "ForcePassport",
                                         passport_params.passport);
                // Запись
                if (checkboxes[6])
                    append_val_elem(situation_elem, "FWriteByPassport", "true");
                SituationParams write_params = situation_params[3];
                if (write_params.passport.selected)
                    append_passport_elem(situation_elem, "WritePassport",
                                         write_params.passport);
                // Параметры
                std::vector<unsigned char> params_types =
                    situation_row[situation_cols.params_types];
                if (params_types.size() == 0)
                    continue;
                std::vector<float> params_vals =
                    situation_row[situation_cols.params_vals];
                tinyxml2::XMLElement *situation_params_elem =
                    append_elem(situation_elem, "SituationParams");
                for (unsigned int i = 0; i < params_types.size(); i++) {
                    std::string param_name;
                    switch (params_types[i]) {
                    case SITUATIONPARAMTYPE_JUMPFROM:
                        param_name = "JumpFrom";
                        break;
                    case SITUATIONPARAMTYPE_LESSTHAN:
                        param_name = "LessThen";
                        break;
                    case SITUATIONPARAMTYPE_GREATERTHAN:
                        param_name = "MoreThen";
                        break;
                    case SITUATIONPARAMTYPE_DISCRETE:
                        param_name = "Discrete";
                        break;
                    case SITUATIONPARAMTYPE_ERRORCODE:
                        param_name = "ErrorCode";
                        break;
                    }
                    append_val_elem(situation_params_elem, param_name.c_str(),
                                    format_float(params_vals[i]).c_str());
                }
                // Звук
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    situation_row[situation_cols.sounds];
                if (liststore_sounds->children().size() > 0) {
                    tinyxml2::XMLElement *soundfiles_elem =
                        append_elem(situation_elem, "SoundFiles");
                    for (const Gtk::TreeModel::Row &soundfile_row :
                         liststore_sounds->children()) {
                        append_val_elem(soundfiles_elem, "File",
                                        static_cast<Glib::ustring>(
                                            soundfile_row[sound_cols.name])
                                            .c_str());
                    }
                }
                // SMS
                if (checkboxes[3]) {
                    append_val_elem(situation_elem, "SendSMS", "true");
                    tinyxml2::XMLElement *sms_elem =
                        append_elem(situation_elem, "SMSOptions");
                    append_val_elem(
                        sms_elem, "Priority",
                        std::to_string(situation_params[0].type1).c_str());
                    append_val_elem(
                        sms_elem, "LifeTime",
                        std::to_string(situation_params[0].int1).c_str());
                    bool use_default_text = situation_params[0].bool1;
                    if (use_default_text)
                        append_val_elem(sms_elem, "TextSMS",
                                        situation_params[0].string1.c_str());
                    append_val_elem(sms_elem, "UseDefaultText",
                                    use_default_text ? "true" : "false");
                    Glib::RefPtr<Gtk::ListStore> sms_contacts =
                        situation_params[0].contacts;
                    if (sms_contacts && sms_contacts->children().size() > 0) {
                        tinyxml2::XMLElement *telephones_elem =
                            append_elem(sms_elem, "Telephones");
                        tinyxml2::XMLElement *comments_elem =
                            append_elem(sms_elem, "Comments");
                        for (const Gtk::TreeModel::Row &contact_row :
                             sms_contacts->children()) {
                            append_val_elem(telephones_elem, "Telephone",
                                            static_cast<Glib::ustring>(
                                                contact_row[contact_cols.phone])
                                                .c_str());
                            append_val_elem(comments_elem, "Comment",
                                            static_cast<Glib::ustring>(
                                                contact_row[contact_cols.name])
                                                .c_str());
                        }
                    }
                    append_val_elem(sms_elem, "NoSend",
                                    situation_params[0].bool2 ? "true"
                                                              : "false");
                    append_val_elem(
                        sms_elem, "NoSendTime",
                        std::to_string(situation_params[0].int2).c_str());
                }
                // e-mail
                if (checkboxes[4]) {
                    append_val_elem(situation_elem, "SendEMail", "true");
                    tinyxml2::XMLElement *email_elem =
                        append_elem(situation_elem, "EMailOptions");
                    append_val_elem(
                        email_elem, "Priority",
                        std::to_string(situation_params[1].type1).c_str());
                    append_val_elem(
                        email_elem, "LifeTime",
                        std::to_string(situation_params[1].int1).c_str());
                    bool use_default_text = situation_params[1].bool2;
                    if (use_default_text)
                        append_val_elem(email_elem, "MessMail",
                                        situation_params[1].string2.c_str());
                    append_val_elem(email_elem, "UseDefaultText",
                                    use_default_text ? "true" : "false");
                    bool use_default_subj = situation_params[1].bool1;
                    if (use_default_subj)
                        append_val_elem(email_elem, "Subject",
                                        situation_params[1].string1.c_str());
                    append_val_elem(email_elem, "UseDefaultSubj",
                                    use_default_subj ? "true" : "false");
                    append_val_elem(email_elem, "NoSend",
                                    situation_params[1].bool3 ? "true"
                                                              : "false");
                    append_val_elem(
                        email_elem, "NoSendTime",
                        std::to_string(situation_params[1].int2).c_str());
                    Glib::RefPtr<Gtk::ListStore> email_contacts =
                        situation_params[1].contacts;
                    if (email_contacts &&
                        email_contacts->children().size() > 0) {
                        tinyxml2::XMLElement *addresses_elem =
                            append_elem(email_elem, "Addresses");
                        tinyxml2::XMLElement *comments_elem =
                            append_elem(email_elem, "Comments");
                        for (const Gtk::TreeModel::Row &contact_row :
                             email_contacts->children()) {
                            append_val_elem(addresses_elem, "Address",
                                            static_cast<Glib::ustring>(
                                                contact_row[contact_cols.email])
                                                .c_str());
                            append_val_elem(comments_elem, "Comment",
                                            static_cast<Glib::ustring>(
                                                contact_row[contact_cols.name])
                                                .c_str());
                        }
                    }
                }
                // Запускать программу
                bool launch_program = checkboxes[7];
                if (launch_program) {
                    tinyxml2::XMLElement *launch_program_elem =
                        append_elem(situation_elem, "CommandLine");
                    append_val_elem(
                        launch_program_elem, "RunProgram",
                        static_cast<Glib::ustring>(
                            situation_row[situation_cols.launch_program_file])
                            .c_str());
                    append_val_elem(
                        launch_program_elem, "Params",
                        static_cast<Glib::ustring>(
                            situation_row[situation_cols.launch_program_params])
                            .c_str());
                }
            }
        }

        for (const Gtk::TreeModel::Row &child_group_row :
             group_row.children()) {
            tinyxml2::XMLElement *group_item =
                append_elem(children_elem, "GroupItem");
            self(self, child_group_row, group_item);
        }
    };

    tinyxml2::XMLElement *alarms = config.NewElement("Alarms");
    config.InsertFirstChild(alarms);
    tinyxml2::XMLElement *root = append_elem(alarms, "Root");
    tinyxml2::XMLElement *group_item = append_elem(root, "GroupItem");
    write_group_recursive(write_group_recursive, station_row, group_item);

    tinyxml2::XMLError save_res = config.SaveFile(station_alarms_path.c_str());
    if (save_res != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось сохранить файл конфигурации: ") +
                  config.ErrorStr() + "\n";
        return save_res;
    }
    int ret = config.ErrorID();
    if (ret != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось сохранить файл конфигурации: ") +
                  config.ErrorStr() + "\n";
        return ret;
    }
    return ret;
}

bool parse_stations_aps_settings(
    const std::string &alarms_dir_path, MainSettings &main_settings,
    const Glib::RefPtr<Gtk::TreeStore> &treestore_stations,
    const GroupCols &group_cols, std::string &errors) {
    unsigned int station_idx = 0;
    for (const Gtk::TreeModel::Row &station_row :
         treestore_stations->children()) {
        bool is_failed = false;
        std::string confname =
            static_cast<Glib::ustring>(station_row[group_cols.confname]);
        std::string station_aps_settings_path =
            alarms_dir_path + "/" + confname + "/Config.xml";

        errno = 0;
        std::ifstream ifs(station_aps_settings_path);
        int err = errno;
        if (!ifs.is_open() || err != 0) {
            errors += std::string("- Не удалось открыть файл конфигурации ") +
                      station_aps_settings_path + ": " + strerror(err) + "\n";
            is_failed = true;
            continue;
        }
        std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());
        ifs.close();
        std::string raw_data(buffer.data(), buffer.size());
        std::string utf8_data;
        if (g_utf8_validate(raw_data.c_str(),
                            static_cast<gssize>(raw_data.size()), nullptr)) {
            utf8_data = raw_data;
        } else {
            utf8_data = cp1251_to_utf8(raw_data);
        }
        tinyxml2::XMLDocument config;
        tinyxml2::XMLError result =
            config.Parse(utf8_data.c_str(), utf8_data.size());
        if (result != tinyxml2::XML_SUCCESS && !is_failed) {
            errors += std::string("- Не удалось прочитать файл конфигурации ") +
                      station_aps_settings_path + ": " + config.ErrorStr() +
                      "\n";
            is_failed = true;
            continue;
        }

        APSModuleSettings &aps_settings =
            main_settings.stations_aps_settings[station_idx];
        tinyxml2::XMLElement *root = config.FirstChildElement("AlarmConfig");
        if (!root)
            return false;
        tinyxml2::XMLElement *settings = root->FirstChildElement("Settings");
        if (!settings)
            return false;
        aps_settings.polling_period_msec =
            GetChildInt(settings, "PollingPeriod");
        aps_settings.tcp_max_buffer_size_bytes =
            GetChildInt(settings, "MaxBufferSize");
        aps_settings.fix_or_auto_checkback =
            GetChildBool(settings, "FixOrAutoChBck");
        aps_settings.fix_timeout_sec = GetChildInt(settings, "FixTimeOut");
        aps_settings.deblock_on_err_timeout_sec =
            GetChildInt(settings, "DeBlockOnErrorTimeout");
        aps_settings.warn_by_default_sound =
            GetChildBool(settings, "WarnByDefaultSound");
        aps_settings.warn_sound_file_path =
            GetChildText(settings, "WarnSoundFile");
        aps_settings.connect_port = GetChildInt(settings, "cPort");
        aps_settings.receive_port = GetChildInt(settings, "rPort");
        aps_settings.receive_port_max = GetChildInt(settings, "rPortMax");
        aps_settings.cfg_confirm_alarms_drag =
            GetChildBool(settings, "CfgConfirmDrag");
        aps_settings.cfg_one_copy = GetChildBool(settings, "CfgOneCopy");
        aps_settings.cfg_save_alarms_xml =
            GetChildBool(settings, "CfgSaveAlarmsXML");
        station_idx++;
    }
    return errors.empty();
}

int write_station_aps_settings(const std::string &alarms_dir_path,
                               const APSModuleSettings &station_aps_settings,
                               const Gtk::TreeModel::Row &station_row,
                               const GroupCols &group_cols,
                               std::string &errors) {
    std::string confname =
        static_cast<Glib::ustring>(station_row[group_cols.confname]);
    std::string station_aps_settings_path =
        alarms_dir_path + "/" + confname + "/Config.xml";

    std::filesystem::path config_p(station_aps_settings_path.c_str());
    if (!std::filesystem::exists(config_p.parent_path())) {
        std::error_code ec;
        std::filesystem::create_directory(config_p.parent_path(), ec);
        if (ec) {
            errors += "- Не удалось создать директорию конфигурации: " +
                      ec.message() + "\n";
            return ec.value();
        }
    }
    errno = 0;
    std::ofstream ofs(station_aps_settings_path);
    int err = errno;
    if (!ofs.is_open() || err != 0) {
        errors +=
            std::string("- Не удалось открыть файл конфигурации для записи ") +
            station_aps_settings_path + ": " + strerror(err) + "\n";
        return err;
    }

    tinyxml2::XMLDocument config;
    auto append_elem = [&config](tinyxml2::XMLElement *parent,
                                 const char *name) {
        tinyxml2::XMLElement *node = config.NewElement(name);
        parent->InsertEndChild(node);
        return node;
    };
    auto append_val_elem = [&config](tinyxml2::XMLElement *parent,
                                     const std::string &name,
                                     const std::string &val) {
        tinyxml2::XMLElement *node = config.NewElement(name.c_str());
        node->SetText(val.c_str());
        parent->InsertEndChild(node);
        return node;
    };

    tinyxml2::XMLElement *alarm_config_elem = config.NewElement("AlarmConfig");
    config.InsertFirstChild(alarm_config_elem);
    tinyxml2::XMLElement *settings_elem =
        append_elem(alarm_config_elem, "Settings");
    append_val_elem(settings_elem, "PollingPeriod",
                    std::to_string(station_aps_settings.polling_period_msec));
    append_val_elem(
        settings_elem, "MaxBufferSize",
        std::to_string(station_aps_settings.tcp_max_buffer_size_bytes));
    append_val_elem(settings_elem, "FixOrAutoChBck",
                    station_aps_settings.fix_or_auto_checkback ? "true"
                                                               : "false");
    append_val_elem(settings_elem, "FixTimeOut",
                    std::to_string(station_aps_settings.fix_timeout_sec));
    append_val_elem(
        settings_elem, "DeBlockOnErrorTimeout",
        std::to_string(station_aps_settings.deblock_on_err_timeout_sec));
    append_val_elem(settings_elem, "WarnByDefaultSound",
                    station_aps_settings.warn_by_default_sound ? "true"
                                                               : "false");
    append_val_elem(settings_elem, "WarnSoundFile",
                    station_aps_settings.warn_sound_file_path);
    append_val_elem(settings_elem, "cPort",
                    std::to_string(station_aps_settings.connect_port));
    append_val_elem(settings_elem, "rPort",
                    std::to_string(station_aps_settings.receive_port));
    append_val_elem(settings_elem, "rPortMax",
                    std::to_string(station_aps_settings.receive_port_max));
    append_val_elem(settings_elem, "CfgConfirmDrag",
                    station_aps_settings.cfg_confirm_alarms_drag ? "true"
                                                                 : "false");
    append_val_elem(settings_elem, "CfgOneCopy",
                    station_aps_settings.cfg_one_copy ? "true" : "false");
    append_val_elem(settings_elem, "CfgSaveAlarmsXML",
                    station_aps_settings.cfg_save_alarms_xml ? "true"
                                                             : "false");

    tinyxml2::XMLError save_res =
        config.SaveFile(station_aps_settings_path.c_str());
    if (save_res != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось сохранить файл конфигурации: ") +
                  config.ErrorStr() + "\n";
        return save_res;
    }
    int ret = config.ErrorID();
    if (ret != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось сохранить файл конфигурации: ") +
                  config.ErrorStr() + "\n";
        return ret;
    }
    return ret;
}

bool parse_controller_bin(
    int controller_idx, const std::string &controller_config_path,
    const Gtk::TreeModel::Children &zones_station_children,
    const Gtk::TreeModel::Row &station_row,
    Glib::RefPtr<Gtk::TreeStore> &treestore_stations,
    const std::vector<UsedAlarmRef> &used_params,
    Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_klogic,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_task,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_fb,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_service_params,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_discrete,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_analog,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_variables_group,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_variables_array,
    const std::array<Glib::RefPtr<Gdk::Pixbuf>, 6> &pixbufs_params,
    const AlarmObjCols &alarmobj_cols, const GroupCols &group_cols,
    std::string &errors) {
    (void)pixbuf_variables_array;
    fastparam::Machins *m = fastparam::machins_get();
    if (!m) {
        errors += "- Не инициализированы данные KLogic (machins)\n";
        return false;
    }
    short unsigned int station_id = station_row[group_cols.station_id];
    unsigned char ma_id = static_cast<unsigned char>(station_id);
    unsigned short cnt_id = static_cast<unsigned short>(controller_idx);

    fastparam::Cntrl *cntrl = fastparam::machins_get_cntrl(m, ma_id, cnt_id);
    std::string controller_name = fastparam::cntrl_get_name(cntrl);
    if (!controller_name.empty())
        controller_name = cp1251_to_utf8(controller_name);
    else
        controller_name = "Контроллер " + std::to_string(controller_idx);

    Gtk::TreeModel::Row controller_row =
        *treestore_zones->append(zones_station_children);
    controller_row[alarmobj_cols.icon] = pixbuf_klogic;
    controller_row[alarmobj_cols.name] = controller_name;
    controller_row[alarmobj_cols.chipher] = controller_config_path;
    controller_row[alarmobj_cols.passport_group_id] = controller_idx;
    std::string station_confname =
        static_cast<Glib::ustring>(station_row[group_cols.confname]);
    std::string passport_group_name = station_confname + "." + controller_name;

    // Служебные параметры
    Gtk::TreeModel::Row service_params_row =
        *treestore_zones->append(controller_row.children());
    service_params_row[alarmobj_cols.icon] = pixbuf_service_params;
    service_params_row[alarmobj_cols.name] = "Служебные параметры";
    service_params_row[alarmobj_cols.alarmobj_type] =
        ALARMOBJTYPE_SERVICEPARAMS;
    auto append_service_param_row = [&](int id, const std::string &name,
                                        const std::string &chipher,
                                        unsigned char type, unsigned char out) {
        Glib::ustring service_param_string_id =
            passport_group_name + "Служебные параметры." + name;
        std::optional<UsedAlarmRef> alarmref_opt =
            get_used_param(used_params, station_id, PASSPORTTYPE_SERVICE,
                           static_cast<short unsigned int>(
                               controller_row[alarmobj_cols.passport_group_id]),
                           static_cast<short unsigned int>(id));

        Gtk::TreeModel::Row param_row =
            *treestore_zones->append(service_params_row.children());
        int param_idx = type * 2 + out;
        if (param_idx > 5)
            param_idx = 0;
        param_row[alarmobj_cols.alarmobj_type] = ALARMOBJTYPE_KLOGICPARAM;
        param_row[alarmobj_cols.icon] = pixbufs_params[param_idx];
        param_row[alarmobj_cols.id] =
            alarmref_opt ? (*alarmref_opt).alarm_id : 0;
        param_row[alarmobj_cols.userpath] =
            alarmref_opt ? (*alarmref_opt).userpath : "";
        param_row[alarmobj_cols.passport_id] = id;
        param_row[alarmobj_cols.name] = name;
        param_row[alarmobj_cols.chipher] = chipher;
        param_row[alarmobj_cols.fullname] = name;
        param_row[alarmobj_cols.station_name] =
            static_cast<Glib::ustring>(station_row[group_cols.name]);
        param_row[alarmobj_cols.passport_group] = "Служебные параметры";
        param_row[alarmobj_cols.passport_val_type_str] =
            get_passport_val_type_str(type, out);
        param_row[alarmobj_cols.type] = type;
        param_row[alarmobj_cols.out] = (out != 0);
        param_row[alarmobj_cols.klogic_string_id] = service_param_string_id;
        param_row[alarmobj_cols.alarm_string_id] =
            "Kl\\St" + std::to_string(station_id) + "\\Cn" +
            std::to_string(static_cast<short unsigned int>(
                controller_row[alarmobj_cols.passport_group_id])) +
            "\\Служебные параметры\\\\" + name;
        param_row[alarmobj_cols.station_id] = station_id;
        param_row[alarmobj_cols.passport_type] = PASSPORTTYPE_SERVICE;
        param_row[alarmobj_cols.passport_group_id] =
            static_cast<short unsigned int>(
                controller_row[alarmobj_cols.passport_group_id]);

        if (!alarmref_opt)
            return;
        Gtk::TreeModel::Path group_path =
            Gtk::TreeModel::Path((*alarmref_opt).userpath);
        Gtk::TreeModel::Row group_row =
            *treestore_stations->get_iter(group_path);
        if (!group_row)
            return;
        Glib::RefPtr<Gtk::ListStore> liststore_alarms =
            group_row[group_cols.alarms];
        Gtk::TreeModel::Row alarm_row = *liststore_alarms->append();
        alarm_row[alarmobj_cols.alarmobj_type] = ALARMOBJTYPE_KLOGICPARAM;
        alarm_row[alarmobj_cols.id] = (*alarmref_opt).alarm_id;
        alarm_row[alarmobj_cols.type] = type;
        alarm_row[alarmobj_cols.out] = (out != 0);
        alarm_row[alarmobj_cols.icon] =
            type == 1 ? pixbuf_discrete : pixbuf_analog;
        alarm_row[alarmobj_cols.name] = chipher;
        alarm_row[alarmobj_cols.chipher] = chipher;
        alarm_row[alarmobj_cols.fullname] = name;
        alarm_row[alarmobj_cols.station_name] =
            static_cast<Glib::ustring>(station_row[group_cols.name]);
        alarm_row[alarmobj_cols.passport_val_type_str] =
            get_passport_val_type_str(type, out);
        alarm_row[alarmobj_cols.passport_group] =
            passport_group_from_string_id(service_param_string_id);
        alarm_row[alarmobj_cols.passport_priority] =
            group_path.size() == 1 ? 3 : 2;
        alarm_row[alarmobj_cols.alarm_string_id] =
            (*alarmref_opt).alarm_string_id;
        alarm_row[alarmobj_cols.klogic_string_id] = service_param_string_id;
        alarm_row[alarmobj_cols.passport_id] =
            (*alarmref_opt).passport.passport_id;
        alarm_row[alarmobj_cols.station_id] = station_id;
        alarm_row[alarmobj_cols.passport_type] = PASSPORTTYPE_SERVICE;
        alarm_row[alarmobj_cols.passport_group_id] =
            static_cast<short unsigned int>(
                controller_row[alarmobj_cols.passport_group_id]);
        group_row[group_cols.alarms] = liststore_alarms;
    };

    append_service_param_row(
        1, "Канал связи", "Тип канала связи: 0 - TCP/IP, 1 - COM-порт", 2, 0);
    append_service_param_row(2, "Инициативный опрос",
                             "Команда на инициативный опрос контроллера", 1, 0);
    append_service_param_row(3, "Длительность цикла опроса",
                             "Длительность последнего цикла опроса, мсек", 2,
                             1);
    append_service_param_row(4, "Связь с контроллером",
                             "Наличие связи с контроллером", 1, 1);
    append_service_param_row(5, "Приостанов опроса", "Приостанов опроса", 1, 0);
    append_service_param_row(6, "Режим генерации демо-значений",
                             "Режим генерации демонстрационный значений", 1, 0);
    append_service_param_row(
        7, "Период опроса", "Период полного принудительного опроса, сек", 2, 0);
    append_service_param_row(8, "Кол-во сигналов \"Тревога\"",
                             "Количество дозвонов \"Снизу\"", 2, 1);
    append_service_param_row(
        9, "Идентификатор станции",
        "Идентификатор станции, на которую получены данные", 2, 1);
    append_service_param_row(20, "Отсутствие связи",
                             "Остутствие связи с контроллером", 1, 1);
    append_service_param_row(21, "Опрос",
                             "Опрос (для GSM - регулярный опрос без разрыва "
                             "соединения",
                             1, 0);
    append_service_param_row(22, "Активный контроллер",
                             "Активный контроллер: 0 - нет, 1 - основной, 2 - "
                             "дублер/резерв. 3 - оба",
                             2, 1);
    append_service_param_row(25, "Связь с сервером", "Наличие связи с сервером",
                             1, 1);

    fastparam::Tree *tree = fastparam::machins_get_tree(m, ma_id, cnt_id);
    if (tree) {
        int n_root = fastparam::tree_get_child_count(tree);
        for (int i = 0; i < n_root; i++) {
            fastparam::TreeChild *task_child =
                fastparam::tree_get_child_at(tree, i);
            if (!task_child)
                continue;
            int task_image_id = fastparam::tree_child_get_imgid(task_child);
            (void)task_image_id;
            std::string task_name =
                cp1251_to_utf8(fastparam::tree_child_get_name(task_child));
            int n_fb = fastparam::tree_child_get_children_count(task_child);
            if (n_fb > 0) {
                Gtk::TreeModel::Row task_row =
                    *treestore_zones->append(controller_row.children());
                task_row[alarmobj_cols.icon] = pixbuf_task;
                task_row[alarmobj_cols.name] = task_name;
                for (int j = 0; j < n_fb; j++) {
                    fastparam::TreeChild *fb_child =
                        fastparam::tree_child_get_child_at(task_child, j);
                    if (!fb_child)
                        continue;
                    int fb_image_id = fastparam::tree_child_get_imgid(fb_child);
                    (void)fb_image_id;
                    std::string fb_name = cp1251_to_utf8(
                        fastparam::tree_child_get_name(fb_child));
                    Gtk::TreeModel::Row fb_row =
                        *treestore_zones->append(task_row.children());
                    fb_row[alarmobj_cols.icon] = pixbuf_fb;
                    fb_row[alarmobj_cols.name] = fb_name;

                    std::string group_prefix =
                        station_confname + "." + task_name + "." + fb_name;

                    std::function<void(Gtk::TreeModel::Row &,
                                       fastparam::TreeChild *,
                                       const std::string &)>
                        add_tree_node;
                    add_tree_node = [&](Gtk::TreeModel::Row &parent_row,
                                        fastparam::TreeChild *node,
                                        const std::string &node_prefix) {
                        std::vector<fastparam::ParamData> params;
                        fastparam::params_list_from_tree_child(m, ma_id, cnt_id,
                                                               node, params);
                        for (const fastparam::ParamData &pd : params) {
                            int param_image_id =
                                fastparam::param_get_imgid(m, &pd);
                            (void)param_image_id;
                            int value_type =
                                fastparam::param_get_value_type(m, &pd);
                            unsigned int type = (value_type == 1) ? 1u : 2u;
                            bool out = false;
                            std::string name = cp1251_to_utf8(
                                fastparam::param_get_name(m, &pd));
                            std::string cipher = cp1251_to_utf8(
                                fastparam::param_get_cipher(m, &pd));
                            if (cipher.empty())
                                cipher = name;
                            std::string string_id = node_prefix + "." + name;
                            std::optional<UsedAlarmRef> alarmref_opt =
                                get_used_param(
                                    used_params, station_id,
                                    PASSPORTTYPE_USERDEF,
                                    static_cast<short unsigned int>(
                                        controller_row[alarmobj_cols
                                                           .passport_group_id]),
                                    static_cast<int>(pd.ParamId));

                            Gtk::TreeModel::Row param_row =
                                *treestore_zones->append(parent_row.children());
                            int param_idx = type * 2 + (out ? 1 : 0);
                            if (param_idx > 5)
                                param_idx = 0;
                            param_row[alarmobj_cols.alarmobj_type] =
                                ALARMOBJTYPE_KLOGICPARAM;
                            param_row[alarmobj_cols.type] = type;
                            param_row[alarmobj_cols.out] = out;
                            param_row[alarmobj_cols.icon] =
                                pixbufs_params[param_idx];
                            param_row[alarmobj_cols.id] =
                                alarmref_opt ? (*alarmref_opt).alarm_id : 0;
                            param_row[alarmobj_cols.userpath] =
                                alarmref_opt ? (*alarmref_opt).userpath : "";
                            param_row[alarmobj_cols.name] = name;
                            param_row[alarmobj_cols.chipher] = cipher;
                            param_row[alarmobj_cols.fullname] = name;
                            param_row[alarmobj_cols.station_name] =
                                static_cast<Glib::ustring>(
                                    station_row[group_cols.name]);
                            param_row[alarmobj_cols.passport_val_type_str] =
                                get_passport_val_type_str(
                                    static_cast<unsigned char>(type),
                                    out ? 1 : 0);
                            param_row[alarmobj_cols.passport_group] =
                                passport_group_from_string_id(string_id);
                            param_row[alarmobj_cols.alarm_string_id] =
                                alarmref_opt ? (*alarmref_opt).alarm_string_id
                                             : "";
                            param_row[alarmobj_cols.klogic_string_id] =
                                string_id;
                            param_row[alarmobj_cols.passport_id] =
                                alarmref_opt
                                    ? (*alarmref_opt).passport.passport_id
                                    : static_cast<int>(pd.ParamId);
                            param_row[alarmobj_cols.station_id] = station_id;
                            param_row[alarmobj_cols.passport_type] =
                                PASSPORTTYPE_USERDEF;
                            param_row[alarmobj_cols.passport_group_id] =
                                static_cast<short unsigned int>(
                                    controller_row[alarmobj_cols
                                                       .passport_group_id]);

                            if (alarmref_opt) {
                                Gtk::TreeModel::Path group_path =
                                    Gtk::TreeModel::Path(
                                        (*alarmref_opt).userpath);
                                Gtk::TreeModel::Row group_row =
                                    *treestore_stations->get_iter(group_path);
                                if (group_row) {
                                    Glib::RefPtr<Gtk::ListStore>
                                        liststore_alarms =
                                            group_row[group_cols.alarms];
                                    Gtk::TreeModel::Row alarm_row =
                                        *liststore_alarms->append();
                                    alarm_row[alarmobj_cols.alarmobj_type] =
                                        ALARMOBJTYPE_GROUPALARM;
                                    alarm_row[alarmobj_cols.id] =
                                        (*alarmref_opt).alarm_id;
                                    alarm_row[alarmobj_cols.type] = type;
                                    alarm_row[alarmobj_cols.out] = out;
                                    alarm_row[alarmobj_cols.icon] =
                                        type == 1 ? pixbuf_discrete
                                                  : pixbuf_analog;
                                    alarm_row[alarmobj_cols.name] = name;
                                    alarm_row[alarmobj_cols.chipher] = cipher;
                                    alarm_row[alarmobj_cols.fullname] = name;
                                    alarm_row[alarmobj_cols.station_name] =
                                        static_cast<Glib::ustring>(
                                            station_row[group_cols.name]);
                                    alarm_row[alarmobj_cols
                                                  .passport_val_type_str] =
                                        get_passport_val_type_str(
                                            static_cast<unsigned char>(type),
                                            out ? 1 : 0);
                                    alarm_row[alarmobj_cols.passport_group] =
                                        passport_group_from_string_id(
                                            string_id);
                                    alarm_row[alarmobj_cols.passport_priority] =
                                        group_path.size() == 1 ? 3 : 2;
                                    alarm_row[alarmobj_cols.alarm_string_id] =
                                        (*alarmref_opt).alarm_string_id;
                                    alarm_row[alarmobj_cols.klogic_string_id] =
                                        string_id;
                                    alarm_row[alarmobj_cols.passport_id] =
                                        pd.ParamId;
                                    alarm_row[alarmobj_cols.station_id] =
                                        station_id;
                                    alarm_row[alarmobj_cols.passport_type] =
                                        PASSPORTTYPE_USERDEF;
                                    alarm_row[alarmobj_cols.passport_group_id] =
                                        static_cast<short unsigned int>(
                                            controller_row
                                                [alarmobj_cols
                                                     .passport_group_id]);
                                    group_row[group_cols.alarms] =
                                        liststore_alarms;
                                }
                            }
                        }
                        int n_children =
                            fastparam::tree_child_get_children_count(node);
                        for (int k = 0; k < n_children; k++) {
                            fastparam::TreeChild *sub =
                                fastparam::tree_child_get_child_at(node, k);
                            if (!sub)
                                continue;
                            int sub_image_id =
                                fastparam::tree_child_get_imgid(sub);
                            (void)sub_image_id;
                            std::string sub_name = cp1251_to_utf8(
                                fastparam::tree_child_get_name(sub));
                            Gtk::TreeModel::Row sub_row =
                                *treestore_zones->append(parent_row.children());
                            sub_row[alarmobj_cols.icon] = pixbuf_fb;
                            sub_row[alarmobj_cols.name] = sub_name;
                            add_tree_node(sub_row, sub,
                                          node_prefix + "." + sub_name);
                        }
                    };
                    add_tree_node(fb_row, fb_child, group_prefix);
                }
            } else {
                int task_image_id = fastparam::tree_child_get_imgid(task_child);
                (void)task_image_id;
                Gtk::TreeModel::Row variables_group_row =
                    *treestore_zones->append(controller_row.children());
                variables_group_row[alarmobj_cols.icon] =
                    pixbuf_variables_group;
                variables_group_row[alarmobj_cols.name] = task_name;
                variables_group_row[alarmobj_cols.alarmobj_type] =
                    ALARMOBJTYPE_VARPARAMS;

                std::vector<fastparam::ParamData> params;
                fastparam::params_list_from_tree_child(m, ma_id, cnt_id,
                                                       task_child, params);
                std::string group_prefix = station_confname + "." + task_name;
                for (const fastparam::ParamData &pd : params) {
                    int param_image_id = fastparam::param_get_imgid(m, &pd);
                    (void)param_image_id;
                    int value_type = fastparam::param_get_value_type(m, &pd);
                    unsigned int type = (value_type == 1) ? 1u : 2u;
                    bool out = false;
                    std::string name =
                        cp1251_to_utf8(fastparam::param_get_name(m, &pd));
                    std::string cipher =
                        cp1251_to_utf8(fastparam::param_get_cipher(m, &pd));
                    if (cipher.empty())
                        cipher = name;
                    std::string string_id = group_prefix + "." + name;
                    std::optional<UsedAlarmRef> alarmref_opt = get_used_param(
                        used_params, station_id, PASSPORTTYPE_USERDEF,
                        static_cast<short unsigned int>(
                            controller_row[alarmobj_cols.passport_group_id]),
                        static_cast<int>(pd.ParamId));

                    Gtk::TreeModel::Row param_row = *treestore_zones->append(
                        variables_group_row.children());
                    param_row[alarmobj_cols.alarmobj_type] =
                        ALARMOBJTYPE_KLOGICPARAM | ALARMOBJTYPE_KLOGICVARPARAM;
                    int param_idx = type * 2 + (out ? 1 : 0);
                    if (param_idx > 5)
                        param_idx = 0;
                    param_row[alarmobj_cols.icon] = pixbufs_params[param_idx];
                    param_row[alarmobj_cols.id] =
                        alarmref_opt ? (*alarmref_opt).alarm_id : 0;
                    param_row[alarmobj_cols.userpath] =
                        alarmref_opt ? (*alarmref_opt).userpath : "";
                    param_row[alarmobj_cols.name] = name;
                    param_row[alarmobj_cols.chipher] = cipher;
                    param_row[alarmobj_cols.fullname] = name;
                    param_row[alarmobj_cols.station_name] =
                        static_cast<Glib::ustring>(
                            station_row[group_cols.name]);
                    param_row[alarmobj_cols.passport_val_type_str] =
                        get_passport_val_type_str(
                            static_cast<unsigned char>(type), out ? 1 : 0);
                    param_row[alarmobj_cols.passport_group] =
                        passport_group_from_string_id(string_id);
                    param_row[alarmobj_cols.alarm_string_id] =
                        alarmref_opt ? (*alarmref_opt).alarm_string_id : "";
                    param_row[alarmobj_cols.klogic_string_id] = string_id;
                    param_row[alarmobj_cols.passport_id] =
                        alarmref_opt ? (*alarmref_opt).passport.passport_id
                                     : static_cast<int>(pd.ParamId);
                    param_row[alarmobj_cols.station_id] = station_id;
                    param_row[alarmobj_cols.passport_type] =
                        PASSPORTTYPE_USERDEF;
                    param_row[alarmobj_cols.passport_group_id] =
                        static_cast<short unsigned int>(
                            controller_row[alarmobj_cols.passport_group_id]);

                    if (alarmref_opt) {
                        Gtk::TreeModel::Path group_path =
                            Gtk::TreeModel::Path((*alarmref_opt).userpath);
                        Gtk::TreeModel::Row group_row =
                            *treestore_stations->get_iter(group_path);
                        if (group_row) {
                            Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                                group_row[group_cols.alarms];
                            Gtk::TreeModel::Row alarm_row =
                                *liststore_alarms->append();
                            alarm_row[alarmobj_cols.alarmobj_type] =
                                ALARMOBJTYPE_GROUPALARM |
                                ALARMOBJTYPE_KLOGICVARPARAM;
                            alarm_row[alarmobj_cols.id] =
                                (*alarmref_opt).alarm_id;
                            alarm_row[alarmobj_cols.type] = type;
                            alarm_row[alarmobj_cols.out] = out;
                            alarm_row[alarmobj_cols.icon] =
                                type == 1 ? pixbuf_discrete : pixbuf_analog;
                            alarm_row[alarmobj_cols.name] = name;
                            alarm_row[alarmobj_cols.chipher] = cipher;
                            alarm_row[alarmobj_cols.fullname] = name;
                            alarm_row[alarmobj_cols.station_name] =
                                static_cast<Glib::ustring>(
                                    station_row[group_cols.name]);
                            alarm_row[alarmobj_cols.passport_val_type_str] =
                                get_passport_val_type_str(
                                    static_cast<unsigned char>(type),
                                    out ? 1 : 0);
                            alarm_row[alarmobj_cols.passport_group] =
                                passport_group_from_string_id(string_id);
                            alarm_row[alarmobj_cols.passport_priority] =
                                group_path.size() == 1 ? 3 : 2;
                            alarm_row[alarmobj_cols.alarm_string_id] =
                                (*alarmref_opt).alarm_string_id;
                            alarm_row[alarmobj_cols.klogic_string_id] =
                                string_id;
                            alarm_row[alarmobj_cols.passport_id] = pd.ParamId;
                            alarm_row[alarmobj_cols.station_id] = station_id;
                            alarm_row[alarmobj_cols.passport_type] =
                                PASSPORTTYPE_USERDEF;
                            alarm_row[alarmobj_cols.passport_group_id] =
                                static_cast<short unsigned int>(
                                    controller_row[alarmobj_cols
                                                       .passport_group_id]);
                            group_row[group_cols.alarms] = liststore_alarms;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool parse_klogic_bins(
    const std::string &alarms_dir_path,
    Glib::RefPtr<Gtk::TreeStore> &treestore_stations,
    const std::vector<UsedAlarmRef> &used_params,
    Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_station,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_klogic,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_task,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_fb,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_service_params,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_discrete,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_analog,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_variables_group,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_variables_array,
    const std::array<Glib::RefPtr<Gdk::Pixbuf>, 6> &pixbufs_params,
    const AlarmObjCols &alarmobj_cols, const GroupCols &group_cols,
    std::string &errors) {
    std::string klogic_dir_path =
        std::filesystem::path(alarms_dir_path).parent_path().string() +
        "/KLogic";

    fastparam::stations_create(
        std::filesystem::path(alarms_dir_path).parent_path().string() +
            "/Configurator",
        false);
    fastparam::machins_create(klogic_dir_path, false);
    fastparam::Machins *m = fastparam::machins_get();
    if (!m)
        return false;
    fastparam::machins_init_all(m);

    Gtk::TreeModel::Row klogic_row = *treestore_zones->children().begin();
    for (const Gtk::TreeModel::Row &station_row :
         treestore_stations->children()) {
        Gtk::TreeModel::Row new_station_row =
            *treestore_zones->append(klogic_row.children());
        new_station_row[alarmobj_cols.icon] = pixbuf_station;
        new_station_row[alarmobj_cols.station_id] =
            static_cast<short unsigned int>(station_row[group_cols.station_id]);
        new_station_row[alarmobj_cols.name] =
            static_cast<Glib::ustring>(station_row[group_cols.name]);
        std::string station_confname =
            static_cast<Glib::ustring>(station_row[group_cols.confname]);
        new_station_row[alarmobj_cols.chipher] = station_confname;

        unsigned char ma_id =
            static_cast<unsigned char>(station_row[group_cols.station_id]);
        fastparam::Controllers *controllers =
            fastparam::machins_get_controllers(m, ma_id);
        if (!controllers)
            continue;
        int n = fastparam::controllers_count(controllers);
        Gtk::TreeModel::Children zones_station_children =
            new_station_row.children();
        for (int i = 0; i < n; i++) {
            int cntrl_id = fastparam::controllers_id_by_index(controllers, i);
            if (cntrl_id < 0)
                continue;
            fastparam::Cntrl *controller = fastparam::machins_get_cntrl(
                m, ma_id, static_cast<short unsigned int>(cntrl_id));
            if (!controller)
                continue;
            std::filesystem::path controller_config_path =
                std::filesystem::path(klogic_dir_path) / station_confname /
                "Cfg" / fastparam::cntrl_config_guid_str(controller);
            (void)parse_controller_bin(
                cntrl_id, controller_config_path.string() + ".xml",
                zones_station_children, station_row, treestore_stations,
                used_params, treestore_zones, pixbuf_klogic, pixbuf_task,
                pixbuf_fb, pixbuf_service_params, pixbuf_discrete,
                pixbuf_analog, pixbuf_variables_group, pixbuf_variables_array,
                pixbufs_params, alarmobj_cols, group_cols, errors);
        }
    }
    return errors.empty();
}

bool parse_contacts_config(
    const std::string &stations_config_path,
    Glib::RefPtr<Gtk::TreeStore> &treestore_contacts,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_contact,
    const Glib::RefPtr<Gdk::Pixbuf> &pixbuf_contact_group,
    const ContactCols &contact_cols, std::string &errors) {
    std::string contacts_config_path =
        std::filesystem::path(stations_config_path).parent_path().string() +
        "/contacts.xml";
    if (!std::filesystem::exists(contacts_config_path)) {
        errors += std::string("- Не удалось найти файл контактов: ") +
                  contacts_config_path + "\n";
        return false;
    }

    errno = 0;
    std::ifstream ifs(contacts_config_path);
    int err = errno;
    if (!ifs.is_open() || err != 0) {
        errors += std::string("- Не удалось открыть файл контактов ") +
                  contacts_config_path + ": " + strerror(err) + "\n";
        return false;
    }
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
    ifs.close();
    std::string raw_data(buffer.data(), buffer.size());
    std::string utf8_data;
    if (g_utf8_validate(raw_data.c_str(), static_cast<gssize>(raw_data.size()),
                        nullptr)) {
        utf8_data = raw_data;
    } else {
        utf8_data = cp1251_to_utf8(raw_data);
    }
    tinyxml2::XMLDocument config;
    tinyxml2::XMLError result =
        config.Parse(utf8_data.c_str(), utf8_data.size());
    if (result != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось прочитать файл контактов ") +
                  contacts_config_path + ": " + config.ErrorStr() + "\n";
        return false;
    }

    auto append_contact_row = [&](tinyxml2::XMLElement *contact_elem,
                                  const Gtk::TreeModel::Row *parent_row) {
        if (!contact_elem)
            return;
        Gtk::TreeModel::Row row =
            parent_row ? *treestore_contacts->append(parent_row->children())
                       : *treestore_contacts->append();
        row[contact_cols.icon] = pixbuf_contact;
        row[contact_cols.is_group] = false;
        row[contact_cols.name] = GetChildText(contact_elem, "Name");
        row[contact_cols.comment] = GetChildText(contact_elem, "Comment");
        row[contact_cols.email] = GetChildText(contact_elem, "EMail");
        row[contact_cols.phone] = GetChildText(contact_elem, "Tel");
    };
    auto append_group_row = [&](const auto &self,
                                tinyxml2::XMLElement *group_elem,
                                const Gtk::TreeModel::Row *parent_row) -> void {
        if (!group_elem)
            return;
        const char *raw_text = group_elem->GetText();
        Glib::ustring group_name;
        if (raw_text)
            group_name = Glib::ustring(trim(raw_text));
        Gtk::TreeModel::Row group_row =
            parent_row ? *treestore_contacts->append(parent_row->children())
                       : *treestore_contacts->append();
        group_row[contact_cols.icon] = pixbuf_contact_group;
        group_row[contact_cols.is_group] = true;
        group_row[contact_cols.name] = group_name;
        group_row[contact_cols.comment] = Glib::ustring();
        group_row[contact_cols.email] = Glib::ustring();
        group_row[contact_cols.phone] = Glib::ustring();
        for (tinyxml2::XMLElement *child = group_elem->FirstChildElement();
             child != nullptr; child = child->NextSiblingElement()) {
            const char *child_name = child->Name();
            if (!child_name)
                continue;
            if (std::strcmp(child_name, "Contact") == 0)
                append_contact_row(child, &group_row);
            else if (std::strcmp(child_name, "Group") == 0)
                self(self, child, &group_row);
        }
    };

    tinyxml2::XMLElement *root = config.FirstChildElement("Settings");
    if (tinyxml2::XMLElement *contacts_elem =
            root->FirstChildElement("Contacts")) {
        for (tinyxml2::XMLElement *elem = contacts_elem->FirstChildElement();
             elem != nullptr; elem = elem->NextSiblingElement()) {
            const char *elem_name = elem->Name();
            if (!elem_name)
                continue;
            if (std::strcmp(elem_name, "Contact") == 0)
                append_contact_row(elem, nullptr);
            else if (std::strcmp(elem_name, "Group") == 0)
                append_group_row(append_group_row, elem, nullptr);
        }
    }
    if (tinyxml2::XMLElement *groups_adr_elem =
            root->FirstChildElement("GroupsAdr")) {
        for (tinyxml2::XMLElement *group_elem =
                 groups_adr_elem->FirstChildElement("Group");
             group_elem != nullptr;
             group_elem = group_elem->NextSiblingElement("Group")) {
            append_group_row(append_group_row, group_elem, nullptr);
        }
    }
    return true;
}

int write_contacts_config(
    const std::string &stations_config_path,
    const Glib::RefPtr<Gtk::TreeStore> &treestore_contacts,
    const ContactCols &contact_cols, std::string &errors) {
    std::string contacts_config_path =
        std::filesystem::path(stations_config_path).parent_path().string() +
        "/contacts.xml";

    std::filesystem::path config_p(contacts_config_path);
    if (!std::filesystem::exists(config_p.parent_path())) {
        std::error_code ec;
        std::filesystem::create_directory(config_p.parent_path(), ec);
        if (ec) {
            errors += "- Не удалось создать директорию для файла контактов: " +
                      ec.message() + "\n";
            return ec.value();
        }
    }

    errno = 0;
    std::ofstream ofs(contacts_config_path);
    int err = errno;
    if (!ofs.is_open() || err != 0) {
        errors +=
            std::string("- Не удалось открыть файл контактов для записи ") +
            contacts_config_path + ": " + strerror(err) + "\n";
        return err;
    }
    ofs.close();

    tinyxml2::XMLDocument config;
    auto append_elem = [&config](tinyxml2::XMLElement *parent,
                                 const char *name) {
        tinyxml2::XMLElement *node = config.NewElement(name);
        parent->InsertEndChild(node);
        return node;
    };
    auto set_elem_text = [](tinyxml2::XMLElement *elem,
                            const Glib::ustring &s) {
        elem->SetText(s.empty() ? "" : s.c_str());
    };

    tinyxml2::XMLElement *root = config.NewElement("Settings");
    config.InsertFirstChild(root);
    tinyxml2::XMLElement *contacts_elem = append_elem(root, "Contacts");
    tinyxml2::XMLElement *groups_adr_elem = append_elem(root, "GroupsAdr");
    for (const Gtk::TreeModel::Row &row : treestore_contacts->children()) {
        bool is_group = row[contact_cols.is_group];
        bool has_children = row.children().size() > 0;

        if (is_group && !has_children) {
            tinyxml2::XMLElement *group_node =
                append_elem(groups_adr_elem, "Group");
            set_elem_text(group_node,
                          static_cast<Glib::ustring>(row[contact_cols.name]));
        } else if (is_group && has_children) {
            tinyxml2::XMLElement *group_node =
                append_elem(contacts_elem, "Group");
            set_elem_text(group_node,
                          static_cast<Glib::ustring>(row[contact_cols.name]));
            auto write_group_children =
                [&](const auto &self, tinyxml2::XMLElement *parent_xml,
                    const Gtk::TreeModel::Row &parent_row) -> void {
                for (const Gtk::TreeModel::Row &child_row :
                     parent_row.children()) {
                    if (child_row[contact_cols.is_group]) {
                        tinyxml2::XMLElement *child_group =
                            append_elem(parent_xml, "Group");
                        set_elem_text(child_group,
                                      static_cast<Glib::ustring>(
                                          child_row[contact_cols.name]));
                        self(self, child_group, child_row);
                    } else {
                        tinyxml2::XMLElement *contact_node =
                            append_elem(parent_xml, "Contact");
                        tinyxml2::XMLElement *name_node =
                            append_elem(contact_node, "Name");
                        set_elem_text(name_node,
                                      static_cast<Glib::ustring>(
                                          child_row[contact_cols.name]));
                        tinyxml2::XMLElement *comment_node =
                            append_elem(contact_node, "Comment");
                        set_elem_text(comment_node,
                                      static_cast<Glib::ustring>(
                                          child_row[contact_cols.comment]));
                        tinyxml2::XMLElement *email_node =
                            append_elem(contact_node, "EMail");
                        set_elem_text(email_node,
                                      static_cast<Glib::ustring>(
                                          child_row[contact_cols.email]));
                        tinyxml2::XMLElement *tel_node =
                            append_elem(contact_node, "Tel");
                        set_elem_text(tel_node,
                                      static_cast<Glib::ustring>(
                                          child_row[contact_cols.phone]));
                    }
                }
            };
            write_group_children(write_group_children, group_node, row);
        } else {
            tinyxml2::XMLElement *contact_node =
                append_elem(contacts_elem, "Contact");
            tinyxml2::XMLElement *name_node = append_elem(contact_node, "Name");
            set_elem_text(name_node,
                          static_cast<Glib::ustring>(row[contact_cols.name]));
            tinyxml2::XMLElement *comment_node =
                append_elem(contact_node, "Comment");
            set_elem_text(comment_node, static_cast<Glib::ustring>(
                                            row[contact_cols.comment]));
            tinyxml2::XMLElement *email_node =
                append_elem(contact_node, "EMail");
            set_elem_text(email_node,
                          static_cast<Glib::ustring>(row[contact_cols.email]));
            tinyxml2::XMLElement *tel_node = append_elem(contact_node, "Tel");
            set_elem_text(tel_node,
                          static_cast<Glib::ustring>(row[contact_cols.phone]));
        }
    }

    tinyxml2::XMLError save_res = config.SaveFile(contacts_config_path.c_str());
    if (save_res != tinyxml2::XML_SUCCESS) {
        errors += std::string("- Не удалось сохранить файл контактов: ") +
                  config.ErrorStr() + "\n";
        return save_res;
    }
    return 0;
}
