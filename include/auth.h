#ifndef DUSTBIN_INCLUDE_AUTH_H
#define DUSTBIN_INCLUDE_AUTH_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h>
class Auth {
  public:
    static const int success = 0;
    static const int failed = 1;
    void set_db_config(mongo::DBClientConnection* db_conn, std::string db_name);
    int login(recycled::Connection* conn);
    int logout(recycled::Connection* conn);
    int auth(recycled::Connection* conn);
    static std::string SHA256(std::string str);
  private:
    mongo::DBClientConnection* db_conn;
    std::string db_name;
};
#endif