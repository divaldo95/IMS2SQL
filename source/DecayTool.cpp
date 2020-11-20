/*
 * DecayTool.cpp
 * Created by David Baranyai
 * 2020.11.20
 */
#include "DecayTool.hpp"

DecayTool::DecayTool() : SQLConnector()
{

}

DecayTool::~DecayTool()
{

}

//feel free to modify
void DecayTool::LOGError(const char* msg)
{
    std::cout << "[Error] " << msg << std::endl;
}

void DecayTool::LOGWarning(const char* msg)
{
    std::cout << "[Warning] " << msg << std::endl;
}

void DecayTool::LOGInfo(const char* msg)
{
    std::cout << "[Info] " << msg << std::endl;
}

void DecayTool::LOGDeveloper(const char* msg)
{
    std::cout << "[Developer] " << msg << std::endl;
}

void DecayTool::SetQCValidFlag(int id, bool valid)
{
    uint64_t nRow;
    query.str("");
    query << "UPDATE QC SET Valid = " << (valid ? "1 " : "0 ") << "WHERE ID = " << std::to_string(id) << ";";
    if(RunQuery(nRow, query.str().c_str()))
    {
        log_msg.str("");
        log_msg << "QC Valid flag updated for ID " << std::to_string(id);
        LOGInfo(log_msg.str().c_str());
    }
}

void DecayTool::SetQCValidFlag(std::vector<int> ids, bool valid)
{
    uint64_t nRow;
    query.str("");
    query << "UPDATE QC SET Valid = " << (valid ? "1 " : "0 ") << "WHERE ID IN (";
    for(int i = 0; i < ids.size() - 1; i++)
    {
        query << ids[i] << ", ";
    }
    query << ids[ids.size() - 1] << ");";
    if(RunQuery(nRow, query.str().c_str()))
    {
        log_msg.str("");
        log_msg << nRow << " QC Valid flag updated out of " << ids.size() << " for the given IDs.";
        LOGInfo(log_msg.str().c_str());
    }
}

std::string DecayTool::GetFileNamebyQCID(int id)
{
    std::string filename = "";
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT FileName FROM Decay_all.ImportedFiles i JOIN QC q ON (i.ID = q.FileID) WHERE q.ID = " << std::to_string(id) << ";";
    if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
    {
        if(nRow == 1 && nColumn == 1) //found it
        {
            MYSQL_ROW row;
            row = mysql_fetch_row(res);
            filename = row[0];
            log_msg.str("");
            log_msg << "File name: " << filename;
            LOGInfo(log_msg.str().c_str());
        }
        else
        {
            log_msg.str("");
            log_msg << "Row and/or column error. Server returned with " << nColumn << " column and " << nRow << " row.";
            LOGError(log_msg.str().c_str());
        }
    }
    return filename;
}

std::vector<int> DecayTool::GetIDsUnderThreshold(int sum)
{
    std::vector<int> ids;
    unsigned int nRow = 0;
    unsigned int nColumn = 0;
    MYSQL_RES *res;
    query.str("");
    query << "SELECT s.QCID, SUM(s.Y) AS totalSum FROM QCSpectrum s JOIN QC q ON (s.QCID  = q.ID) JOIN Detectors d ON (d.ID = q.DetectorID) GROUP BY s.QCID HAVING totalSum < " << std::to_string(sum) << ";";
    if(RunQuery(&res, nRow, nColumn, query.str().c_str()))
    {
        if(nRow > 0 && nColumn == 2) //found some
        {
            MYSQL_ROW row;
            for(int i = 0; i < nRow; i++)
            {
                row = mysql_fetch_row(res);
                ids.push_back(std::stoi(row[0]));
            }
            log_msg.str("");
            log_msg << "Found " << nRow << " ID.";
            LOGInfo(log_msg.str().c_str());
        }
        else
        {
            log_msg.str("");
            log_msg << "Row and/or column error. Server returned with " << nColumn << " column and " << nRow << " row.";
            LOGError(log_msg.str().c_str());
        }
    }
    return ids;
}