#ifndef DUSTBIN_INCLUDE_ADMIN_H
#define DUSTBIN_INCLUDE_ADMIN_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h>
#include "global.h"
#include "handlers.h"

class AdminLoginHandler : public DustbinHandler {
  public:
    bool get();
    bool post();
    static Handler* create() {
        return new AdminLoginHandler;
    }
};

class AdminLogoutHandler : public DustbinHandler {
  public:
    bool post();
    static Handler* create() {
        return new AdminLogoutHandler;
    }
};

class AdminIndexHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new AdminIndexHandler;
    }
};

#endif