#ifndef DUSTBIN_INCLUDE_GLOBAL_H
#define DUSTBIN_INCLUDE_GLOBAL_H
#include <string>
#include <mongo/client/dbclient.h>
#include "theme.h"
#include "auth.h"
#include "install.h"
class Global {
  public:
    std::string get_setting(std::string key);
    int get_int_setting(std::string key);
    void set_setting(std::string key, std::string value);
    mongo::DBClientConnection db_conn;
    std::string db_name;
    Theme theme;
    Auth auth;
};
#endif