#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>
#include "handlers.h"
#include "global.h"
#include "article.h"

extern Global global;

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
    int current_page = 1;
    if (page_str != "") {
        current_page = atoi(page_str.c_str());
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("page", 20).asInt();
    }
    PageInfo page = page_articles(current_page, articles_per_page);
    if (current_page > page.number_of_pages) {
        this->on404();
        return true;
    }
    if (current_page < page.number_of_pages) {
        dict.ShowSection("next");
    }
    if (current_page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", current_page);
    dict.SetIntValue("number_of_pages", page.number_of_pages);
    vector<Article> articles = get_articles(page.limit, page.skip);
    for (vector<Article>::iterator it = articles.begin();
         it != articles.end(); ++it) {
        Article article = *it;
        parse_article(&article);
        TemplateDictionary* article_dict =
         dict.AddSectionDictionary("articles");
        set_article_dict(article_dict, &article);
    }
    dict.ShowSection("articles");
    global.theme.set_template_dict("page", &dict);
    this->render("page", &dict);
    return true;
}

bool ArticleHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("article");
    string id = this->get_regex_result(1);
    Article article;
    if (!get_one_article(id, &article)) {
        this->on404();
        return true;
    }
    parse_article(&article);
    set_article_dict(&dict, &article);
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
    int current_page = 1;
    if (page_str != "") {
        current_page = atoi(page_str.c_str());
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("archives", 20).asInt();
    }
    PageInfo page = page_articles(current_page, articles_per_page);
    if (current_page > page.number_of_pages) {
        this->on404();
        return true;
    }
    if (current_page < page.number_of_pages) {
        dict.ShowSection("next");
    }
    if (current_page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", current_page);
    dict.SetIntValue("number_of_pages", page.number_of_pages);
    TemplateDictionary* year_dict;
    vector<Article> articles = get_articles(page.limit, page.skip);
    for (int i = 0, j = 0; i < articles.size(); ++i) {
        Article article = articles[i];
        parse_article(&article);
        time_t timestamp = article.timestamp;
        tm* timeinfo = localtime(&timestamp);
        int year = timeinfo->tm_year + 1900;
        if (year != j) {
            year_dict = dict.AddSectionDictionary("year");
            year_dict->SetIntValue("year", year);
            j = year;
        }
        TemplateDictionary* article_dict =
         year_dict->AddSectionDictionary("articles");
        set_article_dict(article_dict, &article);
    }
    dict.ShowSection("articles");
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
    string page_str = this->get_argument("page");
    string tag = this->get_regex_result(1);
    int current_page = 1;
    if (page_str != "") {
        current_page = atoi(page_str.c_str());
    }
    if (tag == "") {
        this->on404();
        return true;
    }
    const Json::Value* config = global.theme.get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("tag", 20).asInt();
    }
    PageInfo page = page_articles(current_page, articles_per_page, tag);
    if (current_page > page.number_of_pages) {
        this->on404();
        return true;
    }
    if (current_page < page.number_of_pages) {
        dict.ShowSection("next");
    }
    if (current_page > 1) {
        dict.ShowSection("prev");
    }
    dict.SetIntValue("current_page", current_page);
    dict.SetIntValue("number_of_pages", page.number_of_pages);
    vector<Article> articles = get_articles(page.limit, page.skip, tag);
    int i = 0;
    for (; i < articles.size(); ++i) {
        Article article = articles[i];
        parse_article(&article);
        TemplateDictionary* article_dict =
         dict.AddSectionDictionary("articles");
        set_article_dict(article_dict, &article);
    }
    if (i == 0) {
        this->on404();
        return true;
    } else {
        dict.ShowSection("articles");
    }
    global.theme.set_template_dict("tag", &dict);
    this->render("tag", &dict);
    return true;
}