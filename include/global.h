#ifndef DUSTBIN_INCLUDE_GLOBAL_H
#define DUSTBIN_INCLUDE_GLOBAL_H
#include <mongo/client/dbclient.h>
#include "theme.h"
class Global {
  public:
    mongo::DBClientConnection db_conn;
    std::string db_name;
    Theme theme;
};
#endif