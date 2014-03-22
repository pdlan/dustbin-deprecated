#ifndef DUSTBIN_INCLUDE_MODIFIERS_H
#define DUSTBIN_INCLUDE_MODIFIERS_H
#include <string>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>

#define MODIFY_SIGNATURE_                                               \
 public:                                                                \
  virtual void Modify(const char* in, size_t inlen,                     \
                      const ctemplate::PerExpandData*,                  \
                      ctemplate::ExpandEmitter* outbuf,                 \
                      const std::string& arg) const

class FormatTimeModifier : public ctemplate::TemplateModifier {
  public:
    MODIFY_SIGNATURE_;
    void set_language(Json::Value* language);
  private:
    Json::Value* language;
};

class GetPathModifier : public ctemplate::TemplateModifier {
  public:
    MODIFY_SIGNATURE_;
    void set_url(std::string url);
  private:
    std::string url;
};
#endif