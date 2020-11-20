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
#include "SQLConnector.hpp"

#define SQL_TAG "SQL"

class SQLGenerator : public SQLConnector
{
private:
    /* data */
    std::ofstream outFile;

    std::string converterName = "default";

    //Failed files
    std::vector<std::string> failedFiles;

    //mysql server
    std::stringstream query;

    uint64_t detectorID = 0;
    uint64_t fileID = 0;
    uint64_t QCID = 0;
    uint64_t CertificateID = 0;
    uint64_t IsotopeID = 0;

    void LOGError(const char*) override;
    void LOGWarning(const char*) override;
    void LOGInfo(const char*) override;
    void LOGDeveloper(const char*) override;

    bool CheckDetector(const IMSData&);
    bool CheckImportedFile(const IMSData&);
    bool InsertFileData(const IMSData&);
    bool InsertDetector(const IMSData&);
    bool CheckIsotope(const std::string&, const double&, const std::string&);
    bool InsertIsotope(const std::string&, const double&);
    bool ImportSOH(const IMSData&);
    bool ImportQC(const IMSData&);

    std::string GetCurrentFormattedDate();

    //Log
    DLog::LOG &log = DLog::LOG::GetInstance();
    std::stringstream log_msg;
public:
    SQLGenerator();
    ~SQLGenerator();

    bool UploadIMSData(const IMSData&);
    void SetConverterName(std::string, std::string);
};


#endif //SQLGenerator.hpp