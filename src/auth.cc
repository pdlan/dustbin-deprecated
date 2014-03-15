#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <recycled.h>
#include <mongo/client/dbclient.h>
#include <ctemplate/template.h>
#include <openssl/sha.h>
#include "global.h"

void Auth::set_db_config(mongo::DBClientConnection* db_conn, 
                         std::string db_name) {
    this->db_conn = db_conn;
    this->db_name = db_name;
}

int Auth::login(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    string username = conn->req->get_post_argument("username");
    string password = conn->req->get_post_argument("password");
    string pw_encoded = Auth::SHA256(password);
    BSONObj p = this->db_conn->findOne(this->db_name + ".user", 
                QUERY("username" << username << "password" << pw_encoded));
    this->db_conn->remove(this->db_name + ".session", 
                          QUERY("username" << username));
    if (p.isEmpty()) {
        return Auth::failed;
    }
    srand((int)time(0));
    ostringstream serial;
    serial << username << rand();
    string serial_encoded = Auth::SHA256(serial.str());
    ostringstream token;
    token << username << rand();
    string token_encoded = Auth::SHA256(token.str());
    this->db_conn->insert(this->db_name + ".session", 
                          BSON("username" << username 
                               << "token" << token_encoded
                               << "serial" << serial_encoded));
    conn->resp->set_cookie("username", username, "", "/admin");
    conn->resp->set_cookie("serial", serial_encoded, "", "/admin");
    conn->resp->set_cookie("token", token_encoded, "", "/admin");
    return Auth::success;
}

int Auth::logout(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    if (this->auth(conn) != Auth::success) {
        return Auth::failed;
    }
    string username = conn->req->get_cookie("username");
    this->db_conn->remove(this->db_name + ".session", 
                          QUERY("username" << username));
    return Auth::success;
}

int Auth::auth(recycled::Connection* conn) {
    using namespace std;
    using namespace mongo;
    string username = conn->req->get_cookie("username");
    string token_cookie = conn->req->get_cookie("token");
    string serial_cookie = conn->req->get_cookie("serial");
    BSONObj p = this->db_conn->findOne(this->db_name + ".session", 
                QUERY("username" << username 
                      << "token" << token_cookie
                      << "serial" << serial_cookie));
    if (p.isEmpty()) {
        return Auth::failed;
    }
    ostringstream new_token;
    srand((int)time(0));
    new_token << username << rand();
    string token_encoded = Auth::SHA256(new_token.str());
    this->db_conn->update(this->db_name + ".session", 
                          QUERY("username" << username), 
                          BSON("username" << username 
                               << "token" << token_encoded
                               << "serial" << serial_cookie));
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