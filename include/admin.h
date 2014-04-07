#ifndef DUSTBIN_INCLUDE_ADMIN_H
#define DUSTBIN_INCLUDE_ADMIN_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h>
#include "handlers.h"
class AdminHandler : public recycled::Handler {
  protected:
    void render(std::string template_name,
                ctemplate::TemplateDictionary* dict);
    void on404();
};

class AdminLoginHandler : public AdminHandler {
  public:
    bool get();
    bool post();
    static Handler* create() {
        return new AdminLoginHandler;
    }
};

class AdminLogoutHandler : public AdminHandler {
  public:
    bool post();
    static Handler* create() {
        return new AdminLogoutHandler;
    }
};

class AdminIndexHandler : public AdminHandler {
  public:
    bool get();
    static Handler* create() {
        return new AdminIndexHandler;
    }
};

class AdminThemeHandler : public AdminHandler {
  public:
    bool get();
    bool post();
    static Handler* create() {
        return new AdminThemeHandler;
    }
};

class AdminArticleHandler : public AdminHandler {
  public:
    bool get();
    bool post();
    static Handler* create() {
        return new AdminArticleHandler;
    }
};

class AdminSettingHandler : public AdminHandler {
  public:
    bool get();
    bool post();
    static Handler* create() {
        return new AdminSettingHandler;
    }
};
#endif