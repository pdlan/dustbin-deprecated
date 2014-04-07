#ifndef DUSTBIN_INCLUDE_AUTH_H
#define DUSTBIN_INCLUDE_AUTH_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h>
class Auth {
  public:
    Auth();
    static const int success = 0;
    static const int failed = 1;
    int login(recycled::Connection* conn);
    int logout(recycled::Connection* conn);
    int auth(recycled::Connection* conn);
    static std::string SHA256(std::string str);
};
#endif