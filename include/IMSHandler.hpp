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
    bool explicitValidFlag = false; //if valid flag is ste explicitly
    bool explicitValidFlagState = false; //the value set
    std::string line;

    //Log
    DLog::LOG &log = DLog::LOG::GetInstance();
    std::stringstream log_msg;

    //Failed files
    std::vector<std::filesystem::path> failedFiles;

    //Store file data
    IMSData imsData;

    //Data validation
    bool thresholdMethod = false; //Filter QC files by threshold or when SUM = 0
    uint64_t QCSpectrumZeroCount = 0;
    uint64_t QCSpectrumZeroThreshold = 100; //Not all values are zero, so mark as not valid if the zero count is above the threshold

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
    void SetValidFlag(bool); //override valid flag
    void DisableValidFlagOverride();
};


#endif //IMSHandler.hpp