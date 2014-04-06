#include <stdio.h>
#include <string>
#include <sstream>
#include <markdown.h>
#include <buffer.h>
#include <html.h>
#include "global.h"
#include "theme.h"
#include "plugin.h"
#include "article.h"

bool MarkdownParser(Article* article, bool is_short);

extern "C" void load(Global* global) {
    global->article.set_article_parser(MarkdownParser);
}

bool MarkdownParser(Article* article, bool is_short) {
    using namespace std;
    if (!article) {
        return false;
    }
    string content;
    if (!is_short) {
        content = article->content;
    } else {
        if (article->content.length() <= 500) {
            content = article->content;
        } else {
            content = article->content.substr(0, 500);
        }
    }
    const int OUTPUT_UNIT = 64;
    if (content == "") {
        return false;
    }
    struct buf* ob = bufnew(OUTPUT_UNIT);
    struct sd_callbacks callbacks;
	struct html_renderopt options;
	struct sd_markdown* markdown;
    sdhtml_renderer(&callbacks, &options, 0);
    markdown = sd_markdown_new(0, 16, &callbacks, &options);
    sd_markdown_render(ob, (const uint8_t*)content.c_str(), content.length(),
                       markdown);
    sd_markdown_free(markdown);
    if (ob->size <= 0) {
        return false;
    }
    string output((const char*)ob->data, ob->size);
	bufrelease(ob);
    article->content = output;
    return true;
}

extern "C" void destory() {
}