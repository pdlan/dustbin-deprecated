#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <jsoncpp/json/json.h>
#include <recycled.h>
#include "handlers.h"
#include "theme.h"

extern Global global;

void Theme::set_template_dict(std::string template_name,
                              ctemplate::TemplateDictionary* dict,
                              bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    dict->SetValue("site_name", global.get_setting("site-name"));
    dict->SetValue("site_description", global.get_setting("site-description"));
    dict->SetValue("site_url", global.get_setting("site-url"));
    if (is_admin_template) {
        return;
    }
    Json::Value::Members language_members = this->language.getMemberNames();
    for (Json::Value::Members::iterator it = language_members.begin();
         it != language_members.end(); ++ it) {
        string key = *it;
        if (this->language[key].isString()) {
            string value = this->language[key].asString();
            string cache_name = "language_" + key;
            if (key == "") {
                continue;
            }
            string buffer;
            ExpandTemplate(cache_name, DO_NOT_STRIP, dict, &buffer);
            if (buffer == "") {
                continue;
            }
            dict->SetValue(cache_name, buffer);
        }
    }
}

void Theme::render(std::string template_name,
                   std::string* output,
                   ctemplate::TemplateDictionary* dict,
                   bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    TemplateDictionary* layout_dict = dict->MakeCopy("layout");
    string path;
    if (is_admin_template) {
        path = "admin/template/layout.html";
    } else {
        path = "theme/" + this->theme_path + "/template/layout.html";
    }
    string sub_temp_path;
    if (is_admin_template) {
        sub_temp_path = "admin/template/" + template_name + ".html";
    } else {
        sub_temp_path = "theme/" + this->theme_path + "/template/" +
                               template_name + ".html";
    }
    string buffer;
    string title;
    string cache_name;
    if (is_admin_template) {
        cache_name = "admin_" + template_name + "_title";
    } else {
        cache_name = template_name + "_title";
    }
    ExpandTemplate(cache_name, DO_NOT_STRIP, dict, &title);
    ExpandTemplate(sub_temp_path, DO_NOT_STRIP, dict, &buffer);
    layout_dict->SetValue("block_title", title);
    layout_dict->SetValue("block_content", buffer);
    ExpandTemplate(path, DO_NOT_STRIP, layout_dict, output);
}

bool Theme::set_theme(std::string name) {
    using namespace std;
    using namespace ctemplate;
    std::string theme_path;
    bool has_found = false;
    for (vector<ThemeInfo>::iterator it = this->themes.begin();
         it != this->themes.end(); ++ it) {
        if (name == it->name) {
            theme_path = it->path;
            has_found = true;
            break;
        }
    }
    if (!has_found) {
        return false;
    }
    this->theme_path = theme_path;
    string config_path = "theme/" + theme_path + "/theme.conf";
    if (!Theme::load_json_file(config_path, &this->config)) {
        return false;
    }
    string language = this->config["language"].asString();
    string language_path = "theme/" + theme_path +
                           "/language/" + language + ".lang";
    if (!Theme::load_json_file(language_path, &this->language)) {
        return false;
    }
    this->set_title_templates();
    this->set_language_templates();
    this->static_paths["/static/(.*)"] = "theme/" + theme_path + "/static/";
    this->static_paths["/admin/static/(.*)"] = "admin/static/";
    this->get_path_modifier.set_url(global.get_setting("site-url"));
    this->get_static_file_modifier.set_url(global.get_setting("static-url"));
    this->format_time_modifier.set_language(&this->language);
    if (!AddModifier("x-format-time=", &this->format_time_modifier)) {
        fprintf(stderr, "Unable to add modifier x-format-time.\n");
    }
    if (!AddModifie