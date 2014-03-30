#ifndef DUSTBIN_INCLUDE_PLUGIN_H
#define DUSTBIN_INCLUDE_PLUGIN_H
#include <string>
#include <vector>
struct Plugin {
    std::string name;
    void* handle;
};

class PluginManager {
  public:
    ~PluginManager();
    void initialize();
    void refresh();
    bool load_plugins();
    void destory_plugins();
  private:
    std::vector<Plugin> plugins;
};
#endif