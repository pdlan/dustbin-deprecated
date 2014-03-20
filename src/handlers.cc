#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
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
    BSONObj p = global.db_conn.findOne(global.db_name + ".setting", 
                                       QUERY("key" << key));
    if (!p.hasField("value")) {
        return 0;
    }
    return p.getIntField("value");
}

void DustbinHandler::render(std::string template_name, 
                            ctemplate::TemplateDictionary* dict, 
                            bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    string output;
    global.theme.render(template_name, &output, dict, is_admin_template);
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
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    this->conn->resp->status_code = 404;
    this->conn->resp->status_text = "Not Found";
    TemplateDictionary dict("404");
    global.theme.set_template_dict("404", &dict);
    this->render("404", &dict);
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
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = config->get("articles-per-page", 20).asInt();
    int limit = articles_per_page;
    int skip = articles_per_page * (page - 1);
    int articles_count = 
     global.db_conn.count(global.db_name + ".article", BSONObj());
    int pages = ceil(double(articles_count) / articles_per_page);
    if (page > pages) {
        this->on404();
        return true;
    }
    TemplateDictionary dict("page");
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
    Query qu = Query();
    auto_ptr<DBClientCursor> cursor = 
     global.db_conn.query(global.db_name + ".article", 
                          qu.sort("time", -1), limit, skip);
    while (cursor->more()) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        time_t timestamp = p.getIntField("time");
        TemplateDictionary* article = dict.AddSectionDictionary("articles");
        article->SetValue("id", id);
        article->SetValue("title", title);
        article->SetValue("content", content);
        article->SetIntValue("date", timestamp);
        article->ShowSection("articles");
    }
    global.theme.set_template_dict("page", &dict);
    this->render("page", &dict);
    return true;
}

bool ArticleHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    string id = this->get_regex_result(1);
    BSONObj p = global.db_conn.findOne(global.db_name + ".article", 
                                       QUERY("id" << id));
    if (p.isEmpty()) {
        this->on404();
        return true;
    }
    TemplateDictionary dict("article");
    string title = p.getStringField("title");
    string content = p.getStringField("content");
    time_t timestamp = p.getIntField("time");
    BSONForEach(e, p.getObjectField("tag")) {
        string tag = e.String();
        TemplateDictionary* tag_dict = dict.AddSectionDictionary("tags");
        tag_dict->SetValue("name", tag);
        tag_dict->ShowSection("tags");
    }
    dict.SetValue("id", id);
    dict.SetValue("title", title);
    dict.SetValue("content", content);
    //dict.SetValue("date", date);
    dict.SetIntValue("date", timestamp);
    global.theme.set_template_dict("article", &dict);
    this->render("article", &dict);
    return true;
}

bool ArchivesHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("archives");
    Query qu = Query();
    auto_ptr<DBClientCursor> cursor = 
     global.db_conn.query(global.db_name + ".article", qu.sort("time", -1));
    TemplateDictionary* year_dict;
    for (int i = 0; cursor->more();) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        time_t timestamp = p.getIntField("time");
        tm* timeinfo = localtime(&timestamp);
        int year = timeinfo->tm_year + 1900;
        if (year != i) {
            if (i != 0) {
                year_dict->ShowSection("year");
            }
            year_dict = dict.AddSectionDictionary("year");
            year_dict->SetIntValue("year", year);
            i = year;
        }
        TemplateDictionary* article = year_dict->AddSectionDictionary("articles");
        article->SetValue("id", id);
        article->SetValue("title", title);
        article->SetIntValue("date", timestamp);
        article->ShowSection("articles");
    }
    global.theme.set_template_dict("archives", &dict);
    this->render("archives", &dict);
    return true;
}

bool TagHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("archives");
    string tag = this->get_regex_result(1);
    dict.SetValue("tag", tag);
    auto_ptr<DBClientCursor> cursor = 
     global.db_conn.query(global.db_name + ".article", QUERY("tag" << tag));
    int count;
    for (count = 0; cursor->more(); ++ count) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        time_t timestamp = p.getIntField("time");
        TemplateDictionary* article = dict.AddSectionDictionary("articles");
        article->SetValue("id", id);
        article->SetValue("title", title);
        article->SetIntValue("date", timestamp);
        article->ShowSection("articles");
    }
    if (count == 0) {
        this->on404();
        return true;
    }
    global.theme.set_template_dict("tag", &dict);
    this->render("tag", &dict);
    return true;
}