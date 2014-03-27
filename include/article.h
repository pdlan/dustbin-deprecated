#ifndef DUSTBIN_INCLUDE_ARTICLE_H
#define DUSTBIN_INCLUDE_ARTICLE_H
#include <string>
#include <vector>

struct Article {
    std::string id;
    std::string title;
    std::string content;
    int timestamp;
    std::vector<std::string> tags;
};

struct PageInfo {
    int number_of_pages;
    int limit;
    int skip;
};

PageInfo page_articles(int current_page, int articles_per_page, 
                       std::string tag = "");
std::vector<Article> get_articles(int limit, int skip, std::string tag = "");
void set_article_dict(ctemplate::TemplateDictionary* dict, 
                      const Article* article);
bool get_one_article(std::string id, Article* article);
std::string parse_content(std::string content);
void parse_article(Article* article);

#endif