#include <string>
#include <mongo/client/dbclient.h>
#include "dustbin.h"
#include "setting.h"

std::string Setting::get_str_setting(std::string key) {
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    BSONObj p = dustbin->get_db_conn()->
                findOne(dustbin->get_db_name() + ".setting",
                        QUERY("key" << key));
    if (!p.hasField("value") || p.getField("value").type() != String) {
        return "";
    }
    return p.getStringField("value");
}

void Setting::set_setting(std::string key, std::string value) {
    using namespace mongo;
    Dustbin* dustbin = Dustbin::instance();
    if (key == "" || value == "") {
        return;
    }
    BSONObj p = dustbin->get_db_conn()->
                findOne(dustbin->get_db_name() + ".setting",
                        QUERY("key" << key));
    if (p.isEmpty()) {
        dustbin->get_db_conn()->insert(dustbin->get_db_name() + ".setting",
                                       BSON("key" << key << "value" << value));
    } else {
        dustbin->get_db_conn()->update(dustbin->get_db_name() + ".setting",
                                       QUERY("key" << key),
                                       BSON("key" << key << "value" << value));
    }
    return;
}