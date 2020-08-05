//
//  LOG.cpp
//  LOG
//
//  Created by Baranyai David on 2019. 09. 23..
//  Modified on 2020.07.20
//  Copyright © 2020. Baranyai David. All rights reserved.
//

#include "LOG.hpp"

DLog::LOG::LOG()
{
    try
    {
        auto currDate = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(currDate);

        std::stringstream dateString, filename;
        dateString << std::put_time(std::localtime(&now_c), "%Y-%m-%d");
        
        std::stringstream strPath_res; //if something goes wrong with the base path
        int strPathIterator = 0;
        std::string strPath = "IMS2SQL"; //base path
        std::string baseFilename = "log_";
        std::string extension = ".txt";
        strPath_res.str("");
        strPath_res << strPath;
        while(true)
        {
            if(std::filesystem::exists(strPath_res.str()))
            {
                if(!std::filesystem::is_directory(strPath_res.str()))
                {
                    strPath_res.str("");
                    strPath_res << strPath << strPathIterator;
                    strPathIterator++;
                }
                else 
                {
                    break;
                }
            }
            else
            {
                std::filesystem::create_directory(strPath_res.str());
                break;
            }
            
        }
        //LOG file: $(SOURCE_DIR)/$(strPath_res)/$(baseFilename)$(CURRENT_DATE_TIME).$(extension)
        filename << strPath_res.str() << "/" << baseFilename << dateString.str() << extension;
        logfile.open(filename.str(), std::ios::app);
        if(!logfile.is_open())
        {
            throw "File opening error";
        }
        SetBit(DLog::DEBUG_BIT::INFO_BIT, true);
        SetBit(DLog::DEBUG_BIT::ERROR_BIT, true);
        SetBit(DLog::DEBUG_BIT::WARNING_BIT, true);
        SetBit(DLog::DEBUG_BIT::DEVELOPER_BIT, true);
        logfile << "-----------------------LOG START-----------------------" << std::endl;
    }
    //Check if any errors happen when opening the file
    catch(char param)
    {
        std::cout << param << std::endl;
    }
}

DLog::LOG::~LOG()
{
    logfile << "------------------------LOG END------------------------" << std::endl << std::endl;
    logfile.close();
}

std::string DLog::LOG::GetCurrentFormattedDate()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,20,"%Y.%m.%d %T",timeinfo);
    return std::string(buffer);
}

void DLog::LOG::SetLogLevel(const int loglevel)
{
    LogLevel = loglevel;
}

std::string DLog::LOG::DebugInt2String(const int level)
{
    if(level == MESSAGE_WARNING) return std::string("WARNING");
    else if(level == MESSAGE_ERROR) return std::string("ERROR");
    else if(level == MESSAGE_INFO) return std::string("INFO");
    else if(level == MESSAGE_DEVELOPER) return std::string("DEV");
    else return std::string("OTHER");
}

void DLog::LOG::Append(const int& level, const char* tag, const char* message)
{
    if(!logfile.is_open()) return; //skip if it is not opened
    append_mutex.lock();
    std::stringstream msg;
    bool print = false; //if the correct bit is set, then enable to print
    if(LogLevel == 0) //everything turned off
    {
        return;
    }
    if(GetBit(DLog::DEBUG_BIT::ERROR_BIT) && level == MESSAGE_ERROR)
    {
        print = true;
    }
    else if(GetBit(DLog::DEBUG_BIT::WARNING_BIT) && level == MESSAGE_WARNING)
    {
        print = true;
    }
    else if(GetBit(DLog::DEBUG_BIT::INFO_BIT) && level == MESSAGE_INFO)
    {
        print = true;
    }
    else if(GetBit(DLog::DEBUG_BIT::DEVELOPER_BIT) && level == MESSAGE_DEVELOPER)
    {
        print = true;
    }
    if(print)
    {
        msg << "[" << DebugInt2String(level) << "]" << "[" << std::string(tag) << "]" << "[" << GetCurrentFormattedDate() << "]: " << std::string(message) << std::endl;
        logfile << msg.str();
        if (PrintLog)
        {
            std::cout << std::string(message) << std::endl;
        }
    }
    logfile.flush();
    append_mutex.unlock();
}

void DLog::LOG::Append(const int& level, const char* tag, const std::string& message)
{
    Append(level, tag, message.c_str());
}

void DLog::LOG::InsertSeparator()
{
    std::stringstream separator;
    separator << "-------------------------------------------------------" << std::endl;
    logfile << separator.str();
    if(PrintLog) std::cout << separator.str();
}

void DLog::LOG::SetBit(DEBUG_BIT n, bool state)
{
    if(state)
    {
        LogLevel |= 1UL << n;
    }
    else
    {
        LogLevel &= ~(1UL << n);
    }
    
}

bool DLog::LOG::GetBit(DEBUG_BIT n)
{
    //std::cout << "Bit: " << n << " State: " << (((LogLevel >> n) & 1U == 1) ? "True" : "False") << std::endl;
    return ((LogLevel >> n) & 1U == 1);
}

void DLog::LOG::SetPrintLog(bool state)
{
    PrintLog = state;
}