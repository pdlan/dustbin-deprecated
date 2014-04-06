#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <string>
#include <utility>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
#include <dlfcn.h>
#include "plugin.h"
#include "global.h"

extern Global global;

PluginManager::~PluginManager() {
    this->destory_plugins();
}

void PluginManager::initialize() {
    this->refresh();
    this->load_plugins();
}

void PluginManager::destory_plugins() {
    using namespace std;
    for (vector<Plugin>::iterator it = this->plugins.begin();
         it != this->plugins.end(); ++it) {
        Plugin plugin = *it;
        void* handle = plugin.handle;
        if (handle) {
            typedef void (*fun_destory)();
            fun_destory destory = (fun_destory)dlsym(handle, "destory");
            if (destory) {
                destory();
            }
        }
    }
}

void PluginManager::refresh() {
    using namespace std;
    using namespace mongo;
    this->destory_plugins();
    DIR* dir;
    struct dirent* file;
    struct stat file_stat;
    if (!(dir = opendir("plugin"))) {
        return;
    }
    chdir("plugin");
    while (file = readdir(dir)) {
        lstat(file->d_name, &file_stat);
        if (S_IFDIR & file_stat.st_mode) {
            std::string plugin_path = file->d_name;
            if (plugin_path.length() > 0 && plugin_path[0] == '.') {
                continue;
            }
            string config_path = plugin_path + "/plugin.conf";
            FILE* config_file = fopen(config_path.c_str(), "r");
            if (!config_file) {
                continue;
            }
            fseek(config_file, 0, SEEK_END);
            size_t length = ftell(config_file);
            fseek(config_file, 0, SEEK_SET);
            char* buffer = new char[length];
            fread(buffer, length, sizeof(char), config_file);
            Json::Reader reader;
            Json::Value plugin_config;
            if (!reader.parse(buffer, plugin_config)) {
                delete buffer;
                fclose(config_file);
                continue;
            }
            delete buffer;
            fclose(config_file);
            string lib_name = plugin_config.get("library", "").asString();
            string name = plugin_config.get("name", "").asString();
            string description =
                plugin_config.get("description", "").asString();
            if (name == "" || lib_name == "") {
                continue;
            }
            string lib_path = plugin_path + "/" + lib_name;
            void* handle = dlopen(lib_path.c_str(), RTLD_LAZY);
            if (!handle) {
                continue;
            }
            PluginInfo info;
            info.name = name;
            info.description = description;
            BSONObj p = global.db_conn.findOne(global.db_name + ".plugin",
                                               QUERY("name" << name));
            Plugin plugin;
            plugin.info = info;
            plugin.handle = handle;
            if (p.isEmpty()) {
                plugin.is_enabled = false;
            } else {
                if (p.hasField("enabled") &&
                    p.getField("enabled").type() == Bool &&
                    p.getBoolField("enabled") == true) {
                    plugin.is_enabled = true;
                }
            }
            this->plugins.push_back(plugin);
            
        }
    }
    chdir("..");
}

bool PluginManager::load_plugins() {
    using namespace std;
    for (vector<Plugin>::iterator it = this->plugins.begin();
         it != this->plugins.end(); ++it) {
        Plugin plugin = *it;
        void* handle = plugin.handle;
        PluginInfo info = plugin.info;
        string name = info.name;
        if (handle) {
            typedef void (*fun_load)(Global*);
            fun_load load = (fun_load)dlsym(handle, "load");
            if (load) {
                load(&global);
            } else {
                printf("Unable to load plugin %s\n", name.c_str());
                continue;
            }
        }
    }
}

bool PluginManager::call_hooks(std::string hook_id, const Json::Value* args,
                               Json::Value* result) {
    using namespace std;
    if (hook_id == "" || !args || !result) {
        return false;
    }
    bool is_success;
    pair<HookMap::iterator, HookMap::iterator> p =
        this->hooks.equal_range(hook_id);
    for (HookMap::iterator it = p.first; it != p.second; ++it) {
        Hook hook = it->second;
        if (!hook) {
            return false;
        }
        is_success = hook(hook_id, args, result);
    }
    return is_success;
}

bool PluginManager::add_hook(std::string hook_id, Hook hook) {
    using namespace std;
    if (!hook || hook_id == "") {
        return false;
    }
    pair<HookMap::iterator, HookMap::iterator> p =
        this->hooks.equal_range(hook_id);
    for (HookMap::iterator it = p.first; it != p.second; ++it) {
        if (it->second == hook) {
            return false;
        }
    }
    this->hooks.insert(make_pair(hook_id, hook));
    return true;
}