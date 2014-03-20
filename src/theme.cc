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
        path = "admin/template/" + template_name + ".html";
    } else {
        path = "theme/" + this->theme_path + "/template/layout.html";
    }
    if (!is_admin_template) {
        string sub_temp_path = "theme/" + this->theme_path + "/template/" + 
                               template_name + ".html";
        string buffer;
        string title;
        string cache_name = template_name + "_title";
        ExpandTemplate(cache_name, DO_NOT_STRIP, dict, &title);
        ExpandTemplate(sub_temp_path, DO_NOT_STRIP, dict, &buffer);
        layout_dict->SetValue("block_title", title);
        layout_dict->SetValue("block_content", buffer);
    }
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
    if (this->language["titles"].isNull()) {
        return false;
    }
    Json::Value titles = this->language["titles"];
    Json::Value::Members member = titles.getMemberNames();
    for (Json::Value::Members::iterator it = member.begin();
         it != member.end(); ++ it) {
        string template_name = *it;
        string title = titles[template_name].asString();
        string cache_name = template_name + "_title";
        StringToTemplateCache(cache_name, title, DO_NOT_STRIP);
    }
    this->static_paths["/static/(.*)"] = "theme/" + theme_path + "/static/";
    this->static_paths["/admin/static/(.*)"] = "admin/static/";
    this->get_config_modifier.set_config(&this->config);
    this->get_language_modifier.set_language(&this->language);
    if (!AddModifier("x-format-time=", &this->format_time_modifier)) {
        fprintf(stderr, "Unable to add modifier x-format-time.\n");
    }
    if (!AddModifier("x-load-sub-template=", &this->load_sub_template_modifier)) {
        fprintf(stderr, "Unable to add modifier x-sub-template.\n");
    }
    if (!AddModifier("x-get-config=", &this->get_config_modifier)) {
        fprintf(stderr, "Unable to add modifier x-get-config.\n");
    }
    if (!AddModifier("x-get-language=", &this->get_language_modifier)) {
        fprintf(stderr, "Unable to add modifier x-get-language.\n");
    }
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

bool Theme::load_json_file(std::string path, Json::Value* root) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[length];
    fread(buffer, length, sizeof(char), file);
    Json::Reader reader;
    if (!reader.parse(buffer, *root)) {
        delete buffer;
        fclose(file);
        return false;
    }
    delete buffer;
    fclose(file);
    return true;
}