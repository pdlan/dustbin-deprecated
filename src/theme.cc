#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <jsoncpp/json/json.h>
#include <recycled.h>
#include "dustbin.h"
#include "setting.h"
#include "theme.h"

Theme::~Theme() {
    for (std::map<std::string, std::vector<BlockHandler*>*>::iterator it =
         this->block_handlers.begin();
         it != this->block_handlers.end(); ++it) {
        std::vector<BlockHandler*>* handlers = it->second;
        if (handlers) {
            delete handlers;
        }
    }
}

void Theme::set_template_dict(std::string template_name,
                              ctemplate::TemplateDictionary* dict,
                              bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    dict->SetValue("site_name",
                   dustbin->get_setting()->get_str_setting("site-name"));
    dict->SetValue("site_description",
                   dustbin->get_setting()->get_str_setting("site-description"));
    dict->SetValue("site_url",
                   dustbin->get_setting()->get_str_setting("site-url"));
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
    this->set_blocks(template_name, dict);
}

void Theme::set_blocks(std::string template_name,
                       ctemplate::TemplateDictionary* dict) {
    using namespace std;
    if (this->block_handlers.count(template_name)) {
        vector<BlockHandler*>* handlers = this->block_handlers[template_name];
        if (handlers) { 
            for (vector<BlockHandler*>::iterator it = handlers->begin();
                 it != handlers->end(); ++it) {
                BlockHandler* handler = *it;
                if (!handler) {
                    continue;
                }
                string block_name = handler->get_block_name();
                string key = "block_" + block_name;
                string value = handler->handle();
                if (key == "" || value == "") {
                    continue;
                }
                dict->SetValue(key, value);
            }
        }
    }
}

void Theme::render(std::string template_name,
                   std::string* output,
                   ctemplate::TemplateDictionary* dict) {
    using namespace std;
    using namespace ctemplate;
    TemplateDictionary* layout_dict = dict->MakeCopy("layout");
    string path = "theme/" + this->theme_path + "/template/layout.html";
    string sub_temp_path = "theme/" + this->theme_path + "/template/" +
                           template_name + ".html";
    string buffer;
    string title;
    string cache_name = template_name + "_title";
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
    this->static_paths["/admin/static/(.*)"] = "admin/static/";
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
    this->modifier_manager.load_modifiers(&this->language);
    return true;
}

void Theme::initialize() {
    this->refresh();
    recycled::StaticFileHandler::paths = &static_paths;
}

void Theme::refresh() {
    using namespace std;
    this->themes.clear();
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

void Theme::set_title_templates() {
    using namespace std;
    using namespace ctemplate;
    if (this->language["titles"].isNull()) {
        return;
    }
    Json::Value titles = this->language["titles"];
    Json::Value::Members members = titles.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin();
         it != members.end(); ++ it) {
        string template_name = *it;
        if (titles[template_name].isString()) {
            string title = titles[template_name].asString();
            string cache_name = template_name + "_title";
            StringToTemplateCache(cache_name, title, DO_NOT_STRIP);
        }
    }
}

void Theme::set_language_templates() {
    using namespace std;
    using namespace ctemplate;
    Json::Value::Members members = this->language.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin();
         it != members.end(); ++ it) {
        string key = *it;
        if (this->language[key].isString()) {
            string value = this->language[key].asString();
            string cache_name = "language_" + key;
            if (value == "") {
                continue;
            }
            StringToTemplateCache(cache_name, value, DO_NOT_STRIP);
        }
    }
}

bool Theme::add_block(BlockHandler* handler) {
    using namespace std;
    if (!handler) {
        return false;
    }
    string template_name = handler->get_template_name();
    string block_name = handler->get_block_name();
    vector<BlockHandler*>* handlers_for_template;
    if (!this->block_handlers.count(template_name)) {
        handlers_for_template = new vector<BlockHandler*>;
        this->block_handlers[template_name] = handlers_for_template;
    } else {
        handlers_for_template = this->block_handlers[template_name];
    }
    if (!handlers_for_template) {
        return false;
    }
    handlers_for_template->push_back(handler);
    return true;
}