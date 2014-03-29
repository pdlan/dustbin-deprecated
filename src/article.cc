#include <math.h>
#include <string>
#include <vector>
#include <ctemplate/template.h>
#include <markdown.h>
#include <buffer.h>
#include <html.h>
#include "global.h"
#include "article.h"

extern Global global;

PageInfo page_articles(int current_page, int articles_per_page,
                       std::string tag) {
    using namespace mongo;
    PageInfo info;
    int articles_count;
    if (tag != "") {
        articles_count = global.db_conn.count(global.db_name + ".article",
                                              BSON("tag" << tag));
    } else {
        articles_count = global.db_conn.count(global.db_name + ".article",
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

std::vector<Article> get_articles(int limit, int skip, std::string tag) {
    using namespace std;
    using namespace mongo;
    auto_ptr<DBClientCursor> cursor;
    vector<Article> articles;
    if (tag == "") {
        Query qu = Query();
        cursor = global.db_conn.query(global.db_name + ".article",
                                      qu.sort("time", -1), limit, skip);
    } else {
        Query qu = QUERY("tag" << tag);
        cursor = global.db_conn.query(global.db_name + ".article",
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

bool get_one_article(std::string id, Article* article) {
    using namespace std;
    using namespace mongo;
    if (!article) {
        return false;
    }
    BSONObj p = global.db_conn.findOne(global.db_name + ".article",
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

void set_article_dict(ctemplate::TemplateDictionary* dict,
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

void parse_article(Article* article) {
    using namespace std;
    if (!article) {
        return;
    }
    string content = article->content;
    string content_parsed = parse_content(content);
    article->content = content_parsed;
}