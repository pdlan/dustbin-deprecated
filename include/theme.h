#ifndef DUSTBIN_INCLUDE_THEME_H
#define DUSTBIN_INCLUDE_THEME_H
#include <string>
#include <map>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>

struct ThemeInfo {
    std::string name;
    std::string path;
    std::string author;
};

class Theme {
  public:
    void set_template_dict(std::string template_name,
                           ctemplate::TemplateDictionary* dict, 
                           bool is_admin_template = false);
    void render(std::string template_name, 
                std::string* output, 
                ctemplate::TemplateDictionary* dict, 
                bool is_admin_template);
    bool set_theme(std::string name);
    const Json::Value* get_config();
    void initialize();
    void refresh();
  private:
    std::vector<ThemeInfo> themes;
    std::map<std::string, std::string> static_paths;
    std::string theme_path;
    Json::Value config;
};

inline const Json::Value* Theme::get_config() {
    return &this->config;
}
#endif