#ifndef DUSTBIN_INCLUDE_PLUGIN_H
#define DUSTBIN_INCLUDE_PLUGIN_H
#include <string>

struct PluginInfo {
    std::string name;
    std::string author;
    std::string version;
};

class PluginManager {
  public:
    bool load_plugins();
    ~PluginManager();
};
#endif