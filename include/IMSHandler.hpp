/*
 * IMSHandler.hpp
 * Created by David Baranyai
 * 2020.07.17
 */

#ifndef IMSHandler_hpp
#define IMSHandler_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <ctime> 
#include <vector>
#include <iterator>
#include "filesystem.hpp"
#include "IMSData.hpp"
#include "LOG.hpp"

#define IMS_TAG "IMSHandler"

/*
* ExplodeString(Vector reference to store the splitted strings, string to split, delimiter)
*/
static void ExplodeString(std::vector<std::string>&, const std::string&, const char&);

class IMSHandler
{
private:
    std::ifstream inFile;
    bool fileOK = false;
    std::string line;

    //Log
    DLog::LOG &log = DLog::LOG::GetInstance();
    std::stringstream log_msg;

    //Failed files
    std::vector<std::filesystem::path> failedFiles;

    //Store file data
    IMSData imsData;

    /*
     * Check file header
     */
    bool CheckFile();
    
    void ReadRMSSOH();
    void ReadQCPHD();

    //Split function by Tamás
    inline std::vector<std::string> split(std::string const &input) 
    { 
        std::istringstream buffer(input);
        std::vector<std::string> ret{std::istream_iterator<std::string>(buffer), std::istream_iterator<std::string>()};
        return ret;
    }

public:
    IMSHandler();
    ~IMSHandler();

    bool Open(std::filesystem::path);
    const IMSData& GetIMSData();
    void Close(); 
};


#endif //IMSHandler.hpp