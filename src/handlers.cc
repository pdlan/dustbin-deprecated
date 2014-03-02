#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <ctemplate/template.h> 
#include "handlers.h"
#include "global.h"

extern Global global;

std::string DustbinHandler::get_setting(std::string key) {
    using namespace mongo;
    BSONObj p = global.db_conn.findOne(global.db_name + ".setting", QUERY("key" << key));
    if (!p.hasField("value")) {
        return "";
    }
    return p.getStringField("value");
}

int DustbinHandler::get_int_setting(std::string key) {
    using namespace mongo;
    BSONObj p = global.db_conn.findOne(global.db_name + ".setting", QUERY("key" << key));
    if (!p.hasField("value")) {
        return 0;
    }
    return p.getIntField("value");
}

void DustbinHandler::set_template_dict(ctemplate::TemplateDictionary* dict) {
    (*dict)["site_name"] = get_setting("site-name");
    (*dict)["site_url"] = get_setting("site-url");
}

void DustbinHandler::render(std::string template_name, 
                            ctemplate::TemplateDictionary* dict) {
    using namespace std;
    using namespace ctemplate;
    string path = "theme/" + get_setting("theme") + "/template/" + 
                  template_name + ".tpl";
    
    string output;
    ExpandTemplate(path, DO_NOT_STRIP, dict, &output);
    this->write(output);
}

std::string DustbinHandler::format_time(std::string format, time_t timestamp) {
    tm* timeinfo = localtime(&timestamp);
    char buffer[256];
    strftime(buffer, 256, format.c_str(), timeinfo);
    return buffer;
}

void DustbinHandler::on404()
{
    //TODO
}

bool PageHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    string page_str = this->get_regex_result(1);
    int page = 1;
    if (page_str != "") {
        page = atoi(page_str.c_str());
    }
    int articles_per_page = get_int_setting("articles-per-page");
    int limit = articles_per_page;
    int skip = articles_per_page * (page - 1);
    int articles_count = 
     global.db_conn.count(global.db_name + ".article", BSONObj());
    int pages = ceil(double(articles_count) / articles_per_page);
    if (page > pages) {
        this->on404();
    }
    TemplateDictionary dict("page");
    set_template_dict(&dict);
    if (page < pages) {
        dict.SetIntValue("next_page", page + 1);
        dict.ShowSection("next");
    }
    if (page > 1) {
        dict.SetIntValue("prev_page", page - 1);
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", page);
    dict.SetIntValue("pages", pages);
    auto_ptr<DBClientCursor> cursor = 
     global.db_conn.query(global.db_name + ".article", BSONObj(), limit, skip);
    while (cursor->more()) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        int timestamp = p.getIntField("time");
        string date = format_time(get_setting("time-format"), timestamp);
        TemplateDictionary* article = dict.AddSectionDictionary("articles");
        article->SetValue("id", id);
        article->SetValue("title", title);
        article->SetValue("content", content);
        article->SetValue("date", date);
        article->ShowSection("articles");
    }
    this->render("page", &dict);
    return true;
}

bool ArticleHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    string id = this->get_regex_result(1);
    BSONObj p = global.db_conn.findOne(global.db_name + ".article", QUERY("id" << id));
    if (p.isEmpty()) {
        this->on404();
        return true;
    }
    TemplateDictionary dict("article");
    set_template_dict(&dict);
    string title = p.getStringField("title");
    string content = p.getStringField("content");
    int timestamp = p.getIntField("time");
    string date = format_time(get_setting("time-format"), timestamp);
    BSONForEach(e, p.getObjectField("tag")) {
        string tag = e.String();
        TemplateDictionary* tag_dict = dict.AddSectionDictionary("tags");
        tag_dict->SetValue("name", tag);
        tag_dict->ShowSection("tags");
    }
    dict.SetValue("disqus_shortname", get_setting("disqus-shortname"));
    dict.SetValue("id", id);
    dict.SetValue("title", title);
    dict.SetValue("content", content);
    dict.SetValue("date", date);
    this->render("article", &dict);
    return true;
}