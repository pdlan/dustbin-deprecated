#include <stdio.h>
#include <string>
#include <mongo/client/dbclient.h>
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
    auto_ptr<DBClientCursor> cursor =
        global.db_conn.query(global.db_name + ".plugin");
    while (cursor->more()) {
        BSONObj p = cursor->next();
        bool enabled = p.getBoolField("enabled");
        if (!enabled) {
            continue;
        }
        string name = p.getStringField("name");
        if (name == "") {
            continue;
        }
        string lib_path = "plugin/" + name + "/lib" + name + ".so";
        void* handle = dlopen(lib_path.c_str(), RTLD_LAZY);
        Plugin plugin;
        plugin.name = name;
        plugin.handle = handle;
        this->plugins.push_back(plugin);
    }
}

bool PluginManager::load_plugins() {
    using namespace std;
    for (vector<Plugin>::iterator it = this->plugins.begin();
         it != this->plugins.end(); ++it) {
        Plugin plugin = *it;
        void* handle = plugin.handle;
        string name = plugin.name;
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