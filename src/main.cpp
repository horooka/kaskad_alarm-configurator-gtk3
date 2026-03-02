#include "fastparam_api.hpp"
#include "kaskad_alarm-configurator-gtk3/utils.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <gdkmm.h>
#include <gtkmm.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#define GROUP_TREE_WIDTH 200
#define PADDING_WIDTH 100
#define FORM_WIDTH 1000
#define PADDIND_WIDTH 100
#define ZONES_WIDTH 150

class AlarmConfigurator : public Gtk::Window {
    public:
        AlarmConfigurator(const Glib::RefPtr<Gio::Application> &app_ref,
                          const std::string &project_path)
            : stations_config_path(
                  std::filesystem::path(project_path).parent_path().string() +
                  "/Configurator/Stations.ini"),
              alarms_dir_path(
                  std::filesystem::path(project_path).parent_path().string() +
                  "/Alarms"),
              klogic_dir_path(
                  std::filesystem::path(project_path).parent_path().string() +
                  "/KLogic"),
              soundfiles_dir_path(
                  std::filesystem::path(project_path).parent_path().string() +
                  "/SoundFiles"),
              app(app_ref) {
            set_size_request(GROUP_TREE_WIDTH + PADDING_WIDTH + FORM_WIDTH +
                                 PADDIND_WIDTH + ZONES_WIDTH,
                             800);
            setup_gresources();
            setup_menubuttons();
            setup_topbar();
            setup_ui();
            setup_menus();
            setup_signals();
            setup_accel_groups();

            show_all_children();
            setup_data();
        }

        virtual ~AlarmConfigurator();

    private:
        bool is_station() {
            Gtk::TreeModel::iterator curr_group_iter =
                treeview_station.get_selection()->get_selected();
            if (!curr_group_iter)
                return false;
            return !curr_group_iter->parent();
        }

        // Конвертирует итератор модели filter_groups виджета treeview_station
        // в модель treestore_groups
        Gtk::TreeModel::Row get_curr_group_row() {
            Gtk::TreeModel::iterator curr_filter_group_iter =
                treeview_station.get_selection()->get_selected();
            if (!curr_filter_group_iter)
                return Gtk::TreeModel::Row();
            return *filter_groups->convert_iter_to_child_iter(
                curr_filter_group_iter);
        }

        void button_save_enable() {
            if (recursionguard_alarms_form)
                return;
            button_save_icon.set(pixbuf_save_enabled);
            menuitem_save_icon.set(pixbuf_save_enabled);
            button_save.set_sensitive(true);
            menuitem_save.set_sensitive(true);
            unsaved = true;
        }

        void button_save_disable() {
            button_save_icon.set(pixbuf_save_disabled);
            menuitem_save_icon.set(pixbuf_save_disabled);
            button_save.set_sensitive(false);
            menuitem_save.set_sensitive(false);
            unsaved = false;
        }

        void swap_situation_rows(Gtk::TreeModel::Row &row1,
                                 Gtk::TreeModel::Row &row2) {
            bool temp_bool;
            unsigned char temp_uchar;
            Glib::ustring temp_string;
            std::vector<unsigned char> temp_vector_uchar;
            std::vector<float> temp_vector_float;
            std::array<bool, 7> temp_array_bool;

            temp_bool = row1.get_value(situation_cols.is_enabled);
            row1.set_value(situation_cols.is_enabled,
                           row2.get_value(situation_cols.is_enabled));
            row2.set_value(situation_cols.is_enabled, temp_bool);
            temp_uchar = row1.get_value(situation_cols.situation_code);
            row1.set_value(situation_cols.situation_code,
                           row2.get_value(situation_cols.situation_code));
            row2.set_value(situation_cols.situation_code, temp_uchar);
            temp_string = row1.get_value(situation_cols.name);
            row1.set_value(situation_cols.name,
                           row2.get_value(situation_cols.name));
            row2.set_value(situation_cols.name, temp_string);
            temp_uchar = row1.get_value(situation_cols.alarm_kind);
            row1.set_value(situation_cols.alarm_kind,
                           row2.get_value(situation_cols.alarm_kind));
            row2.set_value(situation_cols.alarm_kind, temp_uchar);
            temp_string = row1.get_value(situation_cols.alarm_message);
            row1.set_value(situation_cols.alarm_message,
                           row2.get_value(situation_cols.alarm_message));
            row2.set_value(situation_cols.alarm_message, temp_string);
            temp_string = row1.get_value(situation_cols.usergroup_name);
            row1.set_value(situation_cols.usergroup_name,
                           row2.get_value(situation_cols.usergroup_name));
            row2.set_value(situation_cols.usergroup_name, temp_string);
            temp_vector_uchar = row1.get_value(situation_cols.params_types);
            row1.set_value(situation_cols.params_types,
                           row2.get_value(situation_cols.params_types));
            row2.set_value(situation_cols.params_types, temp_vector_uchar);
            temp_vector_float = row1.get_value(situation_cols.params_vals);
            row1.set_value(situation_cols.params_vals,
                           row2.get_value(situation_cols.params_vals));
            row2.set_value(situation_cols.params_vals, temp_vector_float);
            temp_array_bool = row1.get_value(situation_cols.checkboxes);
            row1.set_value(situation_cols.checkboxes,
                           row2.get_value(situation_cols.checkboxes));
            row2.set_value(situation_cols.checkboxes, temp_array_bool);
        }

        void swap_sound_rows(Gtk::TreeModel::Row &row1,
                             Gtk::TreeModel::Row &row2) {
            Glib::ustring temp_string;
            temp_string = row1.get_value(sound_cols.name);
            row1.set_value(sound_cols.name, row2.get_value(sound_cols.name));
            row2.set_value(sound_cols.name, temp_string);
            temp_string = row1.get_value(sound_cols.filepath_or_pause_duration);
            row1.set_value(
                sound_cols.filepath_or_pause_duration,
                row2.get_value(sound_cols.filepath_or_pause_duration));
            row2.set_value(sound_cols.filepath_or_pause_duration, temp_string);
        }

        void copy_passport_row(const Gtk::TreeModel::Row &row1,
                               Gtk::TreeModel::Row &row2) {
            row2[alarmobj_cols.icon] = static_cast<Glib::RefPtr<Gdk::Pixbuf>>(
                row1[alarmobj_cols.icon]);
            row2[alarmobj_cols.passport_priority] = static_cast<unsigned char>(
                row1[alarmobj_cols.passport_priority]);
            row2[alarmobj_cols.station_name] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.station_name]);
            row2[alarmobj_cols.fullname] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.fullname]);
            row2[alarmobj_cols.passport_val_type_str] =
                static_cast<Glib::ustring>(
                    row1[alarmobj_cols.passport_val_type_str]);
            row2[alarmobj_cols.userpath] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.userpath]);
            row2[alarmobj_cols.chipher] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.chipher]);
            row2[alarmobj_cols.name] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.name]);
            row2[alarmobj_cols.passport_group] =
                static_cast<Glib::ustring>(row1[alarmobj_cols.passport_group]);
            row2[alarmobj_cols.station_id] =
                static_cast<short unsigned int>(row1[alarmobj_cols.station_id]);
            row2[alarmobj_cols.passport_group_id] =
                static_cast<short unsigned int>(
                    row1[alarmobj_cols.passport_group_id]);
            row2[alarmobj_cols.passport_id] = static_cast<short unsigned int>(
                row1[alarmobj_cols.passport_id]);
            row2[alarmobj_cols.passport_type] =
                static_cast<unsigned char>(row1[alarmobj_cols.passport_type]);
            row2[alarmobj_cols.alarmobj_type] =
                static_cast<unsigned char>(row1[alarmobj_cols.alarmobj_type]);
            row2[alarmobj_cols.type] =
                static_cast<unsigned char>(row1[alarmobj_cols.type]);
            row2[alarmobj_cols.out] =
                static_cast<unsigned char>(row1[alarmobj_cols.out]);
        }

        void copy_alarm_row(const Gtk::TreeModel::Row &row1,
                            Gtk::TreeModel::Row &row2) {
            row2[alarmobj_cols.icon] =
                static_cast<Glib::RefPtr<Gdk::Pixbuf>>(row1[group_cols.icon]);
            row2[alarmobj_cols.name] =
                static_cast<Glib::ustring>(row1[group_cols.name]);
        }

        void show_notification(const Glib::ustring &title,
                               const Glib::ustring &message, int timeout_sec) {
            Glib::ustring id = Glib::ustring::format(
                "notif-%lu-%d", static_cast<unsigned long>(time(nullptr)),
                rand() % 1000);
            auto notification = Gio::Notification::create(title);
            notification->set_body(message);
            notification->set_priority(Gio::NotificationPriority(0));
            Glib::signal_idle().connect([this, notification, id]() {
                if (app) {
                    app->send_notification(id, notification);
                }
                return false;
            });
            Glib::signal_timeout().connect_seconds(
                [this, id]() {
                    if (app) {
                        app->withdraw_notification(id);
                    }
                    return false;
                },
                timeout_sec);
        }

        bool on_exit_clicked() {
            if (unsaved) {
                Gtk::MessageDialog dialog("Сохранить изменения?", false,
                                          Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_NONE);
                dialog.add_button("Да", Gtk::RESPONSE_YES);
                dialog.add_button("Нет", Gtk::RESPONSE_NO);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);

                int res = dialog.run();
                if (res == Gtk::RESPONSE_NO) {
                    return false;
                }
                if (res == Gtk::RESPONSE_YES) {
                    on_save_clicked();
                    if (unsaved)
                        return true;
                    return false;
                }
                return true;
            }
            return false;
        }

        void on_save_clicked() {
            std::string errors;
            Gtk::TreeModel::Row curr_station_row;
            for (const Gtk::TreeModel::Row &station_row :
                 treestore_groups->children()) {
                if (station_row[group_cols.station_id] == curr_station_id)
                    curr_station_row = station_row;
            }
            if (!curr_station_row)
                return;
            (void)write_alarms_config(alarms_dir_path, curr_station_row,
                                      group_cols, alarmobj_cols, situation_cols,
                                      sound_cols, contact_cols, errors);
            if (errors.empty()) {
                button_save_disable();
            } else {
                Gtk::MessageDialog dialog(
                    "Не удалось сохранить файл конфигурации алармов\n\n" +
                        errors,
                    false, Gtk::MESSAGE_ERROR);
                dialog.run();
            }
        }

        void on_delete_clicked() {
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            std::vector<Gtk::TreeModel::Path> selected_passports_paths =
                treeview_passports.get_selection()->get_selected_rows();
            if (selected_passports_paths.empty()) {
                Gtk::MessageDialog dialog(
                    "Вы действительно хотите удалить группу?", false,
                    Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                dialog.add_button("OK", Gtk::RESPONSE_YES);
                dialog.add_button("Отмена", Gtk::RESPONSE_NO);
                int res = dialog.run();
                if (res == Gtk::RESPONSE_NO)
                    return;
                treestore_groups->erase(curr_group_row);
            } else {
                Glib::RefPtr<Gtk::ListStore> liststore_passports =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_passports.get_model());
                std::string message;
                if (selected_passports_paths.size() > 1) {
                    message = "Вы действительно хотите удалить алармы (" +
                              std::to_string(selected_passports_paths.size()) +
                              ") ?";
                } else {
                    Gtk::TreeModel::iterator first_alarm_iter =
                        liststore_passports->get_iter(
                            selected_passports_paths[0]);
                    Glib::ustring first_alarm_chipher =
                        (*first_alarm_iter)[alarmobj_cols.name];
                    message = "Вы действительно хотите удалить аларм \"" +
                              first_alarm_chipher + "\" ?";
                }
                Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_NONE);
                dialog.add_button("OK", Gtk::RESPONSE_YES);
                dialog.add_button("Отмена", Gtk::RESPONSE_NO);
                int res = dialog.run();
                if (res == Gtk::RESPONSE_NO)
                    return;
                for (auto it = selected_passports_paths.rbegin();
                     it != selected_passports_paths.rend(); ++it) {
                    const Gtk::TreeModel::Path &path = *it;
                    Gtk::TreeModel::iterator iter =
                        liststore_passports->get_iter(path);
                    if (!iter)
                        continue;
                    Glib::ustring klogicparam_string_id =
                        (*iter)[alarmobj_cols.klogic_string_id];
                    liststore_passports->erase(iter);
                    Gtk::TreeModel::Row klogicparam_row =
                        get_klogicparam_row_by_string_id(klogicparam_string_id,
                                                         treestore_zones,
                                                         alarmobj_cols);
                    if (!klogicparam_row)
                        continue;
                    klogicparam_row[alarmobj_cols.userpath] = "";
                }
                Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                    curr_group_row[group_cols.alarms];
            }
            button_delete.set_sensitive(false);
            menuitem_delete.set_sensitive(false);
            button_new_group.set_sensitive(true);
            menuitem_new_group.set_sensitive(true);
            button_save_enable();
        }

        void on_new_group_clicked() {
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            Gtk::TreeModel::Row new_row =
                *treestore_groups->append(curr_group_row->children());
            int attempt = 1;
            bool unique = true;
            for (const Gtk::TreeModel::Row &row : curr_group_row->children()) {
                if (row[group_cols.name] == "Новая группа") {
                    unique = false;
                    break;
                }
            }
            std::string new_name = "Новая группа";
            if (!unique) {
                while (true) {
                    new_name = "Новая группа " + std::to_string(attempt);
                    unique = true;
                    for (const Gtk::TreeModel::Row &row :
                         curr_group_row->children()) {
                        if (row[group_cols.name] == new_name) {
                            unique = false;
                            break;
                        }
                    }
                    if (unique || attempt == 1000)
                        break;
                    attempt++;
                }
            }
            int next_group_id = get_next_group_id_of_group(curr_group_row);
            new_row[group_cols.group_id] = next_group_id;
            new_row[group_cols.station_id] = curr_station_id;
            new_row[group_cols.next_group_id] = 0;
            new_row[group_cols.icon] = pixbuf_group;
            new_row[group_cols.name] = new_name;
            new_row[group_cols.use_own_settings] = true;
            new_row[group_cols.alarms] = Gtk::ListStore::create(alarmobj_cols);
            new_row[group_cols.situations] =
                Gtk::ListStore::create(situation_cols);

            Gtk::TreeModel::iterator new_filter_iter =
                filter_groups->convert_child_iter_to_iter(new_row);
            treeview_station.expand_row(
                filter_groups->get_path(
                    treeview_station.get_selection()->get_selected()),
                true);
            treeview_station.get_selection()->select(new_filter_iter);
            button_save_enable();
        }

        void on_group_selection_changed() {
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row) {
                vbox_form.hide();
                return;
            }
            vbox_form.show();
            stored_curr_group_row = curr_group_row;
            treeview_station.expand_row(
                treestore_groups->get_path(*curr_group_row), true);

            menuitem_rename_group.set_sensitive(!is_station());
            Glib::ustring group_name = stored_curr_group_row[group_cols.name];
            label_group_name.set_text(group_name);
            checkbutton_use_own_settings.set_active(
                stored_curr_group_row[group_cols.use_own_settings]);
            button_delete.set_sensitive(!is_station());
            checkbutton_use_own_settings.set_sensitive(!is_station());
            stack_general_settings_alarms.set_visible(
                checkbutton_use_own_settings.get_active());

            switch (curr_menu_idx) {
            case 1:
                redraw_passports_menu();
                break;
            case 0:
                redraw_settings_menu();
                break;
            }
        }

        Passport on_passport_choice(unsigned char type) {
            Gtk::MessageDialog dialog(
                std::string("Выбор параметра") + (type == 1   ? " (Дискретный)"
                                                  : type == 2 ? " (Аналоговый)"
                                                              : ""),
                false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
            dialog.add_button("OK", Gtk::RESPONSE_OK);
            dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
            dialog.set_size_request(800, 500);
            Gtk::Box *vbox = dialog.get_content_area();
            Gtk::Box *hbox =
                Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
            vbox->pack_start(*hbox, Gtk::PACK_EXPAND_WIDGET);
            Gtk::ScrolledWindow *scrolled_tree =
                Gtk::make_managed<Gtk::ScrolledWindow>();
            scrolled_tree->set_hexpand(true);
            scrolled_tree->set_vexpand(true);
            scrolled_tree->set_policy(Gtk::POLICY_AUTOMATIC,
                                      Gtk::POLICY_AUTOMATIC);
            hbox->pack_start(*scrolled_tree, Gtk::PACK_EXPAND_WIDGET);
            Gtk::Frame *frame_tree = Gtk::make_managed<Gtk::Frame>();
            scrolled_tree->add(*frame_tree);
            frame_tree->override_background_color(theme_bg_color,
                                                  Gtk::STATE_FLAG_NORMAL);
            Gtk::TreeView *treeview_tree = Gtk::make_managed<Gtk::TreeView>();
            frame_tree->add(*treeview_tree);
            treeview_tree->append_column("", alarmobj_cols.icon);
            treeview_tree->append_column("", alarmobj_cols.name);
            treeview_tree->set_headers_visible(false);
            set_margin(*treeview_tree, 10, 10);

            Gtk::ScrolledWindow *scrolled_params =
                Gtk::make_managed<Gtk::ScrolledWindow>();
            scrolled_params->set_hexpand(true);
            scrolled_params->set_vexpand(true);
            scrolled_params->set_policy(Gtk::POLICY_AUTOMATIC,
                                        Gtk::POLICY_AUTOMATIC);
            hbox->pack_start(*scrolled_params, Gtk::PACK_EXPAND_WIDGET);
            Gtk::Frame *frame_params = Gtk::make_managed<Gtk::Frame>();
            scrolled_params->add(*frame_params);
            frame_params->override_background_color(theme_bg_color,
                                                    Gtk::STATE_FLAG_NORMAL);
            Gtk::TreeView *treeview_params = Gtk::make_managed<Gtk::TreeView>();
            frame_params->add(*treeview_params);
            Gtk::TreeViewColumn *treecolumn_icon_id =
                Gtk::make_managed<Gtk::TreeViewColumn>();
            Gtk::CellRendererPixbuf renderer_icon;
            treecolumn_icon_id->pack_start(renderer_icon, false);
            treecolumn_icon_id->add_attribute(renderer_icon.property_pixbuf(),
                                              alarmobj_cols.icon);
            Gtk::CellRendererText renderer_id;
            treecolumn_icon_id->pack_start(renderer_id, true);
            treecolumn_icon_id->set_title("Ид.");
            treecolumn_icon_id->set_cell_data_func(
                renderer_id, [this](Gtk::CellRenderer *cr,
                                    const Gtk::TreeModel::iterator &it) {
                    unsigned short int id = (*it)[alarmobj_cols.passport_id];
                    static_cast<Gtk::CellRendererText *>(cr)->property_text() =
                        std::to_string(id);
                });
            treeview_params->append_column(*treecolumn_icon_id);
            treeview_params->append_column("Тип",
                                           alarmobj_cols.passport_val_type_str);
            treeview_params->append_column("Шифр", alarmobj_cols.chipher);
            treeview_params->append_column("Наименование", alarmobj_cols.name);
            treeview_params->append_column("Группа",
                                           alarmobj_cols.passport_group);
            set_margin(*treeview_params, 10, 10);

            Glib::RefPtr<Gtk::TreeStore> treestore_tree =
                Gtk::TreeStore::create(alarmobj_cols);
            for (const Gtk::TreeModel::Row &row_src :
                 treestore_zones->children()) {
                Gtk::TreeModel::Row row_dst = *treestore_tree->append();
                fill_passport_choice_recursive(row_src, treestore_tree,
                                               row_dst);
            }
            Gtk::TreeModel::Row alarm_row = *treestore_tree->append();
            alarm_row[alarmobj_cols.name] = "Алармы";
            alarm_row[alarmobj_cols.icon] = pixbuf_alarm;
            for (const Gtk::TreeModel::Row &row_src :
                 treestore_groups->children()) {
                Gtk::TreeModel::Row row_dst =
                    *treestore_tree->append(alarm_row.children());
                std::string passport_group_trace = "Алармы";
                fill_alarm_choice_recursive(row_src, passport_group_trace,
                                            treestore_tree, row_dst);
            }

            Glib::RefPtr<Gtk::TreeModelFilter> filter_tree =
                Gtk::TreeModelFilter::create(treestore_tree);
            filter_tree->set_visible_func(
                [this](const Gtk::TreeModel::const_iterator &iter) {
                    return (*iter)[alarmobj_cols.alarmobj_type] !=
                           ALARMOBJTYPE_KLOGICPARAM;
                });
            treeview_tree->set_model(filter_tree);
            treeview_params->set_model(liststore_passports_params);
            treeview_tree->get_selection()->signal_changed().connect(
                [this, treeview_tree, filter_tree, type]() {
                    Gtk::TreeModel::Row curr_filter_row =
                        *treeview_tree->get_selection()->get_selected();
                    if (!curr_filter_row)
                        return;
                    Gtk::TreeModel::Row curr_row =
                        *filter_tree->convert_iter_to_child_iter(
                            curr_filter_row);
                    if (!curr_row)
                        return;
                    liststore_passports_params->clear();
                    for (const Gtk::TreeModel::Row &param_row_src :
                         curr_row.children()) {
                        if (param_row_src[alarmobj_cols.alarmobj_type] !=
                                ALARMOBJTYPE_KLOGICPARAM ||
                            (type != param_row_src[alarmobj_cols.type] &&
                             type != 0))
                            continue;
                        Gtk::TreeModel::Row param_row_dst =
                            *liststore_passports_params->append();
                        copy_passport_row(param_row_src, param_row_dst);
                    }
                });
            treeview_tree->expand_row(
                Gtk::TreeModel::Path(filter_tree->children().begin()), true);
            dialog.show_all();
            int res = dialog.run();
            if (res != Gtk::RESPONSE_OK)
                return Passport();
            Gtk::TreeModel::Row passport_row =
                *treeview_params->get_selection()->get_selected();
            if (!passport_row)
                return Passport();
            Gtk::TreeModel::iterator station_iter =
                combobox_station.get_active();
            if (!station_iter)
                return Passport();
            return Passport{
                true,
                passport_row[alarmobj_cols.station_id],
                passport_row[alarmobj_cols.passport_type],
                passport_row[alarmobj_cols.group_id],
                passport_row[alarmobj_cols.passport_id],
                passport_row[alarmobj_cols.type] ==
                    static_cast<unsigned char>(KLOGICTYPE_BOOL ? 2u : 1u),
                static_cast<Glib::ustring>((*station_iter)[group_cols.name]),
                static_cast<Glib::ustring>(
                    passport_row[alarmobj_cols.passport_val_type_str]),
                static_cast<Glib::ustring>(passport_row[alarmobj_cols.chipher]),
                static_cast<Glib::ustring>(passport_row[alarmobj_cols.name]),
                static_cast<Glib::ustring>(
                    passport_row[alarmobj_cols.passport_group]),
                static_cast<Glib::ustring>(
                    passport_row[alarmobj_cols.measure_units]),
                passport_row[alarmobj_cols.type],
                passport_row[alarmobj_cols.out],
            };
        }

        void on_setpoint_choice(const std::string &message,
                                unsigned char setpoint_idx) {
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_INFO);
            Gtk::Box *vbox = dialog.get_content_area();
            Gtk::RadioButton::Group radiogroup_setpoint;
            Gtk::Box *hbox_setpoint_val =
                Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
            vbox->pack_start(*hbox_setpoint_val, Gtk::PACK_SHRINK);
            Gtk::RadioButton *radiobutton_setpoint_val =
                Gtk::make_managed<Gtk::RadioButton>(radiogroup_setpoint,
                                                    "Константа");
            Gtk::Entry *entry_setpoint_val = Gtk::make_managed<Gtk::Entry>();
            hbox_setpoint_val->pack_start(*radiobutton_setpoint_val,
                                          Gtk::PACK_SHRINK);
            hbox_setpoint_val->pack_start(*entry_setpoint_val,
                                          Gtk::PACK_SHRINK);
            Gtk::RadioButton *radiobutton_setpoint_passport =
                Gtk::make_managed<Gtk::RadioButton>(radiogroup_setpoint,
                                                    "Паспорт");
            vbox->pack_start(*radiobutton_setpoint_passport, Gtk::PACK_SHRINK);
            Gtk::Grid *grid_passport = Gtk::make_managed<Gtk::Grid>();
            vbox->pack_start(*grid_passport, Gtk::PACK_SHRINK);
            grid_passport->set_column_spacing(5);
            grid_passport->set_row_spacing(5);
            Gtk::Label *label_station_name =
                Gtk::make_managed<Gtk::Label>("Станция");
            label_station_name->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_station_name, 0, 0, 1, 1);
            Gtk::Label *label_station_val = Gtk::make_managed<Gtk::Label>();
            label_station_val->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_station_val, 1, 0, 1, 1);
            Gtk::Label *label_passport_val_type =
                Gtk::make_managed<Gtk::Label>("Тип параметра");
            label_passport_val_type->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_val_type, 0, 1, 1, 1);
            Gtk::Label *label_passport_val_type_val =
                Gtk::make_managed<Gtk::Label>();
            label_passport_val_type_val->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_val_type_val, 1, 1, 1, 1);
            Gtk::Label *label_passport_id =
                Gtk::make_managed<Gtk::Label>("Идентификатор");
            label_passport_id->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_id, 0, 2, 1, 1);
            Gtk::Label *label_passport_id_val = Gtk::make_managed<Gtk::Label>();
            label_passport_id_val->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_id_val, 1, 2, 1, 1);
            Gtk::Label *label_passport_chipher =
                Gtk::make_managed<Gtk::Label>("Шифр");
            label_passport_chipher->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_chipher, 0, 3, 1, 1);
            Gtk::Label *label_passport_chipher_val =
                Gtk::make_managed<Gtk::Label>();
            label_passport_chipher_val->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_chipher_val, 1, 3, 1, 1);
            Gtk::Label *label_passport_name =
                Gtk::make_managed<Gtk::Label>("Наименование");
            label_passport_name->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_name, 0, 4, 1, 1);
            Gtk::Label *label_passport_name_val =
                Gtk::make_managed<Gtk::Label>();
            label_passport_name_val->set_halign(Gtk::ALIGN_START);
            grid_passport->attach(*label_passport_name_val, 1, 4, 1, 1);
            Gtk::Button *button_passport_choice =
                Gtk::make_managed<Gtk::Button>("Выбрать");
            vbox->pack_start(*button_passport_choice, Gtk::PACK_SHRINK);
            set_margin(*vbox, 10, 10);
            set_margin(*grid_passport, 10, 10);

            radiobutton_setpoint_val->signal_toggled().connect(
                [this, &radiobutton_setpoint_val, &entry_setpoint_val,
                 &curr_group_row, setpoint_idx]() {
                    std::array<bool, 4> crashprecrash_passports_usage =
                        curr_group_row[group_cols
                                           .crashprecrash_passports_usage];
                    if (radiobutton_setpoint_val->get_active()) {
                        crashprecrash_passports_usage[setpoint_idx] = false;
                        curr_group_row[group_cols
                                           .crashprecrash_passports_usage] =
                            crashprecrash_passports_usage;
                        entry_setpoint_val->set_sensitive(true);
                    } else
                        entry_setpoint_val->set_sensitive(false);
                });
            radiobutton_setpoint_passport->signal_toggled().connect(
                [this, &radiobutton_setpoint_passport, &grid_passport,
                 &curr_group_row, setpoint_idx]() {
                    std::array<bool, 4> crashprecrash_passports_usage =
                        curr_group_row[group_cols
                                           .crashprecrash_passports_usage];
                    if (radiobutton_setpoint_passport->get_active()) {
                        crashprecrash_passports_usage[setpoint_idx] = true;
                        curr_group_row[group_cols
                                           .crashprecrash_passports_usage] =
                            crashprecrash_passports_usage;
                        grid_passport->set_sensitive(true);
                    } else
                        grid_passport->set_sensitive(false);
                });
            button_passport_choice->signal_clicked().connect(
                [this, radiobutton_setpoint_passport, curr_group_row,
                 label_station_val, label_passport_val_type_val,
                 label_passport_id_val, label_passport_chipher_val,
                 label_passport_name_val, setpoint_idx]() {
                    Passport new_passport = on_passport_choice(2);
                    if (!new_passport.selected)
                        return;
                    label_station_val->set_text(new_passport.station_name);
                    label_passport_val_type_val->set_text(
                        new_passport.passport_val_type_str);
                    label_passport_id_val->set_text(
                        std::to_string(new_passport.passport_id));
                    label_passport_chipher_val->set_text(new_passport.chipher);
                    label_passport_name_val->set_text(new_passport.fullname);
                    Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                    if (!curr_group_row)
                        return;
                    std::array<Passport, 7> passports =
                        curr_group_row[group_cols.passports];
                    passports[setpoint_idx] = new_passport;
                    curr_group_row[group_cols.passports] = passports;
                    radiobutton_setpoint_passport->set_active(true);
                    button_save_enable();
                });

            std::array<Passport, 7> passports =
                curr_group_row[group_cols.passports];
            std::array<bool, 4> crashprecrash_passports_usage =
                curr_group_row[group_cols.crashprecrash_passports_usage];
            std::array<int, 4> crashprecrash_bounds =
                curr_group_row[group_cols.crashprecrash_borders];
            Passport passport = passports[setpoint_idx];
            if (crashprecrash_passports_usage[setpoint_idx])
                radiobutton_setpoint_passport->set_active(true);
            else
                radiobutton_setpoint_val->set_active(true);
            if (passport.selected) {
                label_station_val->set_text(passport.station_name);
                label_passport_val_type_val->set_text(
                    passport.passport_val_type_str);
                label_passport_id_val->set_text(
                    std::to_string(passport.passport_id));
                label_passport_chipher_val->set_text(passport.chipher);
                label_passport_name_val->set_text(passport.fullname);
            } else {
                label_station_val->set_text("<нет данных");
                label_passport_val_type_val->set_text("<нет данных>");
                label_passport_id_val->set_text("<нет данных>");
                label_passport_chipher_val->set_text("<нет данных>");
                label_passport_name_val->set_text("<нет данных>");
            }
            entry_setpoint_val->set_text(
                std::to_string(crashprecrash_bounds[setpoint_idx]));
            dialog.show_all();
            dialog.run();
            redraw_general_settings_menu();
        }

        void fill_passport_choice_recursive(
            const Gtk::TreeModel::Row &row_src,
            Glib::RefPtr<Gtk::TreeStore> &treestore_dst,
            Gtk::TreeModel::Row &row_dst) {
            copy_passport_row(row_src, row_dst);
            for (const Gtk::TreeModel::Row &child_src : row_src.children()) {
                Gtk::TreeModel::Row child_dst_row =
                    *treestore_dst->append(row_dst.children());
                fill_passport_choice_recursive(child_src, treestore_dst,
                                               child_dst_row);
            }
        }

        void
        fill_alarm_choice_recursive(const Gtk::TreeModel::Row &row_src,
                                    std::string &passport_group_trace,
                                    Glib::RefPtr<Gtk::TreeStore> &treestore_dst,
                                    Gtk::TreeModel::Row &row_dst) {
            copy_alarm_row(row_src, row_dst);
            passport_group_trace += "/" + row_dst[alarmobj_cols.name];
            Gtk::TreeModel::Children children_dst = row_dst.children();
            auto append_alarm_passport = [this, &treestore_dst, children_dst,
                                          passport_group_trace](
                                             short unsigned int passport_id,
                                             const std::string chipher,
                                             const std::string fullname) {
                Gtk::TreeModel::Row row = *treestore_dst->append(children_dst);
                row[alarmobj_cols.alarmobj_type] = ALARMOBJTYPE_KLOGICPARAM;
                row[alarmobj_cols.type] = KLOGICTYPE_FLOAT;
                row[alarmobj_cols.passport_id] = passport_id;
                row[alarmobj_cols.icon] = pixbuf_analog;
                row[alarmobj_cols.chipher] = chipher;
                row[alarmobj_cols.passport_type] = 111;
                row[alarmobj_cols.passport_val_type_str] = "Аналоговый";
                row[alarmobj_cols.passport_group] = passport_group_trace;
                row[alarmobj_cols.name] = fullname;
            };
            short unsigned int passport_id = row_src[group_cols.group_id];

            append_alarm_passport(
                passport_id,
                static_cast<Glib::ustring>(row_src[alarmobj_cols.name]),
                passport_group_trace);
            append_alarm_passport(
                passport_id + 3000, "Алармы",
                "Общее количество сработавших алармов (аварийных и "
                "предупредительных)");
            append_alarm_passport(
                passport_id + 6000, "Алармы активные",
                "Общее количество активных алармов (аварийных "
                "и предупредительных)");
            append_alarm_passport(passport_id + 9000, "Алармы квитированные",
                                  "Общее количество квитированных алармов "
                                  "(аварийных и предупредительных)");
            append_alarm_passport(passport_id + 12000, "Аварии",
                                  "Количество сработавших аварий");
            append_alarm_passport(passport_id + 15000, "Аварии активные",
                                  "Количество активных аварий");
            append_alarm_passport(passport_id + 18000, "Аварии квитированные",
                                  "Количество квитированных аварий");
            append_alarm_passport(passport_id + 21000, "Предупреждения",
                                  "Количество сработанных предупреждений");
            append_alarm_passport(passport_id + 24000,
                                  "Предупреждения активные",
                                  "Количество активных предупреждений");
            append_alarm_passport(passport_id + 27000,
                                  "Предупреждения квитированные",
                                  "Количество квитированных предупреждений");
            append_alarm_passport(passport_id + 33000, "Сообщения",
                                  "Количество сообщений");
            append_alarm_passport(passport_id + 36000, "Алармы (Группа)",
                                  "Общее количество групп сработавших алармов "
                                  "(аварийных и предупредительных)");
            append_alarm_passport(passport_id + 39000,
                                  "Алармы активные (Группа)",
                                  "Общее количество групп активных алармов "
                                  "(аварийных и предупредительных)");
            append_alarm_passport(
                passport_id + 42000, "Алармы квитированные (Группа)",
                "Общее количество групп квитированных алармов "
                "(аварийных и предупредительных)");
            append_alarm_passport(passport_id + 45000, "Аварии (Группа)",
                                  "Количество групп сработавших аварий");
            append_alarm_passport(passport_id + 48000,
                                  "Аварии активные (Группа)",
                                  "Количество групп активных аварий");
            append_alarm_passport(passport_id + 51000,
                                  "Аварии квитированные (Группа)",
                                  "Количество групп квитированных аварий");
            append_alarm_passport(
                passport_id + 54000, "Предупреждения (Группа)",
                "Количество групп сработанных предупреждений");
            append_alarm_passport(passport_id + 57000,
                                  "Предупреждения активные (Группа)",
                                  "Количество групп активных предупреждений");
            append_alarm_passport(
                passport_id + 60000, "Предупреждения квитированные (Группа)",
                "Количество групп квитированных предупреждений");
            append_alarm_passport(passport_id + 63000, "Сообщения (Группа)",
                                  "Количество групп сообщений");

            for (const Gtk::TreeModel::Row &child_src : row_src.children()) {
                Gtk::TreeModel::Row child_dst_row =
                    *treestore_dst->append(row_dst.children());
                fill_alarm_choice_recursive(child_src, passport_group_trace,
                                            treestore_dst, child_dst_row);
            }
        }

        void set_treestore_recursive(
            const Gtk::TreeModel::Children &children, bool is_station,
            const Glib::RefPtr<Gtk::TreeStore> &treestore_zones,
            const AlarmObjCols &alarmobj_cols) {
            for (Gtk::TreeModel::iterator iter = children.begin();
                 iter != children.end(); ++iter) {
                Gtk::TreeModel::Row row = *iter;
                row[group_cols.icon] =
                    is_station ? pixbuf_station : pixbuf_group;
                Glib::RefPtr<Gtk::ListStore> liststore_situations =
                    row[group_cols.situations];
                if (liststore_situations) {
                    for (Gtk::TreeModel::iterator iter_situation =
                             liststore_situations->children().begin();
                         iter_situation !=
                         liststore_situations->children().end();
                         ++iter_situation) {
                        Gtk::TreeModel::Row row_situation = *iter_situation;
                        std::array<SituationParams, 4> situation_params =
                            row_situation[situation_cols.params];
                        if (situation_params[2].passport.selected)
                            fill_passport(situation_params[2].passport,
                                          treestore_zones, treestore_groups,
                                          alarmobj_cols, group_cols);
                        if (situation_params[3].passport.selected)
                            fill_passport(situation_params[3].passport,
                                          treestore_zones, treestore_groups,
                                          alarmobj_cols, group_cols);
                        row_situation[situation_cols.params] = situation_params;
                    }
                } else
                    row[group_cols.situations] =
                        Gtk::ListStore::create(situation_cols);
                Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                    row[group_cols.alarms];
                if (!liststore_alarms)
                    row[group_cols.alarms] =
                        Gtk::ListStore::create(alarmobj_cols);
                std::array<Passport, 7> passports = row[group_cols.passports];
                for (Passport &passport : passports) {
                    if (passport.selected)
                        fill_passport(passport, treestore_zones,
                                      treestore_groups, alarmobj_cols,
                                      group_cols);
                }
                row[group_cols.passports] = passports;
                if (!row.children().empty()) {
                    set_treestore_recursive(row.children(), false,
                                            treestore_zones, alarmobj_cols);
                }
            }
        };

        void redraw_settings_menu() {
            if (!radiobutton_settings_menu.get_active())
                return;
            switch (curr_settings_menu_idx) {
            case 0:
                redraw_general_settings_menu();
                break;
            case 1:
                redraw_alarms_menu();
                break;
            }

            stack_form_menu.set_visible_child(vbox_settings_menu);
            curr_menu_idx = 0;
        }

        void redraw_passports_menu() {
            if (!radiobutton_passports_menu.get_active())
                return;
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                curr_group_row[group_cols.alarms];
            treeview_passports.set_model(liststore_alarms);
            stack_form_menu.set_visible_child(vbox_passports_menu);
            curr_menu_idx = 1;
        }

        void redraw_general_settings_menu() {
            if (!radiobutton_settings_menu.get_active())
                return;
            stack_general_settings_alarms.set_visible_child(
                vbox_general_settings);
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            checkbutton_disable_alarm.set_active(
                curr_group_row[group_cols.disable_alarm]);
            checkbutton_checkback_on_startup.set_active(
                curr_group_row[group_cols.checkback_on_startup]);
            checkbutton_use_own_sound_in_group.set_active(
                curr_group_row[group_cols.use_own_sound_in_group]);
            entry_equipment_description_mask.set_text(
                curr_group_row[group_cols.device_mask]);
            checkbutton_borders_from_passport.set_active(
                curr_group_row[group_cols.borders_from_passport]);
            std::array<Passport, 7> passports =
                curr_group_row[group_cols.passports];
            std::array<bool, 4> crashprecrash_passports_usage =
                curr_group_row[group_cols.crashprecrash_passports_usage];
            std::array<int, 4> crashprecrash_bounds =
                curr_group_row[group_cols.crashprecrash_borders];
            entry_higher_crash_setpoint.set_text(
                passports[0].selected && crashprecrash_passports_usage[0]
                    ? "(пасспорт)"
                    : std::to_string(crashprecrash_bounds[0]));
            entry_lower_crash_setpoint.set_text(
                passports[1].selected && crashprecrash_passports_usage[1]
                    ? "(пасспорт)"
                    : std::to_string(crashprecrash_bounds[1]));
            entry_higher_precrash_setpoint.set_text(
                passports[2].selected && crashprecrash_passports_usage[2]
                    ? "(пасспорт)"
                    : std::to_string(crashprecrash_bounds[2]));
            entry_lower_precrash_setpoint.set_text(
                passports[3].selected && crashprecrash_passports_usage[3]
                    ? "(пасспорт)"
                    : std::to_string(crashprecrash_bounds[3]));

            unsigned char block_by_passport_val =
                curr_group_row[group_cols.block_by_passport_val];
            unsigned char checkback_val =
                curr_group_row[group_cols.checkback_val];
            bool write_on_checkback =
                curr_group_row[group_cols.write_on_checkback];
            checkbutton_checkback.set_active(checkback_val != 0);
            checkbutton_block_by_passport.set_active(block_by_passport_val !=
                                                     0);
            checkbutton_write_on_checkback.set_active(write_on_checkback);
            curr_settings_menu_idx = 0;
            redraw_aps();
        }

        void redraw_alarms_menu() {
            if (recursionguard_alarms_menu || !radiobutton_alarms.get_active())
                return;

            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            Glib::RefPtr<Gtk::ListStore> liststore_situations =
                curr_group_row[group_cols.situations];
            treeview_situations.set_model(liststore_situations);

            if (!liststore_situations->children().empty())
                treeview_situations.get_selection()->select(
                    liststore_situations->children().begin());
            redraw_alarms_form();
            stack_general_settings_alarms.set_visible_child(frame_alarms);
            curr_settings_menu_idx = 1;
        }

        void redraw_alarms_form() {
            if (recursionguard_alarms_form)
                return;
            recursionguard_alarms_form = true;

            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row) {
                vbox_alarms_form.hide();
                button_situation_delete.set_sensitive(false);
                button_situation_up.set_sensitive(false);
                button_situation_down.set_sensitive(false);
                recursionguard_alarms_form = false;
                return;
            }
            Glib::RefPtr<Gtk::ListStore> liststore_situations =
                Glib::RefPtr<Gtk::ListStore>::cast_static(
                    treeview_situations.get_model());
            if (!liststore_situations) {
                vbox_alarms_form.hide();
                recursionguard_alarms_form = false;
                return;
            }
            int amount = liststore_situations->children().size();
            int idx = liststore_situations->get_path(curr_situation_row)[0];
            if (amount == 1) {
                button_situation_up.set_sensitive(false);
                button_situation_down.set_sensitive(false);
            } else if (idx == 0) {
                button_situation_up.set_sensitive(false);
                button_situation_down.set_sensitive(true);
            } else if (idx == amount - 1) {
                button_situation_down.set_sensitive(false);
                button_situation_up.set_sensitive(true);
            } else {
                button_situation_up.set_sensitive(true);
                button_situation_down.set_sensitive(true);
            }
            vbox_alarms_form.show();

            unsigned char alarm_kind =
                curr_situation_row[situation_cols.alarm_kind];
            switch (alarm_kind) {
            case 0:
                radiobutton_alarm_kind_crash.set_active(true);
                break;
            case 1:
                radiobutton_alarm_kind_warning.set_active(true);
                break;
            case 2:
                radiobutton_alarm_kind_notification.set_active(true);
                break;
            }

            std::array<bool, 7> checkboxes =
                curr_situation_row[situation_cols.checkboxes];
            radiobutton_alarms_hysteresis_menu.set_visible(checkboxes[2]);
            radiobutton_alarms_sms_menu.set_visible(checkboxes[3]);
            radiobutton_alarms_email_menu.set_visible(checkboxes[4]);
            radiobutton_alarms_passport_menu.set_visible(checkboxes[5]);
            radiobutton_alarms_write_menu.set_visible(checkboxes[6]);
            switch (curr_alarms_menu_idx) {
            case 0:
                redraw_alarms_params();
                break;
            case 1:
                redraw_alarms_message();
                break;
            case 2:
                redraw_alarms_sound();
                break;
            default: {
                if (!checkboxes[curr_alarms_menu_idx - 1]) {
                    radiobutton_alarms_parameters_menu.set_active(true);
                    break;
                }
                switch (curr_alarms_menu_idx) {
                case 3:
                    redraw_alarms_hysteresis();
                    break;
                case 4:
                    redraw_alarms_sms();
                    break;
                case 5:
                    redraw_alarms_email();
                    break;
                case 6:
                    redraw_alarms_passport();
                    break;
                case 7:
                    redraw_alarms_write();
                    break;
                }
            }
            }

            recursionguard_alarms_form = false;
        }

        void redraw_alarms_params() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                frame_situation_params.hide();
            unsigned char situation_code =
                curr_situation_row[situation_cols.situation_code];
            spin_response_delay.set_value(
                curr_situation_row[situation_cols.alarm_confirm_period]);
            combobox_response_delay.set_active(
                curr_situation_row[situation_cols.alarm_confirm_interval]);
            std::vector<unsigned char> situation_params_types =
                curr_situation_row[situation_cols.params_types];
            std::vector<float> situation_params_vals =
                curr_situation_row[situation_cols.params_vals];
            switch (situation_code) {
            case 5: {
                label_frame_situation_params.set_text("Диапазон значений");
                label_situation_params_range.set_text(" Паспорт < ");
                entry_situation_params_greaterthan.hide();
                entry_situation_params_lessthan.show();
                stack_situation_params.set_visible_child(
                    hbox_situation_params_range);
                frame_situation_params.show();
                bool lessthan_unused = true;
                for (unsigned char i = 0; i < situation_params_types.size();
                     i++) {
                    switch (situation_params_types[i]) {
                    case SITUATIONPARAMTYPE_LESSTHAN:
                        lessthan_unused = false;
                        entry_situation_params_lessthan.set_text(
                            format_float(situation_params_vals[i]));
                        break;
                    }
                }
                if (lessthan_unused)
                    entry_situation_params_lessthan.set_text("100.000");
                break;
            }
            case 6: {
                label_frame_situation_params.set_text("Диапазон значений");
                label_situation_params_range.set_text(" ≤ Паспорт ");
                entry_situation_params_greaterthan.show();
                entry_situation_params_lessthan.hide();
                stack_situation_params.set_visible_child(
                    hbox_situation_params_range);
                frame_situation_params.show();
                bool greaterthan_unused = true;
                for (unsigned char i = 0; i < situation_params_types.size();
                     i++) {
                    switch (situation_params_types[i]) {
                    case SITUATIONPARAMTYPE_LESSTHAN:
                        greaterthan_unused = false;
                        entry_situation_params_greaterthan.set_text(
                            format_float(situation_params_vals[i]));
                        break;
                    }
                }
                if (greaterthan_unused)
                    entry_situation_params_greaterthan.set_text("0.000");
                break;
            }
            case 7: {
                label_frame_situation_params.set_text("Диапазон значений");
                label_situation_params_range.set_text(" ≤ Паспорт < ");
                entry_situation_params_greaterthan.show();
                entry_situation_params_lessthan.show();
                stack_situation_params.set_visible_child(
                    hbox_situation_params_range);
                frame_situation_params.show();
                bool lessthan_unused = true;
                bool greaterthan_unused = true;
                for (unsigned char i = 0; i < situation_params_types.size();
                     i++) {
                    switch (situation_params_types[i]) {
                    case SITUATIONPARAMTYPE_LESSTHAN:
                        lessthan_unused = false;
                        entry_situation_params_lessthan.set_text(
                            format_float(situation_params_vals[i]));
                        break;
                    case SITUATIONPARAMTYPE_GREATERTHAN:
                        greaterthan_unused = false;
                        entry_situation_params_greaterthan.set_text(
                            format_float(situation_params_vals[i]));
                        break;
                    }
                }
                if (lessthan_unused)
                    entry_situation_params_lessthan.set_text("100.000");
                if (greaterthan_unused)
                    entry_situation_params_greaterthan.set_text("0.000");
                break;
            }
            case 8: {
                label_frame_situation_params.set_text("Дискретное значение");
                stack_situation_params.set_visible_child(
                    hbox_situation_params_discrete);
                frame_situation_params.show();
                bool discrete_unused = true;
                for (unsigned char i = 0; i < situation_params_types.size();
                     i++) {
                    switch (situation_params_types[i]) {
                    case SITUATIONPARAMTYPE_DISCRETE:
                        discrete_unused = false;
                        radiobutton_situation_params_discrete_true.set_active(
                            situation_params_vals[i] == 1);
                        break;
                    }
                    if (discrete_unused)
                        radiobutton_situation_params_discrete_true.set_active(
                            true);
                }
                break;
            }
            case 9:
                label_frame_situation_params.set_text("Код ошибки");
                stack_situation_params.set_visible_child(
                    hbox_situation_params_errorcode);
                frame_situation_params.show();
                break;
            default:
                frame_situation_params.hide();
            }
            std::array<bool, 7> checkboxes =
                curr_situation_row[situation_cols.checkboxes];
            checkbutton_checkback_alarm.set_active(checkboxes[0]);
            checkbutton_write_events.set_active(checkboxes[1]);
            checkbutton_send_sms.set_active(checkboxes[3]);
            checkbutton_send_email.set_active(checkboxes[4]);
            checkbutton_passport.set_active(checkboxes[5]);
            checkbutton_write.set_active(checkboxes[6]);
            curr_alarms_menu_idx = 0;
            stack_alarms.set_visible_child(vbox_alarm_parameters);
        }

        void redraw_alarms_message() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            std::string alarm_message = static_cast<Glib::ustring>(
                curr_situation_row[situation_cols.alarm_message]);
            entry_message.set_text(alarm_message);
            render_masks(alarm_message);
            label_message_mask_view.set_text(alarm_message);
            checkbutton_write_event_to_group.set_active(
                curr_situation_row[situation_cols.write_to_usergroup]);
            entry_usergroup.set_text(
                curr_situation_row[situation_cols.usergroup_name]);
            curr_alarms_menu_idx = 1;
            stack_alarms.set_visible_child(vbox_alarm_message);
        }

        void redraw_alarms_sound() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                curr_situation_row[situation_cols.sounds];
            if (!liststore_sounds)
                return;
            treeview_sounds.set_model(liststore_sounds);
            curr_alarms_menu_idx = 2;
            stack_alarms.set_visible_child(vbox_alarm_sound);

            Gtk::TreeModel::iterator first_sound_iter =
                liststore_sounds->children().begin();
            if (!first_sound_iter) {
                button_delete.set_sensitive(false);
                button_sound_up.set_sensitive(false);
                button_sound_down.set_sensitive(false);
                return;
            }
            int amount = liststore_sounds->children().size();
            int idx = liststore_sounds->get_path(first_sound_iter)[0];
            if (amount == 1) {
                button_sound_up.set_sensitive(false);
                button_sound_down.set_sensitive(false);
            } else if (idx == 0) {
                button_sound_up.set_sensitive(false);
                button_sound_down.set_sensitive(true);
            } else if (idx == amount - 1) {
                button_sound_down.set_sensitive(false);
                button_sound_up.set_sensitive(true);
            } else {
                button_sound_up.set_sensitive(true);
                button_sound_down.set_sensitive(true);
            }
        }

        void redraw_alarms_hysteresis() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            if (!situation_additional_menus[0]) {
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::Frame *frame_alarm_hysteresis =
                    Gtk::make_managed<Gtk::Frame>(
                        "Гистерезис срабатывания сигнализации");
                vbox->pack_start(*frame_alarm_hysteresis,
                                 Gtk::PACK_EXPAND_WIDGET);
                Gtk::Box *inner_vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                frame_alarm_hysteresis->add(*inner_vbox);
                Gtk::Box *hbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                inner_vbox->pack_start(*hbox, Gtk::PACK_SHRINK);
                Gtk::Box *vbox_borders =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                hbox->pack_start(*vbox_borders, Gtk::PACK_SHRINK);
                spin_higher_hysteresis.set_digits(15);
                spin_lower_hysteresis.set_digits(15);
                spin_higher_hysteresis.set_numeric(true);
                spin_lower_hysteresis.set_numeric(true);
                spin_higher_hysteresis.signal_output().connect([this]() {
                    double v = spin_higher_hysteresis.get_value();
                    std::ostringstream os;
                    os << std::fixed << std::setprecision(15) << v;
                    std::string s = os.str();
                    while (s.size() > 1 && s.back() == '0')
                        s.resize(s.size() - 1);
                    if (s.size() > 1 && s.back() == '.')
                        s.resize(s.size() - 1);
                    spin_higher_hysteresis.set_text(s);
                    return true;
                });
                spin_lower_hysteresis.signal_output().connect([this]() {
                    double v = spin_lower_hysteresis.get_value();
                    std::ostringstream os;
                    os << std::fixed << std::setprecision(15) << v;
                    std::string s = os.str();
                    while (s.size() > 1 && s.back() == '0')
                        s.resize(s.size() - 1);
                    if (s.size() > 1 && s.back() == '.')
                        s.resize(s.size() - 1);
                    spin_lower_hysteresis.set_text(s);
                    return true;
                });
                vbox_borders->pack_start(spin_higher_hysteresis,
                                         Gtk::PACK_SHRINK);
                spin_higher_hysteresis.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    double val = spin_higher_hysteresis.get_value();
                    double old = curr_situation_row[situation_cols.intense_up];
                    if (old == val ||
                        (std::isfinite(old) && std::isfinite(val) &&
                         std::abs(old - val) <= 1e-18 * (1.0 + std::abs(old))))
                        return;
                    curr_situation_row[situation_cols.intense_up] = val;
                    button_save_enable();
                });
                vbox_borders->pack_start(spin_lower_hysteresis,
                                         Gtk::PACK_SHRINK);
                spin_lower_hysteresis.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    double val = spin_lower_hysteresis.get_value();
                    double old =
                        curr_situation_row[situation_cols.intense_down];
                    if (old == val ||
                        (std::isfinite(old) && std::isfinite(val) &&
                         std::abs(old - val) <= 1e-18 * (1.0 + std::abs(old))))
                        return;
                    curr_situation_row[situation_cols.intense_down] = val;
                    button_save_enable();
                });
                hbox->pack_start(combobox_hysteresis, Gtk::PACK_SHRINK);
                combobox_hysteresis.signal_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    int val = combobox_hysteresis.get_active_row_number();
                    if (curr_situation_row[situation_cols.intense_absolute] ==
                        val)
                        return;
                    curr_situation_row[situation_cols.intense_absolute] = val;
                    button_save_enable();
                });
                combobox_hysteresis.set_valign(Gtk::ALIGN_CENTER);
                combobox_hysteresis.append("В % от шкалы измерения");
                combobox_hysteresis.append("В абсолютных значениях");
                set_margin(*hbox, 10, 10);
                situation_additional_menus[0] = vbox;
                stack_alarms.add(*vbox);
                vbox->show_all();
            }
            spin_higher_hysteresis.set_value(
                curr_situation_row[situation_cols.intense_up]);
            spin_lower_hysteresis.set_value(
                curr_situation_row[situation_cols.intense_down]);
            combobox_hysteresis.set_active(
                curr_situation_row[situation_cols.intense_absolute] ? 1 : 0);
            curr_alarms_menu_idx = 3;
            stack_alarms.set_visible_child(*situation_additional_menus[0]);
        }

        void redraw_alarms_sms() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            if (!situation_additional_menus[1]) {
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::Box *hbox_contacts =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_contacts, Gtk::PACK_SHRINK);
                Gtk::Frame *frame_alarm_sms = Gtk::make_managed<Gtk::Frame>(
                    "Отправлять смс следующим абонентам");
                hbox_contacts->pack_start(*frame_alarm_sms,
                                          Gtk::PACK_EXPAND_WIDGET);
                frame_alarm_sms->override_background_color(
                    theme_bg_color, Gtk::STATE_FLAG_NORMAL);
                treeview_sms_contacts = Gtk::make_managed<Gtk::TreeView>();
                treeview_sms_contacts->append_column("Название(имя)",
                                                     contact_cols.name);
                treeview_sms_contacts->append_column("Телефон",
                                                     contact_cols.phone);
                frame_alarm_sms->add(*treeview_sms_contacts);
                Gtk::Box *vbox_buttons =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                hbox_contacts->pack_start(*vbox_buttons, Gtk::PACK_SHRINK);
                vbox_buttons->set_margin_top(10);
                Gtk::Button *button_contact_address_book =
                    Gtk::make_managed<Gtk::Button>();
                vbox_buttons->pack_start(*button_contact_address_book,
                                         Gtk::PACK_SHRINK);
                button_contact_address_book->signal_clicked().connect([this]() {
                    Gtk::MessageDialog dialog("Адресная книга", false,
                                              Gtk::MESSAGE_INFO,
                                              Gtk::BUTTONS_NONE);
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    dialog.set_size_request(600, 500);
                    Gtk::Box *vbox = dialog.get_content_area();
                    treestore_contacts->clear();
                    std::string errors;
                    (void)parse_contacts_config(
                        stations_config_path, treestore_contacts,
                        pixbuf_contact, pixbuf_contact_group, contact_cols,
                        errors);
                    Gtk::ScrolledWindow *scrolled =
                        Gtk::make_managed<Gtk::ScrolledWindow>();
                    scrolled->set_policy(Gtk::POLICY_AUTOMATIC,
                                         Gtk::POLICY_AUTOMATIC);
                    vbox->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);
                    Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
                    scrolled->add(*frame);
                    frame->override_background_color(theme_bg_color,
                                                     Gtk::STATE_FLAG_NORMAL);
                    Gtk::TreeView *treeview =
                        Gtk::make_managed<Gtk::TreeView>();
                    frame->add(*treeview);
                    treeview->set_model(treestore_contacts);
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer_check =
                            Gtk::make_managed<Gtk::CellRendererToggle>();
                        renderer_check->signal_toggled().connect(
                            [this](const Glib::ustring &path) {
                                Gtk::TreeModel::iterator iter =
                                    treestore_contacts->get_iter(
                                        Gtk::TreeModel::Path(path));
                                if (!iter)
                                    return;
                                bool new_val =
                                    !(*iter)[contact_cols.is_selected];
                                (*iter)[contact_cols.is_selected] = new_val;
                                auto set_children_selected =
                                    [this, new_val](
                                        const auto &self,
                                        const Gtk::TreeNodeChildren &children)
                                    -> void {
                                    for (const Gtk::TreeModel::Row &row :
                                         children) {
                                        row[contact_cols.is_selected] = new_val;
                                        self(self, row.children());
                                    }
                                };
                                set_children_selected(set_children_selected,
                                                      iter->children());
                            });
                        renderer_check->set_fixed_size(-1, 32);
                        column->pack_start(*renderer_check, false);
                        column->add_attribute(renderer_check->property_active(),
                                              contact_cols.is_selected);
                        auto *renderer_icon =
                            Gtk::make_managed<Gtk::CellRendererPixbuf>();
                        renderer_icon->set_fixed_size(-1, 32);
                        column->pack_start(*renderer_icon, false);
                        column->add_attribute(renderer_icon->property_pixbuf(),
                                              contact_cols.icon);
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.name);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_contact_name);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Название(имя)");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.comment);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_description);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Комментарий");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.email);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_email);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("E-mail");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.phone);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_phone);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Телефон");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    dialog.show_all();

                    int res = dialog.run();
                    if (res != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::RefPtr<Gtk::ListStore> contacts =
                        situation_params[0].contacts;
                    if (!contacts) {
                        contacts = Gtk::ListStore::create(contact_cols);
                        treeview_sms_contacts->set_model(contacts);
                    }
                    auto append_selected =
                        [&contacts,
                         this](const auto &self,
                               const Gtk::TreeNodeChildren &children) -> void {
                        for (const Gtk::TreeModel::Row &row : children) {
                            if (!row[contact_cols.is_selected])
                                continue;
                            if (row[contact_cols.is_group]) {
                                self(self, row.children());
                                return;
                            }
                            bool already_exists = false;
                            for (const Gtk::TreeModel::Row
                                     &existing_contact_row :
                                 contacts->children()) {
                                if (existing_contact_row[contact_cols.name] ==
                                        row[contact_cols.name] &&
                                    existing_contact_row[contact_cols.phone] ==
                                        row[contact_cols.phone]) {
                                    already_exists = true;
                                    break;
                                }
                            }
                            if (already_exists)
                                continue;
                            Gtk::TreeModel::Row new_row = *contacts->append();
                            new_row[contact_cols.icon] =
                                static_cast<Glib::RefPtr<Gdk::Pixbuf>>(
                                    row[contact_cols.icon]);
                            new_row[contact_cols.is_group] = false;
                            new_row[contact_cols.name] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.name]);
                            new_row[contact_cols.comment] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.comment]);
                            new_row[contact_cols.email] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.email]);
                            new_row[contact_cols.phone] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.phone]);
                        }
                    };
                    append_selected(append_selected,
                                    treestore_contacts->children());
                    situation_params[0].contacts = contacts;
                    curr_situation_row[situation_cols.params] =
                        situation_params;
                    button_save_enable();
                });
                Gtk::Image *image_contact_address_book =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_address_book->set_image(
                    *image_contact_address_book);
                image_contact_address_book->set(pixbuf_address_book);
                Gtk::Separator *separator1 = Gtk::make_managed<Gtk::Separator>(
                    Gtk::ORIENTATION_HORIZONTAL);
                vbox_buttons->pack_start(*separator1, Gtk::PACK_SHRINK);
                Gtk::Button *button_contact_remove =
                    Gtk::make_managed<Gtk::Button>();
                button_contact_remove->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Gtk::TreeModel::iterator contact_iter =
                        treeview_sms_contacts->get_selection()->get_selected();
                    if (!contact_iter)
                        return;
                    Gtk::TreeModel::Row curr_contact_row = *contact_iter;
                    Gtk::MessageDialog dialog(
                        "Удалить получателя \"" +
                            static_cast<Glib::ustring>(
                                curr_contact_row[contact_cols.name]) +
                            "\"?",
                        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    if (dialog.run() != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::RefPtr<Gtk::ListStore> liststore_contacts =
                        curr_situation_params[0].contacts;
                    if (!liststore_contacts)
                        return;
                    liststore_contacts->erase(curr_contact_row);
                    curr_situation_params[0].contacts = liststore_contacts;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    redraw_alarms_sms();
                    button_save_enable();
                });
                vbox_buttons->pack_start(*button_contact_remove,
                                         Gtk::PACK_SHRINK);
                Gtk::Image *image_contact_remove =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_remove->set_image(*image_contact_remove);
                image_contact_remove->set(pixbuf_delete);
                Gtk::Separator *separator2 = Gtk::make_managed<Gtk::Separator>(
                    Gtk::ORIENTATION_HORIZONTAL);
                vbox_buttons->pack_start(*separator2, Gtk::PACK_SHRINK);
                Gtk::Button *button_contact_clear =
                    Gtk::make_managed<Gtk::Button>();
                vbox_buttons->pack_start(*button_contact_clear,
                                         Gtk::PACK_SHRINK);
                button_contact_clear->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Glib::RefPtr<Gtk::ListStore> liststore_contacts =
                        Gtk::ListStore::create(contact_cols);
                    if (!liststore_contacts)
                        return;
                    Gtk::MessageDialog dialog("Удалить всех получателей?",
                                              false, Gtk::MESSAGE_QUESTION,
                                              Gtk::BUTTONS_NONE);
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    if (dialog.run() != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[0].contacts =
                        Gtk::ListStore::create(contact_cols);
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    redraw_alarms_sms();
                    button_save_enable();
                });
                Gtk::Image *image_contact_clear =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_clear->set_image(*image_contact_clear);
                image_contact_clear->set(pixbuf_clear);
                Gtk::Box *hbox_text =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_text, Gtk::PACK_SHRINK);
                hbox_text->pack_start(checkbutton_sms_text, Gtk::PACK_SHRINK);
                checkbutton_sms_text.signal_toggled().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    bool active = checkbutton_sms_text.get_active();
                    if (curr_situation_params[0].bool1 == active)
                        return;
                    curr_situation_params[0].bool1 = active;
                    entry_sms_text.set_sensitive(active);
                    button_sms_add_param.set_sensitive(active);
                    if (active)
                        entry_sms_text.set_text(
                            curr_situation_params[0].string1);
                    else
                        entry_sms_text.set_text(static_cast<Glib::ustring>(
                            curr_situation_row[situation_cols.alarm_message]));
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_text->pack_start(entry_sms_text, Gtk::PACK_EXPAND_WIDGET);
                entry_sms_text.signal_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::ustring text = entry_sms_text.get_text();
                    if (curr_situation_params[0].string1 == text)
                        return;
                    curr_situation_params[0].string1 = text;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                button_sms_add_param_icon.set(pixbuf_write);
                button_sms_add_param.set_image(button_sms_add_param_icon);
                hbox_text->pack_end(button_sms_add_param, Gtk::PACK_SHRINK);
                button_sms_add_param.signal_clicked().connect([this]() {
                    Passport passport = on_passport_choice(0);
                    if (!passport.selected)
                        return;
                    std::string param_mask =
                        "%{" + std::to_string(passport.station_id) + ":" +
                        std::to_string(passport.passport_type) + ":" +
                        std::to_string(passport.group_id) + ":" +
                        std::to_string(passport.passport_id) + "}";
                    int pos = entry_sms_text.get_position();
                    Glib::ustring ustr(param_mask);
                    entry_sms_text.insert_text(ustr, ustr.bytes(), pos);
                    entry_sms_text.set_position(pos);
                });
                Gtk::Box *hbox_priority =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_priority, Gtk::PACK_SHRINK);
                Gtk::Label *label_priority =
                    Gtk::make_managed<Gtk::Label>("Приоритет (0-4)");
                hbox_priority->pack_start(*label_priority, Gtk::PACK_SHRINK);
                label_priority->set_halign(Gtk::ALIGN_START);
                hbox_priority->pack_end(spin_sms_priority, Gtk::PACK_SHRINK);
                spin_sms_priority.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_sms_priority.get_value_as_int();
                    if (curr_situation_params[0].type1 == val)
                        return;
                    curr_situation_params[0].type1 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_timeout =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_timeout, Gtk::PACK_SHRINK);
                Gtk::Label *label_timeout = Gtk::make_managed<Gtk::Label>(
                    "Время жизни (0 - по умолчанию)");
                hbox_timeout->pack_start(*label_timeout, Gtk::PACK_SHRINK);
                label_timeout->set_halign(Gtk::ALIGN_START);
                hbox_timeout->pack_end(spin_sms_timeout, Gtk::PACK_SHRINK);
                spin_sms_timeout.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_sms_timeout.get_value_as_int();
                    if (curr_situation_params[0].int1 == val)
                        return;
                    curr_situation_params[0].int1 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_delay =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_delay, Gtk::PACK_SHRINK);
                hbox_delay->pack_start(checkbutton_sms_delay, Gtk::PACK_SHRINK);
                checkbutton_sms_delay.signal_toggled().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    bool active = checkbutton_sms_delay.get_active();
                    if (curr_situation_params[0].bool2 == active)
                        return;
                    curr_situation_params[0].bool2 = active;
                    spin_sms_delay.set_sensitive(active);
                    if (active)
                        spin_sms_delay.set_value(curr_situation_params[0].int2);
                    else
                        spin_sms_delay.set_value(0);
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_delay->pack_end(spin_sms_delay, Gtk::PACK_SHRINK);
                spin_sms_delay.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_sms_delay.get_value_as_int();
                    if (curr_situation_params[0].int2 == val)
                        return;
                    curr_situation_params[0].int2 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                set_margin(*treeview_sms_contacts, 10, 10);
                situation_additional_menus[1] = vbox;
                stack_alarms.add(*vbox);
                vbox->show_all();
            }
            std::array<SituationParams, 4> curr_situation_params =
                curr_situation_row[situation_cols.params];
            SituationParams sms_params = curr_situation_params[0];
            if (checkbutton_sms_text.get_active() == sms_params.bool1 &&
                (sms_params.bool1
                     ? static_cast<Glib::ustring>(entry_sms_text.get_text()) ==
                           sms_params.string1
                     : entry_sms_text.get_text() ==
                           static_cast<Glib::ustring>(
                               curr_situation_row[situation_cols
                                                      .alarm_message])) &&
                spin_sms_priority.get_value_as_int() ==
                    static_cast<int>(sms_params.type1) &&
                spin_sms_timeout.get_value_as_int() ==
                    static_cast<int>(sms_params.int1) &&
                checkbutton_sms_delay.get_active() == sms_params.bool2 &&
                spin_sms_delay.get_value_as_int() ==
                    (sms_params.bool2 ? sms_params.int2 : 0) &&
                treeview_sms_contacts->get_model() == sms_params.contacts) {
                curr_alarms_menu_idx = 4;
                stack_alarms.set_visible_child(*situation_additional_menus[1]);
                return;
            }
            treeview_sms_contacts->set_model(sms_params.contacts);
            checkbutton_sms_text.set_active(sms_params.bool1);
            entry_sms_text.set_sensitive(sms_params.bool1);
            button_sms_add_param.set_sensitive(sms_params.bool1);
            if (sms_params.bool1)
                entry_sms_text.set_text(sms_params.string1);
            else
                entry_sms_text.set_text(static_cast<Glib::ustring>(
                    curr_situation_row[situation_cols.alarm_message]));

            spin_sms_priority.set_value(sms_params.type1);
            spin_sms_timeout.set_value(sms_params.int1);
            checkbutton_sms_delay.set_active(sms_params.bool2);
            spin_sms_delay.set_sensitive(sms_params.bool2);
            spin_sms_delay.set_value(sms_params.bool2 ? sms_params.int2 : 0);
            curr_alarms_menu_idx = 4;
            stack_alarms.set_visible_child(*situation_additional_menus[1]);
        }

        void redraw_alarms_email() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            if (!situation_additional_menus[2]) {
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::Box *hbox_contacts =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_contacts, Gtk::PACK_SHRINK);
                Gtk::Frame *frame_alarm_email = Gtk::make_managed<Gtk::Frame>(
                    "Отправлять письмо следующим абонентам");
                hbox_contacts->pack_start(*frame_alarm_email,
                                          Gtk::PACK_EXPAND_WIDGET);
                frame_alarm_email->override_background_color(
                    theme_bg_color, Gtk::STATE_FLAG_NORMAL);
                treeview_email_contacts = Gtk::make_managed<Gtk::TreeView>();
                frame_alarm_email->add(*treeview_email_contacts);
                treeview_email_contacts->append_column("Название(имя)",
                                                       contact_cols.name);
                treeview_email_contacts->append_column("E-mail",
                                                       contact_cols.email);
                Gtk::Box *vbox_buttons =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                hbox_contacts->pack_start(*vbox_buttons, Gtk::PACK_SHRINK);
                vbox_buttons->set_margin_top(10);
                Gtk::Button *button_contact_address_book =
                    Gtk::make_managed<Gtk::Button>();
                vbox_buttons->pack_start(*button_contact_address_book,
                                         Gtk::PACK_SHRINK);
                button_contact_address_book->signal_clicked().connect([this]() {
                    Gtk::MessageDialog dialog("Адресная книга", false,
                                              Gtk::MESSAGE_INFO,
                                              Gtk::BUTTONS_NONE);
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    dialog.set_size_request(600, 500);
                    Gtk::Box *vbox = dialog.get_content_area();
                    treestore_contacts->clear();
                    std::string errors;
                    (void)parse_contacts_config(
                        stations_config_path, treestore_contacts,
                        pixbuf_contact, pixbuf_contact_group, contact_cols,
                        errors);
                    Gtk::ScrolledWindow *scrolled =
                        Gtk::make_managed<Gtk::ScrolledWindow>();
                    scrolled->set_policy(Gtk::POLICY_AUTOMATIC,
                                         Gtk::POLICY_AUTOMATIC);
                    vbox->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);
                    Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
                    scrolled->add(*frame);
                    frame->override_background_color(theme_bg_color,
                                                     Gtk::STATE_FLAG_NORMAL);
                    Gtk::TreeView *treeview =
                        Gtk::make_managed<Gtk::TreeView>();
                    frame->add(*treeview);
                    treeview->set_model(treestore_contacts);
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer_check =
                            Gtk::make_managed<Gtk::CellRendererToggle>();
                        renderer_check->signal_toggled().connect(
                            [this](const Glib::ustring &path) {
                                Gtk::TreeModel::iterator iter =
                                    treestore_contacts->get_iter(
                                        Gtk::TreeModel::Path(path));
                                if (!iter)
                                    return;
                                bool new_val =
                                    !(*iter)[contact_cols.is_selected];
                                (*iter)[contact_cols.is_selected] = new_val;
                                auto set_children_selected =
                                    [this, new_val](
                                        const auto &self,
                                        const Gtk::TreeNodeChildren &children)
                                    -> void {
                                    for (const Gtk::TreeModel::Row &row :
                                         children) {
                                        row[contact_cols.is_selected] = new_val;
                                        self(self, row.children());
                                    }
                                };
                                set_children_selected(set_children_selected,
                                                      iter->children());
                            });
                        renderer_check->set_fixed_size(-1, 32);
                        column->pack_start(*renderer_check, false);
                        column->add_attribute(renderer_check->property_active(),
                                              contact_cols.is_selected);
                        auto *renderer_icon =
                            Gtk::make_managed<Gtk::CellRendererPixbuf>();
                        renderer_icon->set_fixed_size(-1, 32);
                        column->pack_start(*renderer_icon, false);
                        column->add_attribute(renderer_icon->property_pixbuf(),
                                              contact_cols.icon);
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.name);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_contact_name);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Название(имя)");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.comment);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_description);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Комментарий");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.email);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_email);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("E-mail");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    {
                        auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                        auto *renderer =
                            Gtk::make_managed<Gtk::CellRendererText>();
                        renderer->set_fixed_size(-1, 32);
                        column->pack_start(*renderer, true);
                        column->add_attribute(renderer->property_text(),
                                              contact_cols.phone);
                        auto *header_box = Gtk::make_managed<Gtk::Box>(
                            Gtk::ORIENTATION_HORIZONTAL, 4);
                        auto *header_image =
                            Gtk::make_managed<Gtk::Image>(pixbuf_phone);
                        auto *header_label =
                            Gtk::make_managed<Gtk::Label>("Телефон");
                        header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                        header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                        header_box->show_all();
                        column->set_widget(*header_box);
                        treeview->append_column(*column);
                    }
                    dialog.show_all();

                    int res = dialog.run();
                    if (res != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::RefPtr<Gtk::ListStore> contacts =
                        situation_params[1].contacts;
                    if (!contacts) {
                        contacts = Gtk::ListStore::create(contact_cols);
                        treeview_email_contacts->set_model(contacts);
                    }
                    auto append_selected =
                        [&contacts,
                         this](const auto &self,
                               const Gtk::TreeNodeChildren &children) -> void {
                        for (const Gtk::TreeModel::Row &row : children) {
                            if (!row[contact_cols.is_selected])
                                continue;
                            if (row[contact_cols.is_group]) {
                                self(self, row.children());
                                return;
                            }
                            bool already_exists = false;
                            for (const Gtk::TreeModel::Row
                                     &existing_contact_row :
                                 contacts->children()) {
                                if (existing_contact_row[contact_cols.name] ==
                                        row[contact_cols.name] &&
                                    existing_contact_row[contact_cols.email] ==
                                        row[contact_cols.email]) {
                                    already_exists = true;
                                    break;
                                }
                            }
                            if (already_exists)
                                continue;
                            Gtk::TreeModel::Row new_row = *contacts->append();
                            new_row[contact_cols.icon] =
                                static_cast<Glib::RefPtr<Gdk::Pixbuf>>(
                                    row[contact_cols.icon]);
                            new_row[contact_cols.is_group] = false;
                            new_row[contact_cols.name] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.name]);
                            new_row[contact_cols.comment] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.comment]);
                            new_row[contact_cols.email] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.email]);
                            new_row[contact_cols.phone] =
                                static_cast<Glib::ustring>(
                                    row[contact_cols.phone]);
                        }
                    };
                    append_selected(append_selected,
                                    treestore_contacts->children());
                    situation_params[2].contacts = contacts;
                    curr_situation_row[situation_cols.params] =
                        situation_params;
                    button_save_enable();
                });
                Gtk::Image *image_contact_address_book =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_address_book->set_image(
                    *image_contact_address_book);
                image_contact_address_book->set(pixbuf_address_book);
                Gtk::Separator *separator1 = Gtk::make_managed<Gtk::Separator>(
                    Gtk::ORIENTATION_HORIZONTAL);
                vbox_buttons->pack_start(*separator1, Gtk::PACK_SHRINK);
                Gtk::Button *button_contact_remove =
                    Gtk::make_managed<Gtk::Button>();
                vbox_buttons->pack_start(*button_contact_remove,
                                         Gtk::PACK_SHRINK);
                button_contact_remove->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Gtk::TreeModel::iterator contact_iter =
                        treeview_email_contacts->get_selection()
                            ->get_selected();
                    if (!contact_iter)
                        return;
                    Gtk::TreeModel::Row curr_contact_row = *contact_iter;
                    Gtk::MessageDialog dialog(
                        "Удалить получателя \"" +
                            static_cast<Glib::ustring>(
                                curr_contact_row[contact_cols.name]) +
                            "\"?",
                        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    if (dialog.run() != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::RefPtr<Gtk::ListStore> liststore_contacts =
                        curr_situation_params[1].contacts;
                    if (!liststore_contacts)
                        return;
                    liststore_contacts->erase(curr_contact_row);
                    curr_situation_params[1].contacts = liststore_contacts;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    redraw_alarms_email();
                    button_save_enable();
                });
                Gtk::Image *image_contact_remove =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_remove->set_image(*image_contact_remove);
                image_contact_remove->set(pixbuf_delete);
                Gtk::Separator *separator2 = Gtk::make_managed<Gtk::Separator>(
                    Gtk::ORIENTATION_HORIZONTAL);
                vbox_buttons->pack_start(*separator2, Gtk::PACK_SHRINK);
                Gtk::Button *button_contact_clear =
                    Gtk::make_managed<Gtk::Button>();
                vbox_buttons->pack_start(*button_contact_clear,
                                         Gtk::PACK_SHRINK);
                button_contact_clear->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Glib::RefPtr<Gtk::ListStore> liststore_contacts =
                        Gtk::ListStore::create(contact_cols);
                    if (!liststore_contacts)
                        return;
                    Gtk::MessageDialog dialog("Удалить всех получателей?",
                                              false, Gtk::MESSAGE_QUESTION,
                                              Gtk::BUTTONS_NONE);
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                    dialog.add_button("ОК", Gtk::RESPONSE_OK);
                    if (dialog.run() != Gtk::RESPONSE_OK)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[1].contacts =
                        Gtk::ListStore::create(contact_cols);
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    redraw_alarms_email();
                    button_save_enable();
                });
                Gtk::Image *image_contact_clear =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_clear->set_image(*image_contact_clear);
                image_contact_clear->set(pixbuf_clear);
                Gtk::Box *hbox_subject =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_subject, Gtk::PACK_SHRINK);
                hbox_subject->pack_start(checkbutton_email_subject,
                                         Gtk::PACK_SHRINK);
                checkbutton_email_subject.signal_toggled().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    bool active = checkbutton_email_subject.get_active();
                    if (curr_situation_params[1].bool1 == active)
                        return;
                    curr_situation_params[1].bool1 = active;
                    entry_email_subject.set_sensitive(active);
                    button_email_subject_add_param.set_sensitive(active);
                    if (active)
                        entry_email_subject.set_text(
                            curr_situation_params[1].string1);
                    else
                        entry_email_subject.set_text("");
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_subject->pack_start(entry_email_subject,
                                         Gtk::PACK_EXPAND_WIDGET);
                button_email_subject_add_param_icon.set(pixbuf_write);
                button_email_subject_add_param.set_image(
                    button_email_subject_add_param_icon);
                hbox_subject->pack_end(button_email_subject_add_param,
                                       Gtk::PACK_SHRINK);
                button_email_subject_add_param.signal_clicked().connect(
                    [this]() {
                        Passport passport = on_passport_choice(0);
                        if (!passport.selected)
                            return;
                        std::string param_mask =
                            "%{" + std::to_string(passport.station_id) + ":" +
                            std::to_string(passport.passport_type) + ":" +
                            std::to_string(passport.group_id) + ":" +
                            std::to_string(passport.passport_id) + "}";
                        int pos = entry_email_subject.get_position();
                        Glib::ustring ustr(param_mask);
                        entry_email_subject.insert_text(ustr, ustr.bytes(),
                                                        pos);
                        entry_email_subject.set_position(pos);
                    });
                entry_email_subject.signal_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::ustring text = entry_email_subject.get_text();
                    if (curr_situation_params[1].string1 == text)
                        return;
                    curr_situation_params[1].string1 = text;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_text =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_text, Gtk::PACK_SHRINK);
                hbox_text->pack_start(checkbutton_email_text, Gtk::PACK_SHRINK);
                checkbutton_email_text.signal_toggled().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    bool active = checkbutton_email_text.get_active();
                    if (curr_situation_params[1].bool2 == active)
                        return;
                    curr_situation_params[1].bool2 = active;
                    entry_email_text.set_sensitive(active);
                    button_email_text_add_param.set_sensitive(active);
                    if (active)
                        entry_email_text.set_text(
                            curr_situation_params[1].string1);
                    else
                        entry_email_text.set_text(static_cast<Glib::ustring>(
                            curr_situation_row[situation_cols.alarm_message]));
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_text->pack_start(entry_email_text,
                                      Gtk::PACK_EXPAND_WIDGET);
                entry_email_text.signal_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    Glib::ustring text = entry_email_text.get_text();
                    if (curr_situation_params[1].string2 == text)
                        return;
                    curr_situation_params[1].string2 = text;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                button_email_text_add_param_icon.set(pixbuf_write);
                button_email_text_add_param.set_image(
                    button_email_text_add_param_icon);
                hbox_text->pack_end(button_email_text_add_param,
                                    Gtk::PACK_SHRINK);
                button_email_text_add_param.signal_clicked().connect([this]() {
                    Passport passport = on_passport_choice(0);
                    if (!passport.selected)
                        return;
                    std::string param_mask =
                        "%{" + std::to_string(passport.station_id) + ":" +
                        std::to_string(passport.passport_type) + ":" +
                        std::to_string(passport.group_id) + ":" +
                        std::to_string(passport.passport_id) + "}";
                    int pos = entry_email_text.get_position();
                    Glib::ustring ustr(param_mask);
                    entry_email_text.insert_text(ustr, ustr.bytes(), pos);
                    entry_email_text.set_position(pos);
                });
                Gtk::Box *hbox_priority =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_priority, Gtk::PACK_SHRINK);
                Gtk::Label *label_priority =
                    Gtk::make_managed<Gtk::Label>("Приоритет (0-4)");
                hbox_priority->pack_start(*label_priority, Gtk::PACK_SHRINK);
                label_priority->set_halign(Gtk::ALIGN_START);
                hbox_priority->pack_end(spin_email_priority, Gtk::PACK_SHRINK);
                spin_email_priority.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_email_priority.get_value_as_int();
                    if (curr_situation_params[1].type1 == val)
                        return;
                    curr_situation_params[1].type1 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_timeout =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_timeout, Gtk::PACK_SHRINK);
                Gtk::Label *label_timeout =
                    Gtk::make_managed<Gtk::Label>("Время жизни (сек)");
                hbox_timeout->pack_start(*label_timeout, Gtk::PACK_SHRINK);
                label_timeout->set_halign(Gtk::ALIGN_START);
                hbox_timeout->pack_end(spin_email_timeout, Gtk::PACK_SHRINK);
                spin_email_timeout.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_email_timeout.get_value_as_int();
                    if (curr_situation_params[1].int1 == val)
                        return;
                    curr_situation_params[1].int1 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_delay =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_delay, Gtk::PACK_SHRINK);

                hbox_delay->pack_start(checkbutton_email_delay,
                                       Gtk::PACK_SHRINK);
                checkbutton_email_delay.signal_toggled().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    bool active = checkbutton_email_delay.get_active();
                    if (curr_situation_params[1].bool3 == active)
                        return;
                    curr_situation_params[1].bool3 = active;
                    spin_email_delay.set_sensitive(active);
                    if (active)
                        spin_email_delay.set_value(
                            curr_situation_params[1].int2);
                    else
                        spin_email_delay.set_value(0);
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_delay->pack_end(spin_email_delay, Gtk::PACK_SHRINK);
                spin_email_delay.signal_value_changed().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    int val = spin_email_delay.get_value_as_int();
                    if (curr_situation_params[1].int2 == val)
                        return;
                    curr_situation_params[1].int2 = val;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                set_margin(*treeview_email_contacts, 10, 10);
                situation_additional_menus[2] = vbox;
                stack_alarms.add(*vbox);
                vbox->show_all();
            }
            std::array<SituationParams, 4> curr_situation_params =
                curr_situation_row[situation_cols.params];
            SituationParams email_params = curr_situation_params[1];
            if (checkbutton_email_subject.get_active() == email_params.bool1 &&
                static_cast<Glib::ustring>(entry_email_subject.get_text()) ==
                    (email_params.bool1 ? email_params.string1 : "") &&
                checkbutton_email_text.get_active() == email_params.bool2 &&
                (email_params.bool2
                     ? static_cast<Glib::ustring>(
                           entry_email_text.get_text()) == email_params.string2
                     : entry_email_text.get_text() ==
                           static_cast<Glib::ustring>(
                               curr_situation_row[situation_cols
                                                      .alarm_message])) &&
                spin_email_priority.get_value_as_int() ==
                    static_cast<int>(email_params.type1) &&
                spin_email_timeout.get_value_as_int() ==
                    static_cast<int>(email_params.int1) &&
                checkbutton_email_delay.get_active() == email_params.bool3 &&
                spin_email_delay.get_value_as_int() ==
                    (email_params.bool3 ? email_params.int2 : 0) &&
                treeview_email_contacts->get_model() == email_params.contacts) {
                curr_alarms_menu_idx = 5;
                stack_alarms.set_visible_child(*situation_additional_menus[2]);
                return;
            }
            treeview_email_contacts->set_model(email_params.contacts);
            checkbutton_email_subject.set_active(email_params.bool1);
            entry_email_subject.set_sensitive(email_params.bool1);
            button_email_subject_add_param.set_sensitive(email_params.bool1);
            entry_email_subject.set_text(
                email_params.bool1 ? email_params.string1 : "");
            checkbutton_email_text.set_active(email_params.bool2);
            entry_email_text.set_sensitive(email_params.bool2);
            button_email_text_add_param.set_sensitive(email_params.bool2);
            if (email_params.bool2)
                entry_email_text.set_text(email_params.string2);
            else
                entry_email_text.set_text(static_cast<Glib::ustring>(
                    curr_situation_row[situation_cols.alarm_message]));

            spin_email_priority.set_value(email_params.type1);
            spin_email_timeout.set_value(email_params.int1);
            checkbutton_email_delay.set_active(email_params.bool3);
            spin_email_delay.set_sensitive(email_params.bool3);
            spin_email_delay.set_value(email_params.bool3 ? email_params.int2
                                                          : 0);
            curr_alarms_menu_idx = 5;
            stack_alarms.set_visible_child(*situation_additional_menus[2]);
        }

        void redraw_alarms_passport() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            if (!situation_additional_menus[3]) {
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::Frame *frame_passport = Gtk::make_managed<Gtk::Frame>(
                    "Ситуация сработала, если паспорт");
                vbox->pack_start(*frame_passport, Gtk::PACK_SHRINK);
                Gtk::Box *vbox_passport =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                frame_passport->add(*vbox_passport);
                Gtk::Grid *grid_passport = Gtk::make_managed<Gtk::Grid>();
                vbox_passport->pack_start(*grid_passport, Gtk::PACK_SHRINK);
                grid_passport->set_column_spacing(5);
                grid_passport->set_row_spacing(5);
                Gtk::Label *label_passport_station =
                    Gtk::make_managed<Gtk::Label>("Станция");
                grid_passport->attach(*label_passport_station, 0, 0, 1, 1);
                label_passport_station->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(label_passport_station_val, 1, 0, 1, 1);
                label_passport_station_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_passport_type =
                    Gtk::make_managed<Gtk::Label>("Тип");
                grid_passport->attach(*label_passport_type, 0, 1, 1, 1);
                label_passport_type->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(label_passport_type_val, 1, 1, 1, 1);
                label_passport_type_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_passport_id =
                    Gtk::make_managed<Gtk::Label>("Идентификатор");
                label_passport_id->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(*label_passport_id, 0, 2, 1, 1);
                label_passport_id->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(label_passport_id_val, 1, 2, 1, 1);
                label_passport_id_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_passport_chipher =
                    Gtk::make_managed<Gtk::Label>("Шифр");
                grid_passport->attach(*label_passport_chipher, 0, 3, 1, 1);
                label_passport_chipher->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(label_passport_chipher_val, 1, 3, 1, 1);
                label_passport_chipher_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_passport_name =
                    Gtk::make_managed<Gtk::Label>("Имя");
                grid_passport->attach(*label_passport_name, 0, 4, 1, 1);
                label_passport_name->set_halign(Gtk::ALIGN_START);
                grid_passport->attach(label_passport_name_val, 1, 4, 1, 1);
                label_passport_name_val.set_halign(Gtk::ALIGN_START);
                Gtk::Frame *frame_value =
                    Gtk::make_managed<Gtk::Frame>("находится в состоянии");
                vbox->pack_start(*frame_value, Gtk::PACK_SHRINK);
                Gtk::Button *button_select =
                    Gtk::make_managed<Gtk::Button>("Выбрать");
                vbox_passport->pack_start(*button_select, Gtk::PACK_SHRINK);
                button_select->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Passport passport = on_passport_choice(1);
                    if (!passport.selected)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    label_passport_station_val.set_text(passport.station_name);
                    label_passport_type_val.set_text(
                        passport.passport_val_type_str);
                    label_passport_id_val.set_text(
                        std::to_string(passport.passport_id));
                    label_passport_chipher_val.set_text(passport.chipher);
                    label_passport_name_val.set_text(passport.fullname);
                    curr_situation_params[2].passport = passport;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Box *hbox_value =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                frame_value->add(*hbox_value);
                hbox_value->pack_start(radiobutton_passport0, Gtk::PACK_SHRINK);
                radiobutton_passport0.signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[2].bool1 = false;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_value->pack_start(radiobutton_passport1, Gtk::PACK_SHRINK);
                radiobutton_passport1.signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[2].bool1 = true;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                situation_additional_menus[3] = vbox;
                stack_alarms.add(*vbox);
                set_margin(*grid_passport, 10, 10);
                set_margin(*button_select, 10, 10);
                vbox->show_all();
            }
            std::array<SituationParams, 4> curr_situation_params =
                curr_situation_row[situation_cols.params];
            SituationParams passport_params = curr_situation_params[2];
            if (passport_params.passport.selected) {
                label_passport_station_val.set_text(
                    passport_params.passport.station_name);
                label_passport_type_val.set_text(
                    passport_params.passport.passport_val_type_str);
                label_passport_id_val.set_text(
                    std::to_string(passport_params.passport.passport_id));
                label_passport_chipher_val.set_text(
                    passport_params.passport.chipher);
                label_passport_name_val.set_text(
                    passport_params.passport.fullname);
            } else {
                label_passport_station_val.set_text("<нет данных>");
                label_passport_type_val.set_text("<нет данных>");
                label_passport_id_val.set_text("<нет данных>");
                label_passport_chipher_val.set_text("<нет данных>");
                label_passport_name_val.set_text("<нет данных>");
            }
            if (passport_params.bool1)
                radiobutton_passport1.set_active(true);
            else
                radiobutton_passport0.set_active(true);
            curr_alarms_menu_idx = 6;
            stack_alarms.set_visible_child(*situation_additional_menus[3]);
        }

        void redraw_alarms_write() {
            Gtk::TreeModel::Row curr_situation_row =
                *treeview_situations.get_selection()->get_selected();
            if (!curr_situation_row)
                return;
            if (!situation_additional_menus[4]) {
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::Frame *frame_alarm_write = Gtk::make_managed<Gtk::Frame>(
                    "При сработке писать в паспорт");
                vbox->pack_start(*frame_alarm_write, Gtk::PACK_SHRINK);
                Gtk::Box *vbox_write =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                frame_alarm_write->add(*vbox_write);
                Gtk::Grid *grid_write = Gtk::make_managed<Gtk::Grid>();
                vbox_write->pack_start(*grid_write, Gtk::PACK_SHRINK);
                grid_write->set_column_spacing(5);
                grid_write->set_row_spacing(5);
                Gtk::Label *label_write_station =
                    Gtk::make_managed<Gtk::Label>("Станция");
                grid_write->attach(*label_write_station, 0, 0, 1, 1);
                label_write_station->set_halign(Gtk::ALIGN_START);
                grid_write->attach(label_write_station_val, 1, 0, 1, 1);
                label_write_station_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_write_type =
                    Gtk::make_managed<Gtk::Label>("Тип");
                grid_write->attach(*label_write_type, 0, 1, 1, 1);
                label_write_type->set_halign(Gtk::ALIGN_START);
                grid_write->attach(label_write_type_val, 1, 1, 1, 1);
                label_write_type_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_write_id =
                    Gtk::make_managed<Gtk::Label>("Идентификатор");
                grid_write->attach(*label_write_id, 0, 2, 1, 1);
                label_write_id->set_halign(Gtk::ALIGN_START);
                grid_write->attach(label_write_id_val, 1, 2, 1, 1);
                label_write_id_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_write_chipher =
                    Gtk::make_managed<Gtk::Label>("Шифр");
                grid_write->attach(*label_write_chipher, 0, 3, 1, 1);
                label_write_chipher->set_halign(Gtk::ALIGN_START);
                grid_write->attach(label_write_chipher_val, 1, 3, 1, 1);
                label_write_chipher_val.set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_write_name =
                    Gtk::make_managed<Gtk::Label>("Имя");
                grid_write->attach(*label_write_name, 0, 4, 1, 1);
                label_write_name->set_halign(Gtk::ALIGN_START);
                grid_write->attach(label_write_name_val, 1, 4, 1, 1);
                label_write_name_val.set_halign(Gtk::ALIGN_START);
                Gtk::Button *button_select =
                    Gtk::make_managed<Gtk::Button>("Выбрать");
                vbox_write->pack_start(*button_select, Gtk::PACK_SHRINK);
                button_select->signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    Passport passport = on_passport_choice(1);
                    if (!passport.selected)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    label_write_station_val.set_text(passport.station_name);
                    label_write_type_val.set_text(
                        passport.passport_val_type_str);
                    label_write_id_val.set_text(
                        std::to_string(passport.passport_id));
                    label_write_chipher_val.set_text(passport.chipher);
                    label_write_name_val.set_text(passport.fullname);
                    curr_situation_params[3].passport = passport;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                Gtk::Frame *frame_value =
                    Gtk::make_managed<Gtk::Frame>("значение");
                vbox->pack_start(*frame_value, Gtk::PACK_SHRINK);
                Gtk::Box *hbox_value =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                frame_value->add(*hbox_value);
                hbox_value->pack_start(radiobutton_write0, Gtk::PACK_SHRINK);
                radiobutton_write0.signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[3].bool1 = false;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                hbox_value->pack_start(radiobutton_write1, Gtk::PACK_SHRINK);
                radiobutton_write1.signal_clicked().connect([this]() {
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    std::array<SituationParams, 4> curr_situation_params =
                        curr_situation_row[situation_cols.params];
                    curr_situation_params[3].bool1 = true;
                    curr_situation_row[situation_cols.params] =
                        curr_situation_params;
                    button_save_enable();
                });
                situation_additional_menus[4] = vbox;
                stack_alarms.add(*vbox);
                set_margin(*grid_write, 10, 10);
                set_margin(*button_select, 10, 10);
                vbox->show_all();
            }
            std::array<SituationParams, 4> curr_situation_params =
                curr_situation_row[situation_cols.params];
            SituationParams write_params = curr_situation_params[3];
            if (write_params.passport.selected) {
                label_write_station_val.set_text(
                    write_params.passport.station_name);
                label_write_type_val.set_text(
                    write_params.passport.passport_val_type_str);
                label_write_id_val.set_text(
                    std::to_string(write_params.passport.passport_id));
                label_write_chipher_val.set_text(write_params.passport.chipher);
                label_write_name_val.set_text(write_params.passport.fullname);
            } else {
                label_write_station_val.set_text("<нет данных>");
                label_write_type_val.set_text("<нет данных>");
                label_write_id_val.set_text("<нет данных>");
                label_write_chipher_val.set_text("<нет данных>");
                label_write_name_val.set_text("<нет данных>");
            }
            if (write_params.bool1)
                radiobutton_write1.set_active(true);
            else
                radiobutton_write0.set_active(true);
            curr_alarms_menu_idx = 7;
            stack_alarms.set_visible_child(*situation_additional_menus[4]);
        }

        void redraw_aps() {
            Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
            if (!curr_group_row)
                return;
            stack_aps_lowlevel_interaction_left.set_visible(curr_aps_menu_idx !=
                                                            2);
            switch (curr_aps_menu_idx) {
            case 0: {
                unsigned char block_by_passport_val =
                    curr_group_row[group_cols.block_by_passport_val];
                if (block_by_passport_val != 0)
                    checkbutton_block_by_passport.set_active(true);
                switch (block_by_passport_val) {
                case 1:
                    radiobutton0.set_active(true);
                    break;
                case 2:
                    radiobutton1.set_active(true);
                    break;
                }
                hbox_checkback_val.set_sensitive(
                    checkbutton_block_by_passport.get_active());
                stack_aps_lowlevel_interaction_left.set_visible_child(
                    frame_value);
                vbox_aps_lowlevel_interaction_right.set_visible(
                    checkbutton_block_by_passport.get_active());
                break;
            }
            case 1: {
                unsigned char checkback_val =
                    curr_group_row[group_cols.checkback_val];
                if (checkback_val != 0)
                    checkbutton_checkback.set_active(true);
                switch (checkback_val) {
                case 1:
                    radiobutton_checkback_edge.set_active(true);
                    break;
                case 2:
                    radiobutton_checkback_cutoff.set_active(true);
                    break;
                }
                hbox_checkback_val.set_sensitive(
                    checkbutton_checkback.get_active());
                stack_aps_lowlevel_interaction_left.set_visible_child(
                    frame_checkback);
                vbox_aps_lowlevel_interaction_right.set_visible(
                    checkbutton_checkback.get_active());
                break;
            }
            case 2:
                hbox_checkback_val.set_sensitive(false);
                vbox_aps_lowlevel_interaction_right.set_visible(
                    checkbutton_write_on_checkback.get_active());
                break;
            }
            std::array<Passport, 7> passports =
                curr_group_row[group_cols.passports];
            Passport passport = passports[4 + curr_aps_menu_idx];
            if (!passport.selected) {
                label_aps_station_val.set_text("<нет данных>");
                label_aps_type_val.set_text("<нет данных>");
                label_aps_id_val.set_text("<нет данных>");
                label_aps_chipher_val.set_text("<нет данных>");
                label_aps_name_val.set_text("<нет данных>");
                return;
            }
            label_aps_station_val.set_text(passport.station_name);
            label_aps_type_val.set_text(passport.passport_val_type_str);
            label_aps_id_val.set_text(std::to_string(passport.passport_id));
            label_aps_chipher_val.set_text(passport.chipher);
            label_aps_name_val.set_text(passport.fullname);
        }

        void on_zones_cell_data(Gtk::CellRenderer *cell,
                                const Gtk::TreeModel::iterator &iter) {
            Glib::ustring group = (*iter)[alarmobj_cols.userpath];
            bool free = group.empty();
            Gtk::CellRendererText *text_renderer =
                static_cast<Gtk::CellRendererText *>(cell);
            text_renderer->property_foreground_rgba() =
                free ? Gdk::RGBA("black") : Gdk::RGBA("darkgray");
        }

        void set_margin(Gtk::Widget &widget, int margin_horizontal,
                        int margin_vertical) {
            widget.set_margin_top(margin_vertical);
            widget.set_margin_left(margin_horizontal);
            widget.set_margin_right(margin_horizontal);
            widget.set_margin_bottom(margin_vertical);
        };

        int get_station_id_of_group(const Gtk::TreeModel::Row group_row) {
            if (!group_row.parent())
                return group_row[group_cols.station_id];
            return get_station_id_of_group(*group_row.parent());
        }

        int get_next_group_id_of_group(const Gtk::TreeModel::Row group_row) {
            if (!group_row.parent()) {
                int next_group_id = group_row[group_cols.next_group_id];
                group_row[group_cols.next_group_id] = next_group_id + 1;
                return next_group_id;
            }
            return get_station_id_of_group(*group_row.parent());
        }

        void
        play_sound_async(Gtk::TreeModel::iterator curr_sound_iter,
                         const Glib::RefPtr<Gtk::ListStore> &liststore_sounds) {
            if (sound_playing.load())
                pause_sound();
            sound_playing.store(true);
            button_sound_play.set_sensitive(false);
            button_sound_delete.set_sensitive(false);
            button_sound_loop.set_sensitive(false);
            button_sound_new.set_sensitive(false);
            button_sound_pause.set_sensitive(false);
            button_sound_up.set_sensitive(false);
            button_sound_down.set_sensitive(false);
            std::thread([this, curr_sound_iter, liststore_sounds]() {
                Gtk::TreeModel::iterator iter = curr_sound_iter;
                Gtk::TreeModel::iterator loop_start_iter;
                while (sound_playing.load()) {
                    Glib::signal_idle().connect([this, iter]() {
                        treeview_sounds.get_selection()->select(iter);
                        return false;
                    });
                    bool loop_start = (*iter)[sound_cols.loop_start];
                    if (loop_start)
                        loop_start_iter = iter;
                    std::string filepath_or_pause_duration_str =
                        static_cast<Glib::ustring>(
                            (*iter)[sound_cols.filepath_or_pause_duration]);
                    try {
                        int duration =
                            std::stoi(filepath_or_pause_duration_str);
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(duration));
                    } catch (...) {
                        if (!std::filesystem::exists(
                                filepath_or_pause_duration_str)) {
                            show_notification(
                                "Файл не найден",
                                "Звуковой файл \"" +
                                    filepath_or_pause_duration_str +
                                    "\" не найден.",
                                3);
                        }
                        std::string cmd = "aplay -q \"" +
                                          filepath_or_pause_duration_str +
                                          "\" 2>/dev/null";
                        FILE *pipe = popen(cmd.c_str(), "r");
                        if (pipe)
                            pclose(pipe);
                    }
                    iter++;
                    if (!iter)
                        iter = loop_start_iter
                                   ? loop_start_iter
                                   : liststore_sounds->children().begin();
                }
                button_sound_play.set_sensitive(true);
                button_sound_delete.set_sensitive(true);
                button_sound_loop.set_sensitive(true);
                button_sound_new.set_sensitive(true);
                button_sound_pause.set_sensitive(true);
                button_sound_up.set_sensitive(true);
                button_sound_down.set_sensitive(true);
            }).detach();
        }
        void pause_sound() {
            sound_playing.store(false);
            system("pkill -f aplay >/dev/null 2>&1 &");
        }

        void setup_gresources() {
            pixbuf_save_enabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/save-enabled.png");
            pixbuf_save_disabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/save-disabled.png");
            pixbuf_new_group =
                Gdk::Pixbuf::create_from_resource("/org/icons/group-add.png");
            pixbuf_group =
                Gdk::Pixbuf::create_from_resource("/org/icons/group.png");
            pixbuf_group_inherited = Gdk::Pixbuf::create_from_resource(
                "/org/icons/group-inherited.png");
            pixbuf_delete =
                Gdk::Pixbuf::create_from_resource("/org/icons/delete.png");
            pixbuf_help = Gdk::Pixbuf::create_from_resource(
                "/org/icons/question-mark.png");
            pixbuf_station =
                Gdk::Pixbuf::create_from_resource("/org/icons/server.png");
            pixbuf_address_book = Gdk::Pixbuf::create_from_resource(
                "/org/icons/address-book.png");
            pixbuf_run =
                Gdk::Pixbuf::create_from_resource("/org/icons/run.png");
            pixbuf_stop =
                Gdk::Pixbuf::create_from_resource("/org/icons/stop.png");
            pixbuf_add =
                Gdk::Pixbuf::create_from_resource("/org/icons/add.png");
            pixbuf_arrow_up_enabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/arrow-up-enabled.png");
            pixbuf_arrow_up_disabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/arrow-up-disabled.png");
            pixbuf_arrow_down_enabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/arrow-down-enabled.png");
            pixbuf_arrow_down_disabled = Gdk::Pixbuf::create_from_resource(
                "/org/icons/arrow-down-disabled.png");
            pixbuf_klogic =
                Gdk::Pixbuf::create_from_resource("/org/icons/klogic.png");
            pixbuf_task =
                Gdk::Pixbuf::create_from_resource("/org/icons/task.png");
            pixbuf_fb = Gdk::Pixbuf::create_from_resource("/org/icons/fb.png");
            pixbuf_service_params =
                Gdk::Pixbuf::create_from_resource("/org/icons/gear.png");
            pixbuf_discrete =
                Gdk::Pixbuf::create_from_resource("/org/icons/discrete.png");
            pixbuf_analog =
                Gdk::Pixbuf::create_from_resource("/org/icons/analog.png");
            pixbuf_deprecated =
                Gdk::Pixbuf::create_from_resource("/org/icons/deprecated.png");
            pixbuf_address_book = Gdk::Pixbuf::create_from_resource(
                "/org/icons/address-book.png");
            pixbuf_alarm =
                Gdk::Pixbuf::create_from_resource("/org/icons/alarm.png");

            pixbufs_params = {
                Gdk::Pixbuf::create_from_resource("/org/icons/float-in.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/float-out.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/bool-in.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/bool-out.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/int-in.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/int-out.png"),
            };

            pixbuf_pause =
                Gdk::Pixbuf::create_from_resource("/org/icons/pause.png");
            pixbuf_play =
                Gdk::Pixbuf::create_from_resource("/org/icons/play.png");
            pixbuf_stop =
                Gdk::Pixbuf::create_from_resource("/org/icons/stop.png");
            pixbuf_loop =
                Gdk::Pixbuf::create_from_resource("/org/icons/loop.png");
            pixbufs_loop = {
                Gdk::Pixbuf::create_from_resource("/org/icons/loop-empty.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/loop-single.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/loop-start.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/loop-mid.png"),
                Gdk::Pixbuf::create_from_resource("/org/icons/loop-end.png")};

            pixbuf_contact_name = Gdk::Pixbuf::create_from_resource(
                "/org/icons/contact-name.png");
            pixbuf_description =
                Gdk::Pixbuf::create_from_resource("/org/icons/description.png");
            pixbuf_email =
                Gdk::Pixbuf::create_from_resource("/org/icons/email.png");
            pixbuf_phone =
                Gdk::Pixbuf::create_from_resource("/org/icons/phone.png");
            pixbuf_contact =
                Gdk::Pixbuf::create_from_resource("/org/icons/contact.png");
            pixbuf_contact_group = Gdk::Pixbuf::create_from_resource(
                "/org/icons/contact-group.png");
            pixbuf_contact_add =
                Gdk::Pixbuf::create_from_resource("/org/icons/contact-add.png");
            pixbuf_contact_group_add = Gdk::Pixbuf::create_from_resource(
                "/org/icons/contact-group-add.png");
            pixbuf_clear =
                Gdk::Pixbuf::create_from_resource("/org/icons/clear.png");
            pixbuf_write =
                Gdk::Pixbuf::create_from_resource("/org/icons/write.png");
        }

        void setup_ui() {
            set_title("Настройка алармов");
            auto css = Gtk::CssProvider::create();
            css->load_from_data(R"(
                * {
                    font-family: Sans;
                    font-size: 14px;
                }
                #read-only {
                    opacity: 0.75;
                }
                #button-without-border {
                    border-style: none;
                    background-color: transparent;
                }
                #bold {
                    font-weight: bold;
                }
            )");
            auto screen = get_screen();
            Gtk::StyleContext::add_provider_for_screen(
                screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

            set_titlebar(headerbar);
            add(vbox_main);

            vbox_main.set_hexpand(true);
            vbox_main.set_vexpand(true);
            headerbar.set_show_close_button(true);
            headerbar.set_decoration_layout("menu:minimize,maximize,close");
            grid_main.set_column_homogeneous(false);
            grid_main.set_column_spacing(0);
            vbox_main.pack_start(grid_main, Gtk::PACK_EXPAND_WIDGET);
            grid_main.attach(scrolled_station, 0, 0, 1, 1);
            grid_main.attach(grip1, 1, 0, 1, 1);
            grid_main.attach(scrolled_form, 2, 0, 1, 1);
            grid_main.attach(grip2, 3, 0, 1, 1);
            grid_main.attach(vbox_zones, 4, 0, 1, 1);
            vbox_zones.pack_start(hbox_zones_buttons_bar, Gtk::PACK_SHRINK);
            hbox_zones_buttons_bar.pack_start(radiobutton_zones_name,
                                              Gtk::PACK_SHRINK);
            hbox_zones_buttons_bar.pack_start(radiobutton_zones_chipher,
                                              Gtk::PACK_SHRINK);
            vbox_zones.pack_start(scrolled_zones, Gtk::PACK_EXPAND_WIDGET);
            grip1.set_size_request(11, -1);
            grip1.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_MOTION_MASK |
                             Gdk::BUTTON_RELEASE_MASK |
                             Gdk::POINTER_MOTION_MASK | Gdk::ENTER_NOTIFY_MASK |
                             Gdk::LEAVE_NOTIFY_MASK);
            grip2.set_size_request(11, -1);
            grip2.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_MOTION_MASK |
                             Gdk::BUTTON_RELEASE_MASK |
                             Gdk::POINTER_MOTION_MASK | Gdk::ENTER_NOTIFY_MASK |
                             Gdk::LEAVE_NOTIFY_MASK);
            scrolled_station.set_hexpand(true);
            scrolled_station.set_size_request(GROUP_TREE_WIDTH, -1);
            scrolled_form.set_hexpand(true);
            scrolled_form.set_size_request(FORM_WIDTH, -1);
            scrolled_zones.set_hexpand(true);
            scrolled_zones.set_size_request(ZONES_WIDTH, -1);
            col1_size = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
            col2_size = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
            col3_size = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
            col1_size->add_widget(scrolled_station);
            col2_size->add_widget(scrolled_form);
            col3_size->add_widget(scrolled_zones);
            scrolled_station.add(frame_station);
            frame_station.add(treeview_station);
            theme_bg_color =
                treeview_station.get_style_context()->get_background_color(
                    Gtk::STATE_FLAG_NORMAL);
            frame_station.override_background_color(theme_bg_color,
                                                    Gtk::STATE_FLAG_NORMAL);
            scrolled_form.add(vbox_form);
            vbox_form.pack_start(hbox_form_buttons_bar, Gtk::PACK_SHRINK);
            vbox_form.pack_start(frame_stack_form_menu,
                                 Gtk::PACK_EXPAND_WIDGET);
            frame_stack_form_menu.add(stack_form_menu);
            frame_stack_form_menu.override_background_color(
                theme_bg_color, Gtk::STATE_FLAG_NORMAL);
            scrolled_zones.add(frame_zones);
            frame_zones.add(treeview_zones);
            frame_zones.override_background_color(theme_bg_color,
                                                  Gtk::STATE_FLAG_NORMAL);

            scrolled_station.set_margin_top(10);
            scrolled_station.set_margin_right(5);
            scrolled_station.set_margin_bottom(10);
            scrolled_station.set_margin_left(10);
            set_margin(vbox_form, 5, 10);
            scrolled_zones.set_margin_top(10);
            scrolled_zones.set_margin_right(10);
            scrolled_zones.set_margin_bottom(10);
            scrolled_zones.set_margin_left(5);
            set_margin(treeview_station, 10, 10);
            set_margin(treeview_zones, 10, 10);
        }

        void setup_menubuttons() {
            menubutton_file.set_label("Файл");
            menubutton_file.set_name("button-without-border");
            menubutton_file.set_relief(Gtk::RELIEF_NONE);
            headerbar.pack_start(menubutton_file);
            menu_file.append(menuitem_save);
            menuitem_save.add(menuitem_save_box);
            menuitem_save_box.pack_start(menuitem_save_icon, Gtk::PACK_SHRINK);
            menuitem_save_box.pack_start(menuitem_save_label, Gtk::PACK_SHRINK);
            menuitem_save.signal_activate().connect(
                [this]() { on_save_clicked(); });
            menu_file.append(menuitem_exit);
            menuitem_exit.add(menuitem_exit_box);
            menuitem_exit_box.pack_start(menuitem_exit_label, Gtk::PACK_SHRINK);
            menuitem_exit.signal_activate().connect(
                [this]() { on_exit_clicked(); });
            menubutton_file.set_popup(menu_file);
            menu_file.show_all();

            menuitem_edit_icon = Gtk::Image(pixbuf_edit);
            menubutton_edit.set_label("Правка");
            menubutton_edit.set_name("button-without-border");
            menubutton_edit.set_relief(Gtk::RELIEF_NONE);
            headerbar.pack_start(menubutton_edit);
            menu_edit.append(menuitem_delete);
            menuitem_delete_icon = Gtk::Image(pixbuf_delete);
            menuitem_delete.add(menuitem_delete_box);
            menuitem_delete_box.pack_start(menuitem_delete_icon,
                                           Gtk::PACK_SHRINK);
            menuitem_delete_box.pack_start(menuitem_delete_label,
                                           Gtk::PACK_SHRINK);
            menuitem_delete.signal_activate().connect(
                [this]() { on_delete_clicked(); });
            menu_edit.append(menuitem_new_group);
            menuitem_new_group_icon = Gtk::Image(pixbuf_new_group);
            menuitem_new_group.add(menuitem_new_group_box);
            menuitem_new_group_box.pack_start(menuitem_new_group_icon,
                                              Gtk::PACK_SHRINK);
            menuitem_new_group_box.pack_start(menuitem_new_group_label,
                                              Gtk::PACK_SHRINK);
            menuitem_new_group.signal_activate().connect(
                [this]() { on_new_group_clicked(); });
            menu_edit.append(menuitem_rename_group);
            menuitem_rename_group.add(menuitem_rename_group_label);
            menuitem_rename_group.signal_activate().connect([this]() {
                Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                if (!curr_group_row)
                    return;
                renderer_name.property_editable() = true;
                treeview_station.set_cursor(
                    treestore_groups->get_path(*curr_group_row),
                    treecolumn_name, true);
            });
            renderer_name.signal_edited().connect(
                [this](const Glib::ustring &path,
                       const Glib::ustring &new_text) {
                    Gtk::TreeModel::Path model_path(path);
                    Gtk::TreeModel::iterator iter =
                        treestore_groups->get_iter(model_path);
                    if (iter) {
                        (*iter)[group_cols.name] = new_text;
                        renderer_name.property_editable() = false;
                    }
                    button_save_enable();
                });
            menubutton_edit.set_popup(menu_edit);
            menu_edit.show_all();

            menubutton_service.set_label("Сервис");
            menubutton_service.set_name("button-without-border");
            menubutton_service.set_relief(Gtk::RELIEF_NONE);
            headerbar.pack_start(menubutton_service);
            menu_service.append(menuitem_general_settings);
            menuitem_general_settings.add(menuitem_general_settings_label);
            menuitem_general_settings_label.set_text("Основные настройки");
            menuitem_general_settings.signal_activate().connect([this]() {
                Gtk::MessageDialog dialog(*this, "Настройка Модуля АПС", false,
                                          Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_NONE, false);
                dialog.add_button("OK", Gtk::RESPONSE_OK);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                Gtk::Box *vbox = dialog.get_content_area();
                Gtk::Grid *grid_general = Gtk::make_managed<Gtk::Grid>();
                vbox->pack_start(*grid_general, Gtk::PACK_SHRINK);
                grid_general->set_column_spacing(15);
                grid_general->set_row_spacing(5);
                Gtk::Label *label_poll_period =
                    Gtk::make_managed<Gtk::Label>("Период опроса");
                label_poll_period->set_halign(Gtk::ALIGN_START);
                grid_general->attach(*label_poll_period, 0, 0, 1, 1);
                Gtk::Box *hbox_poll_period =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                grid_general->attach(*hbox_poll_period, 1, 0, 1, 1);
                Gtk::SpinButton *spin_poll_period =
                    Gtk::make_managed<Gtk::SpinButton>();
                spin_poll_period->set_adjustment(Gtk::Adjustment::create(
                    1000, 0, std::numeric_limits<unsigned int>::max()));
                hbox_poll_period->pack_start(*spin_poll_period,
                                             Gtk::PACK_SHRINK);
                Gtk::ComboBoxText *combobox_poll_period_units =
                    Gtk::make_managed<Gtk::ComboBoxText>();
                combobox_poll_period_units->append("мсек");
                combobox_poll_period_units->append("сек");
                hbox_poll_period->pack_start(*combobox_poll_period_units,
                                             Gtk::PACK_SHRINK);
                Gtk::Label *label_tcp_buffer_size =
                    Gtk::make_managed<Gtk::Label>("Размер буфера TCP");
                label_tcp_buffer_size->set_halign(Gtk::ALIGN_START);
                grid_general->attach(*label_tcp_buffer_size, 0, 1, 1, 1);
                Gtk::Box *hbox_tcp_buffer_size =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                hbox_tcp_buffer_size->set_halign(Gtk::ALIGN_START);
                grid_general->attach(*hbox_tcp_buffer_size, 1, 1, 1, 1);
                Gtk::ComboBoxText *comboboxtext_tcp_max_buffer_size_bytes =
                    Gtk::make_managed<Gtk::ComboBoxText>();
                comboboxtext_tcp_max_buffer_size_bytes->append("128");
                comboboxtext_tcp_max_buffer_size_bytes->append("256");
                comboboxtext_tcp_max_buffer_size_bytes->append("512");
                comboboxtext_tcp_max_buffer_size_bytes->append("1024");
                comboboxtext_tcp_max_buffer_size_bytes->append("2048");
                comboboxtext_tcp_max_buffer_size_bytes->append("4096");
                comboboxtext_tcp_max_buffer_size_bytes->set_halign(
                    Gtk::ALIGN_END);
                hbox_tcp_buffer_size->pack_start(
                    *comboboxtext_tcp_max_buffer_size_bytes, Gtk::PACK_SHRINK);
                hbox_tcp_buffer_size->pack_start(
                    *Gtk::make_managed<Gtk::Label>("байт"), Gtk::PACK_SHRINK);
                set_margin(*grid_general, 10, 2);

                Gtk::Frame *frame_tcp_ports =
                    Gtk::make_managed<Gtk::Frame>("порты TCP");
                vbox->pack_start(*frame_tcp_ports, Gtk::PACK_SHRINK);
                Gtk::Grid *grid_tcp_ports = Gtk::make_managed<Gtk::Grid>();
                frame_tcp_ports->add(*grid_tcp_ports);
                grid_tcp_ports->set_column_spacing(5);
                grid_tcp_ports->set_row_spacing(5);
                Gtk::Label *label_connection_port =
                    Gtk::make_managed<Gtk::Label>(
                        "- для подключения к серверам АПС:");
                label_connection_port->set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_receive_port = Gtk::make_managed<Gtk::Label>(
                    "- для получения данных с АПС:");
                label_receive_port->set_halign(Gtk::ALIGN_START);
                Gtk::Label *label_max_opened_ports_amount =
                    Gtk::make_managed<Gtk::Label>(
                        "Макс. количество открытых портов:");
                label_max_opened_ports_amount->set_halign(Gtk::ALIGN_START);
                Gtk::SpinButton *spin_connection_port =
                    Gtk::make_managed<Gtk::SpinButton>();
                spin_connection_port->set_adjustment(
                    Gtk::Adjustment::create(22346, 0, 65535));
                Gtk::SpinButton *spin_receive_port =
                    Gtk::make_managed<Gtk::SpinButton>();
                spin_receive_port->set_adjustment(
                    Gtk::Adjustment::create(22348, 0, 65535));
                Gtk::SpinButton *spin_max_opened_ports_amount =
                    Gtk::make_managed<Gtk::SpinButton>();
                grid_tcp_ports->attach(*label_connection_port, 0, 0, 1, 1);
                grid_tcp_ports->attach(*spin_connection_port, 1, 0, 1, 1);
                grid_tcp_ports->attach(*label_receive_port, 0, 1, 1, 1);
                grid_tcp_ports->attach(*spin_receive_port, 1, 1, 1, 1);
                grid_tcp_ports->attach(*label_max_opened_ports_amount, 0, 2, 1,
                                       1);
                grid_tcp_ports->attach(*spin_max_opened_ports_amount, 1, 2, 1,
                                       1);
                set_margin(*grid_tcp_ports, 10, 2);
                set_margin(*frame_tcp_ports, 10, 2);

                Gtk::Frame *frame_processing =
                    Gtk::make_managed<Gtk::Frame>("Обрботка");
                vbox->pack_start(*frame_processing, Gtk::PACK_SHRINK);
                Gtk::Grid *grid_processing = Gtk::make_managed<Gtk::Grid>();
                frame_processing->add(*grid_processing);
                grid_processing->set_column_spacing(5);
                grid_processing->set_row_spacing(5);
                Gtk::Box *vbox_processing_radiobuttons =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::RadioButton::Group radiogroup_processing;
                Gtk::RadioButton *radiobutton_fix =
                    Gtk::make_managed<Gtk::RadioButton>(radiogroup_processing,
                                                        "Снятие защелки");
                radiobutton_fix->set_halign(Gtk::ALIGN_START);
                vbox_processing_radiobuttons->pack_start(*radiobutton_fix,
                                                         Gtk::PACK_SHRINK);
                Gtk::RadioButton *radiobutton_auto_checkback =
                    Gtk::make_managed<Gtk::RadioButton>(
                        radiogroup_processing, "Автоматическое квитирование");
                radiobutton_auto_checkback->set_halign(Gtk::ALIGN_START);
                vbox_processing_radiobuttons->pack_start(
                    *radiobutton_auto_checkback, Gtk::PACK_SHRINK);
                grid_processing->attach(*vbox_processing_radiobuttons, 0, 0, 1,
                                        1);
                Gtk::Label *label_processing_after =
                    Gtk::make_managed<Gtk::Label>("через");
                label_processing_after->set_valign(Gtk::ALIGN_CENTER);
                grid_processing->attach(*label_processing_after, 1, 0, 1, 1);
                Gtk::SpinButton *spin_fix_timeout_sec =
                    Gtk::make_managed<Gtk::SpinButton>();
                spin_fix_timeout_sec->set_adjustment(Gtk::Adjustment::create(
                    0, 0, std::numeric_limits<short unsigned int>::max()));
                spin_fix_timeout_sec->set_valign(Gtk::ALIGN_CENTER);
                Gtk::Label *label_fit_timeout_sec =
                    Gtk::make_managed<Gtk::Label>("сек");
                label_fit_timeout_sec->set_valign(Gtk::ALIGN_CENTER);
                grid_processing->attach(*label_fit_timeout_sec, 2, 0, 1, 1);
                grid_processing->attach(*spin_fix_timeout_sec, 2, 0, 1, 1);
                Gtk::Label *label_commissioning_after_sec =
                    Gtk::make_managed<Gtk::Label>("Ввод в работу через");
                label_commissioning_after_sec->set_halign(Gtk::ALIGN_START);
                grid_processing->attach(*label_commissioning_after_sec, 0, 1, 1,
                                        1);
                Gtk::SpinButton *spin_deblock_on_err_timeout_sec =
                    Gtk::make_managed<Gtk::SpinButton>();
                spin_deblock_on_err_timeout_sec->set_adjustment(
                    Gtk::Adjustment::create(
                        0, 0, std::numeric_limits<short unsigned int>::max()));
                spin_deblock_on_err_timeout_sec->set_valign(Gtk::ALIGN_CENTER);
                grid_processing->attach(*spin_deblock_on_err_timeout_sec, 2, 1,
                                        1, 1);
                Gtk::Label *label_deblock_on_err_timeout_sec =
                    Gtk::make_managed<Gtk::Label>("сек");
                label_deblock_on_err_timeout_sec->set_valign(Gtk::ALIGN_CENTER);
                grid_processing->attach(*label_deblock_on_err_timeout_sec, 2, 1,
                                        1, 1);
                set_margin(*grid_processing, 10, 2);
                set_margin(*frame_processing, 10, 2);

                Gtk::Frame *frame_receiving_err =
                    Gtk::make_managed<Gtk::Frame>("При недостоверности или "
                                                  "отстутствии данных от "
                                                  "сервера");
                vbox->pack_start(*frame_receiving_err, Gtk::PACK_SHRINK);
                Gtk::Box *vbox_receiving_err =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                frame_receiving_err->add(*vbox_receiving_err);
                Gtk::RadioButton::Group radiogroup_receiving_err;
                Gtk::RadioButton *radiobutton_alarms_sound_menu_default =
                    Gtk::make_managed<Gtk::RadioButton>(
                        radiogroup_receiving_err, "по умолчанию");
                vbox_receiving_err->pack_start(
                    *radiobutton_alarms_sound_menu_default, Gtk::PACK_SHRINK);
                Gtk::Box *hbox_sound_from_file =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                Gtk::RadioButton *radiobutton_alarms_sound_menu_from_file =
                    Gtk::make_managed<Gtk::RadioButton>(
                        radiogroup_receiving_err, "из файла");
                hbox_sound_from_file->pack_start(
                    *radiobutton_alarms_sound_menu_from_file, Gtk::PACK_SHRINK);
                Gtk::Box *hbox_sound_file_path =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
                hbox_sound_from_file->pack_start(*hbox_sound_file_path,
                                                 Gtk::PACK_SHRINK);
                hbox_sound_file_path->set_halign(Gtk::ALIGN_START);
                hbox_sound_file_path->set_hexpand(true);
                Gtk::Entry *entry_sound_from_file =
                    Gtk::make_managed<Gtk::Entry>();
                entry_sound_from_file->set_hexpand(true);
                hbox_sound_file_path->pack_start(*entry_sound_from_file,
                                                 Gtk::PACK_EXPAND_WIDGET);
                Gtk::Button *button_select_sound_from_file =
                    Gtk::make_managed<Gtk::Button>("...");
                hbox_sound_file_path->pack_start(*button_select_sound_from_file,
                                                 Gtk::PACK_SHRINK);
                button_select_sound_from_file->signal_clicked().connect(
                    [this, &button_select_sound_from_file,
                     entry_sound_from_file]() {
                        Gtk::FileChooserDialog dialog(
                            "Выберите звуковой файл",
                            Gtk::FILE_CHOOSER_ACTION_OPEN);
                        dialog.set_transient_for(*this);
                        dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                        dialog.add_button("ОК", Gtk::RESPONSE_OK);
                        if (dialog.run() == Gtk::RESPONSE_OK) {
                            std::string filename = dialog.get_filename();
                            entry_sound_from_file->set_text(filename);
                        }
                        dialog.hide();
                    });
                vbox_receiving_err->pack_start(*hbox_sound_from_file,
                                               Gtk::PACK_SHRINK);
                radiobutton_alarms_sound_menu_default->signal_toggled().connect(
                    [this, &radiobutton_alarms_sound_menu_default,
                     &entry_sound_from_file, &button_select_sound_from_file]() {
                        if (radiobutton_alarms_sound_menu_default
                                ->get_active()) {
                            entry_sound_from_file->set_sensitive(false);
                            button_select_sound_from_file->set_sensitive(false);
                        }
                    });
                radiobutton_alarms_sound_menu_from_file->signal_toggled()
                    .connect([this, &radiobutton_alarms_sound_menu_from_file,
                              &entry_sound_from_file,
                              &button_select_sound_from_file]() {
                        if (radiobutton_alarms_sound_menu_from_file
                                ->get_active()) {
                            entry_sound_from_file->set_sensitive(true);
                            button_select_sound_from_file->set_sensitive(true);
                        }
                    });
                set_margin(*vbox_receiving_err, 10, 2);
                set_margin(*frame_receiving_err, 10, 2);

                Gtk::Frame *frame_interface =
                    Gtk::make_managed<Gtk::Frame>("Интерфейс");
                vbox->pack_start(*frame_interface, Gtk::PACK_SHRINK);
                Gtk::Box *vbox_interface =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                frame_interface->add(*vbox_interface);
                Gtk::CheckButton *checkbutton_cfg_one_copy =
                    Gtk::make_managed<Gtk::CheckButton>(
                        "Запретить запуск второй копии приложения");
                vbox_interface->pack_start(*checkbutton_cfg_one_copy,
                                           Gtk::PACK_SHRINK);
                Gtk::CheckButton *checkbutton_confirm_alarm_drag =
                    Gtk::make_managed<Gtk::CheckButton>(
                        "Подтверждать перемещение алармов");
                vbox_interface->pack_start(*checkbutton_confirm_alarm_drag,
                                           Gtk::PACK_SHRINK);
                Gtk::CheckButton *checkbutton_save_alarms_xml =
                    Gtk::make_managed<Gtk::CheckButton>(
                        "Сохранять конфигурацию только в бинарном виде");
                vbox_interface->pack_start(*checkbutton_save_alarms_xml,
                                           Gtk::PACK_SHRINK);
                set_margin(*vbox_interface, 10, 2);
                set_margin(*frame_interface, 10, 2);

                // Чтение
                APSModuleSettings buffer_aps_settings =
                    main_settings.stations_aps_settings[curr_station_id];
                unsigned int polling_period =
                    buffer_aps_settings.polling_period_msec;
                bool higher_than_10000 = polling_period > 10000;
                if (higher_than_10000)
                    polling_period /= 1000;
                combobox_poll_period_units->set_active(higher_than_10000 ? 1
                                                                         : 0);
                spin_poll_period->set_value(polling_period);
                unsigned char tcp_buffer_size_idx = 4;
                switch (buffer_aps_settings.tcp_max_buffer_size_bytes) {
                case 128:
                    tcp_buffer_size_idx = 0;
                    break;
                case 256:
                    tcp_buffer_size_idx = 1;
                    break;
                case 512:
                    tcp_buffer_size_idx = 2;
                    break;
                case 1024:
                    tcp_buffer_size_idx = 3;
                    break;
                case 2048:
                    tcp_buffer_size_idx = 4;
                    break;
                case 4096:
                    tcp_buffer_size_idx = 5;
                    break;
                }
                comboboxtext_tcp_max_buffer_size_bytes->set_active(
                    tcp_buffer_size_idx);
                spin_connection_port->set_value(
                    buffer_aps_settings.connect_port);
                spin_receive_port->set_value(buffer_aps_settings.receive_port);
                spin_max_opened_ports_amount->set_value(
                    buffer_aps_settings.receive_port_max);
                if (buffer_aps_settings.fix_or_auto_checkback) {
                    radiobutton_fix->set_active(true);
                } else {
                    radiobutton_auto_checkback->set_active(true);
                }
                spin_fix_timeout_sec->set_value(
                    buffer_aps_settings.fix_timeout_sec);
                spin_deblock_on_err_timeout_sec->set_value(
                    buffer_aps_settings.deblock_on_err_timeout_sec);
                if (buffer_aps_settings.warn_by_default_sound) {
                    radiobutton_alarms_sound_menu_default->set_active(true);
                    entry_sound_from_file->set_sensitive(false);
                    button_select_sound_from_file->set_sensitive(false);
                } else {
                    radiobutton_alarms_sound_menu_from_file->set_active(true);
                    entry_sound_from_file->set_text(
                        buffer_aps_settings.warn_sound_file_path);
                }
                checkbutton_cfg_one_copy->set_active(
                    buffer_aps_settings.cfg_one_copy);
                checkbutton_confirm_alarm_drag->set_active(
                    buffer_aps_settings.cfg_confirm_alarms_drag);
                checkbutton_save_alarms_xml->set_active(
                    buffer_aps_settings.cfg_save_alarms_xml);
                dialog.show_all();

                int res = dialog.run();
                if (res != Gtk::RESPONSE_OK)
                    return;
                // Запись
                buffer_aps_settings.polling_period_msec =
                    combobox_poll_period_units->get_active_row_number() == 0
                        ? spin_poll_period->get_value()
                        : spin_poll_period->get_value() * 1000;
                buffer_aps_settings.tcp_max_buffer_size_bytes =
                    comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 0
                        ? 128
                    : comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 1
                        ? 256
                    : comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 2
                        ? 512
                    : comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 3
                        ? 1024
                    : comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 4
                        ? 2048
                    : comboboxtext_tcp_max_buffer_size_bytes
                                ->get_active_row_number() == 5
                        ? 4096
                        : 0;
                buffer_aps_settings.connect_port =
                    spin_connection_port->get_value();
                buffer_aps_settings.receive_port =
                    spin_receive_port->get_value();
                buffer_aps_settings.receive_port_max =
                    spin_max_opened_ports_amount->get_value();
                buffer_aps_settings.fix_or_auto_checkback =
                    radiobutton_fix->get_active();
                buffer_aps_settings.fix_timeout_sec =
                    spin_fix_timeout_sec->get_value();
                buffer_aps_settings.deblock_on_err_timeout_sec =
                    spin_deblock_on_err_timeout_sec->get_value();
                if (radiobutton_alarms_sound_menu_default->get_active()) {
                    buffer_aps_settings.warn_by_default_sound = true;
                } else {
                    buffer_aps_settings.warn_by_default_sound = false;
                    buffer_aps_settings.warn_sound_file_path =
                        entry_sound_from_file->get_text();
                }
                buffer_aps_settings.cfg_one_copy =
                    checkbutton_cfg_one_copy->get_active();
                buffer_aps_settings.cfg_confirm_alarms_drag =
                    checkbutton_confirm_alarm_drag->get_active();
                buffer_aps_settings.cfg_save_alarms_xml =
                    checkbutton_save_alarms_xml->get_active();
                main_settings.stations_aps_settings[curr_station_id] =
                    buffer_aps_settings;

                Gtk::TreeModel::Row curr_station_row;
                for (const Gtk::TreeModel::Row &station_row :
                     treestore_groups->children()) {
                    if (station_row[group_cols.station_id] == curr_station_id)
                        curr_station_row = station_row;
                }
                if (!curr_station_row)
                    return;
                std::string errors;
                (void)write_station_aps_settings(
                    alarms_dir_path, buffer_aps_settings, curr_station_row,
                    group_cols, errors);
                if (!errors.empty()) {
                    Gtk::MessageDialog dialog(
                        "Не удалось сохранить настройки АПС\n\n" + errors,
                        false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                }
            });
            menubutton_service.set_popup(menu_service);
            menu_service.show_all();

            menuitem_help_icon = Gtk::Image(pixbuf_help);
            menubutton_help.set_label("Помощь");
            menubutton_help.set_name("button-without-border");
            menubutton_help.set_relief(Gtk::RELIEF_NONE);
            headerbar.pack_start(menubutton_help);
            menu_help.append(menuitem_help);
            menuitem_help.add(menuitem_help_box);
            menuitem_help_box.pack_start(menuitem_help_icon, Gtk::PACK_SHRINK);
            menuitem_help_box.pack_start(menuitem_help_label, Gtk::PACK_SHRINK);
            menubutton_help.set_popup(menu_help);
            menu_help.show_all();
        }

        void setup_topbar() {
            vbox_main.pack_start(topbar, Gtk::PACK_SHRINK);

            button_save.set_tooltip_text("Сохранить (Ctrl+S)");
            topbar.pack_start(button_save, Gtk::PACK_SHRINK);
            button_save.set_image(button_save_icon);
            button_save.set_always_show_image(true);
            button_save_disable();
            topbar.pack_start(topbar_separator1, Gtk::PACK_SHRINK);

            button_new_group_icon = Gtk::Image(pixbuf_new_group);
            topbar.pack_start(button_new_group, Gtk::PACK_SHRINK);
            button_new_group.set_image(button_new_group_icon);
            button_new_group.set_always_show_image(true);

            button_delete_icon = Gtk::Image(pixbuf_delete);
            topbar.pack_start(button_delete, Gtk::PACK_SHRINK);
            button_delete.set_image(button_delete_icon);
            button_delete.set_always_show_image(true);
            topbar.pack_start(topbar_separator2, Gtk::PACK_SHRINK);

            button_address_book_icon = Gtk::Image(pixbuf_address_book);
            topbar.pack_start(button_address_book, Gtk::PACK_SHRINK);
            button_address_book.set_image(button_address_book_icon);
            button_address_book.set_always_show_image(true);
            topbar.pack_start(topbar_separator3, Gtk::PACK_SHRINK);

            topbar.pack_start(combobox_station, Gtk::PACK_SHRINK);

            set_margin(topbar, 0, 5);
            button_save.set_margin_left(5);
        }

        void setup_menus() {
            radiobutton_settings_menu =
                Gtk::RadioButton(group_radiobuttons, "Настройки");
            hbox_form_buttons_bar.pack_start(radiobutton_settings_menu,
                                             Gtk::PACK_SHRINK);
            radiobutton_settings_menu.set_mode(false);
            radiobutton_passports_menu =
                Gtk::RadioButton(group_radiobuttons, "Паспорта");
            radiobutton_passports_menu.set_mode(false);
            hbox_form_buttons_bar.pack_start(radiobutton_passports_menu,
                                             Gtk::PACK_SHRINK);

            // Меню настроек
            vbox_settings_menu.pack_start(hbox_group_name, Gtk::PACK_SHRINK);
            hbox_group_name.pack_start(label_group_settings, Gtk::PACK_SHRINK);
            hbox_group_name.pack_start(label_group_name, Gtk::PACK_SHRINK);
            label_group_name.set_name("bold");
            vbox_settings_menu.pack_start(checkbutton_use_own_settings,
                                          Gtk::PACK_SHRINK);
            checkbutton_use_own_settings.set_name("bold");
            vbox_settings_menu.pack_start(hbox_settings_buttons_bar,
                                          Gtk::PACK_SHRINK);
            radiobutton_general_settings.set_mode(false);
            hbox_settings_buttons_bar.pack_start(radiobutton_general_settings,
                                                 Gtk::PACK_SHRINK);
            radiobutton_alarms.set_mode(false);
            hbox_settings_buttons_bar.pack_start(radiobutton_alarms,
                                                 Gtk::PACK_SHRINK);
            // Подменю общих настроек
            vbox_settings_menu.pack_start(frame_stack_general_settings_alarms,
                                          Gtk::PACK_SHRINK);
            frame_stack_general_settings_alarms.add(
                stack_general_settings_alarms);
            frame_stack_general_settings_alarms.override_background_color(
                theme_bg_color, Gtk::STATE_FLAG_NORMAL);
            stack_general_settings_alarms.add(vbox_general_settings);
            vbox_general_settings.pack_start(
                hbox_general_settings_checkbuttons_row, Gtk::PACK_SHRINK);
            hbox_general_settings_checkbuttons_row.pack_start(
                checkbutton_disable_alarm, Gtk::PACK_SHRINK);
            checkbutton_disable_alarm.set_name("bold");
            hbox_general_settings_checkbuttons_row.pack_start(
                checkbutton_checkback_on_startup, Gtk::PACK_SHRINK);
            vbox_general_settings.pack_start(checkbutton_use_own_sound_in_group,
                                             Gtk::PACK_SHRINK);
            vbox_general_settings.pack_start(grid_equipment_description_mask,
                                             Gtk::PACK_SHRINK);
            grid_equipment_description_mask.set_column_spacing(5);
            grid_equipment_description_mask.attach(
                label_equipment_description_mask, 0, 0, 1, 1);
            label_equipment_description_mask_view.set_halign(Gtk::ALIGN_START);
            grid_equipment_description_mask.attach(
                entry_equipment_description_mask, 1, 0, 1, 1);
            entry_equipment_description_mask.signal_changed().connect([this]() {
                std::string decription =
                    entry_equipment_description_mask.get_text();
                render_masks(decription);
                label_equipment_description_mask_view.set_text(decription);
            });
            grid_equipment_description_mask.attach(
                label_equipment_description_mask_view, 1, 1, 1, 1);
            vbox_general_settings.pack_start(hbox_prioritysetpoints,
                                             Gtk::PACK_SHRINK);
            hbox_prioritysetpoints.pack_start(frame_priority, Gtk::PACK_SHRINK);
            frame_priority.set_label_widget(label_frame_priority);
            frame_priority.add(vbox_priority);
            vbox_priority.pack_start(radiobutton_priority_max,
                                     Gtk::PACK_SHRINK);
            vbox_priority.pack_start(radiobutton_priority_high,
                                     Gtk::PACK_SHRINK);
            vbox_priority.pack_start(radiobutton_priority_medium,
                                     Gtk::PACK_SHRINK);
            vbox_priority.pack_start(radiobutton_priority_low,
                                     Gtk::PACK_SHRINK);
            vbox_priority.pack_start(radiobutton_priority_minimal,
                                     Gtk::PACK_SHRINK);
            hbox_prioritysetpoints.pack_start(frame_setpoints,
                                              Gtk::PACK_SHRINK);
            frame_setpoints.set_label_widget(label_frame_setpoints);
            frame_setpoints.add(vbox_setpoints);
            vbox_setpoints.pack_start(checkbutton_borders_from_passport,
                                      Gtk::PACK_SHRINK);
            vbox_setpoints.pack_start(hbox_crashprecrash_bounds,
                                      Gtk::PACK_SHRINK);
            hbox_crashprecrash_bounds.pack_start(frame_crash_bounds,
                                                 Gtk::PACK_SHRINK);
            frame_crash_bounds.set_label_widget(label_frame_crash_bounds);
            frame_crash_bounds.add(grid_crash_bounds_setpoints);
            grid_crash_bounds_setpoints.set_column_spacing(5);
            grid_crash_bounds_setpoints.set_row_spacing(10);
            grid_crash_bounds_setpoints.attach(label_higher_crash_setpoint, 0,
                                               0, 1, 1);
            grid_crash_bounds_setpoints.attach(entry_higher_crash_setpoint, 1,
                                               0, 1, 1);
            grid_crash_bounds_setpoints.attach(button_higher_crash_setpoint, 2,
                                               0, 1, 1);
            grid_crash_bounds_setpoints.attach(label_lower_crash_setpoint, 0, 1,
                                               1, 1);
            grid_crash_bounds_setpoints.attach(entry_lower_crash_setpoint, 1, 1,
                                               1, 1);
            grid_crash_bounds_setpoints.attach(button_lower_crash_setpoint, 2,
                                               1, 1, 1);
            button_higher_crash_setpoint.signal_clicked().connect(
                [this]() { on_setpoint_choice("ВАУ", 0); });
            button_lower_crash_setpoint.signal_clicked().connect(
                [this]() { on_setpoint_choice("НАУ", 1); });
            hbox_crashprecrash_bounds.pack_start(frame_precrash_bounds,
                                                 Gtk::PACK_SHRINK);
            frame_precrash_bounds.set_label_widget(label_frame_precrash_bounds);
            frame_precrash_bounds.add(grid_precrash_bounds_setpoints);
            grid_precrash_bounds_setpoints.set_column_spacing(5);
            grid_precrash_bounds_setpoints.set_row_spacing(10);
            grid_precrash_bounds_setpoints.attach(
                label_higher_precrash_setpoint, 0, 0, 1, 1);
            grid_precrash_bounds_setpoints.attach(
                entry_higher_precrash_setpoint, 1, 0, 1, 1);
            grid_precrash_bounds_setpoints.attach(
                button_higher_precrash_setpoint, 2, 0, 1, 1);
            grid_precrash_bounds_setpoints.attach(label_lower_precrash_setpoint,
                                                  0, 1, 1, 1);
            grid_precrash_bounds_setpoints.attach(entry_lower_precrash_setpoint,
                                                  1, 1, 1, 1);
            grid_precrash_bounds_setpoints.attach(
                button_lower_precrash_setpoint, 2, 1, 1, 1);
            button_higher_precrash_setpoint.signal_clicked().connect(
                [this]() { on_setpoint_choice("ВПУ", 2); });
            button_lower_precrash_setpoint.signal_clicked().connect(
                [this]() { on_setpoint_choice("НПУ", 3); });
            vbox_general_settings.pack_start(frame_aps_lowlevel_interaction,
                                             Gtk::PACK_SHRINK);
            frame_aps_lowlevel_interaction.set_label_widget(
                label_frame_aps_lowlevel_interaction);
            frame_aps_lowlevel_interaction.set_hexpand(false);
            frame_aps_lowlevel_interaction.add(hbox_aps_lowlevel_interaction);
            hbox_aps_lowlevel_interaction.pack_start(
                vbox_aps_lowlevel_interaction_left, Gtk::PACK_SHRINK);
            vbox_aps_lowlevel_interaction_left.pack_start(
                hbox_block_by_passport, Gtk::PACK_SHRINK);
            hbox_block_by_passport.pack_start(checkbutton_block_by_passport,
                                              Gtk::PACK_SHRINK);
            hbox_block_by_passport.pack_start(eventbox_block_by_passport,
                                              Gtk::PACK_SHRINK);
            eventbox_block_by_passport.add(
                *Gtk::make_managed<Gtk::Label>("Вывод из работы по паспорту"));
            eventbox_block_by_passport.add_events(Gdk::BUTTON_PRESS_MASK);
            eventbox_block_by_passport.signal_button_press_event().connect(
                [this](GdkEventButton *event) {
                    (void)event;
                    curr_aps_menu_idx = 0;
                    redraw_aps();
                    return false;
                });
            vbox_aps_lowlevel_interaction_left.pack_start(hbox_checkback,
                                                          Gtk::PACK_SHRINK);
            hbox_checkback.pack_start(checkbutton_checkback, Gtk::PACK_SHRINK);
            hbox_checkback.pack_start(eventbox_checkback, Gtk::PACK_SHRINK);
            eventbox_checkback.add(
                *Gtk::make_managed<Gtk::Label>("Квитирование паспортом"));
            eventbox_checkback.add_events(Gdk::BUTTON_PRESS_MASK);
            eventbox_checkback.signal_button_press_event().connect(
                [this](GdkEventButton *event) {
                    (void)event;
                    curr_aps_menu_idx = 1;
                    redraw_aps();
                    return false;
                });
            vbox_aps_lowlevel_interaction_left.pack_start(
                hbox_write_on_checkback, Gtk::PACK_SHRINK);
            hbox_write_on_checkback.pack_start(checkbutton_write_on_checkback,
                                               Gtk::PACK_SHRINK);
            hbox_write_on_checkback.pack_start(eventbox_write_on_checkback,
                                               Gtk::PACK_SHRINK);
            eventbox_write_on_checkback.add(
                *Gtk::make_managed<Gtk::Label>("Паспорт для записи состояния"));
            eventbox_write_on_checkback.add_events(Gdk::BUTTON_PRESS_MASK);
            eventbox_write_on_checkback.signal_button_press_event().connect(
                [this](GdkEventButton *event) {
                    (void)event;
                    curr_aps_menu_idx = 2;
                    redraw_aps();
                    return false;
                });
            vbox_aps_lowlevel_interaction_left.pack_start(
                stack_aps_lowlevel_interaction_left, Gtk::PACK_SHRINK);
            stack_aps_lowlevel_interaction_left.add(frame_value);
            frame_value.set_label_widget(label_frame_value);
            frame_value.set_hexpand(false);
            frame_value.add(hbox_value);
            hbox_value.pack_start(radiobutton0, Gtk::PACK_SHRINK);
            hbox_value.pack_start(radiobutton1, Gtk::PACK_SHRINK);
            stack_aps_lowlevel_interaction_left.add(frame_checkback);
            frame_checkback.set_label_widget(label_frame_checkback);
            frame_checkback.set_hexpand(false);
            frame_checkback.add(hbox_checkback_val);
            hbox_checkback_val.pack_start(radiobutton_checkback_edge,
                                          Gtk::PACK_SHRINK);
            hbox_checkback_val.pack_start(radiobutton_checkback_cutoff,
                                          Gtk::PACK_SHRINK);
            stack_aps_lowlevel_interaction_left.show_all();
            hbox_aps_lowlevel_interaction.pack_start(
                vbox_aps_lowlevel_interaction_right, Gtk::PACK_EXPAND_WIDGET);
            vbox_aps_lowlevel_interaction_right.pack_start(grid_aps);
            grid_aps.set_halign(Gtk::ALIGN_CENTER);
            grid_aps.set_column_spacing(10);
            grid_aps.set_row_spacing(5);
            grid_aps.attach(label_aps_station, 0, 0, 1, 1);
            label_aps_station.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_station_val, 1, 0, 1, 1);
            label_aps_station_val.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_type, 0, 1, 1, 1);
            label_aps_type.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_type_val, 1, 1, 1, 1);
            label_aps_type_val.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_id, 0, 3, 1, 1);
            label_aps_id.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_id_val, 1, 3, 1, 1);
            label_aps_id_val.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_chipher, 0, 4, 1, 1);
            label_aps_chipher.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_chipher_val, 1, 4, 1, 1);
            label_aps_chipher_val.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_name, 0, 5, 1, 1);
            label_aps_name.set_halign(Gtk::ALIGN_START);
            grid_aps.attach(label_aps_name_val, 1, 5, 1, 1);
            label_aps_name_val.set_halign(Gtk::ALIGN_START);
            vbox_aps_lowlevel_interaction_right.pack_start(hbox_aps_buttons_bar,
                                                           Gtk::PACK_SHRINK);
            hbox_aps_buttons_bar.set_halign(Gtk::ALIGN_CENTER);
            hbox_aps_buttons_bar.pack_start(button_aps_info, Gtk::PACK_SHRINK);
            button_aps_info.signal_clicked().connect([this]() {
                Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                if (!curr_group_row)
                    return;
                std::array<Passport, 7> passports =
                    curr_group_row[group_cols.passports];
                Passport passport = passports[4 + curr_aps_menu_idx];
                if (!passport.selected)
                    return;
                Gtk::MessageDialog dialog("Информация о параметре", false,
                                          Gtk::MESSAGE_INFO);
                Gtk::Box *vbox = dialog.get_content_area();
                Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
                vbox->pack_start(*grid, Gtk::PACK_EXPAND_WIDGET);
                grid->set_column_spacing(10);
                grid->set_row_spacing(10);
                grid->set_hexpand(true);
                grid->set_halign(Gtk::ALIGN_FILL);
                Gtk::Label *label_station =
                    Gtk::make_managed<Gtk::Label>("Станция");
                label_station->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_station, 0, 0, 1, 1);
                Gtk::Label *label_station_info =
                    Gtk::make_managed<Gtk::Label>();
                label_station_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_station_info, 1, 0, 1, 1);
                Gtk::Label *label_type =
                    Gtk::make_managed<Gtk::Label>("Тип параметра");
                label_type->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_type, 0, 1, 1, 1);
                Gtk::Box *hbox_type =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                Gtk::Image *image_type = Gtk::make_managed<Gtk::Image>();
                hbox_type->pack_start(*image_type, Gtk::PACK_SHRINK);
                Gtk::Label *label_type_info = Gtk::make_managed<Gtk::Label>();
                hbox_type->pack_start(*label_type_info, Gtk::PACK_SHRINK);
                label_type_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*hbox_type, 1, 1, 1, 1);
                Gtk::Label *label_group =
                    Gtk::make_managed<Gtk::Label>("Группа");
                label_group->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_group, 0, 2, 1, 1);
                Gtk::Box *hbox_group =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                Gtk::Image *image_group = Gtk::make_managed<Gtk::Image>();
                hbox_group->pack_start(*image_group, Gtk::PACK_SHRINK);
                Gtk::Label *label_group_info = Gtk::make_managed<Gtk::Label>();
                hbox_group->pack_start(*label_group_info, Gtk::PACK_SHRINK);
                label_group_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*hbox_group, 1, 2, 1, 1);
                Gtk::Label *label_id =
                    Gtk::make_managed<Gtk::Label>("Идентификатор");
                label_id->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_id, 0, 3, 1, 1);
                Gtk::Label *label_id_info = Gtk::make_managed<Gtk::Label>();
                label_id_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_id_info, 1, 3, 1, 1);
                Gtk::Label *label_chipher =
                    Gtk::make_managed<Gtk::Label>("Шифр");
                label_chipher->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_chipher, 0, 4, 1, 1);
                Gtk::Label *label_chipher_info =
                    Gtk::make_managed<Gtk::Label>();
                label_chipher_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_chipher_info, 1, 4, 1, 1);
                Gtk::Label *label_name =
                    Gtk::make_managed<Gtk::Label>("Наименование");
                label_name->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_name, 0, 5, 1, 1);
                Gtk::Label *label_name_info = Gtk::make_managed<Gtk::Label>();
                label_name_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_name_info, 1, 5, 1, 1);
                Gtk::Label *label_measure_unit =
                    Gtk::make_managed<Gtk::Label>("Единица измерения");
                label_measure_unit->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_measure_unit, 0, 6, 1, 1);
                Gtk::Label *label_measure_unit_info =
                    Gtk::make_managed<Gtk::Label>();
                label_measure_unit_info->set_halign(Gtk::ALIGN_START);
                grid->attach(*label_measure_unit_info, 1, 6, 1, 1);
                set_margin(*grid, 10, 10);

                label_station_info->set_text(passport.station_name);
                int idx = passport.type * 2 + passport.out;
                if (idx < 5)
                    idx = 0;
                image_type->set(pixbufs_params[idx]);
                label_type_info->set_text(passport.passport_val_type_str);
                std::string group_name =
                    static_cast<Glib::ustring>(passport.passport_group);
                image_group->set(
                    group_name == "Служебные параметры" ? pixbuf_service_params
                    : std::memcmp(group_name.data(), "Алармы/", 7) == 0
                        ? pixbuf_group_inherited
                        : pixbuf_fb);
                label_group_info->set_text(group_name);
                label_id_info->set_text(std::to_string(passport.passport_id));
                label_chipher_info->set_text(passport.chipher);
                label_name_info->set_text(passport.fullname);
                dialog.show_all();
                dialog.run();
            });
            hbox_aps_buttons_bar.pack_start(button_aps_select,
                                            Gtk::PACK_SHRINK);
            button_aps_select.signal_clicked().connect([this]() {
                Passport new_passport =
                    on_passport_choice(curr_aps_menu_idx != 2);
                if (!new_passport.selected)
                    return;
                Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                if (!curr_group_row)
                    return;
                label_aps_station_val.set_text(new_passport.station_name);
                label_aps_type_val.set_text(new_passport.passport_val_type_str);
                label_aps_id_val.set_text(
                    std::to_string(new_passport.passport_id));
                label_aps_chipher_val.set_text(new_passport.chipher);
                label_aps_name_val.set_text(new_passport.fullname);
                std::array<Passport, 7> passports =
                    curr_group_row[group_cols.passports];
                passports[4 + curr_aps_menu_idx] = new_passport;
                curr_group_row[group_cols.passports] = passports;
            });
            set_margin(vbox_aps_lowlevel_interaction_left, 5, 5);
            set_margin(vbox_aps_lowlevel_interaction_right, 5, 5);

            radiobutton_general_settings.get_style_context()->add_class(
                "selected-button");
            radiobutton_alarms.get_style_context()->add_class(
                "unselected-button");
            stack_general_settings_alarms.set_visible_child(
                vbox_general_settings);
            label_group_settings.set_margin_left(10);
            checkbutton_use_own_settings.set_margin_left(10);
            hbox_settings_buttons_bar.set_margin_left(10);
            grid_equipment_description_mask.set_margin_left(4);
            set_margin(stack_form_menu, 5, 10);
            set_margin(frame_stack_general_settings_alarms, 5, 0);
            set_margin(stack_general_settings_alarms, 4, 10);
            set_margin(frame_priority, 4, 0);
            set_margin(frame_aps_lowlevel_interaction, 4, 0);
            set_margin(hbox_crashprecrash_bounds, 10, 5);
            set_margin(grid_crash_bounds_setpoints, 10, 5);
            set_margin(grid_precrash_bounds_setpoints, 10, 5);
            checkbutton_borders_from_passport.set_margin_left(5);
            set_margin(frame_value, 10, 5);
            set_margin(frame_checkback, 10, 5);

            // Подменю сигнализируемых ситуаций
            stack_general_settings_alarms.add(frame_alarms);
            frame_alarms.set_label_widget(label_frame_alarms);
            frame_alarms.add(hbox_alarms);
            hbox_alarms.pack_start(frame_treeview_situations, Gtk::PACK_SHRINK);
            frame_treeview_situations.add(vbox_alarms_treeview);
            vbox_alarms_treeview.pack_start(hbox_alarms_treeview_buttons_var,
                                            Gtk::PACK_SHRINK);
            icon_new_alarm.set(pixbuf_add);
            hbox_alarms_treeview_buttons_var.pack_start(icon_new_alarm,
                                                        Gtk::PACK_SHRINK);
            hbox_alarms_treeview_buttons_var.pack_start(
                button_situation_new_expand, Gtk::PACK_SHRINK);
            button_situation_new_expand.set_image_from_icon_name(
                "pan-down-symbolic", Gtk::ICON_SIZE_BUTTON);
            button_situation_new_expand.set_always_show_image(true);
            hbox_alarms_treeview_buttons_var.pack_start(alarms_bar_separator1,
                                                        Gtk::PACK_SHRINK);
            hbox_alarms_treeview_buttons_var.pack_start(button_situation_delete,
                                                        Gtk::PACK_SHRINK);
            button_situation_delete_icon.set(pixbuf_delete);
            button_situation_delete.set_image(button_situation_delete_icon);
            button_situation_delete.set_always_show_image(true);
            hbox_alarms_treeview_buttons_var.pack_start(alarms_bar_separator2,
                                                        Gtk::PACK_SHRINK);
            hbox_alarms_treeview_buttons_var.pack_start(button_situation_up,
                                                        Gtk::PACK_SHRINK);
            button_situation_up_icon.set(pixbuf_arrow_up_enabled);
            button_situation_up.set_image(button_situation_up_icon);
            button_situation_up.set_always_show_image(true);
            hbox_alarms_treeview_buttons_var.pack_start(alarms_bar_separator3,
                                                        Gtk::PACK_SHRINK);
            hbox_alarms_treeview_buttons_var.pack_start(button_situation_down,
                                                        Gtk::PACK_SHRINK);
            button_situation_down_icon.set(pixbuf_arrow_down_enabled);
            button_situation_down.set_image(button_situation_down_icon);
            vbox_alarms_treeview.pack_start(scrolled_alarms,
                                            Gtk::PACK_EXPAND_WIDGET);
            scrolled_alarms.add(treeview_situations);
            treeview_situations.set_headers_visible(false);
            treeview_situations.append_column(treecolumn_alarm_toggle);
            treecolumn_alarm_toggle.pack_start(renderer_alarms_toggle, false);
            treecolumn_alarm_toggle.add_attribute(
                renderer_alarms_toggle.property_active(),
                situation_cols.is_enabled);
            renderer_alarms_toggle.signal_toggled().connect(
                [this](const Glib::ustring &path) {
                    Glib::RefPtr<Gtk::ListStore> liststore_stations =
                        Glib::RefPtr<Gtk::ListStore>::cast_static(
                            treeview_situations.get_model());
                    if (!liststore_stations)
                        return;
                    Gtk::TreeModel::Row curr_situation_row =
                        *liststore_stations->get_iter(path);
                    if (!curr_situation_row)
                        return;
                    curr_situation_row[situation_cols.is_enabled] =
                        !curr_situation_row[situation_cols.is_enabled];
                    button_save_enable();
                });
            treeview_situations.append_column("", situation_cols.name);
            frame_treeview_situations.override_background_color(
                theme_bg_color, Gtk::STATE_FLAG_NORMAL);
            hbox_alarms.pack_start(vbox_alarms_form, Gtk::PACK_SHRINK);
            vbox_alarms_form.pack_start(hbox_alarms_form_buttons_bar,
                                        Gtk::PACK_SHRINK);
            vbox_alarms_form.pack_start(hbox_alarms_form_additional_buttons_bar,
                                        Gtk::PACK_SHRINK);
            radiobutton_alarms_parameters_menu.set_mode(false);
            radiobutton_alarms_parameters_menu.signal_toggled().connect(
                [this]() {
                    if (!radiobutton_alarms_parameters_menu.get_active())
                        return;
                    redraw_alarms_params();
                });
            hbox_alarms_form_buttons_bar.pack_start(
                radiobutton_alarms_parameters_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_message_menu.set_mode(false);
            radiobutton_alarms_message_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_message_menu.get_active())
                    return;
                redraw_alarms_message();
            });
            hbox_alarms_form_buttons_bar.pack_start(
                radiobutton_alarms_message_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_sound_menu.set_mode(false);
            radiobutton_alarms_sound_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_sound_menu.get_active())
                    return;
                redraw_alarms_sound();
            });
            hbox_alarms_form_buttons_bar.pack_start(
                radiobutton_alarms_sound_menu, Gtk::PACK_SHRINK);
            hbox_alarms_form_buttons_bar.pack_start(
                radiobutton_alarms_hysteresis_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_hysteresis_menu.signal_toggled().connect(
                [this]() {
                    if (!radiobutton_alarms_hysteresis_menu.get_active())
                        return;
                    redraw_alarms_hysteresis();
                });
            radiobutton_alarms_hysteresis_menu.set_mode(false);
            hbox_alarms_form_additional_buttons_bar.pack_start(
                radiobutton_alarms_sms_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_sms_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_sms_menu.get_active())
                    return;
                redraw_alarms_sms();
            });
            radiobutton_alarms_sms_menu.set_mode(false);
            hbox_alarms_form_additional_buttons_bar.pack_start(
                radiobutton_alarms_email_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_email_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_email_menu.get_active())
                    return;
                redraw_alarms_email();
            });
            radiobutton_alarms_email_menu.set_mode(false);
            hbox_alarms_form_additional_buttons_bar.pack_start(
                radiobutton_alarms_passport_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_passport_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_passport_menu.get_active())
                    return;
                redraw_alarms_passport();
            });
            radiobutton_alarms_passport_menu.set_mode(false);
            hbox_alarms_form_additional_buttons_bar.pack_start(
                radiobutton_alarms_write_menu, Gtk::PACK_SHRINK);
            radiobutton_alarms_write_menu.signal_toggled().connect([this]() {
                if (!radiobutton_alarms_write_menu.get_active())
                    return;
                redraw_alarms_write();
            });
            radiobutton_alarms_write_menu.set_mode(false);
            vbox_alarms_form.pack_start(stack_alarms, Gtk::PACK_SHRINK);
            // Подподменю Параметры
            stack_alarms.add(vbox_alarm_parameters);
            vbox_alarm_parameters.add(hbox_alarm_kind);
            hbox_alarm_kind.pack_start(frame_alarm_kind, Gtk::PACK_SHRINK);
            frame_alarm_kind.add(vbox_alarm_kind);
            frame_alarm_kind.set_label_widget(label_frame_alarm_kind);
            radiobutton_alarm_kind_crash =
                Gtk::RadioButton(radiogroup_alarm_kind, "авария");
            radiobutton_alarm_kind_crash.signal_toggled().connect([this]() {
                if (!radiobutton_alarm_kind_crash.get_active())
                    return;
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                curr_situation_row[situation_cols.alarm_kind] = 0;
            });
            vbox_alarm_kind.pack_start(radiobutton_alarm_kind_crash,
                                       Gtk::PACK_SHRINK);
            radiobutton_alarm_kind_warning =
                Gtk::RadioButton(radiogroup_alarm_kind, "предупреждение");
            radiobutton_alarm_kind_warning.signal_toggled().connect([this]() {
                if (!radiobutton_alarm_kind_warning.get_active())
                    return;
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                curr_situation_row[situation_cols.alarm_kind] = 1;
            });
            vbox_alarm_kind.pack_start(radiobutton_alarm_kind_warning,
                                       Gtk::PACK_SHRINK);
            radiobutton_alarm_kind_notification =
                Gtk::RadioButton(radiogroup_alarm_kind, "уведомление");
            radiobutton_alarm_kind_notification.signal_toggled().connect(
                [this]() {
                    if (!radiobutton_alarm_kind_notification.get_active())
                        return;
                    Gtk::TreeModel::Row curr_situation_row =
                        *treeview_situations.get_selection()->get_selected();
                    if (!curr_situation_row)
                        return;
                    curr_situation_row[situation_cols.alarm_kind] = 2;
                });
            vbox_alarm_kind.pack_start(radiobutton_alarm_kind_notification,
                                       Gtk::PACK_SHRINK);
            hbox_alarm_kind.pack_start(vbox_alarm_actions, Gtk::PACK_SHRINK);
            vbox_alarm_actions.pack_start(checkbutton_checkback_alarm,
                                          Gtk::PACK_SHRINK);
            checkbutton_checkback_alarm.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[0] = checkbutton_checkback_alarm.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            checkbutton_send_sms.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[3] = checkbutton_send_sms.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            vbox_alarm_actions.pack_start(checkbutton_send_sms,
                                          Gtk::PACK_SHRINK);
            checkbutton_send_email.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[4] = checkbutton_send_email.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            vbox_alarm_actions.pack_start(checkbutton_send_email,
                                          Gtk::PACK_SHRINK);
            checkbutton_write_events.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[1] = checkbutton_write_events.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            vbox_alarm_actions.pack_start(checkbutton_write_events,
                                          Gtk::PACK_SHRINK);
            checkbutton_passport.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[5] = checkbutton_passport.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            vbox_alarm_parameters.pack_start(checkbutton_passport,
                                             Gtk::PACK_SHRINK);
            checkbutton_write.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                std::array<bool, 7> checkboxes =
                    curr_situation_row[situation_cols.checkboxes];
                checkboxes[6] = checkbutton_write.get_active();
                curr_situation_row[situation_cols.checkboxes] = checkboxes;
                button_save_enable();
                redraw_alarms_form();
            });
            vbox_alarm_parameters.pack_start(checkbutton_write,
                                             Gtk::PACK_SHRINK);
            vbox_alarm_parameters.pack_start(hbox_response_delay,
                                             Gtk::PACK_SHRINK);
            hbox_response_delay.pack_start(label_response_delay,
                                           Gtk::PACK_SHRINK);
            hbox_response_delay.pack_start(spin_response_delay,
                                           Gtk::PACK_SHRINK);
            spin_response_delay.signal_value_changed().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                if (spin_response_delay.get_value_as_int() ==
                    curr_situation_row[situation_cols.alarm_confirm_period])
                    return;
                curr_situation_row[situation_cols.alarm_confirm_period] =
                    spin_response_delay.get_value_as_int();
                button_save_enable();
            });
            hbox_response_delay.pack_start(combobox_response_delay,
                                           Gtk::PACK_SHRINK);
            combobox_response_delay.append("сек.");
            combobox_response_delay.append("мин.");
            combobox_response_delay.append("ч.");
            combobox_response_delay.set_active(0);
            combobox_response_delay.signal_changed().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                int curr_idx = Gtk::TreeModel::Path(
                    combobox_response_delay.get_active())[0];
                if (curr_situation_row[situation_cols.alarm_confirm_interval] ==
                    curr_idx)
                    return;
                curr_situation_row[situation_cols.alarm_confirm_interval] =
                    curr_idx;
                button_save_enable();
            });
            vbox_alarm_parameters.pack_start(frame_situation_params,
                                             Gtk::PACK_SHRINK);
            frame_situation_params.set_label_widget(
                label_frame_situation_params);
            frame_situation_params.add(stack_situation_params);
            stack_situation_params.add(hbox_situation_params_range);
            stack_situation_params.add(hbox_situation_params_discrete);
            stack_situation_params.add(hbox_situation_params_errorcode);
            hbox_situation_params_range.pack_start(
                entry_situation_params_greaterthan, Gtk::PACK_SHRINK);
            hbox_situation_params_range.pack_start(label_situation_params_range,
                                                   Gtk::PACK_SHRINK);
            hbox_situation_params_range.pack_start(
                entry_situation_params_lessthan, Gtk::PACK_SHRINK);
            hbox_situation_params_discrete.pack_start(
                radiobutton_situation_params_discrete_false, Gtk::PACK_SHRINK);
            hbox_situation_params_discrete.pack_start(
                radiobutton_situation_params_discrete_true, Gtk::PACK_SHRINK);
            hbox_situation_params_errorcode.pack_start(
                combobox_situation_params_errorcode, Gtk::PACK_SHRINK);
            combobox_situation_params_errorcode.append(
                "[255] - Нет связи с Сервером Доступа к Данным");
            combobox_situation_params_errorcode.append(
                "[43] - Недостоверное значение тега");
            combobox_situation_params_errorcode.append(
                "[15] - Превышение допустимой скорости изменения");
            combobox_situation_params_errorcode.append(
                "[1] - Предаварийный минимум");
            combobox_situation_params_errorcode.append(
                "[2] - Предаварийный максимум");
            combobox_situation_params_errorcode.append(
                "[3] - Аварийный минимум");
            combobox_situation_params_errorcode.append(
                "[4] - Аварийный максимум");
            combobox_situation_params_errorcode.append(
                "[80] - Ноль в знаменателе");
            combobox_situation_params_errorcode.append(
                "[90] - Отрицательное подкоренное выражение");
            combobox_situation_params_errorcode.append(
                "[93] - Неизвестная функция");
            combobox_situation_params_errorcode.append(
                "[95] - Недопустимое значение аргумента функции");
            combobox_situation_params_errorcode.append(
                "[110] - Запрещена запись в паспорт");
            combobox_situation_params_errorcode.append(
                "[120] - Управление параметром запрещено");
            combobox_situation_params_errorcode.append(
                "[11] - Значение ни разу не вычислялось");
            combobox_situation_params_errorcode.append(
                "[20] - Неправильный тип паспорта");
            combobox_situation_params_errorcode.append(
                "[30] - Неправильный тип коррекции");
            combobox_situation_params_errorcode.append(
                "[40] - Ошибка получения опер. значения тега");
            combobox_situation_params_errorcode.append(
                "[41] - Ошибка получения ист. значения тега");
            combobox_situation_params_errorcode.append(
                "[45] - Не удалось произвести запись в тег");
            combobox_situation_params_errorcode.append(
                "[50] - Полученный код меньше минимального");
            combobox_situation_params_errorcode.append(
                "[60] - Полученный код больше максимального");
            combobox_situation_params_errorcode.append(
                "[70] - Запрошенное значение лежит за пределами");
            combobox_situation_params_errorcode.append(
                "[100] - Неизвестная ошибка обсчета параметра");
            combobox_situation_params_errorcode.append(
                "[150] - Ошибка запроса сетевого паспорта");
            combobox_situation_params_errorcode.append(
                "[160] - Ошибка исполнения скрипта");
            combobox_situation_params_errorcode.append(
                "[170] - В скрипте не задан выход");
            combobox_situation_params_errorcode.append(
                "[180] - Ошибка компиляции скрипта");
            stack_situation_params.show_all();
            hbox_response_delay.set_margin_left(10);
            set_margin(frame_situation_params, 10, 5);
            set_margin(stack_situation_params, 10, 5);

            // Подподменю Сообщения
            stack_alarms.add(vbox_alarm_message);
            vbox_alarm_message.pack_start(label_message, Gtk::PACK_SHRINK);
            label_message.set_halign(Gtk::ALIGN_START);
            label_message.set_name("bold");
            label_message.set_text("Выдаваемое сообщение");
            vbox_alarm_message.pack_start(entry_message, Gtk::PACK_SHRINK);
            entry_message.signal_changed().connect([this]() {
                std::string message = entry_message.get_text();
                render_masks(message);
                label_message_mask_view.set_text(message);
            });
            vbox_alarm_message.pack_start(label_message_mask_view,
                                          Gtk::PACK_SHRINK);
            label_message_mask_view.set_halign(Gtk::ALIGN_START);
            vbox_alarm_message.pack_start(checkbutton_write_event_to_group,
                                          Gtk::PACK_SHRINK);
            checkbutton_write_event_to_group.signal_toggled().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                if (curr_situation_row[situation_cols.write_to_usergroup] ==
                    checkbutton_write_event_to_group.get_active())
                    return;
                curr_situation_row[situation_cols.write_to_usergroup] =
                    checkbutton_write_event_to_group.get_active();
                button_save_enable();
            });
            vbox_alarm_message.pack_start(grid_usergroup, Gtk::PACK_SHRINK);
            grid_usergroup.set_column_spacing(5);
            grid_usergroup.set_row_spacing(5);
            grid_usergroup.attach(label_usergroup, 0, 0, 1, 1);
            label_usergroup.set_halign(Gtk::ALIGN_START);
            label_usergroup.set_text("Имя:");
            hbox_usergroup.pack_start(entry_usergroup, Gtk::PACK_EXPAND_WIDGET);
            hbox_usergroup.pack_end(button_usergroup, Gtk::PACK_SHRINK);
            grid_usergroup.attach(hbox_usergroup, 1, 0, 1, 1);
            button_usergroup.set_label("…");
            button_usergroup.signal_clicked().connect([this]() {
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                Gtk::Dialog dialog("Выбор группы", true);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                dialog.add_button("ОК", Gtk::RESPONSE_OK);
                dialog.set_size_request(400, 400);
                Gtk::Box *vbox = dialog.get_content_area();
                Gtk::ScrolledWindow *scrolled =
                    Gtk::make_managed<Gtk::ScrolledWindow>();
                scrolled->set_policy(Gtk::POLICY_AUTOMATIC,
                                     Gtk::POLICY_AUTOMATIC);
                vbox->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);
                Glib::RefPtr<Gtk::ListStore> store =
                    Gtk::ListStore::create(usergroup_cols);
                for (const std::string &desc : usergroup_names()) {
                    Gtk::TreeModel::Row row = *store->append();
                    row[usergroup_cols.description] = desc;
                }
                Gtk::TreeView *treeview = Gtk::make_managed<Gtk::TreeView>();
                treeview->set_model(store);
                treeview->append_column("", usergroup_cols.description);
                scrolled->add(*treeview);
                dialog.show_all();
                int res = dialog.run();
                if (res != Gtk::RESPONSE_OK)
                    return;
                Gtk::TreeModel::iterator sel =
                    treeview->get_selection()->get_selected();
                if (!sel)
                    return;
                Glib::ustring chosen = (*sel)[usergroup_cols.description];
                entry_usergroup.set_text(chosen);
                curr_situation_row[situation_cols.usergroup_name] = chosen;
                button_save_enable();
            });
            grid_usergroup.attach(label_usergroup_description, 0, 1, 1, 1);
            label_usergroup_description.set_halign(Gtk::ALIGN_START);
            label_usergroup_description.set_text("Описание:");
            entry_usergroup_description.set_sensitive(false);
            grid_usergroup.attach(entry_usergroup_description, 1, 1, 1, 1);
            entry_usergroup.signal_changed().connect([this]() {
                entry_usergroup_description.set_text(
                    usergroup_description(entry_usergroup.get_text()));
            });
            vbox_alarm_message.pack_start(frame_message_masks,
                                          Gtk::PACK_SHRINK);
            frame_message_masks.add(vbox_message_masks);
            vbox_message_masks.pack_start(hbox_message_masks_top,
                                          Gtk::PACK_SHRINK);
            hbox_message_masks_top.pack_start(grid_message_masks_left,
                                              Gtk::PACK_SHRINK);
            grid_message_masks_left.set_column_spacing(5);
            grid_message_masks_left.set_row_spacing(5);
            grid_message_masks_left.attach(label_message_mask_station, 0, 0, 1,
                                           1);
            label_message_mask_station.set_halign(Gtk::ALIGN_START);
            label_message_mask_station.set_text("Станция");
            grid_message_masks_left.attach(label_message_mask_station_val, 1, 0,
                                           1, 1);
            label_message_mask_station_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_station_val.set_text("%s");
            grid_message_masks_left.attach(label_message_mask_type, 0, 1, 1, 1);
            label_message_mask_type.set_halign(Gtk::ALIGN_START);
            label_message_mask_type.set_text("Тип");
            grid_message_masks_left.attach(label_message_mask_type_val, 1, 1, 1,
                                           1);
            label_message_mask_type_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_type_val.set_text("%t");
            grid_message_masks_left.attach(label_message_mask_group, 0, 2, 1,
                                           1);
            label_message_mask_group.set_halign(Gtk::ALIGN_START);
            label_message_mask_group.set_text("Группа");
            grid_message_masks_left.attach(label_message_mask_group_val, 1, 2,
                                           1, 1);
            label_message_mask_group_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_group_val.set_text("%g");
            grid_message_masks_left.attach(label_message_mask_measure_unit, 0,
                                           3, 1, 1);
            label_message_mask_measure_unit.set_halign(Gtk::ALIGN_START);
            label_message_mask_measure_unit.set_text("Ед.изм.");
            grid_message_masks_left.attach(label_message_mask_measure_unit_val,
                                           1, 3, 1, 1);
            label_message_mask_measure_unit_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_measure_unit_val.set_text("%u");
            grid_message_masks_left.attach(label_message_mask_time, 0, 4, 1, 1);
            label_message_mask_time.set_halign(Gtk::ALIGN_START);
            label_message_mask_time.set_text("Время");
            grid_message_masks_left.attach(label_message_mask_time_val, 1, 4, 1,
                                           1);
            label_message_mask_time_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_time_val.set_text("%d");
            grid_message_masks_left.attach(label_message_mask_value, 0, 5, 1,
                                           1);
            label_message_mask_value.set_halign(Gtk::ALIGN_START);
            label_message_mask_value.set_text("Значение");
            grid_message_masks_left.attach(label_message_mask_value_val, 1, 5,
                                           1, 1);
            label_message_mask_value_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_value_val.set_text("%v");
            hbox_message_masks_top.pack_start(grid_message_masks_right,
                                              Gtk::PACK_EXPAND_WIDGET);
            grid_message_masks_right.set_halign(Gtk::ALIGN_END);
            grid_message_masks_right.set_column_spacing(5);
            grid_message_masks_right.set_row_spacing(5);
            grid_message_masks_right.attach(label_message_mask_passport_id, 0,
                                            0, 1, 1);
            label_message_mask_passport_id.set_halign(Gtk::ALIGN_START);
            label_message_mask_passport_id.set_text("Ид. паспорта");
            grid_message_masks_right.attach(label_message_mask_passport_id_val,
                                            1, 0, 1, 1);
            label_message_mask_passport_id_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_passport_id_val.set_text("%i");
            grid_message_masks_right.attach(label_message_mask_chipher, 0, 1, 1,
                                            1);
            label_message_mask_chipher.set_halign(Gtk::ALIGN_START);
            label_message_mask_chipher.set_text("Шифр");
            grid_message_masks_right.attach(label_message_mask_chipher_val, 1,
                                            1, 1, 1);
            label_message_mask_chipher_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_chipher_val.set_text("%c");
            grid_message_masks_right.attach(label_message_mask_name, 0, 2, 1,
                                            1);
            label_message_mask_name.set_halign(Gtk::ALIGN_START);
            label_message_mask_name.set_text("Наименование");
            grid_message_masks_right.attach(label_message_mask_name_val, 1, 2,
                                            1, 1);
            label_message_mask_name_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_name_val.set_text("%n");
            grid_message_masks_right.attach(label_message_mask_error, 0, 3, 1,
                                            1);
            label_message_mask_error.set_halign(Gtk::ALIGN_START);
            label_message_mask_error.set_text("Ошибка");
            grid_message_masks_right.attach(label_message_mask_error_val, 1, 3,
                                            1, 1);
            label_message_mask_error_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_error_val.set_text("%q");
            grid_message_masks_right.attach(label_message_mask_zone, 0, 4, 1,
                                            1);
            label_message_mask_zone.set_halign(Gtk::ALIGN_START);
            label_message_mask_zone.set_text("Зона");
            grid_message_masks_right.attach(label_message_mask_zone_val, 1, 4,
                                            1, 1);
            label_message_mask_zone_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_zone_val.set_text("%z");
            grid_message_masks_right.attach(label_message_mask_prev_value, 0, 5,
                                            1, 1);
            label_message_mask_prev_value.set_halign(Gtk::ALIGN_START);
            label_message_mask_prev_value.set_text("Пред. значение");
            grid_message_masks_right.attach(label_message_mask_prev_value_val,
                                            1, 5, 1, 1);
            label_message_mask_prev_value_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_prev_value_val.set_text("%p");
            grid_message_masks_right.attach(label_message_mask_device, 0, 6, 1,
                                            1);
            label_message_mask_device.set_halign(Gtk::ALIGN_START);
            label_message_mask_device.set_text("Оборудование");
            grid_message_masks_right.attach(label_message_mask_device_val, 1, 6,
                                            1, 1);
            label_message_mask_device_val.set_halign(Gtk::ALIGN_START);
            label_message_mask_device_val.set_text("%o");
            vbox_message_masks.pack_start(hbox_message_masks_bottom,
                                          Gtk::PACK_SHRINK);
            hbox_message_masks_bottom.set_halign(Gtk::ALIGN_CENTER);
            hbox_message_masks_bottom.pack_start(
                label_message_mask_level_of_passport_group, Gtk::PACK_SHRINK);
            label_message_mask_level_of_passport_group.set_text(
                "Группа паспортов уровня (0 - тек.)");
            hbox_message_masks_bottom.pack_start(
                label_message_mask_level_of_passport_group_val,
                Gtk::PACK_SHRINK);
            label_message_mask_level_of_passport_group_val.set_text("%pg");
            set_margin(grid_message_masks_left, 10, 2);
            set_margin(grid_message_masks_right, 10, 2);

            // Подподменю Звук
            stack_alarms.add(vbox_alarm_sound);
            vbox_alarm_sound.pack_start(hbox_sound_treeview_buttons_bar,
                                        Gtk::PACK_SHRINK);
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_new,
                                                       Gtk::PACK_SHRINK);
            button_sound_new.set_image(button_sound_new_icon);
            button_sound_new_icon.set(pixbuf_add);
            button_sound_new.set_always_show_image(true);
            button_sound_new.signal_clicked().connect([this]() {
                Gtk::FileChooserDialog dialog("Выберите звуковой файл",
                                              Gtk::FILE_CHOOSER_ACTION_OPEN);
                dialog.set_transient_for(*this);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                dialog.add_button("ОК", Gtk::RESPONSE_OK);
                if (dialog.run() != Gtk::RESPONSE_OK)
                    return;
                Gtk::TreeModel::Row situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!situation_row)
                    return;
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    situation_row[situation_cols.sounds];
                std::string filepath = dialog.get_filename();
                std::string errors;
                (void)copy_sound_to_sounds_dir(filepath, soundfiles_dir_path,
                                               errors);
                if (!errors.empty()) {
                    Gtk::MessageDialog dialog(
                        "Не удалось сохранить звуковой файл\n\n" + errors,
                        false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                    return;
                }
                std::string filename =
                    std::filesystem::path(filepath).filename().string();
                Gtk::TreeModel::Row sound_row = *liststore_sounds->prepend();
                sound_row[sound_cols.icon] = pixbufs_loop[0];
                sound_row[sound_cols.loop_start] = false;
                sound_row[sound_cols.name] = filename;
                sound_row[sound_cols.filepath_or_pause_duration] =
                    soundfiles_dir_path + "/" + filename;
                situation_row[situation_cols.sounds] = liststore_sounds;
            });
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_pause,
                                                       Gtk::PACK_SHRINK);
            button_sound_pause.set_image(button_sound_pause_icon);
            button_sound_pause_icon.set(pixbuf_pause);
            button_sound_pause.set_always_show_image(true);
            button_sound_pause.signal_clicked().connect([this]() {
                Gtk::MessageDialog dialog("Введите длительность паузы", false,
                                          Gtk::MESSAGE_QUESTION,
                                          Gtk::BUTTONS_NONE);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                dialog.add_button("OK", Gtk::RESPONSE_OK);
                Gtk::Box *vbox = dialog.get_content_area();
                Gtk::Box *hbox_duration =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*hbox_duration, Gtk::PACK_SHRINK);
                hbox_duration->pack_start(
                    *Gtk::make_managed<Gtk::Label>("Длительность (мсек)"),
                    Gtk::PACK_SHRINK);
                Gtk::Entry *entry_duration = Gtk::make_managed<Gtk::Entry>();
                hbox_duration->pack_start(*entry_duration, Gtk::PACK_SHRINK);
                entry_duration->set_text("1000");
                dialog.show_all();
                int res = dialog.run();
                std::string duration_str = entry_duration->get_text();
                if (res != Gtk::RESPONSE_OK)
                    return;
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!liststore_sounds)
                    return;
                Gtk::TreeModel::Row pause_row = *liststore_sounds->append();
                pause_row[sound_cols.name] = "<Pause>" + duration_str;
                pause_row[sound_cols.filepath_or_pause_duration] = duration_str;
            });
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_delete,
                                                       Gtk::PACK_SHRINK);
            button_sound_delete.set_image(button_sound_delete_icon);
            button_sound_delete_icon.set(pixbuf_delete);
            button_sound_delete.set_always_show_image(true);
            button_sound_delete.signal_clicked().connect([this]() {
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!liststore_sounds)
                    return;
                Gtk::TreeModel::Row row =
                    *treeview_sounds.get_selection()->get_selected();
                if (!row)
                    return;
                Gtk::MessageDialog dialog(
                    "Вы действительно хотите удалить этот звуковой файл?",
                    false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                dialog.add_button("ОК", Gtk::RESPONSE_YES);
                dialog.show_all();
                int res = dialog.run();
                if (res != Gtk::RESPONSE_YES)
                    return;
                liststore_sounds->erase(row);
                if (liststore_sounds->children().size() == 0) {
                    button_sound_delete.set_sensitive(false);
                    button_sound_up.set_sensitive(false);
                    button_sound_down.set_sensitive(false);
                }
            });
            hbox_sound_treeview_buttons_bar.pack_start(sound_separator1,
                                                       Gtk::PACK_SHRINK);
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_up,
                                                       Gtk::PACK_SHRINK);
            button_sound_up.set_image(button_sound_up_icon);
            button_sound_up_icon.set(pixbuf_arrow_up_enabled);
            button_sound_up.set_always_show_image(true);
            button_sound_up.signal_clicked().connect([this]() {
                Gtk::TreeModel::Row curr_iter =
                    *treeview_sounds.get_selection()->get_selected();
                if (!curr_iter)
                    return;
                Glib::RefPtr<Gtk::ListStore> store =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!store)
                    return;

                Gtk::TreeModel::Path curr_path = store->get_path(curr_iter);
                if (curr_path.empty())
                    return;
                Gtk::TreeModel::Path next_path(
                    std::to_string((int)curr_path[0] - 1));
                Gtk::TreeModel::iterator next_iter = store->get_iter(next_path);
                if (!next_iter)
                    return;
                Gtk::TreeModel::Row curr_row = *curr_iter;
                Gtk::TreeModel::Row next_row = *next_iter;
                swap_sound_rows(curr_row, next_row);
                treeview_sounds.get_selection()->select(next_iter);
                button_save_enable();
            });
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_down,
                                                       Gtk::PACK_SHRINK);
            button_sound_down.set_image(button_sound_down_icon);
            button_sound_down_icon.set(pixbuf_arrow_down_enabled);
            button_sound_down.set_always_show_image(true);
            button_sound_down.signal_clicked().connect([this]() {
                Gtk::TreeModel::Row curr_iter =
                    *treeview_sounds.get_selection()->get_selected();
                if (!curr_iter)
                    return;
                Glib::RefPtr<Gtk::ListStore> store =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!store)
                    return;

                Gtk::TreeModel::Path curr_path = store->get_path(curr_iter);
                if (curr_path.empty())
                    return;
                Gtk::TreeModel::Path prev_path(
                    std::to_string((int)curr_path[0] + 1));
                Gtk::TreeModel::iterator prev_iter = store->get_iter(prev_path);
                if (!prev_iter)
                    return;
                Gtk::TreeModel::Row curr_row = *curr_iter;
                Gtk::TreeModel::Row prev_row = *prev_iter;
                swap_sound_rows(curr_row, prev_row);
                treeview_sounds.get_selection()->select(prev_iter);
                button_save_enable();
            });
            hbox_sound_treeview_buttons_bar.pack_start(sound_separator2,
                                                       Gtk::PACK_SHRINK);
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_play,
                                                       Gtk::PACK_SHRINK);
            button_sound_play.set_image(button_sound_play_icon);
            button_sound_play_icon.set(pixbuf_play);
            button_sound_play.set_always_show_image(true);
            button_sound_play.signal_clicked().connect([this]() {
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!liststore_sounds)
                    return;
                if (liststore_sounds->children().empty())
                    return;
                Gtk::TreeModel::iterator curr_sound_iter =
                    treeview_sounds.get_selection()->get_selected();
                if (!curr_sound_iter)
                    curr_sound_iter = liststore_sounds->children().begin();
                play_sound_async(curr_sound_iter, liststore_sounds);
            });
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_stop,
                                                       Gtk::PACK_SHRINK);
            button_sound_stop.set_image(button_sound_stop_icon);
            button_sound_stop_icon.set(pixbuf_stop);
            button_sound_stop.set_always_show_image(true);
            button_sound_stop.signal_clicked().connect(
                [this]() { pause_sound(); });
            hbox_sound_treeview_buttons_bar.pack_start(sound_separator3,
                                                       Gtk::PACK_SHRINK);
            hbox_sound_treeview_buttons_bar.pack_start(button_sound_loop,
                                                       Gtk::PACK_SHRINK);
            button_sound_loop.set_image(button_sound_loop_icon);
            button_sound_loop_icon.set(pixbuf_loop);
            button_sound_loop.set_always_show_image(true);
            button_sound_loop.signal_clicked().connect([this]() {
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!liststore_sounds)
                    return;
                Gtk::TreeModel::iterator curr_sound_iter =
                    treeview_sounds.get_selection()->get_selected();
                if (!curr_sound_iter)
                    curr_sound_iter = liststore_sounds->children().begin();
                Gtk::TreeModel ::Children sound_children =
                    liststore_sounds->children();
                for (Gtk::TreeModel::iterator iter = sound_children.begin();
                     iter != curr_sound_iter; iter++) {
                    (*iter)[sound_cols.icon] = pixbufs_loop[0];
                    (*iter)[sound_cols.loop_start] = false;
                }
                Gtk::TreeModel::iterator last_iter =
                    std::prev(sound_children.end());
                if (curr_sound_iter == last_iter) {
                    (*curr_sound_iter)[sound_cols.icon] = pixbufs_loop[1];
                    (*curr_sound_iter)[sound_cols.loop_start] = true;
                    return;
                }
                (*curr_sound_iter)[sound_cols.icon] = pixbufs_loop[2];
                (*curr_sound_iter)[sound_cols.loop_start] = true;
                curr_sound_iter++;
                while (curr_sound_iter != last_iter) {
                    (*curr_sound_iter)[sound_cols.icon] = pixbufs_loop[3];
                    (*curr_sound_iter)[sound_cols.loop_start] = false;
                    curr_sound_iter++;
                }
                (*last_iter)[sound_cols.icon] = pixbufs_loop[4];
                (*last_iter)[sound_cols.loop_start] = false;
            });

            stack_alarms.show_all();
            alarms_bar_separator1.set_margin_left(10);
            alarms_bar_separator1.set_margin_right(10);
            alarms_bar_separator2.set_margin_left(10);
            alarms_bar_separator2.set_margin_right(10);
            alarms_bar_separator3.set_margin_left(10);
            alarms_bar_separator3.set_margin_right(10);
            set_margin(hbox_alarms_treeview_buttons_var, 10, 5);
            set_margin(treeview_situations, 10, 10);
            set_margin(frame_treeview_situations, 10, 10);
            frame_treeview_situations.set_margin_right(0);
            set_margin(vbox_alarms_form, 10, 10);
            vbox_alarms_form.set_margin_left(0);
            set_margin(vbox_alarm_kind, 5, 5);
            stack_form_menu.add(vbox_settings_menu);
            vbox_alarm_sound.pack_start(scrolled_sounds,
                                        Gtk::PACK_EXPAND_WIDGET);
            scrolled_sounds.add(frame_sounds);
            frame_sounds.add(treeview_sounds);
            treeview_sounds.set_headers_visible(false);
            treeview_sounds.append_column("", sound_cols.icon);
            treeview_sounds.append_column("", sound_cols.name);
            set_margin(treeview_sounds, 10, 10);

            // Меню паспортов
            treeview_passports.append_column("T", alarmobj_cols.icon);
            treeview_passports.append_column("Приоритет",
                                             alarmobj_cols.passport_priority);
            treeview_passports.append_column("Станция",
                                             alarmobj_cols.station_name);
            treecolumn_passport_id.set_title("Ид.");
            treecolumn_passport_id.pack_start(renderer_passport_id, false);
            treecolumn_passport_id.set_cell_data_func(
                renderer_passport_id,
                [this](Gtk::CellRenderer *cr,
                       const Gtk::TreeModel::iterator &it) {
                    unsigned short int id = (*it)[alarmobj_cols.passport_id];
                    static_cast<Gtk::CellRendererText *>(cr)->property_text() =
                        std::to_string(id);
                });
            treeview_passports.append_column(treecolumn_passport_id);
            treeview_passports.append_column("Шифр параметра",
                                             alarmobj_cols.fullname);
            treeview_passports.append_column(
                "Тип паспорта", alarmobj_cols.passport_val_type_str);
            treeview_passports.append_column("Наименование",
                                             alarmobj_cols.name);
            treeview_passports.append_column("Группа",
                                             alarmobj_cols.passport_group);
            vbox_passports_menu.pack_start(treeview_passports,
                                           Gtk::PACK_EXPAND_WIDGET);
            stack_form_menu.add(vbox_passports_menu);
        }

        void setup_signals() {
            signal_delete_event().connect([this](GdkEventAny *event) {
                (void)event;
                bool ret = on_exit_clicked();
                return ret;
            });

            button_save.signal_clicked().connect(
                [this]() { on_save_clicked(); });
            button_delete.signal_clicked().connect(
                [this]() { on_delete_clicked(); });
            button_new_group.signal_clicked().connect(
                [this]() { on_new_group_clicked(); });
            button_address_book.signal_clicked().connect([this]() {
                std::string errors;
                treestore_contacts->clear();
                (void)parse_contacts_config(
                    stations_config_path, treestore_contacts, pixbuf_contact,
                    pixbuf_contact_group, contact_cols, errors);
                if (!errors.empty()) {
                    Gtk::MessageDialog dialog(
                        *this,
                        "Не удалось прочитать файл контактов\n\n" + errors,
                        false, Gtk::MESSAGE_ERROR);
                    dialog.run();
                    return;
                }

                Gtk::MessageDialog dialog("Адресная книга", false,
                                          Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE);
                dialog.add_button("ОК", Gtk::RESPONSE_OK);
                dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                dialog.set_size_request(600, 500);
                Gtk::Box *vbox = dialog.get_content_area();
                Gtk::Box *topbar =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
                vbox->pack_start(*topbar, Gtk::PACK_SHRINK);
                Gtk::Button *button_save = Gtk::make_managed<Gtk::Button>();
                topbar->pack_start(*button_save, Gtk::PACK_SHRINK);
                Gtk::Image *image_save = Gtk::make_managed<Gtk::Image>();
                button_save->set_image(*image_save);
                image_save->set(pixbuf_save_disabled);
                Gtk::Button *button_contact_add =
                    Gtk::make_managed<Gtk::Button>();
                topbar->pack_start(*button_contact_add, Gtk::PACK_SHRINK);
                Gtk::Image *image_contact_add = Gtk::make_managed<Gtk::Image>();
                button_contact_add->set_image(*image_contact_add);
                image_contact_add->set(pixbuf_contact_add);
                Gtk::Button *button_contact_group_add =
                    Gtk::make_managed<Gtk::Button>();
                topbar->pack_start(*button_contact_group_add, Gtk::PACK_SHRINK);
                Gtk::Image *image_contact_group_add =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_group_add->set_image(*image_contact_group_add);
                image_contact_group_add->set(pixbuf_contact_group_add);
                Gtk::Button *button_contact_delete =
                    Gtk::make_managed<Gtk::Button>();
                topbar->pack_start(*button_contact_delete, Gtk::PACK_SHRINK);
                Gtk::Image *image_contact_delete =
                    Gtk::make_managed<Gtk::Image>();
                button_contact_delete->set_image(*image_contact_delete);
                image_contact_delete->set(pixbuf_delete);

                auto contacts_save_enable = [this, button_save, image_save]() {
                    button_save->set_sensitive(true);
                    image_save->set(pixbuf_save_enabled);
                };
                auto contacts_save_disable = [this, button_save, image_save]() {
                    button_save->set_sensitive(false);
                    image_save->set(pixbuf_save_disabled);
                };
                auto save_contacts = [this, contacts_save_disable]() {
                    std::string errors;
                    write_contacts_config(stations_config_path,
                                          treestore_contacts, contact_cols,
                                          errors);
                    if (!errors.empty()) {
                        Gtk::MessageDialog err_dialog(
                            *this,
                            "Не удалось записать файл контактов\n\n" + errors,
                            false, Gtk::MESSAGE_ERROR);
                        err_dialog.run();
                        return;
                    }
                    contacts_save_disable();
                };
                contacts_save_disable();
                button_save->signal_clicked().connect(save_contacts);

                Gtk::ScrolledWindow *scrolled =
                    Gtk::make_managed<Gtk::ScrolledWindow>();
                scrolled->set_policy(Gtk::POLICY_AUTOMATIC,
                                     Gtk::POLICY_AUTOMATIC);
                vbox->pack_start(*scrolled, Gtk::PACK_EXPAND_WIDGET);
                Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
                scrolled->add(*frame);
                frame->override_background_color(theme_bg_color,
                                                 Gtk::STATE_FLAG_NORMAL);
                Gtk::TreeView *treeview = Gtk::make_managed<Gtk::TreeView>();
                frame->add(*treeview);
                treeview->set_model(treestore_contacts);
                {
                    auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                    auto *renderer_icon =
                        Gtk::make_managed<Gtk::CellRendererPixbuf>();
                    renderer_icon->set_fixed_size(-1, 32);
                    column->pack_start(*renderer_icon, false);
                    column->add_attribute(renderer_icon->property_pixbuf(),
                                          contact_cols.icon);
                    auto *renderer = Gtk::make_managed<Gtk::CellRendererText>();
                    renderer->set_fixed_size(-1, 32);
                    column->pack_start(*renderer, true);
                    column->add_attribute(renderer->property_text(),
                                          contact_cols.name);
                    auto *header_box = Gtk::make_managed<Gtk::Box>(
                        Gtk::ORIENTATION_HORIZONTAL, 4);
                    auto *header_image =
                        Gtk::make_managed<Gtk::Image>(pixbuf_contact_name);
                    auto *header_label =
                        Gtk::make_managed<Gtk::Label>("Название(имя)");
                    header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                    header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                    header_box->show_all();
                    column->set_widget(*header_box);
                    treeview->append_column(*column);
                }
                {
                    auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                    auto *renderer = Gtk::make_managed<Gtk::CellRendererText>();
                    renderer->set_fixed_size(-1, 32);
                    column->pack_start(*renderer, true);
                    column->add_attribute(renderer->property_text(),
                                          contact_cols.comment);
                    auto *header_box = Gtk::make_managed<Gtk::Box>(
                        Gtk::ORIENTATION_HORIZONTAL, 4);
                    auto *header_image =
                        Gtk::make_managed<Gtk::Image>(pixbuf_description);
                    auto *header_label =
                        Gtk::make_managed<Gtk::Label>("Комментарий");
                    header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                    header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                    header_box->show_all();
                    column->set_widget(*header_box);
                    treeview->append_column(*column);
                }
                {
                    auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                    auto *renderer = Gtk::make_managed<Gtk::CellRendererText>();
                    renderer->set_fixed_size(-1, 32);
                    column->pack_start(*renderer, true);
                    column->add_attribute(renderer->property_text(),
                                          contact_cols.email);
                    auto *header_box = Gtk::make_managed<Gtk::Box>(
                        Gtk::ORIENTATION_HORIZONTAL, 4);
                    auto *header_image =
                        Gtk::make_managed<Gtk::Image>(pixbuf_email);
                    auto *header_label =
                        Gtk::make_managed<Gtk::Label>("E-mail");
                    header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                    header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                    header_box->show_all();
                    column->set_widget(*header_box);
                    treeview->append_column(*column);
                }
                {
                    auto *column = Gtk::make_managed<Gtk::TreeViewColumn>();
                    auto *renderer = Gtk::make_managed<Gtk::CellRendererText>();
                    renderer->set_fixed_size(-1, 32);
                    column->pack_start(*renderer, true);
                    column->add_attribute(renderer->property_text(),
                                          contact_cols.phone);
                    auto *header_box = Gtk::make_managed<Gtk::Box>(
                        Gtk::ORIENTATION_HORIZONTAL, 4);
                    auto *header_image =
                        Gtk::make_managed<Gtk::Image>(pixbuf_phone);
                    auto *header_label =
                        Gtk::make_managed<Gtk::Label>("Телефон");
                    header_box->pack_start(*header_image, Gtk::PACK_SHRINK);
                    header_box->pack_start(*header_label, Gtk::PACK_SHRINK);
                    header_box->show_all();
                    column->set_widget(*header_box);
                    treeview->append_column(*column);
                }
                Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
                vbox->pack_start(*grid, Gtk::PACK_SHRINK);
                grid->set_column_spacing(10);
                grid->set_row_spacing(10);
                Gtk::Image *image_name = Gtk::make_managed<Gtk::Image>();
                grid->attach(*image_name, 0, 0, 1, 1);
                image_name->set(pixbuf_contact_name);
                Gtk::Entry *entry_name = Gtk::make_managed<Gtk::Entry>();
                entry_name->signal_changed().connect(
                    [this, contacts_save_enable, treeview, entry_name]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        if (curr_contact_row[contact_cols.name] ==
                            entry_name->get_text())
                            return;
                        curr_contact_row[contact_cols.name] =
                            entry_name->get_text();
                        contacts_save_enable();
                    });
                entry_name->set_hexpand(true);
                grid->attach(*entry_name, 1, 0, 1, 1);
                Gtk::Image *image_description = Gtk::make_managed<Gtk::Image>();
                grid->attach(*image_description, 2, 0, 1, 1);
                image_description->set(pixbuf_description);
                Gtk::Entry *entry_description = Gtk::make_managed<Gtk::Entry>();
                entry_description->signal_changed().connect(
                    [this, contacts_save_enable, treeview,
                     entry_description]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        if (curr_contact_row[contact_cols.comment] ==
                            entry_description->get_text())
                            return;
                        curr_contact_row[contact_cols.comment] =
                            entry_description->get_text();
                        contacts_save_enable();
                    });
                entry_description->set_hexpand(true);
                grid->attach(*entry_description, 3, 0, 1, 1);
                Gtk::Image *image_email = Gtk::make_managed<Gtk::Image>();
                grid->attach(*image_email, 0, 1, 1, 1);
                image_email->set(pixbuf_email);
                Gtk::Entry *entry_email = Gtk::make_managed<Gtk::Entry>();
                entry_email->signal_changed().connect(
                    [this, contacts_save_enable, treeview, entry_email]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        if (curr_contact_row[contact_cols.email] ==
                            entry_email->get_text())
                            return;
                        curr_contact_row[contact_cols.email] =
                            entry_email->get_text();
                        contacts_save_enable();
                    });
                entry_email->set_hexpand(true);
                grid->attach(*entry_email, 1, 1, 1, 1);
                Gtk::Image *image_phone = Gtk::make_managed<Gtk::Image>();
                grid->attach(*image_phone, 2, 1, 1, 1);
                image_phone->set(pixbuf_phone);
                Gtk::Entry *entry_phone = Gtk::make_managed<Gtk::Entry>();
                entry_phone->signal_changed().connect(
                    [this, contacts_save_enable, treeview, entry_phone]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        if (curr_contact_row[contact_cols.phone] ==
                            entry_phone->get_text())
                            return;
                        curr_contact_row[contact_cols.phone] =
                            entry_phone->get_text();
                        contacts_save_enable();
                    });
                entry_phone->set_hexpand(true);
                grid->attach(*entry_phone, 3, 1, 1, 1);

                treeview->get_selection()->signal_changed().connect([&]() {
                    Gtk::TreeModel::Row curr_contact_row =
                        *(treeview->get_selection()->get_selected());
                    if (!curr_contact_row)
                        return;
                    entry_name->set_text(curr_contact_row[contact_cols.name]);
                    entry_description->set_text(
                        curr_contact_row[contact_cols.comment]);
                    entry_email->set_text(curr_contact_row[contact_cols.email]);
                    entry_phone->set_text(curr_contact_row[contact_cols.phone]);
                    bool is_group = curr_contact_row[contact_cols.is_group];
                    entry_phone->set_sensitive(!is_group);
                    entry_email->set_sensitive(!is_group);
                });
                button_contact_add->signal_clicked().connect(
                    [this, treeview, contacts_save_enable]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        Gtk::TreeModel::Row group_row;
                        if (curr_contact_row[contact_cols.is_group])
                            group_row = curr_contact_row;
                        else
                            group_row = *curr_contact_row.parent();
                        Gtk::TreeModel::Row new_contact_row =
                            *treestore_contacts->append(group_row.children());
                        new_contact_row[contact_cols.icon] = pixbuf_contact;
                        new_contact_row[contact_cols.is_group] = false;
                        new_contact_row[contact_cols.name] = "Контакт";
                        new_contact_row[contact_cols.comment] = Glib::ustring();
                        new_contact_row[contact_cols.email] = Glib::ustring();
                        new_contact_row[contact_cols.phone] = Glib::ustring();
                        contacts_save_enable();
                    });
                button_contact_group_add->signal_clicked().connect(
                    [this, treeview, contacts_save_enable]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        Gtk::TreeModel::Row new_group_row;
                        if (curr_contact_row[contact_cols.is_group])
                            new_group_row = *treestore_contacts->append(
                                curr_contact_row.children());
                        else
                            new_group_row = *treestore_contacts->append(
                                curr_contact_row.parent()->children());
                        new_group_row[contact_cols.icon] = pixbuf_contact_group;
                        new_group_row[contact_cols.is_group] = true;
                        new_group_row[contact_cols.name] = "Группа контактов";
                        new_group_row[contact_cols.comment] = Glib::ustring();
                        new_group_row[contact_cols.email] = Glib::ustring();
                        new_group_row[contact_cols.phone] = Glib::ustring();
                        contacts_save_enable();
                    });
                button_contact_delete->signal_clicked().connect(
                    [this, treeview, contacts_save_enable]() {
                        Gtk::TreeModel::Row curr_contact_row =
                            *(treeview->get_selection()->get_selected());
                        if (!curr_contact_row)
                            return;
                        Glib::ustring name =
                            curr_contact_row[contact_cols.name];
                        Gtk::MessageDialog dialog(
                            *this,
                            curr_contact_row[contact_cols.is_group]
                                ? "Удалить группу \"" + name + "\"?"
                                : "Удалить контакт \"" + name + "\"?",
                            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                        dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                        dialog.add_button("ОК", Gtk::RESPONSE_OK);
                        dialog.show_all();

                        int res = dialog.run();
                        if (res != Gtk::RESPONSE_OK)
                            return;
                        treestore_contacts->erase(curr_contact_row);
                        contacts_save_enable();
                    });

                set_margin(*topbar, 10, 2);
                set_margin(*treeview, 10, 10);
                set_margin(*grid, 10, 2);
                dialog.show_all();

                int res = dialog.run();
                if (res != Gtk::RESPONSE_OK)
                    return;
                save_contacts();
            });

            treeview_station.get_selection()->signal_changed().connect(
                [this]() { on_group_selection_changed(); });
            grip1.signal_button_press_event().connect(
                [this](GdkEventButton *event) -> bool {
                    if (event->button == 1) {
                        drag_start_x = event->x_root;
                        current_grip = 1;
                        return true;
                    }
                    return false;
                });
            grip1.signal_motion_notify_event().connect(
                [this](GdkEventMotion *event) -> bool {
                    if (current_grip == 1 &&
                        (event->state & GDK_BUTTON1_MASK)) {
                        int delta = event->x_root - drag_start_x;
                        int station_w = scrolled_station.get_allocated_width();
                        int form_w = scrolled_form.get_allocated_width();
                        scrolled_station.set_size_request(station_w + delta,
                                                          -1);
                        scrolled_form.set_size_request(form_w - delta, -1);
                        drag_start_x = event->x_root;
                    }
                    return true;
                });
            grip1.signal_enter_notify_event().connect(
                [this](GdkEventCrossing *) -> bool {
                    Glib::RefPtr<Gdk::Cursor> resize_cursor =
                        Gdk::Cursor::create(Gdk::SB_H_DOUBLE_ARROW);
                    if (grip1.get_window()) {
                        grip1.get_window()->set_cursor(resize_cursor);
                    }
                    return false;
                });
            grip1.signal_leave_notify_event().connect(
                [this](GdkEventCrossing *) -> bool {
                    if (grip1.get_window()) {
                        grip1.get_window()->set_cursor();
                    }
                    return false;
                });
            grip1.signal_draw().connect(
                [this](const Cairo::RefPtr<Cairo::Context> &cr) {
                    Gtk::Allocation alloc = grip1.get_allocation();
                    int width = alloc.get_width();
                    int height = alloc.get_height();
                    int line_x = width / 2;
                    cr->set_source_rgb(0.83, 0.81, 0.8);
                    cr->rectangle(line_x, 0, 1, height);
                    cr->fill();
                    return true;
                });
            grip2.signal_button_press_event().connect(
                [this](GdkEventButton *event) -> bool {
                    if (event->button == 1) {
                        drag_start_x = event->x_root;
                        current_grip = 2;
                        return true;
                    }
                    return false;
                });
            grip2.signal_motion_notify_event().connect(
                [this](GdkEventMotion *event) -> bool {
                    if (current_grip == 2 &&
                        (event->state & GDK_BUTTON1_MASK)) {
                        int delta = event->x_root - drag_start_x;
                        int form_w = scrolled_form.get_allocated_width();
                        int passports_w = scrolled_zones.get_allocated_width();
                        scrolled_form.set_size_request(form_w + delta, -1);
                        scrolled_zones.set_size_request(passports_w - delta,
                                                        -1);
                        drag_start_x = event->x_root;
                    }
                    return true;
                });
            grip2.signal_enter_notify_event().connect(
                [this](GdkEventCrossing *) -> bool {
                    Glib::RefPtr<Gdk::Cursor> resize_cursor =
                        Gdk::Cursor::create(Gdk::SB_H_DOUBLE_ARROW);
                    if (grip2.get_window()) {
                        grip2.get_window()->set_cursor(resize_cursor);
                    }
                    return false;
                });
            grip2.signal_leave_notify_event().connect(
                [this](GdkEventCrossing *) -> bool {
                    if (grip2.get_window()) {
                        grip2.get_window()->set_cursor();
                    }
                    return false;
                });
            grip2.signal_draw().connect(
                [this](const Cairo::RefPtr<Cairo::Context> &cr) {
                    Gtk::Allocation alloc = grip2.get_allocation();
                    int width = alloc.get_width();
                    int height = alloc.get_height();
                    int line_x = width / 2;
                    cr->set_source_rgb(0.83, 0.81, 0.8);
                    cr->rectangle(line_x, 0, 1, height);
                    cr->fill();
                    return true;
                });
            grid_main.signal_button_release_event().connect(
                [this](GdkEventButton *) {
                    drag_grip = nullptr;
                    return false;
                });

            radiobutton_settings_menu.signal_toggled().connect(
                sigc::mem_fun(*this, &AlarmConfigurator::redraw_settings_menu));
            radiobutton_passports_menu.signal_toggled().connect(sigc::mem_fun(
                *this, &AlarmConfigurator::redraw_passports_menu));
            radiobutton_general_settings.signal_toggled().connect(sigc::mem_fun(
                *this, &AlarmConfigurator::redraw_general_settings_menu));
            radiobutton_alarms.signal_toggled().connect(
                sigc::mem_fun(*this, &AlarmConfigurator::redraw_alarms_menu));

            checkbutton_use_own_settings.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.use_own_settings] ==
                    checkbutton_use_own_settings.get_active())
                    return;
                hbox_settings_buttons_bar.set_visible(
                    checkbutton_use_own_settings.get_active());
                stack_general_settings_alarms.set_visible(
                    checkbutton_use_own_settings.get_active());
                stored_curr_group_row[group_cols.icon] =
                    checkbutton_use_own_settings.get_active()
                        ? pixbuf_group
                        : pixbuf_group_inherited;
                stored_curr_group_row[group_cols.use_own_settings] =
                    checkbutton_use_own_settings.get_active();
                button_save_enable();
            });
            checkbutton_disable_alarm.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.disable_alarm] ==
                    checkbutton_disable_alarm.get_active())
                    return;
                stored_curr_group_row[group_cols.disable_alarm] =
                    checkbutton_disable_alarm.get_active();
                button_save_enable();
            });
            checkbutton_checkback_on_startup.signal_clicked().connect([this]() {
                if (stored_curr_group_row[group_cols.checkback_on_startup] ==
                    checkbutton_checkback_on_startup.get_active())
                    return;
                stored_curr_group_row[group_cols.checkback_on_startup] =
                    checkbutton_checkback_on_startup.get_active();
                button_save_enable();
            });
            checkbutton_use_own_sound_in_group.signal_clicked().connect(
                [this]() {
                    if (stored_curr_group_row[group_cols
                                                  .use_own_sound_in_group] ==
                        checkbutton_use_own_sound_in_group.get_active())
                        return;
                    stored_curr_group_row[group_cols.use_own_sound_in_group] =
                        checkbutton_use_own_sound_in_group.get_active();
                    button_save_enable();
                });
            checkbutton_borders_from_passport.signal_clicked().connect(
                [this]() {
                    hbox_crashprecrash_bounds.set_sensitive(
                        !checkbutton_borders_from_passport.get_active());
                    if (stored_curr_group_row[group_cols
                                                  .borders_from_passport] ==
                        checkbutton_borders_from_passport.get_active())
                        return;
                    stored_curr_group_row[group_cols.borders_from_passport] =
                        checkbutton_borders_from_passport.get_active();
                });
            checkbutton_block_by_passport.signal_clicked().connect([this]() {
                if (checkbutton_block_by_passport.get_active() ==
                    (stored_curr_group_row[group_cols.block_by_passport_val] !=
                     0))
                    return;
                if (checkbutton_block_by_passport.get_active())
                    stack_aps_lowlevel_interaction_left.set_visible_child(
                        frame_value);
                else
                    stored_curr_group_row[group_cols.block_by_passport_val] = 0;
                curr_aps_menu_idx = 0;
                redraw_aps();
                button_save_enable();
            });
            radiobutton0.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.block_by_passport_val] ==
                    1)
                    return;
                if (!radiobutton0.get_active())
                    return;
                stored_curr_group_row[group_cols.block_by_passport_val] = 1;
                button_save_enable();
            });
            radiobutton1.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.block_by_passport_val] ==
                    2)
                    return;
                if (!radiobutton1.get_active())
                    return;
                stored_curr_group_row[group_cols.block_by_passport_val] = 2;
                button_save_enable();
            });
            checkbutton_checkback.signal_toggled().connect([this]() {
                if (checkbutton_checkback.get_active() ==
                    (stored_curr_group_row[group_cols.checkback_val] != 0))
                    return;
                if (checkbutton_checkback.get_active())
                    stack_aps_lowlevel_interaction_left.set_visible_child(
                        frame_checkback);
                else
                    stored_curr_group_row[group_cols.checkback_val] = 0;
                curr_aps_menu_idx = 1;
                redraw_aps();
                button_save_enable();
            });
            radiobutton_checkback_edge.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.checkback_val] == 1)
                    return;
                if (!radiobutton_checkback_edge.get_active())
                    return;
                stored_curr_group_row[group_cols.checkback_val] = 1;
                button_save_enable();
            });
            radiobutton_checkback_cutoff.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.checkback_val] == 2)
                    return;
                if (!radiobutton_checkback_cutoff.get_active())
                    return;
                stored_curr_group_row[group_cols.checkback_val] = 2;
                button_save_enable();
            });
            checkbutton_write_on_checkback.signal_toggled().connect([this]() {
                if (stored_curr_group_row[group_cols.write_on_checkback] ==
                    (checkbutton_write_on_checkback.get_active()))
                    return;
                stored_curr_group_row[group_cols.write_on_checkback] =
                    checkbutton_write_on_checkback.get_active();
                curr_aps_menu_idx = 2;
                redraw_aps();
                button_save_enable();
            });
            radiobutton_zones_chipher.signal_toggled().connect([this]() {
                if (radiobutton_zones_chipher.get_active())
                    change_zone_params_view(treestore_zones, alarmobj_cols, 1);
            });
            radiobutton_zones_name.signal_toggled().connect([this]() {
                if (radiobutton_zones_name.get_active())
                    change_zone_params_view(treestore_zones, alarmobj_cols, 0);
            });

            button_situation_new_expand.signal_clicked().connect([this]() {
                auto popover = Gtk::make_managed<Gtk::Popover>();
                popover->set_relative_to(button_situation_new_expand);
                popover->set_position(Gtk::POS_BOTTOM);
                Gtk::Box *vbox =
                    Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
                Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                if (!curr_group_row)
                    return;

                auto new_situation_button = [this, popover, vbox,
                                             curr_group_row](
                                                unsigned int situation_code,
                                                std::string name,
                                                unsigned char alarm_kind,
                                                std::string alarm_message,
                                                std::vector<unsigned char>
                                                    params_types,
                                                std::vector<float> params_vals,
                                                bool use_hysteresis) {
                    Gtk::Button *button_alarm =
                        Gtk::make_managed<Gtk::Button>(name);
                    vbox->pack_start(*button_alarm, Gtk::PACK_SHRINK);
                    button_alarm->signal_clicked().connect([this, popover,
                                                            situation_code,
                                                            name, alarm_kind,
                                                            alarm_message,
                                                            params_types,
                                                            params_vals,
                                                            curr_group_row,
                                                            use_hysteresis]() {
                        Glib::RefPtr<Gtk::ListStore> liststore_stations =
                            Glib::RefPtr<Gtk::ListStore>::cast_static(
                                treeview_situations.get_model());
                        if (!liststore_stations)
                            return;
                        std::array<bool, 4> unique_situations_used =
                            curr_group_row[group_cols.unique_situations_used];
                        if (1 <= situation_code && situation_code <= 4) {
                            unique_situations_used[situation_code - 1] = true;
                        }
                        curr_group_row[group_cols.unique_situations_used] =
                            unique_situations_used;

                        Gtk::TreeModel::Row new_row =
                            *liststore_stations->append();
                        new_row[situation_cols.is_enabled] = true;
                        new_row[situation_cols.situation_code] = situation_code;
                        new_row[situation_cols.name] = name;
                        new_row[situation_cols.alarm_kind] = alarm_kind;
                        new_row[situation_cols.alarm_message] = alarm_message;
                        new_row[situation_cols.usergroup_name] =
                            "--группа не указана--";
                        new_row[situation_cols.params_types] = params_types;
                        new_row[situation_cols.params_vals] = params_vals;
                        new_row[situation_cols.sounds] =
                            Gtk::ListStore::create(sound_cols);
                        new_row[situation_cols.checkboxes] = {
                            false, false, use_hysteresis, false,
                            false, false, false};
                        std::array<SituationParams, 4> init_params;
                        for (SituationParams &p : init_params)
                            p.contacts = Gtk::ListStore::create(contact_cols);
                        new_row[situation_cols.params] = init_params;
                        new_row[situation_cols.intense_absolute] = false;
                        new_row[situation_cols.intense_up] = 0.0;
                        new_row[situation_cols.intense_down] = 0.0;

                        redraw_alarms_menu();
                        treeview_situations.get_selection()->select(new_row);
                        popover->popdown();
                        button_save_enable();
                    });
                    return button_alarm;
                };

                std::array<bool, 4> unique_situations_used =
                    curr_group_row[group_cols.unique_situations_used];
                std::vector<unsigned char> params_types_jumpfrom = {
                    SITUATIONPARAMTYPE_JUMPFROM};
                std::vector<float> params_vals_jumpfrom = {255.0};
                (void)new_situation_button(0, "Норма", 0, "Параметр %c в норме",
                                           params_types_jumpfrom,
                                           params_vals_jumpfrom, false);
                Gtk::Button *button_uniq_situation1 = new_situation_button(
                    1, "Верхняя аварийная уставка", 0,
                    "Превышена ВАУ параметром %c", params_types_jumpfrom,
                    params_vals_jumpfrom, true);
                if (unique_situations_used[0]) {
                    button_uniq_situation1->set_sensitive(true);
                }
                Gtk::Button *button_uniq_situation2 = new_situation_button(
                    2, "Верхняя предаварийная уставка", 1,
                    "Превышена ВПУ параметром %c", params_types_jumpfrom,
                    params_vals_jumpfrom, true);
                if (unique_situations_used[1]) {
                    button_uniq_situation2->set_sensitive(true);
                }
                Gtk::Button *button_uniq_situation3 = new_situation_button(
                    3, "Нижняя аварийная уставка", 0,
                    "Превышена НАУ параметром %c", params_types_jumpfrom,
                    params_vals_jumpfrom, true);
                if (unique_situations_used[2]) {
                    button_uniq_situation3->set_sensitive(true);
                }
                Gtk::Button *button_uniq_situation4 = new_situation_button(
                    4, "Нижняя предаварийная уставка", 1,
                    "Превышена НПУ параметром %c", params_types_jumpfrom,
                    params_vals_jumpfrom, true);
                if (unique_situations_used[3]) {
                    button_uniq_situation4->set_sensitive(true);
                }
                curr_group_row[group_cols.unique_situations_used] =
                    unique_situations_used;
                (void)new_situation_button(
                    5, "Меньше значения", 0,
                    "Значение параметра %c меньше допустимого",
                    {SITUATIONPARAMTYPE_LESSTHAN, SITUATIONPARAMTYPE_JUMPFROM},
                    {100.0, 255.0}, true);
                (void)new_situation_button(
                    6, "Больше значения", 0,
                    "Значение параметра %c больше допустимого",
                    {SITUATIONPARAMTYPE_GREATERTHAN,
                     SITUATIONPARAMTYPE_JUMPFROM},
                    {0.0, 255.0}, true);
                (void)new_situation_button(
                    7, "Значение в диапазоне", 0,
                    "Параметр %c находится в заданном диапазоне значений",
                    {SITUATIONPARAMTYPE_LESSTHAN,
                     SITUATIONPARAMTYPE_GREATERTHAN,
                     SITUATIONPARAMTYPE_JUMPFROM},
                    {100.0, 0.0, 255.0}, true);
                (void)new_situation_button(
                    8, "Дискретное значение", 0, "%n",
                    {SITUATIONPARAMTYPE_DISCRETE, SITUATIONPARAMTYPE_JUMPFROM},
                    {1.0, 255.0}, false);
                (void)new_situation_button(
                    9, "Ошибка", 0, "Ошибка параметра %c",
                    {SITUATIONPARAMTYPE_ERRORCODE, SITUATIONPARAMTYPE_JUMPFROM},
                    {255.0, 255.0}, false);
                (void)new_situation_button(
                    10, "Переход", 2, "Параметр %c изменил значение с %p на %v",
                    {SITUATIONPARAMTYPE_JUMPFROM}, {255.0}, true);

                vbox->show_all();
                popover->add(*vbox);
                popover->popup();
            });
            button_situation_delete.signal_clicked().connect([this]() {
                Gtk::TreeModel::Row curr_group_row = get_curr_group_row();
                if (!curr_group_row)
                    return;
                Gtk::TreeModel::Row curr_situation_row =
                    *treeview_situations.get_selection()->get_selected();
                if (!curr_situation_row)
                    return;
                Glib::RefPtr<Gtk::ListStore> liststore_stations =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_situations.get_model());
                if (!liststore_stations)
                    return;
                int situation_code =
                    curr_situation_row[situation_cols.situation_code];
                if (1 <= situation_code && situation_code <= 4) {
                    std::array<bool, 4> unique_situations_used =
                        curr_group_row[group_cols.unique_situations_used];
                    unique_situations_used[situation_code - 1] = false;
                    curr_group_row[group_cols.unique_situations_used] =
                        unique_situations_used;
                }
                liststore_stations->erase(curr_situation_row);
                button_save_enable();
            });
            button_situation_up.signal_clicked().connect([this]() {
                Gtk::TreeModel::iterator curr_iter =
                    treeview_situations.get_selection()->get_selected();
                if (!curr_iter)
                    return;
                Glib::RefPtr<Gtk::ListStore> store =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_situations.get_model());
                if (!store)
                    return;

                Gtk::TreeModel::Path curr_path = store->get_path(curr_iter);
                if (curr_path.empty() || curr_path[0] <= 0)
                    return;
                Gtk::TreeModel::Path prev_path(
                    std::to_string((int)curr_path[0] - 1));
                Gtk::TreeModel::iterator prev_iter = store->get_iter(prev_path);
                if (!prev_iter)
                    return;
                Gtk::TreeModel::Row curr_row = *curr_iter;
                Gtk::TreeModel::Row prev_row = *prev_iter;
                swap_situation_rows(curr_row, prev_row);
                treeview_situations.get_selection()->select(prev_iter);
                button_save_enable();
            });
            button_situation_down.signal_clicked().connect([this]() {
                Gtk::TreeModel::iterator curr_iter =
                    treeview_situations.get_selection()->get_selected();
                if (!curr_iter)
                    return;
                Glib::RefPtr<Gtk::ListStore> store =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_situations.get_model());
                if (!store)
                    return;

                Gtk::TreeModel::Path curr_path = store->get_path(curr_iter);
                if (curr_path.empty())
                    return;
                Gtk::TreeModel::Path next_path(
                    std::to_string((int)curr_path[0] + 1));
                auto next_iter = store->get_iter(next_path);
                if (!next_iter)
                    return;
                Gtk::TreeModel::Row curr_row = *curr_iter;
                Gtk::TreeModel::Row next_row = *next_iter;
                swap_situation_rows(curr_row, next_row);
                treeview_situations.get_selection()->select(next_iter);
                button_save_enable();
            });

            treeview_situations.get_selection()->signal_changed().connect(
                sigc::mem_fun(*this, &AlarmConfigurator::redraw_alarms_form));
            treeview_sounds.get_selection()->signal_changed().connect([this]() {
                Gtk::TreeModel::Row curr_sound_row =
                    *treeview_sounds.get_selection()->get_selected();
                if (!curr_sound_row) {
                    button_sound_delete.set_sensitive(false);
                    button_sound_up.set_sensitive(false);
                    button_sound_down.set_sensitive(false);
                    return;
                }
                Glib::RefPtr<Gtk::ListStore> liststore_sounds =
                    Glib::RefPtr<Gtk::ListStore>::cast_static(
                        treeview_sounds.get_model());
                if (!liststore_sounds)
                    return;
                if (sound_playing.load())
                    return;
                int amount = liststore_sounds->children().size();
                int idx = liststore_sounds->get_path(curr_sound_row)[0];
                if (amount == 1) {
                    button_sound_up.set_sensitive(false);
                    button_sound_down.set_sensitive(false);
                } else if (idx == 0) {
                    button_sound_up.set_sensitive(false);
                    button_sound_down.set_sensitive(true);
                } else if (idx == amount - 1) {
                    button_sound_down.set_sensitive(false);
                    button_sound_up.set_sensitive(true);
                } else {
                    button_sound_up.set_sensitive(true);
                    button_sound_down.set_sensitive(true);
                }
            });
            treeview_passports.get_selection()->signal_changed().connect(
                [this]() {
                    bool has_selection = !treeview_passports.get_selection()
                                              ->get_selected_rows()
                                              .empty();
                    button_delete.set_sensitive(has_selection);
                    menuitem_delete.set_sensitive(has_selection);
                });
            treeview_zones.signal_row_activated().connect(
                [this](const Gtk::TreeModel::Path &path,
                       const Gtk::TreeViewColumn *column) {
                    (void)column;
                    Gtk::TreeModel::Row curr_zone_row =
                        *treestore_zones->get_iter(path);
                    if (curr_zone_row[alarmobj_cols.alarmobj_type] !=
                        ALARMOBJTYPE_KLOGICPARAM)
                        return;
                    Glib::ustring user_path_str =
                        curr_zone_row[alarmobj_cols.userpath];
                    if (user_path_str.empty())
                        return;
                    Gtk::TreeModel::Path user_path(user_path_str);
                    Gtk::TreeModel::Row user_row =
                        *treestore_groups->get_iter(user_path);
                    if (!user_row)
                        return;
                    int dst_station_id = (*user_row)[group_cols.station_id];
                    if (dst_station_id != curr_station_id) {
                        if (unsaved) {
                            Gtk::MessageDialog dialog(
                                "Сохранить изменения?", false,
                                Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                            dialog.add_button("Да", Gtk::RESPONSE_YES);
                            dialog.add_button("Нет", Gtk::RESPONSE_NO);
                            dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);

                            int res = dialog.run();
                            if (res == Gtk::RESPONSE_YES) {
                                on_save_clicked();
                            } else if (res == Gtk::RESPONSE_CANCEL) {
                                return;
                            } else {
                                button_save_disable();
                                Gtk::TreeModel::Row curr_station_row;
                                for (const Gtk::TreeModel::Row &row :
                                     treestore_groups->children()) {
                                    int station_id = row[group_cols.station_id];
                                    if (station_id == curr_station_id) {
                                        curr_station_row = row;
                                        break;
                                    }
                                }
                                if (!curr_station_row)
                                    return;
                                while (!curr_station_row->children().empty()) {
                                    Gtk::TreeModel::iterator child =
                                        curr_station_row->children().begin();
                                    treestore_groups->erase(child);
                                }
                                std::string errors;
                                std::unordered_map<std::string, AlarmRef>
                                    used_params;
                                (void)parse_alarms_config(
                                    alarms_dir_path, curr_station_row,
                                    used_params, treestore_groups, pixbufs_loop,
                                    group_cols, alarmobj_cols, situation_cols,
                                    sound_cols, contact_cols, errors);

                                fastparam::Machins *m =
                                    fastparam::machins_get();
                                if (!m)
                                    return;
                                unsigned char ma_id = static_cast<
                                    unsigned char>(
                                    curr_station_row[group_cols.station_id]);
                                fastparam::Controllers *controllers =
                                    fastparam::machins_get_controllers(m,
                                                                       ma_id);
                                if (!controllers)
                                    return;
                                int n =
                                    fastparam::controllers_count(controllers);
                                Gtk::TreeModel::Row new_station_row;
                                for (const Gtk::TreeModel::Row &row :
                                     treestore_zones->children()) {
                                    if (row[alarmobj_cols.chipher] ==
                                        curr_station_row[group_cols.confname])
                                        new_station_row = row;
                                }
                                if (!new_station_row)
                                    return;
                                while (!new_station_row->children().empty()) {
                                    Gtk::TreeModel::iterator child =
                                        new_station_row->children().begin();
                                    treestore_zones->erase(child);
                                }
                                Gtk::TreeModel::Children
                                    zones_station_children =
                                        new_station_row.children();
                                for (int i = 0; i < n; i++) {
                                    int cntrl_id =
                                        fastparam::controllers_id_by_index(
                                            controllers, i);
                                    if (cntrl_id < 0)
                                        return;
                                    fastparam::Cntrl *controller =
                                        fastparam::machins_get_cntrl(
                                            m, ma_id,
                                            static_cast<short unsigned int>(
                                                cntrl_id));
                                    if (!controller)
                                        return;
                                    std::string station_confname = static_cast<
                                        Glib::ustring>(
                                        curr_station_row[group_cols.confname]);
                                    std::filesystem::path
                                        controller_config_path =
                                            std::filesystem::path(
                                                klogic_dir_path) /
                                            station_confname / "Cfg" /
                                            fastparam::cntrl_config_guid_str(
                                                controller);
                                    (void)parse_controller_config(
                                        cntrl_id,
                                        controller_config_path.string() +
                                            ".xml",
                                        zones_station_children,
                                        curr_station_row, treestore_groups,
                                        used_params, treestore_zones,
                                        pixbuf_klogic, pixbuf_task, pixbuf_fb,
                                        pixbuf_service_params, pixbuf_discrete,
                                        pixbuf_analog, pixbufs_params,
                                        alarmobj_cols, group_cols, errors);
                                }
                                set_treestore_recursive(
                                    curr_station_row->children(), false,
                                    treestore_zones, alarmobj_cols);
                            }
                        }
                        curr_station_id = dst_station_id;
                    }
                    filter_groups->refilter();
                    treeview_station.expand_all();
                    Glib::RefPtr<Gtk::ListStore> liststore_alarms =
                        user_row[group_cols.alarms];
                    treeview_passports.set_model(liststore_alarms);
                    stack_form_menu.set_visible_child(vbox_passports_menu);
                    curr_menu_idx = 1;
                    Gtk::TreeModel::Row filter_user_row =
                        *filter_groups->convert_child_iter_to_iter(user_row);
                    treeview_station.get_selection()->select(filter_user_row);
                    treeview_passports.get_selection()->unselect_all();
                    for (const Gtk::TreeModel::Row &alarm_row :
                         liststore_alarms->children()) {
                        if ((alarm_row[alarmobj_cols.klogic_string_id] ==
                             curr_zone_row[alarmobj_cols.klogic_string_id])) {
                            treeview_passports.get_selection()->select(
                                alarm_row);
                            break;
                        }
                    }
                    radiobutton_passports_menu.set_active(true);
                });

            // Настройка dnd для паспортов
            std::vector<Gtk::TargetEntry> targets = {
                Gtk::TargetEntry("KLOGICPARAM_ROW", Gtk::TargetFlags(0), 0)};
            treeview_zones.enable_model_drag_source(
                targets, Gdk::BUTTON1_MASK, Gdk::DragAction::ACTION_COPY);
            treeview_passports.enable_model_drag_dest(
                targets, Gdk::DragAction::ACTION_COPY);
            treeview_zones.signal_drag_begin().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context) {
                    Gtk::TreeModel::Row row =
                        *treeview_zones.get_selection()->get_selected();
                    if (row[alarmobj_cols.alarmobj_type] !=
                            ALARMOBJTYPE_KLOGICPARAM ||
                        curr_menu_idx != 1) {
                        gtk_drag_set_icon_pixbuf(
                            context->gobj(), pixbuf_deprecated->gobj(), -4, -4);
                        return;
                    }
                });
            treeview_zones.signal_drag_data_get().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context,
                       const Gtk::SelectionData &selection_data, guint info,
                       guint time) {
                    (void)context;
                    (void)time;
                    if (info != 0)
                        return;
                    Gtk::TreeModel::Row row =
                        *treeview_zones.get_selection()->get_selected();
                    if (row[alarmobj_cols.alarmobj_type] ==
                        ALARMOBJTYPE_KLOGICPARAM) {
                        Gtk::TreePath path =
                            treeview_zones.get_model()->get_path(row);
                        Gtk::SelectionData &sel_data =
                            const_cast<Gtk::SelectionData &>(selection_data);
                        Glib::ustring path_str = path.to_string();
                        sel_data.set("KLOGICPARAM_ROW", path_str);
                    }
                });
            treeview_passports.signal_drag_data_received().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context, int x,
                       int y, const Gtk::SelectionData &selection_data,
                       guint info, guint time) {
                    (void)x;
                    (void)y;
                    (void)info;
                    if (!context->get_selected_action()) {
                        context->drag_status(Gdk::DragAction::ACTION_COPY,
                                             time);
                        return;
                    }
                    if (selection_data.get_length() == 0 ||
                        selection_data.get_format() != 8)
                        return;
                    Glib::ustring path_str(reinterpret_cast<const char *>(
                                               selection_data.get_data()),
                                           selection_data.get_length());
                    Gtk::TreeModel::Path source_path(path_str);
                    Gtk::TreeModel::Row source_row =
                        *treeview_zones.get_model()->get_iter(source_path);
                    if (!source_row)
                        return;
                    Glib::ustring source_path_str =
                        source_row[alarmobj_cols.userpath];
                    Gtk::TreeModel::Row user_row;
                    if (!source_path_str.empty()) {
                        Gtk::TreeModel::Path user_path(source_path_str);
                        user_row = *treestore_groups->get_iter(user_path);
                    }
                    Gtk::TreeModel::Row new_user_row = get_curr_group_row();
                    if (user_row && *user_row == *new_user_row)
                        return;
                    if (user_row) {
                        Gtk::MessageDialog dialog(
                            "Один или несколько алармов ссылаются на "
                            "указанные паспорта.\nЗаменить при "
                            "возникновении конфликтов старые алармы вновь "
                            "создаваемыми?",
                            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
                        dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);
                        dialog.add_button("ОК", Gtk::RESPONSE_YES);
                        dialog.show_all();
                        int res = dialog.run();
                        if (res != Gtk::RESPONSE_YES)
                            return;
                        Glib::RefPtr<Gtk::ListStore> liststore_user_alarms =
                            user_row[group_cols.alarms];
                        if (!liststore_user_alarms)
                            return;
                        Glib::ustring source_string_id =
                            source_row[alarmobj_cols.klogic_string_id];
                        for (const Gtk::TreeModel::Row &row :
                             liststore_user_alarms->children()) {
                            if (static_cast<Glib::ustring>(
                                    row[alarmobj_cols.klogic_string_id]) ==
                                source_string_id) {
                                liststore_user_alarms->erase(row);
                                break;
                            }
                        }
                    }

                    Glib::RefPtr<Gtk::ListStore> liststore_passports =
                        Glib::RefPtr<Gtk::ListStore>::cast_static(
                            treeview_passports.get_model());
                    if (!liststore_passports)
                        return;
                    Gtk::TreeModel::Row new_row =
                        *liststore_passports->append();
                    new_row[alarmobj_cols.alarmobj_type] =
                        ALARMOBJTYPE_GROUPALARM;
                    unsigned char type = source_row[alarmobj_cols.type];
                    new_row[alarmobj_cols.icon] = type == KLOGICTYPE_BOOL
                                                      ? pixbuf_discrete
                                                      : pixbuf_analog;
                    new_row[alarmobj_cols.passport_priority] =
                        is_station() ? 3 : 2;
                    new_row[alarmobj_cols.station_name] =
                        static_cast<Glib::ustring>(
                            source_row[alarmobj_cols.station_name]);
                    new_row[alarmobj_cols.passport_id] =
                        static_cast<short unsigned int>(
                            source_row[alarmobj_cols.passport_id]);
                    new_row[alarmobj_cols.fullname] =
                        static_cast<Glib::ustring>(
                            source_row[alarmobj_cols.fullname]);
                    new_row[alarmobj_cols.passport_val_type_str] =
                        static_cast<Glib::ustring>(
                            source_row[alarmobj_cols.passport_val_type_str]);
                    new_row[alarmobj_cols.name] = static_cast<Glib::ustring>(
                        source_row[alarmobj_cols.chipher]);
                    new_row[alarmobj_cols.passport_group] =
                        static_cast<Glib::ustring>(
                            source_row[alarmobj_cols.passport_group]);
                    new_row[alarmobj_cols.klogic_string_id] =
                        static_cast<Glib::ustring>(
                            source_row[alarmobj_cols.klogic_string_id]);
                    source_row[alarmobj_cols.userpath] =
                        treestore_groups->get_path(new_user_row).to_string();

                    button_save_enable();
                    context->drag_finish(true, false, time);
                });
            treeview_passports.get_selection()->set_mode(
                Gtk::SELECTION_MULTIPLE);
            treeview_passports.signal_drag_drop().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context, int x,
                       int y, guint time) -> bool {
                    (void)x;
                    (void)y;
                    Gtk::TreePath path;
                    treeview_passports.drag_get_data(context, "KLOGICPARAM_ROW",
                                                     time);
                    return true;
                });
        }

        void setup_accel_groups() {
            auto accel_group = Gtk::AccelGroup::create();
            add_accel_group(accel_group);
            button_save.add_accelerator("clicked", accel_group, GDK_KEY_s,
                                        Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
        }

        void setup_data() {
            treestore_groups = Gtk::TreeStore::create(group_cols);
            treestore_zones = Gtk::TreeStore::create(alarmobj_cols);
            liststore_passports_params = Gtk::ListStore::create(alarmobj_cols);
            treestore_contacts = Gtk::TreeStore::create(contact_cols);
            std::string errors;
            (void)parse_stations_config(stations_config_path, main_settings,
                                        treestore_groups, group_cols, errors);
            if (!errors.empty()) {
                Gtk::MessageDialog dialog(
                    "Не удалось прочитать файл станций\n\n" + errors, false,
                    Gtk::MESSAGE_ERROR);
                dialog.run();
            }
            errors.clear();
            std::unordered_map<std::string, AlarmRef> used_params;
            (void)parse_alarms_configs(
                alarms_dir_path, used_params, treestore_groups, pixbufs_loop,
                group_cols, alarmobj_cols, situation_cols, sound_cols,
                contact_cols, errors);
            if (!errors.empty()) {
                Gtk::MessageDialog dialog(
                    "Не удалось прочитать файлы алармов\n\n" + errors, false,
                    Gtk::MESSAGE_ERROR);
                dialog.run();
            }
            errors.clear();
            (void)parse_stations_aps_settings(alarms_dir_path, main_settings,
                                              treestore_groups, group_cols,
                                              errors);
            Gtk::TreeModel::Row klogic_row = *treestore_zones->append();
            klogic_row[alarmobj_cols.icon] = pixbuf_klogic;
            klogic_row[alarmobj_cols.name] = "KLogic";
            errors.clear();
            (void)parse_klogic_configs(
                alarms_dir_path, treestore_groups, used_params, treestore_zones,
                pixbuf_station, pixbuf_klogic, pixbuf_task, pixbuf_fb,
                pixbuf_service_params, pixbuf_discrete, pixbuf_analog,
                pixbufs_params, alarmobj_cols, group_cols, errors);
            set_treestore_recursive(treestore_groups->children(), true,
                                    treestore_zones, alarmobj_cols);

            curr_station_id = main_settings.local_station_id > 0
                                  ? main_settings.local_station_id
                                  : 1;
            filter_combobox_station =
                Gtk::TreeModelFilter::create(treestore_groups);
            filter_combobox_station->set_visible_func(
                [this](const Gtk::TreeModel::const_iterator &iter) {
                    return !iter->parent();
                });
            combobox_station.set_model(filter_combobox_station);
            combobox_station.set_active(curr_station_id - 1);
            combobox_station.pack_start(renderer_name);
            combobox_station.add_attribute(renderer_name.property_text(),
                                           group_cols.name);
            combobox_station.signal_changed().connect([this]() {
                if (unsaved) {
                    Gtk::MessageDialog dialog("Сохранить изменения?", false,
                                              Gtk::MESSAGE_QUESTION,
                                              Gtk::BUTTONS_NONE);
                    dialog.add_button("Да", Gtk::RESPONSE_YES);
                    dialog.add_button("Нет", Gtk::RESPONSE_NO);
                    dialog.add_button("Отмена", Gtk::RESPONSE_CANCEL);

                    int res = dialog.run();
                    if (res == Gtk::RESPONSE_YES) {
                        on_save_clicked();
                    } else if (res == Gtk::RESPONSE_CANCEL) {
                        return;
                    } else {
                        button_save_disable();
                        Gtk::TreeModel::Row curr_station_row;
                        for (const Gtk::TreeModel::Row &row :
                             treestore_groups->children()) {
                            int station_id = row[group_cols.station_id];
                            if (station_id == curr_station_id) {
                                curr_station_row = row;
                                break;
                            }
                        }
                        if (!curr_station_row)
                            return;
                        while (!curr_station_row->children().empty()) {
                            Gtk::TreeModel::iterator child =
                                curr_station_row->children().begin();
                            treestore_groups->erase(child);
                        }
                        std::string errors;
                        std::unordered_map<std::string, AlarmRef> used_params;
                        (void)parse_alarms_config(
                            alarms_dir_path, curr_station_row, used_params,
                            treestore_groups, pixbufs_loop, group_cols,
                            alarmobj_cols, situation_cols, sound_cols,
                            contact_cols, errors);

                        fastparam::Machins *m = fastparam::machins_get();
                        if (!m)
                            return;
                        unsigned char ma_id = static_cast<unsigned char>(
                            curr_station_row[group_cols.station_id]);
                        fastparam::Controllers *controllers =
                            fastparam::machins_get_controllers(m, ma_id);
                        if (!controllers)
                            return;
                        int n = fastparam::controllers_count(controllers);
                        Gtk::TreeModel::Row new_station_row;
                        for (const Gtk::TreeModel::Row &row :
                             treestore_zones->children()) {
                            if (row[alarmobj_cols.chipher] ==
                                curr_station_row[group_cols.confname])
                                new_station_row = row;
                        }
                        if (!new_station_row)
                            return;
                        while (!new_station_row->children().empty()) {
                            Gtk::TreeModel::iterator child =
                                new_station_row->children().begin();
                            treestore_zones->erase(child);
                        }
                        Gtk::TreeModel::Children zones_station_children =
                            new_station_row.children();
                        for (int i = 0; i < n; i++) {
                            int cntrl_id = fastparam::controllers_id_by_index(
                                controllers, i);
                            if (cntrl_id < 0)
                                return;
                            fastparam::Cntrl *controller =
                                fastparam::machins_get_cntrl(
                                    m, ma_id,
                                    static_cast<short unsigned int>(cntrl_id));
                            if (!controller)
                                return;
                            std::string station_confname =
                                static_cast<Glib::ustring>(
                                    curr_station_row[group_cols.confname]);
                            std::filesystem::path controller_config_path =
                                std::filesystem::path(klogic_dir_path) /
                                station_confname / "Cfg" /
                                fastparam::cntrl_config_guid_str(controller);
                            (void)parse_controller_config(
                                cntrl_id,
                                controller_config_path.string() + ".xml",
                                zones_station_children, curr_station_row,
                                treestore_groups, used_params, treestore_zones,
                                pixbuf_klogic, pixbuf_task, pixbuf_fb,
                                pixbuf_service_params, pixbuf_discrete,
                                pixbuf_analog, pixbufs_params, alarmobj_cols,
                                group_cols, errors);
                        }
                        set_treestore_recursive(curr_station_row->children(),
                                                false, treestore_zones,
                                                alarmobj_cols);
                    }
                }
                Gtk::TreeModel::iterator filter_iter =
                    combobox_station.get_active();
                if (!filter_iter)
                    return;
                Gtk::TreePath path =
                    filter_combobox_station->get_path(filter_iter);
                Gtk::TreeModel::iterator treestore_iter =
                    treestore_groups->get_iter(path);
                if (!treestore_iter)
                    return;
                curr_station_id = (*treestore_iter)[group_cols.station_id];
                filter_groups->refilter();
                treeview_station.get_selection()->select(
                    filter_groups->children().begin());
            });

            filter_groups = Gtk::TreeModelFilter::create(treestore_groups);
            filter_groups->set_visible_func(
                [this](const Gtk::TreeModel::const_iterator &iter) {
                    return (*iter)[group_cols.station_id] == curr_station_id ||
                           get_station_id_of_group(*iter) == curr_station_id;
                });
            treeview_station.set_model(filter_groups);
            treeview_station.expand_all();
            treeview_station.set_headers_visible(false);
            treeview_station.set_hexpand(true);
            treeview_station.set_vexpand(true);
            treecolumn_icon.pack_start(renderer_icon, false);
            treecolumn_icon.add_attribute(renderer_icon.property_pixbuf(),
                                          group_cols.icon);
            treeview_station.append_column(treecolumn_icon);
            treecolumn_name.pack_start(renderer_name, true);
            treecolumn_name.add_attribute(renderer_name.property_text(),
                                          group_cols.name);
            treeview_station.append_column(treecolumn_name);

            treeview_zones.set_model(treestore_zones);
            treeview_zones.set_headers_visible(false);
            treeview_zones.set_hexpand(true);
            treeview_zones.set_vexpand(true);
            treeview_zones.append_column("", alarmobj_cols.icon);
            treeview_zones.append_column(treecolumn_zones_name);
            treecolumn_zones_name.pack_start(renderer_zones_name, false);
            treecolumn_zones_name.add_attribute(
                renderer_zones_name.property_text(), alarmobj_cols.name);
            treecolumn_zones_name.set_cell_data_func(
                renderer_zones_name,
                sigc::mem_fun(*this, &AlarmConfigurator::on_zones_cell_data));
            treeview_station.get_selection()->select(
                filter_groups->children().begin());
        }

        Gtk::HeaderBar headerbar;
        Gtk::Box topbar{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box vbox_main{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Separator topbar_separator1{Gtk::ORIENTATION_HORIZONTAL},
            topbar_separator2{Gtk::ORIENTATION_HORIZONTAL},
            topbar_separator3{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::ComboBox combobox_station;
        Gtk::Box hbox_main{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Frame frame_station, frame_zones;
        Gtk::Box vbox_zones{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box hbox_zones_buttons_bar{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box hbox_form_buttons_bar{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Frame frame_stack_form_menu;
        Gtk::Box vbox_settings_menu{Gtk::ORIENTATION_VERTICAL, 10},
            vbox_passports_menu{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box hbox_group_name{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box hbox_settings_buttons_bar{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box vbox_general_settings{Gtk::ORIENTATION_VERTICAL, 10},
            hbox_alarms{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box hbox_general_settings_checkbuttons_row{
            Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box hbox_prioritysetpoints{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box vbox_priority{Gtk::ORIENTATION_VERTICAL, 5},
            vbox_setpoints{Gtk::ORIENTATION_VERTICAL, 5},
            hbox_aps_lowlevel_interaction{Gtk::ORIENTATION_HORIZONTAL, 5},
            vbox_aps_lowlevel_interaction_left{Gtk::ORIENTATION_VERTICAL, 5},
            vbox_aps_lowlevel_interaction_right{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::Box hbox_crashprecrash_bounds{Gtk::ORIENTATION_HORIZONTAL, 10};
        Gtk::Box vbox_alarms_treeview{Gtk::ORIENTATION_VERTICAL},
            vbox_alarms_form{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Box hbox_alarms_treeview_buttons_var{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Separator alarms_bar_separator1{Gtk::ORIENTATION_HORIZONTAL},
            alarms_bar_separator2{Gtk::ORIENTATION_HORIZONTAL},
            alarms_bar_separator3{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::Box hbox_alarms_form_buttons_bar{Gtk::ORIENTATION_HORIZONTAL, 10},
            hbox_alarms_form_additional_buttons_bar{Gtk::ORIENTATION_HORIZONTAL,
                                                    10};

        Gtk::MenuButton menubutton_file, menubutton_edit, menubutton_service,
            menubutton_help;
        Gtk::Menu menu_file, menu_edit, menu_service, menu_help;
        Gtk::MenuItem menuitem_save, menuitem_delete, menuitem_new_group,
            menuitem_rename_group, menuitem_exit, menuitem_general_settings,
            menuitem_help;
        Gtk::Label menuitem_save_label{"Сохранить"},
            menuitem_exit_label{"Выход"}, menuitem_delete_label{"Удалить"},
            menuitem_new_group_label{"Добавить группу"},
            menuitem_rename_group_label{"Переименовать группу"},
            menuitem_general_settings_label{"Основные настройки"},
            menuitem_help_label{"Помощь"};
        Gtk::Box menuitem_save_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_delete_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_new_group_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_exit_box{Gtk::ORIENTATION_HORIZONTAL, 5},
            menuitem_help_box{Gtk::ORIENTATION_HORIZONTAL, 5};

        Gtk::Grid grid_main;
        Gtk::DrawingArea grip1, grip2;
        Gtk::TreeView treeview_station, treeview_passports, treeview_zones;
        Gtk::ScrolledWindow scrolled_station, scrolled_form, scrolled_zones;
        Gtk::RadioButton radiobutton_settings_menu, radiobutton_passports_menu;
        Gtk::RadioButton::Group group_radiobuttons;
        Gtk::Box vbox_form{Gtk::ORIENTATION_VERTICAL, 10};
        Gtk::Stack stack_form_menu;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_save_enabled, pixbuf_save_disabled,
            pixbuf_new_group, pixbuf_group, pixbuf_group_inherited,
            pixbuf_delete, pixbuf_edit, pixbuf_help, pixbuf_station,
            pixbuf_address_book, pixbuf_run, pixbuf_add,
            pixbuf_arrow_up_enabled, pixbuf_arrow_up_disabled,
            pixbuf_arrow_down_enabled, pixbuf_arrow_down_disabled,
            pixbuf_klogic, pixbuf_task, pixbuf_fb, pixbuf_service_params,
            pixbuf_discrete, pixbuf_analog, pixbuf_deprecated, pixbuf_alarm,
            pixbuf_contact_name, pixbuf_description, pixbuf_email, pixbuf_phone,
            pixbuf_contact, pixbuf_contact_group, pixbuf_contact_add,
            pixbuf_contact_group_add, pixbuf_clear, pixbuf_write;
        std::array<Glib::RefPtr<Gdk::Pixbuf>, 6> pixbufs_params;
        Gtk::Button button_save, button_new_group, button_delete,
            button_address_book;
        Gtk::Image menuitem_save_icon, menuitem_delete_icon,
            menuitem_new_group_icon, menuitem_edit_icon, menuitem_help_icon,
            button_save_icon, button_new_group_icon, button_delete_icon,
            button_address_book_icon;
        Gtk::CellRendererPixbuf renderer_icon;
        Gtk::CellRendererText renderer_name;
        Gtk::TreeViewColumn treecolumn_icon, treecolumn_name;

        // Меню настроек
        Gtk::Label label_group_settings{"Настройки группы"}, label_group_name;
        Gtk::CheckButton checkbutton_use_own_settings{
            "Использовать собственные настройки"};
        Gtk::RadioButton::Group radiogroup_settings;
        Gtk::RadioButton radiobutton_general_settings{radiogroup_settings,
                                                      "Общие настройки"},
            radiobutton_alarms{radiogroup_settings, "Сигнализируемые ситуации"};
        Gtk::Frame frame_stack_general_settings_alarms;
        Gtk::Stack stack_general_settings_alarms;
        // Подменю общих настроек
        Gtk::CheckButton checkbutton_disable_alarm{"Отключить обработку"},
            checkbutton_checkback_on_startup{"Квитировать при запуске сервера"},
            checkbutton_use_own_sound_in_group{
                "Разрешить дополнительную обработку отдельных паспортов"};
        Gtk::Grid grid_equipment_description_mask;
        Gtk::Label label_equipment_description_mask{
            "Маска для описания оборудования"},
            label_equipment_description_mask_view;
        Gtk::Entry entry_equipment_description_mask;
        Gtk::Label label_frame_priority{"Приоритет"},
            label_frame_setpoints{"Уставки"};
        Gtk::Frame frame_priority, frame_setpoints,
            frame_aps_lowlevel_interaction;
        Gtk::RadioButton::Group radiogroup_priority;
        Gtk::RadioButton radiobutton_priority_max{radiogroup_priority,
                                                  "Максимальный"},
            radiobutton_priority_high{radiogroup_priority, "Высокий"},
            radiobutton_priority_medium{radiogroup_priority, "Средний"},
            radiobutton_priority_low{radiogroup_priority, "Низкий"},
            radiobutton_priority_minimal{radiogroup_priority, "Минимальный"};
        Gtk::Grid grid_crash_bounds_setpoints, grid_precrash_bounds_setpoints;
        Gtk::CheckButton checkbutton_borders_from_passport{
            "Брать из соответсвующих границ паспортов"};
        Gtk::Label label_frame_crash_bounds{"Аварийные границы"},
            label_frame_precrash_bounds{"Предаварийные границы"},
            label_frame_aps_lowlevel_interaction{
                "Взаимодействие АПС с нижним уровнем"};
        Gtk::Frame frame_crash_bounds, frame_precrash_bounds;
        Gtk::Label label_higher_crash_setpoint{"ВАУ"},
            label_lower_crash_setpoint{"НАУ"},
            label_higher_precrash_setpoint{"ВПУ"},
            label_lower_precrash_setpoint{"НПУ"};
        Gtk::Entry entry_higher_crash_setpoint, entry_lower_crash_setpoint,
            entry_higher_precrash_setpoint, entry_lower_precrash_setpoint;
        Gtk::Button button_higher_crash_setpoint{"..."},
            button_lower_crash_setpoint{"..."},
            button_higher_precrash_setpoint{"..."},
            button_lower_precrash_setpoint{"..."};
        Gtk::Box hbox_block_by_passport{Gtk::ORIENTATION_HORIZONTAL, 5},
            hbox_checkback{Gtk::ORIENTATION_HORIZONTAL, 5},
            hbox_write_on_checkback{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::CheckButton checkbutton_block_by_passport, checkbutton_checkback,
            checkbutton_write_on_checkback;
        Gtk::EventBox eventbox_block_by_passport, eventbox_checkback,
            eventbox_write_on_checkback;
        Gtk::Stack stack_aps_lowlevel_interaction_left;
        Gtk::Box hbox_value{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Label label_frame_value{"Значение"};
        Gtk::Frame frame_value;
        Gtk::RadioButton::Group radiogroup_val;
        Gtk::RadioButton radiobutton0{radiogroup_val, "лог. 0"},
            radiobutton1{radiogroup_val, "лог. 1"};
        Gtk::Box hbox_checkback_val{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Label label_frame_checkback{"Квитировать"};
        Gtk::Frame frame_checkback;
        Gtk::RadioButton::Group radiogroup_checkback;
        Gtk::RadioButton radiobutton_checkback_edge{radiogroup_checkback,
                                                    "по фронту"},
            radiobutton_checkback_cutoff{radiogroup_checkback, "по срезу"};
        Gtk::Box vbox_aps{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::Grid grid_aps;
        Gtk::Label label_aps_station{"Станция"};
        Gtk::Label label_aps_station_val;
        Gtk::Label label_aps_type{"Тип параметра"};
        Gtk::Label label_aps_type_val;
        Gtk::Label label_aps_id{"Идентификатор"};
        Gtk::Label label_aps_id_val;
        Gtk::Label label_aps_chipher{"Шифр"};
        Gtk::Label label_aps_chipher_val;
        Gtk::Label label_aps_name{"Наименование"};
        Gtk::Label label_aps_name_val;
        Gtk::Box hbox_aps_buttons_bar{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Button button_aps_info{"Информация"}, button_aps_select{"Выбрать"};
        // Подменю ситуаций
        Gtk::Label label_frame_alarms{
            "Ситуации (Приоритет уменьшается сверху вниз)"};
        Gtk::Frame frame_alarms;
        Gtk::Frame frame_treeview_situations;
        Gtk::ScrolledWindow scrolled_alarms;
        Gtk::TreeView treeview_situations;
        Gtk::TreeViewColumn treecolumn_alarm_toggle;
        Gtk::CellRendererToggle renderer_alarms_toggle;
        Gtk::Button button_situation_new_expand, button_situation_delete,
            button_situation_up, button_situation_down;
        Gtk::Image icon_new_alarm, button_situation_delete_icon,
            button_situation_up_icon, button_situation_down_icon;
        Gtk::RadioButton::Group radiogroup_alarms_menus;
        Gtk::RadioButton radiobutton_alarms_parameters_menu{
            radiogroup_alarms_menus, "Параметры"},
            radiobutton_alarms_message_menu{radiogroup_alarms_menus,
                                            "Сообщение"},
            radiobutton_alarms_sound_menu{radiogroup_alarms_menus, "Звук"},
            radiobutton_alarms_hysteresis_menu{radiogroup_alarms_menus,
                                               "Гистерезис"},
            radiobutton_alarms_sms_menu{radiogroup_alarms_menus, "SMS"},
            radiobutton_alarms_email_menu{radiogroup_alarms_menus, "e-mail"},
            radiobutton_alarms_passport_menu{radiogroup_alarms_menus,
                                             "Пасспорт"},
            radiobutton_alarms_write_menu{radiogroup_alarms_menus, "Запись"};
        Gtk::Stack stack_alarms;
        // Подподменю Параметры
        Gtk::Label label_frame_alarm_kind{"Тип ситуации"};
        Gtk::Frame frame_alarm_kind;
        Gtk::Box hbox_alarm_kind{Gtk::ORIENTATION_HORIZONTAL, 5},
            vbox_alarm_kind{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::RadioButton::Group radiogroup_alarm_kind;
        Gtk::RadioButton radiobutton_alarm_kind_crash,
            radiobutton_alarm_kind_warning, radiobutton_alarm_kind_notification;
        Gtk::Box vbox_alarm_actions{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::CheckButton checkbutton_checkback_alarm{"Защелкивать ситуацию"},
            checkbutton_send_sms{"Отправлять SMS"},
            checkbutton_send_email{"Отправлять e-mail"},
            checkbutton_write_events{"Писать события"};
        Gtk::CheckButton checkbutton_passport{
            "Отслеживать ситуацию по паспорту"},
            checkbutton_write{"Запись в паспорт при срабатывании"};
        Gtk::Box hbox_response_delay{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Label label_response_delay{"Задержка срабатывания"};
        Gtk::SpinButton spin_response_delay{
            Gtk::Adjustment::create(0, 0, 65535, 1)};
        Gtk::Label label_frame_situation_params;
        Gtk::Frame frame_situation_params;
        Gtk::Stack stack_situation_params;
        Gtk::Box hbox_situation_params_range{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Label label_situation_params_range{"<= Паспорт <"};
        Gtk::Entry entry_situation_params_greaterthan;
        Gtk::Entry entry_situation_params_lessthan;
        Gtk::Box hbox_situation_params_discrete{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::RadioButton::Group radiogroup_situation_params_discrete;
        Gtk::RadioButton radiobutton_situation_params_discrete_false{
            radiogroup_situation_params_discrete, "Лог. 0"},
            radiobutton_situation_params_discrete_true{
                radiogroup_situation_params_discrete, "Лог. 1"};
        Gtk::Box hbox_situation_params_errorcode{Gtk::ORIENTATION_HORIZONTAL,
                                                 5};
        Gtk::ComboBoxText combobox_situation_params_errorcode;
        Gtk::ComboBoxText combobox_response_delay;
        Gtk::CheckButton combobox_launch_program;
        Gtk::Frame frame_launch_program;
        Gtk::Label label_file_name{"Имя файла"};
        Gtk::Entry entry_file_name;
        Gtk::Button button_select_file;
        Gtk::Label label_cmd_parameters;
        Gtk::Entry entry_cmd_parameters;
        // Подподменю Сообщения
        Gtk::Label label_message;
        Gtk::Entry entry_message;
        Gtk::Label label_message_mask_view;
        Gtk::CheckButton checkbutton_write_event_to_group{
            "Записывать событие в группу"};
        Gtk::Grid grid_usergroup;
        Gtk::Label label_usergroup;
        Gtk::Box hbox_usergroup{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Entry entry_usergroup;
        Gtk::Button button_usergroup;
        Gtk::Label label_usergroup_description;
        Gtk::Entry entry_usergroup_description;
        Gtk::Label label_frame_message_masks;
        Gtk::Frame frame_message_masks{"Маски для сообщений"};
        Gtk::Box vbox_message_masks{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::Box hbox_message_masks_top{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Grid grid_message_masks_left;
        Gtk::Label label_message_mask_station;
        Gtk::Label label_message_mask_station_val;
        Gtk::Label label_message_mask_type;
        Gtk::Label label_message_mask_type_val;
        Gtk::Label label_message_mask_group;
        Gtk::Label label_message_mask_group_val;
        Gtk::Label label_message_mask_measure_unit;
        Gtk::Label label_message_mask_measure_unit_val;
        Gtk::Label label_message_mask_time;
        Gtk::Label label_message_mask_time_val;
        Gtk::Label label_message_mask_value;
        Gtk::Label label_message_mask_value_val;
        Gtk::Grid grid_message_masks_right;
        Gtk::Label label_message_mask_passport_id;
        Gtk::Label label_message_mask_passport_id_val;
        Gtk::Label label_message_mask_chipher;
        Gtk::Label label_message_mask_chipher_val;
        Gtk::Label label_message_mask_name;
        Gtk::Label label_message_mask_name_val;
        Gtk::Label label_message_mask_error;
        Gtk::Label label_message_mask_error_val;
        Gtk::Label label_message_mask_zone;
        Gtk::Label label_message_mask_zone_val;
        Gtk::Label label_message_mask_prev_value;
        Gtk::Label label_message_mask_prev_value_val;
        Gtk::Label label_message_mask_device;
        Gtk::Label label_message_mask_device_val;
        Gtk::Box hbox_message_masks_bottom{Gtk::ORIENTATION_HORIZONTAL, 5};
        Gtk::Label label_message_mask_level_of_passport_group;
        Gtk::Label label_message_mask_level_of_passport_group_val;
        // Подподменю Звук
        Gtk::Box hbox_sound_treeview_buttons_bar{Gtk::ORIENTATION_HORIZONTAL,
                                                 5};
        Gtk::Button button_sound_new, button_sound_delete, button_sound_pause,
            button_sound_up, button_sound_down, button_sound_play,
            button_sound_stop, button_sound_loop;
        Gtk::Image button_sound_new_icon, button_sound_delete_icon,
            button_sound_pause_icon, button_sound_up_icon,
            button_sound_down_icon, button_sound_play_icon,
            button_sound_stop_icon, button_sound_loop_icon;
        Gtk::Separator sound_separator1{Gtk::ORIENTATION_HORIZONTAL},
            sound_separator2{Gtk::ORIENTATION_HORIZONTAL},
            sound_separator3{Gtk::ORIENTATION_HORIZONTAL};
        Gtk::ScrolledWindow scrolled_sounds;
        Gtk::Frame frame_sounds;
        Gtk::TreeView treeview_sounds;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_pause, pixbuf_play, pixbuf_stop,
            pixbuf_loop;
        std::array<Glib::RefPtr<Gdk::Pixbuf>, 5> pixbufs_loop;
        // Подподменю гистерезис
        Gtk::SpinButton spin_higher_hysteresis{
            Gtk::Adjustment::create(0.0, -1e9, 1e9, 1.0, 10.0)},
            spin_lower_hysteresis{
                Gtk::Adjustment::create(0.0, -1e9, 1e9, 1.0, 10.0)};
        Gtk::ComboBoxText combobox_hysteresis;
        // Подподменю SMS
        Gtk::CheckButton checkbutton_sms_text{"Текст SMS"},
            checkbutton_sms_delay{
                "Игнорировать повторные отправки в течение (секунд)"};
        Gtk::Entry entry_sms_text;
        Gtk::Button button_sms_add_param;
        Gtk::Image button_sms_add_param_icon;
        Gtk::SpinButton spin_sms_priority{Gtk::Adjustment::create(0, 0, 4, 1)},
            spin_sms_timeout{Gtk::Adjustment::create(0, 0, 65535, 1)},
            spin_sms_delay{Gtk::Adjustment::create(0, 0, 65535, 1)};
        Gtk::TreeView *treeview_sms_contacts = nullptr;
        // Подподменю e-mail
        Gtk::CheckButton checkbutton_email_subject{"Тема письма"},
            checkbutton_email_text{"Текст письма"},
            checkbutton_email_delay{
                "Игнорировать повторные отправки в течение (секунд)"};
        Gtk::Entry entry_email_subject, entry_email_text;
        Gtk::Button button_email_subject_add_param, button_email_text_add_param;
        Gtk::Image button_email_subject_add_param_icon,
            button_email_text_add_param_icon;
        Gtk::SpinButton spin_email_priority{
            Gtk::Adjustment::create(0, 0, 4, 1)},
            spin_email_timeout{Gtk::Adjustment::create(0, 0, 65535, 1)},
            spin_email_delay{Gtk::Adjustment::create(0, 0, 65535, 1)};
        Gtk::Box vbox_alarm_parameters{Gtk::ORIENTATION_VERTICAL, 5},
            vbox_alarm_message{Gtk::ORIENTATION_VERTICAL, 5},
            vbox_alarm_sound{Gtk::ORIENTATION_VERTICAL, 5};
        Gtk::TreeView *treeview_email_contacts = nullptr;
        // Подподменю пасспорт
        Gtk::Label label_passport_station_val, label_passport_type_val,
            label_passport_id_val, label_passport_chipher_val,
            label_passport_name_val;
        Gtk::RadioButton::Group radiogroup_alarm_passport;
        Gtk::RadioButton radiobutton_passport0{radiogroup_alarm_passport,
                                               "лог. 0"},
            radiobutton_passport1{radiogroup_alarm_passport, "лог. 1"};
        // Подподменю запись
        Gtk::Label label_write_station_val, label_write_type_val,
            label_write_id_val, label_write_chipher_val, label_write_name_val;
        Gtk::RadioButton::Group radiogroup_alarm_write;
        Gtk::RadioButton radiobutton_write0{radiogroup_alarm_write, "лог. 0"},
            radiobutton_write1{radiogroup_alarm_write, "лог. 1"};

        Gtk::TreeViewColumn treecolumn_zones_view;
        Gtk::RadioButton::Group radiogroup_zones_view;
        Gtk::RadioButton radiobutton_zones_name{radiogroup_zones_view,
                                                "Отображать наименование"},
            radiobutton_zones_chipher{radiogroup_zones_view,
                                      "Отображать шифры"};

        std::array<Gtk::Box *, 5> situation_additional_menus{};

        // Подменю паспортов
        Gtk::TreeViewColumn treecolumn_passport_id;
        Gtk::CellRendererText renderer_passport_id;

        // Дерево зон
        Gtk::TreeViewColumn treecolumn_zones_name;
        Gtk::CellRendererText renderer_zones_name;

        Glib::RefPtr<Gtk::TreeStore> treestore_groups, treestore_zones;
        Glib::RefPtr<Gtk::TreeModelFilter> filter_groups;
        Glib::RefPtr<Gtk::TreeModelFilter> filter_combobox_station;
        Glib::RefPtr<Gtk::ListStore> liststore_passports_params;
        Glib::RefPtr<Gtk::TreeStore> treestore_contacts;
        // Используется при обновлении элементов формы для эффективного
        // последовательного доступа к данным выбранного обьекта
        Gtk::TreeModel::Row stored_curr_group_row, stored_curr_zone_row;
        GroupCols group_cols;
        AlarmObjCols alarmobj_cols;
        SituationCols situation_cols;
        SoundCols sound_cols;
        UsergroupCols usergroup_cols;
        ContactCols contact_cols;
        int drag_start_x = 0;
        int current_grip = 0;
        Gtk::EventBox *drag_grip = nullptr;
        Glib::RefPtr<Gtk::SizeGroup> col1_size, col2_size, col3_size;
        unsigned char curr_menu_idx = 0;
        unsigned char curr_settings_menu_idx = 0;
        unsigned char curr_alarms_menu_idx = 0;
        unsigned char curr_aps_menu_idx = 0;
        int curr_station_id = 0;
        MainSettings main_settings;
        std::string stations_config_path;
        std::string alarms_dir_path;
        std::string klogic_dir_path;
        std::string soundfiles_dir_path;
        std::atomic<bool> sound_playing{false};
        bool recursionguard_alarms_menu = false;
        bool recursionguard_alarms_form = false;
        bool unsaved = false;
        // Уведомления
        Glib::RefPtr<Gio::Application> app;
        Gdk::RGBA theme_bg_color;
};

AlarmConfigurator::~AlarmConfigurator() {}
int main(int argc, char *argv[]) {
    std::string project_path;
    if (argc > 1) {
        project_path = argv[1];
    } else {
        project_path = "/usr/share/SCADAProject/kaskad.kpr";
    }

    auto app = Gtk::Application::create("kaskad.alarm-configurator");
    AlarmConfigurator window(app, project_path);
    return app->run(window);
}
