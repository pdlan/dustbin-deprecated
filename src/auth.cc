#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <ctemplate/template.h>
#include <openssl/sha.h>
#include "auth.h"
#include "dustbin.h"

Auth::Auth() {
}

int Auth::login(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    DBClientConnection* db_conn = dustbin->get_db_conn();
    string db_name = dustbin->get_db_name();
    string username = conn->req->get_post_argument("username");
    string password = conn->req->get_post_argument("password");
    string pw_encoded = Auth::SHA256(password);
    string ip = conn->req->get_ip();
    BSONObj p = db_conn->findOne(db_name + ".user", 
                QUERY("username" << username << "password" << pw_encoded));
    db_conn->remove(db_name + ".session", 
                    QUERY("username" << username << "ip" << ip));
    if (p.isEmpty()) {
        return Auth::failed;
    }
    srand((int)time(0));
    ostringstream sessionid;
    sessionid << username << rand();
    string sessionid_encoded = Auth::SHA256(sessionid.str());
    ostringstream token;
    token << username << rand();
    string token_encoded = Auth::SHA256(token.str());
    db_conn->insert(db_name + ".session", 
                    BSON("username" << username <<
                         "token" << token_encoded <<
                         "sessionid" << sessionid_encoded <<
                         "ip" << ip));
    conn->resp->set_cookie("username", username, "", "/admin");
    conn->resp->set_cookie("sessionid", sessionid_encoded, "", "/admin");
    conn->resp->set_cookie("token", token_encoded, "", "/admin");
    return Auth::success;
}

int Auth::logout(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    DBClientConnection* db_conn = dustbin->get_db_conn();
    string db_name = dustbin->get_db_name();
    if (this->auth(conn) != Auth::success) {
        return Auth::failed;
    }
    string username = conn->req->get_cookie("username");
    string ip = conn->req->get_ip();
    db_conn->remove(db_name + ".session", 
                    QUERY("username" << username << "ip" << ip));
    return Auth::success;
}

int Auth::auth(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    DBClientConnection* db_conn = dustbin->get_db_conn();
    string db_name = dustbin->get_db_name();
    string username = conn->req->get_cookie("username");
    string token_cookie = conn->req->get_cookie("token");
    string sessionid_cookie = conn->req->get_cookie("sessionid");
    string ip = conn->req->get_ip();
    BSONObj p = db_conn->findOne(db_name + ".session", 
                QUERY("username" << username <<
                      "token" << token_cookie <<
                      "sessionid" << sessionid_cookie <<
                      "ip" << ip));
    if (p.isEmpty()) {
        return Auth::failed;
    }
    ostringstream new_token;
    srand((int)time(0));
    new_token << username << rand();
    string token_encoded = Auth::SHA256(new_token.str());
    db_conn->update(db_name + ".session", 
                    QUERY("username" << username << "ip" << ip), 
                    BSON("username" << username <<
                         "token" << token_encoded <<
                         "sessionid" << sessionid_cookie <<
                         "ip" << ip));
    conn->resp->set_cookie("token", token_encoded, "", "/admin");
    return Auth::success;
}

std::string Auth::SHA256(std::string str) {
    using namespace std;
    SHA256_CTX context;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    ostringstream buf;
    SHA256_Init(&context);
    SHA256_Update(&context, str.c_str(), str.length());
    SHA256_Final(hash, &context);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++ i) {
        buf << setiosflags(ios::uppercase) 
            << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return buf.str();
}