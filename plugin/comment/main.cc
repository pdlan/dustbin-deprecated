#include <stdio.h>
#include <string>
#include "global.h"
#include "theme.h"

class CommentBlockHandler : public BlockHandler{
  public:
    std::string handle();
    inline std::string get_block_name() {return "comment";}
    inline std::string get_template_name() {return "article";}
};

std::string CommentBlockHandler::handle() {
    return "test";
}

CommentBlockHandler comment_block_handler;

extern "C" void load(Global* global) {
    if (!global->theme.add_block(&comment_block_handler)) {
        printf("[comment]Unable to add comment block.\n");
        return;
    }
}

extern "C" void destory() {
}