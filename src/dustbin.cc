#include <mongo/client/dbclient.h>
#include <recycled.h>
#include "setting.h"
#include "article.h"
#include "plugin.h"
#include "auth.h"
#include "theme.h"
#include "handlers.h"
#include "admin.h"
#include "dustbin.h"

Dustbin::Dustbin() {
    using namespace recycled;
    this->db_conn = NULL;
    this->setting = new Setting();
    this->article = new ArticleManager();
    this->plugin = new PluginManager();
    this->theme = new Theme();
    this->auth = new Auth();
    this->router = new Router();
    this->server = NULL;
    this->router->add("/", PageHandler::create);
    this->router->add("/page/(\\d*?)(?:/|)", PageHandler::create);
    this->router->add("/article/(.*?)(?:/|)", ArticleHandler::create);
    this->router->add("/tag/(.*?)(?:/|)", TagHandler::create);
    this->router->add("/archives(?:/|)", ArchivesHandler::create);
    this->router->add("/archives/page/(\\d*?)(?:/|)", ArchivesHandler::create);
    this->router->add("/admin/user/login(?:/|)", AdminLoginHandler::create);
    this->router->add("/admin/user/logout(?:/|)", AdminLogoutHandler::create);
    this->router->add("/admin(?:/|)", AdminIndexHandler::create);
    this->router->add("/admin/theme(?:/|)", AdminThemeHandler::create);
    this->router->add("/admin/article/(.*?)/(.*?)(?:/|)",
                      AdminArticleHandler::create);
    this->router->add("/admin/setting(?:/|)", AdminSettingHandler::create);
    this->router->add("/static/(.*)", StaticFileHandler::create);
    this->router->add("/admin/static/(.*)", StaticFileHandler::create);
}

Dustbin::~Dustbin() {
    delete this->setting;
    delete this->article;
    delete this->plugin;
    delete this->theme;
    delete this->auth;
    delete this->db_conn;
    delete this->router;
}

bool Dustbin::set_server(recycled::Server* server) {
    if (!server) {
        return false;
    }
    this->server = server;
    this->server->initialize();
    return true;
}

bool Dustbin::start() {
    if (!this->server) {
        return false;
    }
    return this->server->start();
}

bool Dustbin::connect_db(std::string host, std::string name) {
    this->db_conn = new mongo::DBClientConnection;
    try {
        this->db_conn->connect(host);
    } catch (const mongo::DBException &e) {
        return false;
    }
    this->db_name = name;
    return true;
}

bool Dustbin::initialize() {
    if (!this->server || !this->db_conn) {
        return false;
    }
    this->article->initialize();
    this->theme->initialize();
    this->theme->set_theme(this->setting->get_str_setting("theme"));
    this->plugin->initialize();
    return true;
}