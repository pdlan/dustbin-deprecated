#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <jsoncpp/json/json.h>
#include <recycled.h>
#include "theme.h"
#include "handlers.h"

void Theme::set_template_dict(std::string template_name,
                              ctemplate::TemplateDictionary* dict,
                              bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    dict->SetValue("site_name", DustbinHandler::get_setting("site-name"));
    dict->SetValue("site_description", 
                   DustbinHandler::get_setting("site-description"));
    dict->SetValue("site_url", DustbinHandler::get_setting("site-url"));
    if (is_admin_template) {
        return;
    }
    if (!this->config["custom-dictionaries"].isNull()) {
        Json::Value custom_dictionaries = this->config["custom-dictionaries"];
        for (int i = 0; i < custom_dictionaries.size(); ++ i) {
            if (custom_dictionaries[i].size() < 2) {
                continue;
            }
            string name = custom_dictionaries[i][0].asString();
            if (name == template_name) {
                for (int j = 0; j < custom_dictionaries[i][1].size(); ++ j) {
                    Json::Value dictionary = custom_dictionaries[i][1][j];
                    string type = dictionary["type"].asString();
                    string key = dictionary["key"].asString();
                    if (type == "string") {
                        string value = dictionary["value"].asString();
                        dict->SetValue(key, value);
                    } else if (type == "template-file") {
                        string value = dictionary["value"].asString();
                        string output;
                        string path = "theme/" + this->theme_path + "/template/" + value;
                        ExpandTemplate(path, DO_NOT_STRIP, dict, &output);
                        dict->SetValue(key, output);
                    } else if (type == "template-string") {
                        string value = dictionary["value"].asString();
                        string output;
                        string cache_name = name + "_" + key;
                        StringToTemplateCache(cache_name, value, DO_NOT_STRIP);
                        ExpandTemplate(cache_name, DO_NOT_STRIP, dict, &output);
                        dict->SetValue(key, output);
                    } else if (type == "int") {
                        int value = dictionary["value"].asInt();
                        dict->SetIntValue(key, value);
                    }
                }
            }
        }
    }
}

void Theme::render(std::string template_name, 
                   std::string* output, 
                   ctemplate::TemplateDictionary* dict, 
                   bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    string path;
    if (is_admin_template) {
        path = "admin/template/" + template_name + ".html";
    } else {
        path = "theme/" + this->theme_path + "/template/" + 
                  template_name + ".html";
    }
    if (!is_admin_template && !this->config["custom-templates"].isNull()) {
        Json::Value custom_templates = this->config["custom-templates"];
        if (!custom_templates[template_name].isNull()) {
            path = "theme/" + this->theme_path + "/template/" + 
                   custom_templates[template_name].asString();
        }
    }
    ExpandTemplate(path, DO_NOT_STRIP, dict, output);
}

bool Theme::set_theme(std::string name) {
    using namespace std;
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
    FILE* file = fopen(config_path.c_str(), "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[length];
    fread(buffer, length, sizeof(char), file);
    Json::Reader reader;
    if (!reader.parse(buffer, this->config)) {
        delete buffer;
        fclose(file);
        return false;
    }
    delete buffer;
    fclose(file);
    this->static_paths["/static/(.*)"] = "theme/" + theme_path + "/static/";
    this->static_paths["/admin/static/(.*)"] = "admin/static/";
    return true;
}

void Theme::initialize() {
    this->refresh();
    recycled::StaticFileHandler::paths = &static_paths;
}

void Theme::refresh() {
    using namespace std;
    DIR* dir;
    struct dirent* file;
    struct stat file_stat;
    if (!(dir = opendir("theme"))) {
        return;
    }
    chdir("theme");
    while (file = readdir(dir)) {
        lstat(file->d_name, &file_stat);
        if (S_IFDIR & file_stat.st_mode) {
            std::string theme_path = file->d_name;
            if (theme_path.length() > 0 && theme_path[0] == '.') {
                continue;
            }
            string config_path = theme_path + "/theme.conf";
            FILE* config_file = fopen(config_path.c_str(), "r");
            if (!config_file) {
                continue;
            }
            fseek(config_file, 0, SEEK_END);
            size_t length = ftell(config_file);
            fseek(config_file, 0, SEEK_SET);
            char* buffer = new char[length];
            fread(buffer, length, sizeof(char), config_file);
            Json::Reader reader;
            Json::Value theme_config;
            if (!reader.parse(buffer, theme_config)) {
                delete buffer;
                fclose(config_file);
                continue;
            }
            std::string name = theme_config["name"].asString();
            std::string author = theme_config["author"].asString();
            ThemeInfo info;
            info.name = name;
            info.author = author;
            info.path = theme_path;
            this->themes.push_back(info);
            delete buffer;
            fclose(config_file);
        }
    }
    chdir("..");
}