#include <math.h>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>
#include <ctemplate/template.h>
#include "dustbin.h"
#include "article.h"

void ArticleManager::initialize() {
    this->article_parser = NULL;
}

PageInfo ArticleManager::page_articles(int current_page, int articles_per_page,
                                       std::string tag) {
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    PageInfo info;
    int articles_count;
    if (tag != "") {
        articles_count =
            dustbin->get_db_conn()->count(dustbin->get_db_name() + ".article",
                                         BSON("tag" << tag));
    } else {
        articles_count =
            dustbin->get_db_conn()->count(dustbin->get_db_name() + ".article",
                                          BSONObj());
    }
    if (articles_count == 0) {
        info.limit = 0;
        info.skip = 0;
        int number_of_pages = 0;
    }
    if (articles_per_page != 0) {
        info.limit = articles_per_page;
        info.skip = articles_per_page * (current_page - 1);
        info.number_of_pages = ceil(double(articles_count) / articles_per_page);
    } else {
        info.limit = articles_count;
        info.skip = 0;
        info.number_of_pages = 1;
    }
    return info;
}

std::vector<Article> ArticleManager::get_articles(int limit, int skip,
                                                  std::string tag) {
    using namespace std;
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    auto_ptr<DBClientCursor> cursor;
    vector<Article> articles;
    if (tag == "") {
        Query qu = Query();
        cursor =
            dustbin->get_db_conn()->query(dustbin->get_db_name() + ".article",
                                          qu.sort("time", -1), limit, skip);
    } else {
        Query qu = QUERY("tag" << tag);
        cursor =
            dustbin->get_db_conn()->query(dustbin->get_db_name() + ".article",
                                          qu.sort("time", -1), limit, skip);
    }
    while (cursor->more()) {
        BSONObj p = cursor->next();
        string id = p.getStringField("id");
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        time_t timestamp = p.getIntField("time");
        if (id == "" || title == "" || content == "") {
            continue;
        }
        Article article;
        article.id = id;
        article.title = title;
        article.content = content;
        article.timestamp = timestamp;
        BSONForEach(e, p.getObjectField("tag")) {
            string tag = e.String();
            if (tag == "") {
                continue;
            }
            article.tags.push_back(tag);
        }
        articles.push_back(article);
    }
    return articles;
}

bool ArticleManager::get_one_article(std::string id, Article* article) {
    using namespace std;
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    if (!article) {
        return false;
    }
    BSONObj p =
        dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article",
                                        QUERY("id" << id));
    if (p.isEmpty()) {
        return false;
    }
    string title = p.getStringField("title");
    string content = p.getStringField("content");
    time_t timestamp = p.getIntField("time");
    if (id == "" || title == "" || content == "") {
        return false;
    }
    article->id = id;
    article->title = title;
    article->content = content;
    article->timestamp = timestamp;
    BSONForEach(e, p.getObjectField("tag")) {
        string tag = e.String();
        if (tag == "") {
            continue;
        }
        article->tags.push_back(tag);
    }
    return true;
}

void ArticleManager::set_article_dict(ctemplate::TemplateDictionary* dict,
                      const Article* article) {
    using namespace std;
    using namespace ctemplate;
    if (!article || !dict) {
        return;
    }
    dict->SetValue("id", article->id);
    dict->SetValue("content", article->content);
    dict->SetValue("title", article->title);
    dict->SetIntValue("date", article->timestamp);
    for (vector<string>::const_iterator it = article->tags.begin();
         it != article->tags.end(); ++it) {
        TemplateDictionary* tag_dict = dict->AddSectionDictionary("tags");
        tag_dict->SetValue("name", *it);
    }
    dict->ShowSection("tags");
}

bool ArticleManager::parse_article(Article* article, bool is_short) {
    using namespace std;
    if (!article || !this->article_parser) {
        return false;
    }
    this->article_parser(article, is_short);
    return true;
}

bool ArticleManager::article_to_json(const Article* article,
                                     Json::Value* json) {
    if (json == NULL || article == NULL) {
        return false;
    }
    if (article->id == "" || article->title == "" || article->content == "") {
        return false;
    }
    (*json)["id"] = article->id;
    (*json)["title"] = article->title;
    (*json)["contnet"] = article->content;
    Json::Value tags;
    for (std::vector<std::string>::const_iterator it = article->tags.begin();
         it != article->tags.end(); ++it) {
        std::string tag = *it;
        tags.append(tag);
    }
    (*json)["tags"] = tags;
    return true;
}

bool ArticleManager::articles_to_json(const std::vector<Article>* articles,
                                      Json::Value* json) {
    if (articles == NULL || json == NULL) {
        return false;
    }
    for (std::vector<Article>::const_iterator it = articles->begin();
         it != articles->end(); ++it) {
        Article article = *it;
        Json::Value article_json;
        this->article_to_json(&article, &article_json);
        json->append(article_json);
    }
    return false;
}

bool ArticleManager::set_article_parser(ArticleParser parser) {
    if (parser == NULL || this->article_parser != NULL) {
        return false;
    }
    this->article_parser = parser;
    return true;
}