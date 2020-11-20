/*
 * SQLConnector.cpp
 * Created by David Baranyai
 * 2020.11.06
 */
#include "SQLConnector.hpp"

SQLConnector::SQLConnector()
{
    
}

SQLConnector::SQLConnector(const char* db)
{
    database = db;
}

SQLConnector::~SQLConnector()
{
    Close();
}

bool SQLConnector::Connect()
{
    if(mysqlserver != NULL)
    {
        log_msg.str("");
        log_msg << "MySQL connection already initialized.";
        LOGWarning(log_msg.str().c_str());
        return true;
    }
    mysqlserver = mysql_init(NULL);
    try
    {
        if(!mysql_real_connect(mysqlserver, host.c_str(), username.c_str(), password.c_str(), database.c_str(), 0, NULL, 0))
        {
                throw "Can't connect to the server";
        }
        else
        {
            log_msg << "Successfully connected" << std::endl;
            log_msg << "Host info: " << mysql_get_host_info(mysqlserver) << std::endl;
            log_msg << "Client info: " << mysql_get_client_info();
            LOGInfo(log_msg.str().c_str());
        }
    }
    catch(char const* err)
    {
        LOGError(err);
        return false;
    }
    GetAutocommitState();
    return true;
}

bool SQLConnector::Connect(const char* user, const char* pass)
{
    username = user;
    password = pass;
    return Connect();
}

bool SQLConnector::Connect(const char* user, const char* pass, const char* db)
{
    username = user;
    password = pass;
    database = db;
    return Connect();
}

bool SQLConnector::Connect(const char* h, const char* user, const char* pass, const char* db)
{
    host = h;
    username = user;
    password = pass;
    database = db;
    return Connect();
}

void SQLConnector::Close()
{
    if(mysqlserver != NULL)
    {
        mysql_close(mysqlserver);
        LOGInfo("MySQL Connection closed");
        mysqlserver = NULL;
    }
}

bool SQLConnector::RunQuery(MYSQL_RES** res, unsigned int& nRow, unsigned int& nColumn, const char * query)
{
    if(*res != NULL)
    {
        //mysql_free_result(*res); //clear it
    }
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return false;
    }
    if(!autocommit && !transaction_started)
    {
        if(strstr(query, "INSERT") != NULL || strstr(query, "UPDATE") != NULL || strstr(query, "DELETE") != NULL)
        {
            log_msg.str("");
            log_msg << "Autocommit is disabled and looks like you are trying to INSERT/UPDATE/DELETE row(s) without START TRANSACTION";
            LOGInfo(log_msg.str().c_str());
        }
    }
    if(!mysql_query(mysqlserver, query)) //start query
    {
        *res = mysql_store_result(mysqlserver); //store result
        nColumn = mysql_num_fields(*res); //store column number
        nRow = mysql_num_rows(*res); //store number of rows
        return true;
    }
    else
    {
        log_msg.str("");
        log_msg << "Could not run query. Server said: " << mysql_error(mysqlserver) << " (Code: " << mysql_errno(mysqlserver) << ")";
        LOGError(log_msg.str().c_str());
        return false;
    }
}

bool SQLConnector::RunQuery(uint64_t& nRow, const char * query)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return false;
    }
    if(!autocommit && !transaction_started)
    {
        if(strstr(query, "INSERT") != NULL || strstr(query, "UPDATE") != NULL || strstr(query, "DELETE") != NULL)
        {
            log_msg.str("");
            log_msg << "Autocommit is disabled and looks like you are trying to INSERT/UPDATE/DELETE row(s) without START TRANSACTION";
            LOGInfo(log_msg.str().c_str());
        }
    }
    if(!mysql_query(mysqlserver, query)) //start query
    {
        nRow = mysql_affected_rows(mysqlserver);
        return true;
    }
    else
    {
        log_msg.str("");
        log_msg << "Could not run query. Server said: " << mysql_error(mysqlserver) << " (Code: " << mysql_errno(mysqlserver) << ")";
        LOGError(log_msg.str().c_str());
        return false;
    }
}

bool SQLConnector::RunQuery(const char * query)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return false;
    }
    if(!autocommit && !transaction_started)
    {
        if(strstr(query, "INSERT") != NULL || strstr(query, "UPDATE") != NULL || strstr(query, "DELETE") != NULL)
        {
            log_msg.str("");
            log_msg << "Autocommit is disabled and looks like you are trying to INSERT/UPDATE/DELETE row(s) without START TRANSACTION";
            LOGInfo(log_msg.str().c_str());
        }
    }
    if(!mysql_query(mysqlserver, query)) //start query
    {
        return true;
    }
    else
    {
        log_msg.str("");
        log_msg << "Could not run query. Server said: " << mysql_error(mysqlserver) << " (Code: " << mysql_errno(mysqlserver) << ")";
        LOGError(log_msg.str().c_str());
        return false;
    }
}

void SQLConnector::SetAutocommit(bool state)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return;
    }
    mysql_autocommit(mysqlserver, state);
    autocommit = state;
}

/*
 * Returns true if not connected.
 * If connected, it will check if database exists, and select it.
 * In that case, the return value is depends on a successful query.
 */
bool SQLConnector::SelectDatabase(const char* db)
{
    database = db;
    if(mysqlserver == NULL) return true;
    else return CheckDatabase();
}

bool SQLConnector::CheckDatabase()
{
    MYSQL_RES* res = mysql_list_dbs(mysqlserver, database.c_str());
    unsigned int nColumn = mysql_num_fields(res); // store column number
    unsigned int nRow = mysql_num_rows(res); // store number of rows
    if(nColumn == 1 && nRow == 1)
    {
        return SelectDatabase();
    }
    else
    {
        log_msg.str("");
        log_msg << "Database (" << database << ") not exists.";
        LOGError(log_msg.str().c_str());
        return false;
    }
}

bool SQLConnector::SelectDatabase()
{
    query.str("");
    query << "USE " << database << ";";
    return RunQuery(query.str().c_str());
}

bool SQLConnector::GetAutocommitState()
{
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT @@autocommit;";
    if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
    {
        if(nRow == 1 && nColumn == 1) //not exists, insert it and query the id
        {
            MYSQL_ROW row; // for storing rows
            row = mysql_fetch_row(res);
            autocommit = atoi(row[0]);
        }
        else
        {
            autocommit = false;
            log_msg.str("");
            log_msg << "Could not query autocommit state. Explicitly set to false";
            LOGError(log_msg.str().c_str());
        }
    }
    return autocommit;
}

void SQLConnector::CommitChanges()
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "(Commit) Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return;
    }
    mysql_commit(mysqlserver);
    log_msg.str("");
    log_msg << "Changes committed.";
    LOGInfo(log_msg.str().c_str());
    transaction_started = false;
}

void SQLConnector::RollbackChanges()
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "(Rollback) Could not ping mysql server.";
        LOGError(log_msg.str().c_str());
        return;
    }
    mysql_rollback(mysqlserver);
    log_msg.str("");
    log_msg << "Changes rolled back.";
    LOGInfo(log_msg.str().c_str());
    transaction_started = false;
}

void SQLConnector::StartTransaction()
{
    transaction_started = true;
    query.str("");
    query << "START TRANSACTION;";
    if(!RunQuery(query.str().c_str()))
    {
        log_msg.str("");
        log_msg << "Start transaction command failed.";
        LOGError(log_msg.str().c_str());
        transaction_started = false;
    }
}

uint64_t SQLConnector::GetLastInsertID()
{
    return mysql_insert_id(mysqlserver);
}