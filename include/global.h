#ifndef DUSTBIN_INCLUDE_GLOBAL_H
#define DUSTBIN_INCLUDE_GLOBAL_H
#include <string>
#include <mongo/client/dbclient.h>
#include "theme.h"
#include "auth.h"
#include "setting.h"
class Global {
  public:
    mongo::DBClientConnection db_conn;
    std::string db_name;
    Theme theme;
    Auth auth;
    Setting setting;
};
#endif