#include <string>
#include <jsoncpp/json/json.h>
#include "theme.h"
#include "handlers.h"

void Theme::set_template_dict(std::string template_name,
                              ctemplate::TemplateDictionary* dict) {
    using namespace std;
    using namespace ctemplate;
    dict->SetValue("site_name", DustbinHandler::get_setting("site-name"));
    dict->SetValue("site_url", DustbinHandler::get_setting("site-url"));
    if (!this->config["custom-dictionaries"].isNull()) {
        Json::Value custom_dictionaries = this->config["custom-dictionaries"];
        for (int i = 0; i < custom_dictionaries.size(); i ++) {
            if (custom_dictionaries[i].size() < 2) {
                continue;
            }
            string name = custom_dictionaries[i][0].asString();
            if (name == template_name) {
                for (int j = 0; j < custom_dictionaries[i][1].size(); j ++) {
                    Json::Value dictionary = custom_dictionaries[i][1][j];
                    string type = dictionary["type"].asString();
                    string key = dictionary["key"].asString();
                    if (type == "string") {
                        string value = dictionary["value"].asString();
                        dict->SetValue(key, value);
                    } else if (type == "template-file") {
                        string value = dictionary["value"].asString();
                        string output;
                        string path = "theme/" + this->theme + "/template/" + value;
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
                   ctemplate::TemplateDictionary* dict) {
    using namespace std;
    using namespace ctemplate;
    string path = "theme/" + this->theme + "/template/" + 
                  template_name + ".html";
    if (!this->config["custom-templates"].isNull()) {
        Json::Value custom_templates = this->config["custom-templates"];
        if (!custom_templates[template_name].isNull()) {
            path = "theme/" + this->theme + "/template/" + 
                   custom_templates[template_name].asString();
        }
    }
    ExpandTemplate(path, DO_NOT_STRIP, dict, output);
}

void Theme::set_theme(std::string theme) {
    using namespace std;
    this->theme = theme;
    string config_path = "theme/" + theme + "/theme.conf";
    FILE* file = fopen(config_path.c_str(), "r");
    if (!file) {
        return;
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
        return;
    }
    delete buffer;
    fclose(file);
}