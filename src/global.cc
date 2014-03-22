#include <string>
#include <mongo/client/dbclient.h>
#include "global.h"

std::string Global::get_setting(std::string key) {
    using namespace mongo;
    BSONObj p = this->db_conn.findOne(this->db_name + ".setting", 
                                      QUERY("key" << key));
    if (!p.hasField("value")) {
        return "";
    }
    return p.getStringField("value");
}

int Global::get_int_setting(std::string key) {
    using namespace mongo;
    BSONObj p = this->db_conn.findOne(this->db_name + ".setting", 
                                      QUERY("key" << key));
    if (!p.hasField("value")) {
        return 0;
    }
    return p.getIntField("value");
}

void Global::set_setting(std::string key, std::string value) {
    using namespace mongo;
    this->db_conn.insert(this->db_name + ".setting",
                         BSON("key" << key << "value" << value));
    return;
}