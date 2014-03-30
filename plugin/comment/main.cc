#include <stdio.h>
#include <string>
#include <sstream>
#include "global.h"
#include "theme.h"

class CommentBlockHandler : public BlockHandler{
  public:
    std::string handle();
    inline std::string get_block_name() {return "comment";}
    inline std::string get_template_name() {return "article";}
};

mongo::DBClientConnection* db_conn;
std::string db_name;
Setting* setting;
CommentBlockHandler comment_block_handler;

std::string CommentBlockHandler::handle() {
    using namespace std;
    using namespace mongo;
    string type = setting->get_str_setting("comment-type");
    ostringstream buf;
    buf.str("");
    if (type == "duoshuo") {
        string short_name =
            setting->get_str_setting("comment-duoshuo-shortname");
        buf
        << "<!-- Duoshuo Comment BEGIN -->" << endl
        << "<div class=\"ds-thread\"></div>" << endl
        << "<script type=\"text/javascript\">" << endl
        << "var duoshuoQuery = {short_name:\""
        << short_name
        << "\"};" << endl
        << "(function() {" << endl
		<< "var ds = document.createElement('script');" << endl
		<< "ds.type = 'text/javascript';ds.async = true;" << endl
		<< "ds.src = 'http://static.duoshuo.com/embed.js';" << endl
		<< "ds.charset = 'UTF-8';" << endl
		<< "(document.getElementsByTagName('head')[0]" << endl
		<< "|| document.getElementsByTagName('body')[0]).appendChild(ds);" << endl
        << "})();" << endl
        << "</script>" << endl
        << "<!-- Duoshuo Comment END -->" << endl;
    } else if (type == "disqus") {
        string short_name =
            setting->get_str_setting("comment-disqus-shortname");
        buf
        << "<script type=\"text/javascript\">" << endl
        << "var disqus_title = \"\";" << endl
        << "(function() {" << endl
        << "var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;" << endl
        << "var shortname = \""
        << short_name
        << "\";" << endl
        << "dsq.src = 'http://' + shortname + '.disqus.com/embed.js';" << endl
        << "(document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);" << endl
        << "})();" << endl
        << "</script>" << endl;
    }
    return buf.str();
}

extern "C" void load(Global* global) {
    if (!global->theme.add_block(&comment_block_handler)) {
        printf("[comment]Unable to add comment block.\n");
        return;
    }
    db_conn = &global->db_conn;
    db_name = global->db_name;
    setting = &global->setting;
}

extern "C" void destory() {
}