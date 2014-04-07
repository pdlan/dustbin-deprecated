#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <recycled.h>
#include <jsoncpp/json/json.h>
#include "handlers.h"
#include "admin.h"
#include "install.h"
#include "dustbin.h"

bool load_json_file(std::string path, Json::Value& json);

int main(int argc, char** argv) {
    using namespace std;
    using namespace recycled;
    Dustbin* dustbin = Dustbin::instance();
    Router* router = dustbin->get_router();
    Server* server;
    string config_path = "dustbin.conf";
    Json::Value config;
    load_json_file(config_path, config);
    string deploy_type = config.get("deploy_type", "http").asString();
    if (deploy_type == "http") {
        int port = config.get("port", 8080).asInt();
        if (port == 0) {
            printf("Failed: Invalid port.\n");
            return 0;
        }
        server = new DirectServer("0.0.0.0", port, router);
    } else if (deploy_type == "fastcgi") {
        server = new FCGIServer(router);
    }
    string db_host = config.get("db_host", "localhost").asString();
    string db_name = config.get("db_name", "").asString();
    if (!dustbin->set_server(server)) {
        printf("Failed to set server.\n");
        return 0;
    }
    if (!dustbin->connect_db(db_host, db_name)) {
        printf("Failed to connect database.\n");
        return 0;
    }
    if (!dustbin->initialize()) {
        printf("Failed to initialize dustbin.\n");
        return 0;
    }
    if (argc == 2 && string(argv[1]) == "install") {
        Install::install();
        return 0;
    }
    if (!dustbin->start()) {
        printf("Failed to start server.\n");
        return 0;
    }
    return 0;
}

bool load_json_file(std::string path, Json::Value& json) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[length];
    fread(buffer, length, sizeof(char), file);
    Json::Reader reader;
    if (!reader.parse(buffer, json)) {
        delete buffer;
        fclose(file);
        return false;
    }
    delete buffer;
    fclose(file);
    return true;
}