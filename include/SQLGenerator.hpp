/*
 * SQLGenerator.hpp
 * Created by David Baranyai
 * 2020.06.11
 */

#ifndef SQLGenerator_hpp
#define SQLGenerator_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <ctime> 
#include <vector>
#include "LOG.hpp"
#include <mysql/mysql.h>
#include "filesystem.hpp"
#include "IMSData.hpp"

#define SQL_TAG "SQL"

class SQLGenerator
{
private:
    /* data */
    std::ofstream outFile;

    std::string database = "Decay";

    std::string converterName = "default";

    //mysql server
    MYSQL *mysqlserver;
    std::stringstream query;

    uint64_t detectorID = 0;
    uint64_t fileID = 0;
    uint64_t QCID = 0;
    uint64_t CertificateID = 0;
    uint64_t IsotopeID = 0;

    bool CheckDatabase(); //Check if it is exists
    bool SelectDatabase();
    bool CheckDetector(const IMSData&);
    bool CheckImportedFile(const IMSData&);
    bool InsertFileData(const IMSData&);
    bool InsertDetector(const IMSData&);
    bool CheckIsotope(const std::string&, const double&);
    bool InsertIsotope(const std::string&, const double&);
    bool ImportSOH(const IMSData&);
    bool ImportQC(const IMSData&);
    void CommitChanges();
    void RollbackChanges();
    void StartTransaction();
    uint64_t GetLastInsertID();

    std::string GetCurrentFormattedDate();

    //Log
    DLog::LOG &log = DLog::LOG::GetInstance();
    std::stringstream log_msg;
public:
    SQLGenerator();
    ~SQLGenerator();

    void Connect();
    bool RunQuery(MYSQL_RES**, unsigned int&, unsigned int&, const char *);
    bool RunQuery(uint64_t&, const char *);
    bool RunQuery(const char *);

    void SetAutocommit(bool);
    void Close();

    void GenerateDBStructure();
    void UploadIMSData(const IMSData&);

    void SetDatabaseName(std::string);
    void SetConverterName(std::string, std::string);
};


#endif //SQLGenerator.hpp