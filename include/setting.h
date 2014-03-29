#ifndef DUSTBIN_INCLUDE_SETTING_H
#define DUSTBIN_INCLUDE_SETTING_H
#include <string>
#include <mongo/client/dbclient.h>
class Setting {
  public:
    std::string get_str_setting(std::string key);
    void set_setting(std::string key, std::string value);
};
#endif