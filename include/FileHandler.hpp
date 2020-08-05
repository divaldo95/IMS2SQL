/*
 * FileHandler.hpp
 * Created by David Baranyai
 * 2020.07.20
 */

#ifndef FileHandler_hpp
#define FileHandler_hpp

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "LOG.hpp"
#include "filesystem.hpp"

#define FILE_TAG "FileHandler"

class FileHandler
{
private:
    /* data */
    std::vector<std::filesystem::path> filePathList;
    bool recursive = false;
    DLog::LOG &log = DLog::LOG::GetInstance();
    std::stringstream log_msg;

public:
    FileHandler();
    ~FileHandler();

    void EnableRecursive();
    void DisableRecursive();

    void AppendDirectory(std::string);
    void AppendFile(std::string);

    //Return a const reference to avoid any modifications to the list
    const std::vector<std::filesystem::path>& GetFileList();
};


#endif //FileHandler.hpp