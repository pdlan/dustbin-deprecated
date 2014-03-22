#include <stdio.h>
#include <string>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>
#include "modifiers.h"

using namespace ctemplate;

void FormatTimeModifier::Modify(const char* in, size_t inlen,
                                const PerExpandData* per_expand_data,
                                ExpandEmitter* outbuf, 
                                const std::string& arg) const {
    using namespace std;
    string format;
    string arg_str = arg.substr(1, arg.length() - 1);
    Json::Reader reader;
    Json::Value args;
    if (reader.parse(arg_str, args)) {
        if (!args.isArray() || args.size() < 2) {
            return;
        }
        string type = args[0].asString();
        if (type == "string") { 
            format = args[1].asString();
        } else if (type == "language") {
            string key = args[1].asString();
            format = this->language->get(key, "").asString();
        }
    } else {
        return;
    }
    time_t timestamp = atoi(in);
    tm* timeinfo = localtime(&timestamp);
    char buffer[256];
    strftime(buffer, 256, format.c_str(), timeinfo);
    outbuf->Emit(buffer);
}

void FormatTimeModifier::set_language(Json::Value* language) {
    if (language) {
        this->language = language;
    }
}

void GetPathModifier::Modify(const char* in, size_t inlen,
                             const PerExpandData* per_expand_data,
                             ExpandEmitter* outbuf, 
                             const std::string& arg) const {
    using namespace std;
    string path = arg.substr(1, arg.length() - 1);
    string full_path = this->url + path;
    outbuf->Emit(full_path);
}

void GetPathModifier::set_url(std::string url) {
    if (url != "") {
        this->url = url;
    }
}