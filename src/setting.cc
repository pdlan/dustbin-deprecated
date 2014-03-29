#include <string>
#include <mongo/client/dbclient.h>
#include "global.h"
#include "setting.h"

extern Global global;

std::string Setting::get_str_setting(std::string key) {
    using namespace mongo;
    BSONObj p = global.db_conn.findOne(global.db_name + ".setting", 
                                       QUERY("key" << key));
    if (!p.hasField("value") || p.getField("value").type() != String) {
        return "";
    }
    return p.getStringField("value");
}

void Setting::set_setting(std::string key, std::string value) {
    using namespace mongo;
    if (key == "" || value == "") {
        return;
    }
    BSONObj p = global.db_conn.findOne(global.db_name + ".setting", 
                                       QUERY("key" << key));
    if (p.isEmpty()) {
        global.db_conn.insert(global.db_name + ".setting",
                              BSON("key" << key << "value" << value));
    } else {
        global.db_conn.update(global.db_name + ".setting",
                              QUERY("key" << key),
                              BSON("key" << key << "value" << value));
    }
    return;
}