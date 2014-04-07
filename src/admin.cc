#include <time.h>
#include <string>
#include <map>
#include <sstream>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <jsoncpp/json/json.h>
#include <boost/regex.hpp>
#include <ctemplate/template.h> 
#include "admin.h"
#include "handlers.h"
#include "auth.h"
#include "theme.h"
#include "setting.h"
#include "dustbin.h"

bool AdminLoginHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->login(this->conn) == Auth::success) {
        string location = this->get_argument("redirect");
        if (location != "") {
            this->redirect("/admin/" + location + "/");
        } else {
            this->redirect("/admin/");
        }
    } else {
        this->set_header("Content-Type", "text/html");
        TemplateDictionary dict("login");
        dustbin->get_theme()->set_template_dict("login", &dict, true);
        dict.ShowSection("failed");
        this->render("login", &dict, true);
    }
    return true;
}

bool AdminLoginHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    string action = this->get_regex_result(1);
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("login");
    dustbin->get_theme()->set_template_dict("login", &dict, true);
    this->render("login", &dict, true);
    return true;
}

bool AdminLogoutHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        return true;
    }
    dustbin->get_auth()->logout(this->conn);
    return true;
}

bool AdminIndexHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("index");
    dustbin->get_theme()->set_template_dict("index", &dict, true);
    this->render("index", &dict, true);
    return true;
}

bool AdminThemeHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=theme/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    dustbin->get_theme()->refresh();
    TemplateDictionary dict("theme");
    dustbin->get_theme()->set_template_dict("theme", &dict, true);
    const vector<ThemeInfo>* themes = dustbin->get_theme()->get_themes_info();
    for (vector<ThemeInfo>::const_iterator it = themes->begin();
         it != themes->end(); ++ it) {
        ThemeInfo info = *it;
        TemplateDictionary* themes = dict.AddSectionDictionary("themes");
        themes->SetValue("name", info.name);
        themes->SetValue("author", info.author);
        dict.ShowSection("themes");
    }
    this->render("theme", &dict, true);
    return true;
}

bool AdminThemeHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    Json::Value response;
    Json::FastWriter writer;
    string action = this->get_post_argument("action");
    string theme = this->get_post_argument("theme");
    this->set_header("Content-Type", "text/javascript");
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        response["status"] = "failed";
        response["reason"] = "forbidden";
        this->write(writer.write(response));
        return true;
    }
    if (action == "enable") {
        if (dustbin->get_theme()->set_theme(theme)) {
            response["status"] = "success";
            dustbin->get_db_conn()->update(dustbin->get_db_name() + ".setting",
                                  QUERY("key" << "theme"),
                                  BSON("key" << "theme" << "value" << theme));
            this->write(writer.write(response));
        } else {
            response["status"] = "failed";
            response["reason"] = "notexist";
            this->write(writer.write(response));
        }
    }
    return true;
}

bool AdminArticleHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=article/list/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("article");
    dustbin->get_theme()->set_template_dict("article", &dict, true);
    string action = this->get_regex_result(1);
    if (action == "list") {
        Query qu = Query();
        auto_ptr<DBClientCursor> cursor = 
         dustbin->get_db_conn()->query(dustbin->get_db_name() + ".article",
                                       qu.sort("time", -1));
        while (cursor->more()) {
            BSONObj p = cursor->next();
            string id = p.getStringField("id");
            string title = p.getStringField("title");
            time_t timestamp = p.getIntField("time");
            string time_format = "%b %d, %Y";
            char buffer[256];
            tm* timeinfo = localtime(&timestamp);
            strftime(buffer, 256, time_format.c_str(), timeinfo);
            string date = buffer;
            TemplateDictionary* article = dict.AddSectionDictionary("articles");
            article->SetValue("id", id);
            article->SetValue("title", title);
            article->SetValue("date", date);
            article->ShowSection("articles");
    }
        this->render("list-articles", &dict, true);
    } else if (action == "new") {
        this->render("new-article", &dict, true);
    } else if (action == "edit") {
        string id = this->get_regex_result(2);
        BSONObj p =
            dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article", 
                                            QUERY("id" << id));
        if (p.isEmpty()) {
            this->on404();
            return true;
        }
        string title = p.getStringField("title");
        string content = p.getStringField("content");
        string tags;
        BSONForEach(e, p.getObjectField("tag")) {
            string tag = e.String();
            tags += "," + tag;
        }
        if (tags != "") {
            tags = tags.substr(1, tags.length() - 1);
        }
        time_t timestamp = p.getIntField("time");
        dict.SetValue("id", id);
        dict.SetValue("title", title);
        dict.SetValue("content", content);
        dict.SetValue("tags", tags);
        this->render("edit-article", &dict, true);
    } else if (action == "delete") {
        Json::Value response;
        Json::FastWriter writer;
        string id = this->get_regex_result(2);
        BSONObj p =
            dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article", 
                                            QUERY("id" << id));
        if (p.isEmpty()) {
            response["status"] = "failed";
            response["reson"] = "notexist";
        } else {
            response["status"] = "success";
            dustbin->get_db_conn()->remove(dustbin->get_db_name() + ".article",
                                  QUERY("id" << id));
        }
        this->write(writer.write(response));
    }
    return true;
}

bool AdminArticleHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    using namespace boost;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=article/list/");
        return true;
    }
    string action = this->get_regex_result(1);
    if (action == "new") {
        time_t timestamp = time(0);
        string title = this->get_post_argument("title");
        string content = this->get_post_argument("content");
        string tag_str = this->get_post_argument("tags");
        ostringstream id;
        id << title;
        BSONObj p =
            dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article", 
                                            QUERY("id" << id.str()));
        //Generate different id for the articles having the same title.
        for (int i = 1; !p.isEmpty(); ++ i) {
            id.str("");
            id << title << "-" << i;
            p = dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article", 
                                                QUERY("id" << id.str()));
        }
        regex re(",");
        sregex_token_iterator rit(tag_str.begin(), tag_str.end(), re, -1);
        sregex_token_iterator end;
        BSONArrayBuilder tags;
        for (int i = 0; rit != end; ++ i) {
            string tag = *rit++;
            tags.append(tag);
        }
        dustbin->get_db_conn()->insert(dustbin->get_db_name() + ".article", 
                              BSON("id" << id.str()
                                   << "content" << content
                                   << "tag" << tags.arr()
                                   << "title" << title
                                   << "time" << (int)timestamp));
        this->redirect("/article/" + id.str() + "/");
    } else if (action == "edit") {
        string id = this->get_regex_result(2);
        BSONObj p =
            dustbin->get_db_conn()->findOne(dustbin->get_db_name() + ".article", 
                                           QUERY("id" << id));
        if (p.isEmpty()) {
            this->on404();
            return true;
        }
        time_t timestamp = p.getIntField("time");
        string title = this->get_post_argument("title");
        string content = this->get_post_argument("content");
        string tag_str = this->get_post_argument("tags");
        regex re(",");
        sregex_token_iterator rit(tag_str.begin(), tag_str.end(), re, -1);
        sregex_token_iterator end;
        BSONArrayBuilder tags;
        for (int i = 0; rit != end; ++ i) {
            string tag = *rit++;
            tags.append(tag);
        }
        dustbin->get_db_conn()->update(dustbin->get_db_name() + ".article", 
                                       QUERY("id" << id), 
                                       BSON("id" << id <<
                                            "content" << content <<
                                            "tag" << tags.arr() <<
                                            "title" << title <<
                                            "time" << (int)timestamp));
        this->redirect("/article/" + id + "/");
    }
    return true;
}

bool AdminSettingHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=setting/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("setting");
    dustbin->get_theme()->set_template_dict("setting", &dict, true);
    dict.SetValue("comment",
                  dustbin->get_setting()->get_str_setting("commenting-system"));
    this->render("setting", &dict, true);
    return true;
}

bool AdminSettingHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    using namespace boost;
    Dustbin* dustbin = Dustbin::instance();
    if (dustbin->get_auth()->auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=setting/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("setting");
    string site_name = this->get_post_argument("sitename");
    string site_url = this->get_post_argument("siteurl");
    string site_description = this->get_post_argument("sitedescription");
    string comment = this->get_post_argument("comment");
    dustbin->get_setting()->set_setting("site-name", site_name);
    dustbin->get_setting()->set_setting("site-url", site_url);
    dustbin->get_setting()->set_setting("site-description", site_description);
    dustbin->get_setting()->set_setting("commenting-system", comment);
    dustbin->get_theme()->set_template_dict("setting", &dict, true);
    dict.SetValue("comment", comment);
    this->render("setting", &dict, true);
    return true;
}