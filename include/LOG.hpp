//
//  LOG.hpp
//  LOG
//
//  Created by Baranyai David on 2019. 09. 23..
//  Copyright © 2019. Baranyai David. All rights reserved.
//

#ifndef LOG_hpp
#define LOG_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime> 
#include <mutex>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "filesystem.hpp"

namespace DLog
{
    enum DEBUG_BIT
    {
        ERROR_BIT = 0,
        WARNING_BIT = 1,
        INFO_BIT = 2,
        DEVELOPER_BIT = 3
    };

    enum MESSAGE_TYPE
    {
        MESSAGE_INFO,
        MESSAGE_WARNING,
        MESSAGE_ERROR,
        MESSAGE_DEVELOPER
    };

    class LOG
    {
    public:
        static LOG& GetInstance()
        {
            static LOG instance;
            return instance;
        }
        void SetLogLevel(const int);
        void Append(const int&, const char*, const char*);
        void Append(const int&, const char*, const std::string&);
        void InsertSeparator();
        ~LOG();

        //prevent copying or moving the instance
        LOG(const LOG&) = delete;
        LOG(LOG&&) = delete;
        LOG& operator=(const LOG&) = delete;
        LOG& operator=(LOG&&) = delete;
        
        void SetBit(DEBUG_BIT, bool);
        bool GetBit(DEBUG_BIT);

        void SetPrintLog(bool);

    private:
        LOG();
        std::string GetCurrentFormattedDate();

        unsigned long LogLevel = 0;
        bool PrintLog = true;
        std::string DebugInt2String(const int);
        
        std::ofstream logfile;

        std::mutex append_mutex;
    };
}

#endif /* LOG_hpp */