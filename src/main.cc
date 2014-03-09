#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <recycled.h>
#include <jsoncpp/json/json.h>
#include <mongo/client/dbclient.h>
#include "handlers.h"
#include "global.h"

extern Global global;
Global global;

bool initialize(std::string config_path, 
                recycled::Server** server, 
                recycled::Router* router);

int main(int argc, char** argv) {
    using namespace std;
    using namespace recycled;
    string config_path = "dustbin.conf";
    Server* server;
    Router router;
    router.add("/", PageHandler::create);
    router.add("/page/(\\d*)(?:/|)", PageHandler::create);
    router.add("/article/(.*)(?:/|)", ArticleHandler::create);
    router.add("/tag/(.*)(?:/|)", TagHandler::create);
    router.add("/archives(?:/|)", ArchivesHandler::create);
    router.add("/admin/login(?:/|)", LoginHandler::create);
    router.add("/static/(.*)", StaticFileHandler::create);
    if (argc == 2) {
        config_path = argv[1];
    }
    if (!initialize(config_path, &server, &router)) {
        printf("Unable to load config file.\n");
        return 0;
    }
    if (!server->initialize()) {
        printf("Unable to initialize http server.\n");
        return 0;
    }
    global.theme.set_theme(DustbinHandler::get_setting("theme"));
    server->start();
    return 0;
}

bool initialize(std::string config_path, 
                recycled::Server** server, 
                recycled::Router* router) {
    using namespace std;
    using namespace recycled;
    FILE* file = fopen(config_path.c_str(), "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[length];
    fread(buffer, length, sizeof(char), file);
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(buffer, root)) {
        delete buffer;
        fclose(file);
        return false;
    }
    delete buffer;
    fclose(file);
    string deploy_type = root.get("deploy_type", "http").asString();
    if (deploy_type == "http") {
        int port = root.get("port", 8080).asInt();
        if (port == 0) {
            return false;
        }
        (*server) = new DirectServer("0.0.0.0", port, router);
        printf("Dustbin has started.\nPort: %d\n", port);
    } else if (deploy_type == "fastcgi") {
        (*server) = new FCGIServer(router);
    }
    string db_host = root.get("db_host", "localhost").asString();
    string db_name = root.get("db_name", "").asString();
    try {
      global.db_conn.connect(db_host);
    } catch (const mongo::DBException &e) {
        printf("caught %s\n", e.what());
        return false;
    }
    global.db_name = db_name;
    string static_path = "theme/" + DustbinHandler::get_setting("theme")
                          + "/static/";
    StaticFileHandler::set_path(static_path);
    return true;
}