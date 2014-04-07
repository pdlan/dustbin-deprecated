#include <stdio.h>
#include <string>
#include <sstream>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>
#include "dustbin.h"
#include "setting.h"
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

void GetStaticFileModifier::Modify(const char* in, size_t inlen,
                                   const PerExpandData* per_expand_data,
                                   ExpandEmitter* outbuf, 
                                   const std::string& arg) const {
    using namespace std;
    string path = arg.substr(1, arg.length() - 1);
    string full_path = this->url + path;
    outbuf->Emit(full_path);
}

void GetStaticFileModifier::set_url(std::string url) {
    if (url != "") {
        this->url = url;
    }
}

void PlusModifier::Modify(const char* in, size_t inlen,
                          const PerExpandData* per_expand_data,
                          ExpandEmitter* outbuf, 
                          const std::string& arg) const {
    using namespace std;
    string a_str(in, inlen);
    string b_str = arg.substr(1, arg.length() - 1);
    int a = atoi(a_str.c_str());
    int b = atoi(b_str.c_str());
    ostringstream buf;
    buf << a + b;
    outbuf->Emit(buf.str());
}

void LoadAdminSidebarModifier::Modify(const char* in, size_t inlen,
                                      const PerExpandData* per_expand_data,
                                      ExpandEmitter* outbuf, 
                                      const std::string& arg) const {
    using namespace std;
    using namespace ctemplate;
    string active = arg.substr(1, arg.length() - 1);
    TemplateDictionary dict("sidebar");
    dict.ShowSection("active_" + active);
    Template* temp =
        Template::GetTemplate("admin/template/sidebar.html", DO_NOT_STRIP);
    temp->Expand(outbuf, &dict);
}

bool ModifierManager::load_modifiers(Json::Value* language) {
    Dustbin* dustbin = Dustbin::instance();
    std::string url = dustbin->get_setting()->get_str_setting("site-url");
    this->get_path_modifier.set_url(url);
    this->get_static_file_modifier.set_url(url);
    this->format_time_modifier.set_language(language);
    if (!AddModifier("x-format-time=", &this->format_time_modifier)) {
        return false;
    }
    if (!AddModifier("x-get-path=", &this->get_path_modifier)) {
        return false;
    }
    if (!AddModifier("x-get-static-file=", &this->get_static_file_modifier)) {
        return false;
    }
    if (!AddModifier("x-plus=", &this->plus_modifier)) {
        return false;
    }
    if (!AddModifier("x-load-admin-sidebar=",
        &this->load_admin_sidebar_modifier)) {
        return false;
    }
}