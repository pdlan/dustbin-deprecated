#ifndef DUSTBIN_INCLUDE_THEME_H
#define DUSTBIN_INCLUDE_THEME_H
#include <string>
#include <map>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>
#include "modifiers.h"
struct ThemeInfo {
    std::string name;
    std::string path;
    std::string author;
};

class BlockHandler {
  public:
    virtual std::string handle() = 0;
    virtual std::string get_block_name() = 0;
    virtual std::string get_template_name() = 0;
};

class Theme {
  public:
    ~Theme();
    void set_template_dict(std::string template_name,
                           ctemplate::TemplateDictionary* dict,
                           bool is_admin_template = false);
    void set_blocks(std::string template_name,
                    ctemplate::TemplateDictionary* dict);
    void render(std::string template_name,
                std::string* output,
                ctemplate::TemplateDictionary* dict);
    bool set_theme(std::string name);
    const Json::Value* get_config();
    const std::vector<ThemeInfo>* get_themes_info();
    void initialize();
    void refresh();
    bool add_block(BlockHandler* handler);
  private:
    std::vector<ThemeInfo> themes;
    std::map<std::string, std::string> static_paths;
    std::string theme_path;
    Json::Value config;
    Json::Value language;
    static bool load_json_file(std::string path, Json::Value* root);
    void set_title_templates();
    void set_language_templates();
    ModifierManager modifier_manager;
    std::map<std::string, std::vector<BlockHandler*>*> block_handlers;
};

inline const Json::Value* Theme::get_config() {
    return &this->config;
}

inline const std::vector<ThemeInfo>* Theme::get_themes_info() {
    return &this->themes;
}
#endif