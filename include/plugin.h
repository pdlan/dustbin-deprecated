#ifndef DUSTBIN_INCLUDE_PLUGIN_H
#define DUSTBIN_INCLUDE_PLUGIN_H
#include <string>
#include <vector>
#include <map>
#include <jsoncpp/json/json.h>
typedef bool (*Hook)(std::string, const Json::Value*, Json::Value*);
typedef std::pair<std::string, Hook> HookPair;
typedef std::multimap<std::string, Hook> HookMap;

struct PluginInfo {
    std::string name;
    std::string description;
};

struct Plugin {
    PluginInfo info;
    void* handle;
    bool is_enabled;
};

class PluginManager {
  public:
    ~PluginManager();
    void initialize();
    void refresh();
    bool load_plugins();
    void destory_plugins();
    bool add_hook(std::string hook_id, Hook hook);
    bool call_hooks(std::string hook_id, const Json::Value* args,
                    Json::Value* result);
  private:
    std::vector<Plugin> plugins;
    HookMap hooks;
};
#endif