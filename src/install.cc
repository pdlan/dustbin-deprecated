#include <time.h>
#include <iostream>
#include <mongo/client/dbclient.h>
#include "global.h"
#include "install.h"

extern Global global;

void Install::install() {
    //TODO
    using namespace std;
    using namespace mongo;
    cout << "Please input your username:";
    string username;
    getline(cin, username);
    cout << "Please input your password:";
    string password;
    getline(cin, password);
    cout << "Please input the name of your site:";
    string site_name;
    getline(cin, site_name);
    cout << "Please input the url of your site:";
    string site_url;
    getline(cin, site_url);
    cout << "Please input the description of your site:";
    string site_description;
    getline(cin, site_description);
    cout << "Please input the theme you want to use:";
    string theme;
    getline(cin, theme);
    cout << "Please input the commenting system you want to use(disqus, duoshuo):";
    string comment;
    getline(cin, comment);
    cout << "Installing...\n";
    global.set_setting("site-name", site_name);
    global.set_setting("site-description", site_description);
    global.set_setting("site-url", site_url);
    global.set_setting("theme", theme);
    global.set_setting("commenting-system", comment);
    string password_encoded = Auth::SHA256(password);
    global.db_conn.insert(global.db_name + ".user",
                          BSON("username" << username << 
                               "password" << password_encoded));
    string content = "Congratulation! You have installed Dustbin!";
    BSONArrayBuilder tags;
    tags.append("hello");
    global.db_conn.insert(global.db_name + ".article", 
                          BSON("id" << "Hello"
                               << "content" << content
                               << "tag" << tags.arr()
                               << "title" << "Hello"
                               << "time" << (int)time(0)));
    cout << "Finished.\n";
}