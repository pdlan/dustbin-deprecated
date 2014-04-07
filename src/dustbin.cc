#include <mongo/client/dbclient.h>
#include "setting.h"
#include "article.h"
#include "plugin.h"
#include "auth.h"
#include "theme.h"
#include "dustbin.h"

Dustbin::Dustbin() {
    this->db_conn = new mongo::DBClientConnection;
    this->setting = new Setting();
    this->article = new ArticleManager();
    this->plugin = new PluginManager();
    this->theme = new Theme();
    this->auth = new Auth();
}

Dustbin::~Dustbin() {
    delete this->setting;
    delete this->article;
    delete this->plugin;
    delete this->theme;
    delete this->auth;
    delete this->db_conn;
}

bool Dustbin::connect_db(std::string host, std::string name) {
    try {
        this->db_conn->connect(host);
    } catch (const mongo::DBException &e) {
        return false;
    }
    this->db_name = name;
    this->article->initialize();
    this->theme->initialize();
    this->theme->set_theme(this->setting->get_str_setting("theme"));
    this->plugin->initialize();
    return true;
}
