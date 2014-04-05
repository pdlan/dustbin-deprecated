#ifndef DUSTBIN_INCLUDE_HANDLERS_H
#define DUSTBIN_INCLUDE_HANDLERS_H
#include <string>
#include <recycled.h>
#include <ctemplate/template.h> 
#include "global.h"
class DustbinHandler : public recycled::Handler {
  protected:
    void render(std::string template_name, 
                ctemplate::TemplateDictionary* dict, 
                bool is_admin_template = false);
    void on404();
};

class PageHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new PageHandler;
    }
};

class ArticleHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new ArticleHandler;
    }
};

class ArchivesHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new ArchivesHandler;
    }
};

class TagHandler : public DustbinHandler {
  public:
    bool get();
    static Handler* create() {
        return new TagHandler;
    }
};
#endif