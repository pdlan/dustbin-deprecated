#ifndef DUSTBIN_INCLUDE_THEME_H
#define DUSTBIN_INCLUDE_THEME_H
#include <string>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h> 
class Theme {
  public:
    void set_template_dict(std::string template_name,
                           ctemplate::TemplateDictionary* dict);
    void render(std::string template_name, 
                std::string* output, 
                ctemplate::TemplateDictionary* dict);
    void set_theme(std::string theme);
    const Json::Value* get_config();
  private:
    std::string theme;
    Json::Value config;
};

inline const Json::Value* Theme::get_config() {
    return &this->config;
}
#endif