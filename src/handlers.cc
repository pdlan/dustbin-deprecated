#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
#include <markdown.h>
#include <buffer.h>
#include <html.h>
#include <ctemplate/template.h>
#include "handlers.h"
#include "global.h"

extern Global global;

std::string parse_content(std::string content);
std::auto_ptr<mongo::DBClientCursor>
    page_articles(int page, int articles_per_page, int* number_of_pages);

void DustbinHandler::render(std::string template_name,
                            ctemplate::TemplateDictionary* dict,
                            bool is_admin_template) {
    using namespace std;
    using namespace ctemplate;
    string output;
    global.theme.render(template_name, &output, dict, is_admin_template);
    this->write(output);
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
    TemplateDictionary dict("page");
    string page_str = this->get_regex_result(1);
    int page = 1;
    if (page_str != "") {
        page = atoi(page_str.c_str());
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("page", 20).asInt();
    }
    int number_of_pages = 0;
    auto_ptr<DBClientCursor> cursor =
      page_articles(page, articles_per_page, &number_of_pages);
    if (page > number_of_pages) {
        this->on404();
        return true;
    }
    if (page < number_of_pages) {
        dict.ShowSection("next");
    }
    if (page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", page);
    dict.SetIntValue("number_of_pages", number_of_pages);
    while (cursor->more()) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        string content_parsed = parse_content(content);
        time_t timestamp = p.getIntField("time");
        if (id == "" || title == "" || content_parsed == "") {
            continue;
        }
        TemplateDictionary* article = dict.AddSectionDictionary("articles");
        article->SetValue("id", id);
        article->SetValue("title", title);
        article->SetValue("content", content_parsed);
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
        if (tag == "") {
            continue;
        }
        tag_dict->SetValue("name", tag);
        tag_dict->ShowSection("tags");
    }
    string comment = this->load_comment();
    string content_parsed = parse_content(content);
    if (id == "" || title == "" || content_parsed == "") {
        this->on404();
        return true;
    }
    dict.SetValue("id", id);
    dict.SetValue("title", title);
    dict.SetValue("content", content_parsed);
    if (comment != "") {
        dict.SetValue("comment", comment);
    }
    dict.SetIntValue("date", timestamp);
    global.theme.set_template_dict("article", &dict);
    this->render("article", &dict);
    return true;
}

std::string ArticleHandler::load_comment() {
    using namespace std;
    string type = global.get_setting("commenting-system");
    string path = "comment/" + type + ".html";
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        return "";
    }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[length + 1];
    fread(buffer, length, sizeof(char), file);
    buffer[length] = '\0';
    string comment = buffer;
    delete buffer;
    fclose(file);
    return comment;
}

bool ArchivesHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("archives");
    string page_str = this->get_regex_result(1);
    int page = 1;
    if (page_str != "") {
        page = atoi(page_str.c_str());
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("archives", 20).asInt();
    }
    int number_of_pages = 0;
    auto_ptr<DBClientCursor> cursor =
      page_articles(page, articles_per_page, &number_of_pages);
    if (page > number_of_pages) {
        this->on404();
        return true;
    }
    if (page < number_of_pages) {
        dict.ShowSection("next");
    }
    if (page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", page);
    dict.SetIntValue("number_of_pages", number_of_pages);
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
        if (id == "" || title == "") {
            continue;
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
    TemplateDictionary dict("tag");
    string tag = this->get_regex_result(1);
    string page_str = this->get_argument("page");
    int page = 1;
    if (page_str != "") {
        page = atoi(page_str.c_str());
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("tag", 20).asInt();
    }
    int number_of_pages = 0;
    int articles_count =
     global.db_conn.count(global.db_name + ".article", BSON("tag" << tag));
    int limit, skip;
    if (articles_per_page != 0) {
        limit = articles_per_page;
        skip = articles_per_page * (page - 1);
        number_of_pages = ceil(double(articles_count) / articles_per_page);
    } else {
        limit = articles_count;
        skip = 0;
        number_of_pages = 1;
    }
    Query qu = QUERY("tag" << tag);
    std::auto_ptr<DBClientCursor> cursor =
     global.db_conn.query(global.db_name + ".article",
                          qu.sort("time", -1), limit, skip);
    if (page > number_of_pages) {
        this->on404();
        return true;
    }
    if (page < number_of_pages) {
        dict.ShowSection("next");
    }
    if (page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", page);
    dict.SetIntValue("number_of_pages", number_of_pages);
    dict.SetValue("tag", tag);
    int count;
    for (count = 0; cursor->more(); ++ count) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        time_t timestamp = p.getIntField("time");
        if (id == "" || title == "" || content == "") {
            continue;
        }
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

std::string parse_content(std::string content) {
    using namespace std;
    const int OUTPUT_UNIT = 64;
    if (content == "") {
        return "";
    }
    struct buf* ob = bufnew(OUTPUT_UNIT);
    struct sd_callbacks callbacks;
	struct html_renderopt options;
	struct sd_markdown* markdown;
    sdhtml_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);
    sd_markdown_render(ob, (const uint8_t*)content.c_str(), content.length(), markdown);
    sd_markdown_free(markdown);
    if (ob->size <= 0) {
        return "";
    }
    string output((const char*)ob->data, ob->size);
	bufrelease(ob);
    return output;
}

std::auto_ptr<mongo::DBClientCursor>
    page_articles(int page, int articles_per_page, int* number_of_pages) {
    using namespace mongo;
    int articles_count =
     global.db_conn.count(global.db_name + ".article", BSONObj());
    int limit, skip;
    int pages;
    if (articles_per_page != 0) {
        limit = articles_per_page;
        skip = articles_per_page * (page - 1);
        pages = ceil(double(articles_count) / articles_per_page);
    } else {
        limit = articles_count;
        skip = 0;
        pages = 1;
    }
    *number_of_pages = pages;
    Query qu = Query();
    std::auto_ptr<DBClientCursor> cursor =
     global.db_conn.query(global.db_name + ".article",
                          qu.sort("time", -1), limit, skip);
    return cursor;
}