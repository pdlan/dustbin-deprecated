#include <string>
#include <recycled.h>
#include <mongo/client/dbclient.h>
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
        this->redirect("/admin/login/");
        return true;
    }
    this->set_header("Content-Type", "text/html");
    TemplateDictionary dict("index");
    global.theme.set_template_dict("index", &dict, true);
    this->render("index", &dict, true);
    return true;
}