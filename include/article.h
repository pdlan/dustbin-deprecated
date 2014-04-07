#ifndef DUSTBIN_INCLUDE_ARTICLE_H
#define DUSTBIN_INCLUDE_ARTICLE_H
#include <string>
#include <vector>
#include <ctemplate/template.h>
#include <jsoncpp/json/json.h>
struct Article {
    std::string id;
    std::string title;
    std::string content;
    time_t timestamp;
    std::vector<std::string> tags;
};

struct PageInfo {
    int number_of_pages;
    int limit;
    int skip;
};

typedef bool (*ArticleParser)(Article*, bool);

class ArticleManager {
  public:
    void initialize();
    PageInfo page_articles(int current_page, int articles_per_page,
                           std::string tag = "");
    std::vector<Article> get_articles(int limit, int skip,
                                      std::string tag = "");
    void set_article_dict(ctemplate::TemplateDictionary* dict,
                          const Article* article);
    bool get_one_article(std::string id, Article* article);
    bool parse_article(Article* article, bool is_short = false);
    bool article_to_json(const Article* article, Json::Value* json);
    bool articles_to_json(const std::vector<Article>* articles,
                          Json::Value* json);
    bool set_article_parser(ArticleParser parser);
  private:
    ArticleParser article_parser;
};
#endif