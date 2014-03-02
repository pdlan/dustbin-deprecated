#ifndef DUSTBIN_INCLUDE_GLOBAL_H
#define DUSTBIN_INCLUDE_GLOBAL_H
class Global {
  public:
    mongo::DBClientConnection db_conn;
    std::string db_name;
};
#endif