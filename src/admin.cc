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
#include "global.h"

extern Global global;

bool AdminLoginHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    if (global.auth.login(this->conn) == Auth::success) {
        string location = this->get_argument("redirect");
        if (location != "") {
            this->redirect("/admin/" + location + "/");
        } else {
            this->redirect("/admin/");
        }
    } else {
        this->set_header("Content-Type", "text/html");
        TemplateDictionary dict("login");
        global.theme.set_template_dict("login", &dict, true);
        dict.ShowSection("failed");
        this->render("login", &dict, true);
    }
    return true;
}

bool AdminLoginHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    string action = this->get_regex_result(1);
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("login");
    global.theme.set_template_dict("login", &dict, true);
    this->render("login", &dict, true);
    return true;
}

bool AdminLogoutHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    if (global.auth.auth(this->conn) != Auth::success) {
        return true;
    }
    global.auth.logout(this->conn);
    return true;
}

bool AdminIndexHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    if (global.auth.auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("index");
    global.theme.set_template_dict("index", &dict, true);
    this->render("index", &dict, true);
    return true;
}

bool AdminThemeHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    if (global.auth.auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=theme/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("theme");
    global.theme.set_template_dict("theme", &dict, true);
    this->render("theme", &dict, true);
    return true;
}

bool AdminThemeHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    Json::Value response;
    Json::FastWriter writer;
    string action = this->get_post_argument("action");
    string theme = this->get_post_argument("theme");
    this->set_header("Content-Type", "text/javascript");
    if (global.auth.auth(this->conn) != Auth::success) {
        response["status"] = "failed";
        response["reason"] = "forbidden";
        this->write(writer.write(response));
        return true;
    }
    if (action == "enable") {
        if (global.theme.set_theme(theme)) {
            response["status"] = "success";
            global.db_conn.update(global.db_name + ".article", 
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
    if (global.auth.auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=article/list/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("article");
    global.theme.set_template_dict("article", &dict, true);
    string action = this->get_regex_result(1);
    if (action == "list") {
        Query qu = Query();
        auto_ptr<DBClientCursor> cursor = 
         global.db_conn.query(global.db_name + ".article", qu.sort("time", -1));
        while (cursor->more()) {
            BSONObj p = cursor->next();
            string id = p.getStringField("id");
            string title = p.getStringField("title");
            time_t timestamp = p.getIntField("time");
            string time_format = "%b %d, %Y";
            string date = DustbinHandler::format_time(time_format, timestamp);
            TemplateDictionary* article = dict.AddSectionDictionary("articles");
            article->SetValue("id", id);
            article->SetValue("title", title);
            article->SetValue("date", date);
            article->ShowSection("articles");
    }
        this->render("list-articles", &dict, true);
    } else if (action == "new") {
        dict.ShowSection("new");
        this->render("new-edit-article", &dict, true);
    } else if (action == "edit") {
        string id = this->get_regex_result(2);
        BSONObj p = global.db_conn.findOne(global.db_name + ".article", 
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
        dict.ShowSection("edit");
        this->render("new-edit-article", &dict, true);
    }
    return true;
}

bool AdminArticleHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    using namespace boost;
    if (global.auth.auth(this->conn) != Auth::success) {
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
        BSONObj p = global.db_conn.findOne(global.db_name + ".article", 
                                           QUERY("id" << id.str()));
        //Generate different id for the articles having the same title.
        for (int i = 1; !p.isEmpty(); ++ i) {
            id.str("");
            id << title << "-" << i;
            p = global.db_conn.findOne(global.db_name + ".article", 
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
        global.db_conn.insert(global.db_name + ".article", 
                              BSON("id" << id.str()
                                   << "content" << content
                                   << "tag" << tags.arr()
                                   << "title" << title
                                   << "time" << (int)timestamp));
        this->redirect("/article/" + id.str() + "/");
    } else if (action == "edit") {
        string id = this->get_regex_result(2);
        BSONObj p = global.db_conn.findOne(global.db_name + ".article", 
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
        global.db_conn.update(global.db_name + ".article", 
                              QUERY("id" << id), 
                              BSON("id" << id 
                                   << "content" << content
                                   << "tag" << tags.arr()
                                   << "title" << title
                                   << "time" << (int)timestamp));
        this->redirect("/article/" + id + "/");
    }
    return true;
}

bool AdminSettingHandler::get() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    if (global.auth.auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=setting/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("setting");
    global.theme.set_template_dict("setting", &dict, true);
    this->render("setting", &dict, true);
    return true;
}

bool AdminSettingHandler::post() {
    using namespace std;
    using namespace mongo;
    using namespace ctemplate;
    using namespace boost;
    if (global.auth.auth(this->conn) != Auth::success) {
        this->redirect("/admin/user/login/?redirect=setting/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("setting");
    string site_name = this->get_post_argument("sitename");
    string site_url = this->get_post_argument("siteurl");
    string site_description = this->get_post_argument("sitedescription");
    map<string, string> settings;
    settings["site-name"] = site_name;
    settings["site_url"] = site_url;
    settings["site-description"] = site_description;
    for (map<string, string>::iterator it = settings.begin();
         it != settings.end(); ++ it) {
        string key = it->first;
        string value = it->second;
        global.db_conn.update(global.db_name + ".setting", 
                              QUERY("key" << key), 
                              BSON("key" << key << "value" << value));
    }
    global.theme.set_template_dict("setting", &dict, true);
    this->render("setting", &dict, true);
    return true;
}