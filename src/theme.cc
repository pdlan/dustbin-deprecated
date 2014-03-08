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
    if (!this->config["custom_dictionaries"].isNull()) {
        Json::Value custom_dictionaries = this->config["custom_dictionaries"];
        for (int i = 0; i < custom_dictionaries.size(); i ++) {
            Json::Value dictionary = custom_dictionaries[i];
            string name = dictionary["template"].asString();
            if (name == template_name) {
                string type = dictionary["type"].asString();
                string key = dictionary["key"].asString();
                string value = dictionary["value"].asString();
                if (type == "string") {
                    dict->SetValue(key, value);
                } else if (type == "sub_template") {
                    string output;
                    string path = "theme/" + this->theme + "/template/" + value;
                    ExpandTemplate(path, DO_NOT_STRIP, dict, output);
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
                  template_name + ".tpl";
    if (!this->config["custom_templates"].isNull()) {
        Json::Value custom_templates = this->config["custom_templates"];
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