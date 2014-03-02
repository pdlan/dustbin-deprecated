#ifndef DUSTBIN_INCLUDE_HANDLERS_H
#define DUSTBIN_INCLUDE_HANDLERS_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h> 
#include "global.h"
class DustbinHandler : public recycled::Handler {
  public:
    static std::string get_setting(std::string key);
    static int get_int_setting(std::string key);
  protected:
    static std::string format_time(std::string format, time_t timestamp);
    static void set_template_dict(ctemplate::TemplateDictionary* dict);
    void render(std::string template_name, ctemplate::TemplateDictionary* dict);
};

class PageHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new PageHandler;
    }
};
#endif