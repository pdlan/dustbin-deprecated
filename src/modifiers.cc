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
    string format = arg.substr(1, arg.length() - 1);
    time_t timestamp = atoi(in);
    tm* timeinfo = localtime(&timestamp);
    char buffer[256];
    strftime(buffer, 256, format.c_str(), timeinfo);
    outbuf->Emit(buffer);
}

void GetConfigModifier::Modify(const char* in, size_t inlen,
                               const PerExpandData* per_expand_data,
                               ExpandEmitter* outbuf, 
                               const std::string& arg) const {
    using namespace std;
    string key = arg.substr(1, arg.length() - 1);
    string value;
    if (this->config) {
        value = this->config->get(key, "").asString();
    }
    outbuf->Emit(value);
}

void GetLanguageModifier::Modify(const char* in, size_t inlen,
                               const PerExpandData* per_expand_data,
                               ExpandEmitter* outbuf, 
                               const std::string& arg) const {
    using namespace std;
    string key = arg.substr(1, arg.length() - 1);
    string value;
    if (this->language) {
        value = this->language->get(key, "").asString();
    }
    outbuf->Emit(value);
}

void LoadSubTemplateModifier::Modify(const char* in, size_t inlen,
                                     const PerExpandData* per_expand_data,
                                     ExpandEmitter* outbuf, 
                                     const std::string& arg) const {
    using namespace std;
    string template_name(in, inlen);
    printf("%s\n", template_name.c_str());
}

void GetConfigModifier::set_config(Json::Value* config) {
    if (config) {
        this->config = config;
    }
}

void GetLanguageModifier::set_language(Json::Value* language) {
    if (language) {
        this->language = language;
    }
}