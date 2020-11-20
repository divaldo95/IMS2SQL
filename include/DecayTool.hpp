/*
 * DecayTool.hpp
 * Created by David Baranyai
 * 2020.11.20
 */

#ifndef DecayTool_hpp
#define DecayTool_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "SQLConnector.hpp"

class DecayTool : public SQLConnector
{
private:
    std::stringstream query, log_msg;

    //Must handle the SQL connection output
    void LOGError(const char*) override;
    void LOGWarning(const char*) override;
    void LOGInfo(const char*) override;
    void LOGDeveloper(const char*) override;
public:
    DecayTool();
    ~DecayTool();

    void SetQCValidFlag(int, bool);
    void SetQCValidFlag(std::vector<int>, bool);

    std::string GetFileNamebyQCID(int);

    std::vector<int> GetIDsUnderThreshold(int);
};


#endif //DecayTool.hpp