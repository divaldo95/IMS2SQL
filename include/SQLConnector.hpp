/*
 * SQLConnector.hpp
 * Created by David Baranyai
 * 2020.11.06
 */

#ifndef SQLConnector_hpp
#define SQLConnector_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <ctime> 
#include <vector>
#include <sstream>
#include <mysql/mysql.h>

class SQLConnector
{
private:
    std::string database = "";
    std::string host = "localhost";
    std::string username = "username";
    std::string password = "password"; //not safe

    //mysql server
    MYSQL *mysqlserver = NULL;
    bool autocommit = true; //by default it is on, to disable, use the SetAutocommit function
    bool transaction_started = false;
    std::stringstream log_msg, query;

protected:
    bool CheckDatabase(); //Check if it is exists
    bool SelectDatabase(); //Called from SelectDatabase(const char*)

    virtual void LOGError(const char*) = 0; //implement your own error log function
    virtual void LOGWarning(const char*) = 0; //implement your own warning log function
    virtual void LOGInfo(const char*) = 0; //implement your own info log function
    virtual void LOGDeveloper(const char*) = 0; //implement your developer error log function

public:
    SQLConnector();
    SQLConnector(const char*); //Set database name 
    ~SQLConnector();

    bool Connect();
    bool Connect(const char*, const char*); //username, password
    bool Connect(const char*, const char*, const char*); //username, password, database
    bool Connect(const char*, const char*, const char*, const char*); //username, password, database
    void Close();
    bool SelectDatabase(const char*);
    bool RunQuery(MYSQL_RES**, unsigned int&, unsigned int&, const char *);
    bool RunQuery(uint64_t&, const char *);
    bool RunQuery(const char *);
    void SetAutocommit(bool); //if you want to insert or update without start transaction and commit/rollback changes
    bool GetAutocommitState();
    void CommitChanges();
    void RollbackChanges();
    void StartTransaction();
    uint64_t GetLastInsertID();
};


#endif //SQLConnector.hpp
