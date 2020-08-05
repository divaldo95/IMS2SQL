/*
 * FileHandler.hpp
 * Created by David Baranyai
 * 2020.07.20
 */
#include "FileHandler.hpp"

FileHandler::FileHandler()
{

}

FileHandler::~FileHandler()
{

}

void FileHandler::AppendDirectory(std::string path)
{
    if(!std::filesystem::exists(std::filesystem::path(path)))
    {
        log_msg.str("");
        log_msg << "Directory (" << path << ") does not exists. Skipping.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_WARNING, FILE_TAG, log_msg.str());
        return;
    }

    if(recursive)
    {
        for (const auto & entry : std::filesystem::recursive_directory_iterator(path))
        {
            //Most of IMS files don't have extension. Every file will be checked after opening.
            if(std::filesystem::is_directory(entry.path())) continue; //skip directories from list
            filePathList.push_back(entry.path());
        }
    }
    else
    {
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            //Most of IMS files don't have extension. Every file will be checked after opening.
            if(std::filesystem::is_directory(entry.path())) continue; //skip directories from list
            filePathList.push_back(entry.path());
        }
    }
    
}

void FileHandler::AppendFile(std::string filename)
{
    std::filesystem::path path(filename);
    if(!std::filesystem::exists(path))
    {
        log_msg.str("");
        log_msg << "File (" << path << ") does not exists. Skipping.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_WARNING, FILE_TAG, log_msg.str());
        return;
    }
    /*
    if(path.extension().compare(".root") != 0)
    {
        std::cout << "WARNING: File extension is not .root" << std::endl;
    }
    */
    filePathList.push_back(path);
}

const std::vector<std::filesystem::path>& FileHandler::GetFileList()
{
    return filePathList;
}

void FileHandler::EnableRecursive()
{
    recursive = true;
    log_msg.str("");
    log_msg << "Recursive directory scan enabled";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, FILE_TAG, log_msg.str());
}

void FileHandler::DisableRecursive()
{
    recursive = false;
    log_msg.str("");
    log_msg << "Recursive directory scan disabled";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, FILE_TAG, log_msg.str());
}