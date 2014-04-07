#ifndef DUSTBIN_INCLUDE_GLOBAL_H
#define DUSTBIN_INCLUDE_GLOBAL_H
#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
class Theme;
class Auth;
class Setting;
class ArticleManager;
class PluginManager;

class Dustbin {
  public:
    Dustbin();
    ~Dustbin();
    static Dustbin* instance();
    bool connect_db(std::string host, std::string name);
    bool set_server(recycled::Server* server);
    bool start();
    bool initialize();
    Theme* get_theme();
    Auth* get_auth();
    Setting* get_setting();
    ArticleManager* get_article();
    PluginManager* get_plugin();
    std::string get_db_name();
    mongo::DBClientConnection* get_db_conn();
    recycled::Server* get_server();
    recycled::Router* get_router();
  private:
    mongo::DBClientConnection* db_conn;
    std::string db_name;
    recycled::Server* server;
    recycled::Router* router;
    Theme* theme;
    Auth* auth;
    Setting* setting;
    ArticleManager* article;
    PluginManager* plugin;
};

inline Dustbin* Dustbin::instance() {
    static Dustbin dustbin;
    return &dustbin;
}

inline Theme* Dustbin::get_theme() {
    return this->theme;
}

inline Auth* Dustbin::get_auth() {
    return this->auth;
}

inline Setting* Dustbin::get_setting() {
    return this->setting;
}

inline ArticleManager* Dustbin::get_article() {
    return this->article;
}

inline PluginManager* Dustbin::get_plugin() {
    return this->plugin;
}

inline std::string Dustbin::get_db_name() {
    return this->db_name;
}

inline mongo::DBClientConnection* Dustbin::get_db_conn() {
    return this->db_conn;
}

inline recycled::Server* Dustbin::get_server() {
    return this->server;
}

inline recycled::Router* Dustbin::get_router() {
    return this->router;
}
#endif