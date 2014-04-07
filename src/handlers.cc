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
#include "theme.h"
#include "plugin.h"
#include "article.h"
#include "dustbin.h"

void DustbinHandler::render(std::string template_name,
                            ctemplate::TemplateDictionary* dict) {
    using namespace std;
    using namespace ctemplate;
    Theme* theme = Dustbin::instance()->get_theme();
    string output;
    theme->render(template_name, &output, dict);
    this->write(output);
}

void DustbinHandler::on404()
{
    using namespace ctemplate;
    Theme* theme = Dustbin::instance()->get_theme();
    this->set_header("Content-Type", "text/html");
    this->conn->resp->status_code = 404;
    this->conn->resp->status_text = "Not Found";
    TemplateDictionary dict("404");
    theme->set_template_dict("404", &dict);
    this->render("404", &dict);
}

bool PageHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    Theme* theme = dustbin->get_theme();
    ArticleManager* article_manager = dustbin->get_article();
    PluginManager* plugin = dustbin->get_plugin();
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("page");
    string page_str = this->get_regex_result(1);
    int current_page = 1;
    if (page_str != "") {
        current_page = atoi(page_str.c_str());
    }
    const Json::Value* config = theme->get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("page", 20).asInt();
    }
    PageInfo page =
        article_manager->page_articles(current_page, articles_per_page);
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
    vector<Article> articles =
        article_manager->get_articles(page.limit, page.skip);
    for (vector<Article>::iterator it = articles.begin();
         it != articles.end(); ++it) {
        Article article = *it;
        article_manager->parse_article(&article, true);
        TemplateDictionary* article_dict =
        dict.AddSectionDictionary("articles");
        article_manager->set_article_dict(article_dict, &article);
    }
    dict.ShowSection("articles");
    Json::Value args, articles_json, page_json, result;
    article_manager->articles_to_json(&articles, &articles_json);
    page_json["number_of_pages"] = page.number_of_pages;
    page_json["current_page"] = current_page;
    page_json["articles_per_page"] = articles_per_page;
    args["articles"] = articles_json;
    args["pageinfo"] = page_json;
    plugin->call_hooks("on_show_page", &args, &result);
    theme->set_template_dict("page", &dict);
    this->render("page", &dict);
    return true;
}

bool ArticleHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    Theme* theme = dustbin->get_theme();
    ArticleManager* article_manager = dustbin->get_article();
    PluginManager* plugin = dustbin->get_plugin();
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("article");
    string id = this->get_regex_result(1);
    Article article;
    if (!article_manager->get_one_article(id, &article)) {
        this->on404();
        return true;
    }
    article_manager->parse_article(&article);
    article_manager->set_article_dict(&dict, &article);
    theme->set_template_dict("article", &dict);
    Json::Value article_json, result;
    article_manager->article_to_json(&article, &article_json);
    plugin->call_hooks("on_show_article", &article_json, &result);
    this->render("article", &dict);
    return true;
}

bool ArchivesHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    Theme* theme = dustbin->get_theme();
    ArticleManager* article_manager = dustbin->get_article();
    PluginManager* plugin = dustbin->get_plugin();
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("archives");
    string page_str = this->get_regex_result(1);
    int current_page = 1;
    if (page_str != "") {
        current_page = atoi(page_str.c_str());
    }
    const Json::Value* config = theme->get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("archives", 20).asInt();
    }
    PageInfo page =
        article_manager->page_articles(current_page, articles_per_page);
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
    vector<Article> articles =
        article_manager->get_articles(page.limit, page.skip);
    for (int i = 0, j = 0; i < articles.size(); ++i) {
        Article article = articles[i];
        article_manager->parse_article(&article, true);
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
        article_manager->set_article_dict(article_dict, &article);
    }
    dict.ShowSection("articles");
    Json::Value args, articles_json, page_json, result;
    article_manager->articles_to_json(&articles, &articles_json);
    page_json["number_of_pages"] = page.number_of_pages;
    page_json["current_page"] = current_page;
    page_json["articles_per_page"] = articles_per_page;
    args["articles"] = articles_json;
    args["pageinfo"] = page_json;
    plugin->call_hooks("on_show_archive", &args, &result);
    theme->set_template_dict("archives", &dict);
    this->render("archives", &dict);
    return true;
}

bool TagHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    Theme* theme = dustbin->get_theme();
    ArticleManager* article_manager = dustbin->get_article();
    PluginManager* plugin = dustbin->get_plugin();
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
    const Json::Value* config = theme->get_config();
    int articles_per_page = 20;
    if (config->get("articles-per-page", 0).isObject()) {
        Json::Value obj = (*config)["articles-per-page"];
        articles_per_page = obj.get("tag", 20).asInt();
    }
    PageInfo page =
        article_manager->page_articles(current_page, articles_per_page, tag);
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
    vector<Article> articles =
        article_manager->get_articles(page.limit, page.skip, tag);
    int i = 0;
    for (; i < articles.size(); ++i) {
        Article article = articles[i];
        article_manager->parse_article(&article, true);
        TemplateDictionary* article_dict =
        dict.AddSectionDictionary("articles");
        article_manager->set_article_dict(article_dict, &article);
    }
    if (i == 0) {
        this->on404();
        return true;
    } else {
        dict.ShowSection("articles");
    }
    Json::Value args, articles_json, page_json, result;
    article_manager->articles_to_json(&articles, &articles_json);
    page_json["number_of_pages"] = page.number_of_pages;
    page_json["current_page"] = current_page;
    page_json["articles_per_page"] = articles_per_page;
    args["articles"] = articles_json;
    args["pageinfo"] = page_json;
    args["tag"] = tag;
    plugin->call_hooks("on_show_tag", &args, &result);
    theme->set_template_dict("tag", &dict);
    this->render("tag", &dict);
    return true;
}