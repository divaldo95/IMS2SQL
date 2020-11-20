/*
 * SQLGenerator.hpp
 * Created by David Baranyai
 * 2020.06.11
 */
#include "SQLGenerator.hpp"

SQLGenerator::SQLGenerator() : SQLConnector()
{
    
}

SQLGenerator::~SQLGenerator()
{
    if(failedFiles.size() != 0)
    {
        std::cout << "There were an error while uploading some files. Check log file." << std::endl;
        log_msg.str("");
        log_msg << "Failed file list (" << failedFiles.size() << "):";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        for(int i = 0; i < failedFiles.size(); i++)
        {
            log_msg.str("");
            log_msg << failedFiles[i];
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        }
    }
    else
    {
        log_msg.str("");
        log_msg << "All files uploaded.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, log_msg.str());
    }
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

void SQLGenerator::SetConverterName(std::string firstname, std::string lastname)
{
    std::stringstream name;
    name << firstname << " " << lastname;
    converterName = name.str();
}

bool SQLGenerator::UploadIMSData(const IMSData& imsData)
{
    if(!imsData.fileOK)
    {
        log_msg.str("");
        log_msg << "Error in file " << imsData.filename;
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        return false;
    }
    SetAutocommit(false);
    StartTransaction();
    if(!CheckDatabase())
    {
        log_msg.str("");
        log_msg << "Could not upload " << imsData.filename << ". Error while checking database";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        RollbackChanges();
        failedFiles.push_back(imsData.filename);
        return false;
    }
    if(!CheckDetector(imsData))
    {
        log_msg.str("");
        log_msg << "Could not upload " << imsData.filename << ". Error while checking detector in database";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        RollbackChanges();
        failedFiles.push_back(imsData.filename);
        return false;
    }
    if(!CheckImportedFile(imsData))
    {
        log_msg.str("");
        log_msg << "Could not upload " << imsData.filename << ". Error while checking imported file";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
        RollbackChanges();
        failedFiles.push_back(imsData.filename);
        return false;
    }
    if(imsData.dataType.compare("QCPHD") == 0) 
    {
        if(!ImportQC(imsData))
        {
            log_msg.str("");
            log_msg << "Could not upload " << imsData.filename << ". Error while import QC data";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            RollbackChanges();
            failedFiles.push_back(imsData.filename);
            return false;
        }
    }
    else if(imsData.dataType.compare("RMSSOH") == 0) 
    {
        if(!ImportSOH(imsData))
        {
            log_msg.str("");
            log_msg << "Could not upload " << imsData.filename << ". Error while import SOH data";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            RollbackChanges();
            failedFiles.push_back(imsData.filename);
            return false;
        }
    }
    CommitChanges();
    return true;
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
            log_msg << "File (" << imsData.filename << ") already imported";
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
    log_msg.str("");
    log_msg << "Processing file " << imsData.filename << "...";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str().c_str());
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
        query << "INSERT INTO SOH(DetectorID, FileID, RoomTemperature, ShieldStatus, Humidity, HighVoltage, CrystalTemperature, DateTime, MeasurementTime, Valid) VALUES (" 
            << detectorID << ", " 
            << fileID << ", "
            << imsData.sohData.RoomTemperature[i] << ", "
            << imsData.sohData.ShieldStatus[i] << ", "
            << imsData.sohData.Humidity[i] << ", "
            << imsData.sohData.HighVoltage[i] << ", "
            << imsData.sohData.CrystalTemperature[i] << ", "
            << "STR_TO_DATE(\'" << imsData.sohData.DateTime[i] << "\',\'%Y/%m/%d %H:%i:%s\'), "
            << imsData.sohData.MeasurementTime[i] << ", "
            << (imsData.sohData.Valid ? "TRUE" : "FALSE")
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
    query << "INSERT INTO QC(DetectorID, FileID, AcquisitionStartTime, Realtime, AcquisitionLength, CalibrationDate, Valid) VALUES (" 
    << detectorID << ", "
    << fileID << ", "
    << "STR_TO_DATE(\'" << imsData.qcData.AcquisitionStartTime << "\',\'%Y/%m/%d %H:%i:%s\'), "
    << imsData.qcData.Realtime << ", "
    << imsData.qcData.AcquisitionLength << ", "
    << "STR_TO_DATE(\'" << imsData.qcData.CalibrationDate << "\',\'%Y/%m/%d %H:%i:%s\'), "
    << (imsData.qcData.Valid ? "TRUE" : "FALSE")
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

    if(imsData.qcData.Certificate.AbsoluteActivity != 0)
    {
        query.str("");
        query << "INSERT INTO QCCertificate(QCID, AbsoluteActivity, DateTime) VALUES (" 
            << QCID << ", " 
            << imsData.qcData.Certificate.AbsoluteActivity << ", "
            << "STR_TO_DATE(\'" << imsData.qcData.Certificate.DateTime << "\',\'%Y/%m/%d %H:%i:%s\')"
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert QCCertificate failed";
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
                return false;
            }
            CertificateID = GetLastInsertID();
        }
        else
        {
            return false;
        }
    }

    for(int i = 0; i < imsData.qcData.Certificate.intensity.EnergyPercentage.size(); i++) //all lists should contain the same ammount of data
    {
        if(!CheckIsotope(imsData.qcData.Certificate.intensity.IsotopeName[i], imsData.qcData.Certificate.intensity.HalfLife[i], imsData.filename))
        {
            return false;
        }

        query.str("");
        query << "INSERT INTO Intensity(CertificateID, IsotopeID, RelativeActivity, Energy, EnergyPercentage) VALUES (" 
            << CertificateID << ", " 
            << IsotopeID << ", "
            << imsData.qcData.Certificate.intensity.RelativeActivity[i] << ", "
            << imsData.qcData.Certificate.intensity.Energy[i] << ", "
            << imsData.qcData.Certificate.intensity.EnergyPercentage[i]
            << ");";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
        if(RunQuery(nRow, query.str().c_str()))
        {
            if(nRow != 1) 
            {
                log_msg.str("");
                log_msg << "Insert Intensity failed at " << imsData.qcData.Certificate.intensity.EnergyPercentage.size() << "/" << i;
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

bool SQLGenerator::CheckIsotope(const std::string& isotope, const double& halflife, const std::string& filename)
{
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT ID, HalfLife FROM Isotopes WHERE Name LIKE \"" << isotope << "\";";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str());
    try
    {
        if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
        {
            if(nRow == 0) //not exists, insert it and query the id
            {
                return InsertIsotope(isotope, halflife);
            }
            else
            {
                MYSQL_ROW row;
                row = mysql_fetch_row(res);
                IsotopeID = std::stoi(row[0]);
                double hl = std::stod(row[1]);
                log_msg.str("");
                log_msg << "Isotope already imported"; 
                if(hl != halflife)
                {
                    log_msg << ", but the half life doesn't match. Database returned with " << std::to_string(hl) << " for " << isotope << ", but the file contains " << std::to_string(halflife) << ". (" << filename << ")";
                }
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str().c_str());
                return true;
            }
            
        }
        else
        {
            return false;
        }
    }
    catch(const std::invalid_argument& e)
    {
        log_msg.str("");
        log_msg << "Exception occured on CheckIsotope: " << e.what();
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str().c_str());
        return false;
    }
}

bool SQLGenerator::InsertIsotope(const std::string& name, const double& halflife)
{
    uint64_t nRow = 0;
    query.str("");
    query << "INSERT INTO Isotopes(Name, HalfLife) VALUES (" 
        << "\"" << name << "\", " 
        << halflife
        << ");";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, query.str().c_str());
    if(RunQuery(nRow, query.str().c_str()))
    {
        if(nRow != 1) 
        {
            log_msg.str("");
            log_msg << "Insert Isotope failed";
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, log_msg.str());
            return false;
        }
        else
        {
            IsotopeID = GetLastInsertID();
            return true;
        }
        
    }
    else
    {
        return false;
    }
}

void SQLGenerator::LOGError(const char* msg)
{
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, SQL_TAG, msg);
}

void SQLGenerator::LOGWarning(const char* msg)
{
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_WARNING, SQL_TAG, msg);
}

void SQLGenerator::LOGInfo(const char* msg)
{
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, SQL_TAG, msg);
}

void SQLGenerator::LOGDeveloper(const char* msg)
{
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_DEVELOPER, SQL_TAG, msg);
}
