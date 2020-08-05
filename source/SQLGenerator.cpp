/*
 * SQLGenerator.hpp
 * Created by David Baranyai
 * 2020.06.11
 */
#include "SQLGenerator.hpp"

SQLGenerator::SQLGenerator()
{
    
}

void SQLGenerator::Connect()
{
    mysqlserver = mysql_init(NULL);
    try
    {
        //Replace username and password with the corresponding values
        if(!mysql_real_connect(mysqlserver, "localhost", "username", "password", "", 0, NULL, 0))
        {
                throw "Can't connect to the server";
        }
        else
        {
            log_msg << "Successfully connected" << std::endl;
            log_msg << "Host info: " << mysql_get_host_info(mysqlserver) << std::endl;
            log_msg << "Client info: " << mysql_get_client_info();
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
        }
    }
    catch(char const* err)
    {
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, err);
    }
}

void SQLGenerator::Close()
{
    mysql_close(mysqlserver);
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, "MySQL Connection closed");
}

SQLGenerator::~SQLGenerator()
{
}

std::string SQLGenerator::GetCurrentFormattedDate()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,20,"%Y.%m.%d %T",timeinfo);
    return std::string(buffer);
}

void SQLGenerator::SetDatabaseName(std::string dbname)
{
    database = dbname;
    log_msg.str("");
    log_msg << "Using " << dbname << " as database name";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
}

void SQLGenerator::SetConverterName(std::string firstname, std::string lastname)
{
    std::stringstream name;
    name << firstname << " " << lastname;
    converterName = name.str();
}

void SQLGenerator::UploadIMSData(const IMSData& imsData)
{
    SetAutocommit(false);
    StartTransaction();
    if(!CheckDatabase())
    {
        RollbackChanges();
        return;
    }
    if(!CheckDetector(imsData))
    {
        RollbackChanges();
        return;
    }
    if(!CheckImportedFile(imsData))
    {
        RollbackChanges();
        return;
    }
    if(imsData.dataType.compare("QCPHD") == 0) 
    {
        if(!ImportQC(imsData))
        {
            RollbackChanges();
            return;
        }
    }
    else if(imsData.dataType.compare("RMSSOH") == 0) 
    {
        if(!ImportSOH(imsData))
        {
            RollbackChanges();
            return;
        }
    }
    CommitChanges();
}

bool SQLGenerator::RunQuery(MYSQL_RES** res, unsigned int& nRow, unsigned int& nColumn, const char * query)
{
    if(*res != NULL)
    {
        //mysql_free_result(*res); //clear it
    }
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
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
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
}

bool SQLGenerator::RunQuery(uint64_t& nRow, const char * query)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
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
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
}

bool SQLGenerator::RunQuery(const char * query)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    if(!mysql_query(mysqlserver, query)) //start query
    {
        return true;
    }
    else
    {
        log_msg.str("");
        log_msg << "Could not run query. Server said: " << mysql_error(mysqlserver) << " (Code: " << mysql_errno(mysqlserver) << ")";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
}

uint64_t SQLGenerator::GetLastInsertID()
{
    return mysql_insert_id(mysqlserver);
}

void SQLGenerator::SetAutocommit(bool state)
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return;
    }
    mysql_autocommit(mysqlserver, state);
}

bool SQLGenerator::CheckDatabase()
{
    detectorID = 0;
    fileID = 0;
    QCID = 0;
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
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
}

bool SQLGenerator::SelectDatabase()
{
    query.str("");
    query << "USE " << database << ";";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(query.str().c_str()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SQLGenerator::CheckDetector(const IMSData& imsData)
{
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT ID, Name FROM Detectors WHERE Name LIKE \"" << imsData.detectorName << "\";";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
    {
        if(nRow == 0 && nColumn == 2) //not exists, insert it and query the id
        {
            log_msg.str("");
            log_msg << "Detector not exists in database.";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
            return InsertDetector(imsData);
        }
        else if(nRow == 1 && nColumn == 2) //could not be more than one because of constraints
        {
            MYSQL_ROW row; // for storing rows
            row = mysql_fetch_row(res);
            detectorID = atoi(row[0]);
            return true;
        }
        else
        {
            log_msg.str("");
            log_msg << "Row and/or column error";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            return false;
        }
        
    }
    else
    {
        return false;
    }
    
}

bool SQLGenerator::CheckImportedFile(const IMSData& imsData)
{
    if(detectorID == 0)
    {
        log_msg.str("");
        log_msg << "Wrong detector id.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str().c_str());
        return false;
    }
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT ID, FileName FROM ImportedFiles WHERE FileName LIKE \"" << imsData.filename << "\";";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str());
    if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
    {
        if(nRow == 0 && nColumn == 2) //not exists, insert it and query the id
        {
            return InsertFileData(imsData);
        }
        else
        {
            log_msg.str("");
            log_msg << "File already imported";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str().c_str());
            return false;
        }
        
    }
    else
    {
        return false;
    }
}

bool SQLGenerator::InsertFileData(const IMSData& imsData)
{
    uint64_t nRow = 0;
    query.str("");
    query << "INSERT INTO ImportedFiles(DetectorID, FilePath, FileName, ConvertDate, ConvertedBy, DataType) VALUES (" 
        << detectorID << ", " 
        << "\"" << imsData.absolutepath << "\", "
        << "\"" << imsData.filename << "\", "
        << "STR_TO_DATE(\'" << GetCurrentFormattedDate() << "\',\'%Y.%m.%d %H:%i:%s\'), "
        << "\"" << converterName << "\", "
        << "\"" << imsData.dataType << "\""
        << ");";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(nRow, query.str().c_str()))
    {
        if(nRow == 1) 
        {
            fileID = GetLastInsertID();
            if(fileID == 0) return false;
            log_msg.str("");
            log_msg << "File data inserted successfully.";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
            return true;
        }
        else
        {
            log_msg.str("");
            log_msg << "Insert file data failed";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool SQLGenerator::InsertDetector(const IMSData& imsData)
{
    uint64_t nRow = 0;
    query.str("");
    query << "INSERT INTO Detectors(Name) VALUES (\"" << imsData.detectorName << "\");";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(nRow, query.str().c_str()))
    {
        if(nRow == 1) 
        {
            detectorID = GetLastInsertID();
            if(detectorID == 0) return false;
            log_msg.str("");
            log_msg << "Detector data inserted successfully.";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
            return true;
        }
        else
        {
            log_msg.str("");
            log_msg << "Insert detector data failed";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool SQLGenerator::ImportSOH(const IMSData& imsData)
{
    if(detectorID == 0 || fileID == 0)
    {
        log_msg.str("");
        log_msg << "Invalid IDs on SOH import";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    if(imsData.dataType.compare("RMSSOH") != 0)
    {
        log_msg.str("");
        log_msg << "Invalid data type on SOH import";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    uint64_t nRow = 0;
    for(int i = 0; i < imsData.sohData.CrystalTemperature.size(); i++) //all lists should contain the same ammount of data
    {
        query.str("");
        query << "INSERT INTO SOH(DetectorID, FileID, RoomTemperature, ShieldStatus, Humidity, HighVoltage, CrystalTemperature, Timestamp, DateTime, MeasurementTime) VALUES (" 
            << detectorID << ", " 
            << fileID << ", "
            << imsData.sohData.RoomTemperature[i] << ", "
            << imsData.sohData.ShieldStatus[i] << ", "
            << imsData.sohData.Humidity[i] << ", "
            << imsData.sohData.HighVoltage[i] << ", "
            << imsData.sohData.CrystalTemperature[i] << ", "
            << "FROM_UNIXTIME(" << imsData.sohData.Timestamp[i]/1000 << "), " //timestamp is in ms, but FROM_UNIXTIME takes s
            << "STR_TO_DATE(\'" << imsData.sohData.DateTime[i] << "\',\'%Y/%m/%d %H:%i:%s\'), "
            << imsData.sohData.MeasurementTime[i]
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert SOH failed at " << imsData.sohData.CrystalTemperature.size() << "/" << i;
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    log_msg.str("");
    log_msg << "SOH data inserted successfully.";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
    return true;
}

bool SQLGenerator::ImportQC(const IMSData& imsData)
{
    if(detectorID == 0 || fileID == 0)
    {
        log_msg.str("");
        log_msg << "Invalid IDs on QC import";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    if(imsData.dataType.compare("QCPHD") != 0)
    {
        log_msg.str("");
        log_msg << "Invalid data type on QC import";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    uint64_t nRow = 0;

    query.str("");
    query << "INSERT INTO QC(DetectorID, FileID, AcquisitionStartTime, Realtime, AcquisitionLength, CalibrationDate) VALUES (" 
    << detectorID << ", "
    << fileID << ", "
    << "STR_TO_DATE(\'" << imsData.qcData.AcquisitionStartTime << "\',\'%Y/%m/%d %H:%i:%s\'), "
    << imsData.qcData.Realtime << ", "
    << imsData.qcData.AcquisitionLength << ", "
    << "STR_TO_DATE(\'" << imsData.qcData.CalibrationDate << "\',\'%Y/%m/%d %H:%i:%s\')"
    << ");";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(nRow, query.str().c_str()))
    {
        if(nRow == 1) 
        {
            QCID = GetLastInsertID();
            if(QCID == 0) return false;
        }
        else
        {
            log_msg.str("");
            log_msg << "Insert QC main data failed";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            return false;
        }
    }
    else
    {
        return false;
    }
    for(int i = 0; i < imsData.qcData.Energy.X.size(); i++) //all lists should contain the same ammount of data
    {
        query.str("");
        query << "INSERT INTO QCEnergy(QCID, X, Y, Error) VALUES (" 
            << QCID << ", " 
            << imsData.qcData.Energy.X[i] << ", "
            << imsData.qcData.Energy.Y[i] << ", "
            << imsData.qcData.Energy.Error[i]
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert QCEnergy failed at " << imsData.qcData.Energy.X.size() << "/" << i;
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    for(int i = 0; i < imsData.qcData.Efficiency.X.size(); i++) //all lists should contain the same ammount of data
    {
        query.str("");
        query << "INSERT INTO QCEfficiency(QCID, X, Y, Error) VALUES (" 
            << QCID << ", " 
            << imsData.qcData.Efficiency.X[i] << ", "
            << imsData.qcData.Efficiency.Y[i] << ", "
            << imsData.qcData.Efficiency.Error[i]
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert QCEfficiency failed at " << imsData.qcData.Efficiency.X.size() << "/" << i;
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    for(int i = 0; i < imsData.qcData.Spectrum.X.size(); i++) //all lists should contain the same ammount of data
    {
        query.str("");
        query << "INSERT INTO QCSpectrum(QCID, X, Y, Error) VALUES (" 
            << QCID << ", " 
            << imsData.qcData.Spectrum.X[i] << ", "
            << imsData.qcData.Spectrum.Y[i] << ", "
            << imsData.qcData.Spectrum.Error[i]
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert QCSpectrum failed at " << imsData.qcData.Spectrum.X.size() << "/" << i;
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    log_msg.str("");
    log_msg << "QC data inserted successfully.";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
    return true;
}

void SQLGenerator::CommitChanges()
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "(Commit) Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return;
    }
    mysql_commit(mysqlserver);
    log_msg.str("");
    log_msg << "Changes committed.";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
}

void SQLGenerator::RollbackChanges()
{
    if(mysqlserver == NULL || mysql_ping(mysqlserver) != 0)
    {
        log_msg.str("");
        log_msg << "(Rollback) Could not ping mysql server.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return;
    }
    mysql_rollback(mysqlserver);
    log_msg.str("");
    log_msg << "Changes rolled back.";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
}

void SQLGenerator::StartTransaction()
{
    query.str("");
    query << "START TRANSACTION;";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(!RunQuery(query.str().c_str()))
    {
        log_msg.str("");
        log_msg << "Start transaction command failed.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
    }
}