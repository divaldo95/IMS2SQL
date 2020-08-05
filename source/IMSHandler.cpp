/*
* IMSHandler.hpp
* Created by David Baranyai
* 2020.07.17
*/

#include "IMSHandler.hpp"

IMSHandler::IMSHandler()
{

}

IMSHandler::~IMSHandler()
{
    if(failedFiles.size() != 0)
    {
        std::cout << "There were an error while processing some files. Check log file." << std::endl;
        log_msg.str("");
        log_msg << "Failed file list (" << failedFiles.size() << "):";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        for(int i = 0; i < failedFiles.size(); i++)
        {
            log_msg.str("");
            log_msg << failedFiles[i];
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        }
    }
    else
    {
        log_msg.str("");
        log_msg << "All files processed.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
    }
    
}

bool IMSHandler::Open(std::filesystem::path path)
{
    bool success = false;
    log_msg.str("");
    log_msg << "Opening file (" << path << ")...";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
    inFile.open(path, std::ifstream::in);
    if(inFile.is_open())
    {
        imsData = {}; //reset struct
        imsData.relativepath = path.parent_path();
        imsData.absolutepath = std::filesystem::absolute(path); 
        imsData.filename = path.filename();
        if(CheckFile())
        {
            if(imsData.dataType.compare("QCPHD") == 0) ReadQCPHD();
            else if(imsData.dataType.compare("RMSSOH") == 0) ReadRMSSOH();
            success = true;
        }
        else
        {
            success = false;
            failedFiles.push_back(path);
        }
        
    }
    else
    {
        success = false;
        log_msg.str("");
        log_msg << "Error while opening file";
        failedFiles.push_back(path);
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
    }
    return success;
}

bool IMSHandler::CheckFile()
{
    bool header = false;
    std::vector<std::string> elems;
    std::string item;
    char delim = ' ';
    std::getline(inFile, line);
    ExplodeString(elems, line, ' ');
    if(elems.size() != 2)
    {
        log_msg.str("");
        log_msg << "Error before file begin check. Probably not a valid IMS2.0 file.";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    if(elems[0].compare("BEGIN") !=0 || elems[1].compare("IMS2.0") != 0) 
    {
        log_msg.str("");
        log_msg << "Not a valid IMS2.0 file";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    elems.clear();

    std::getline(inFile, line);
    ExplodeString(elems, line, ' ');
    if(elems.size() != 2) 
    {
        log_msg.str("");
        log_msg << "Unexpected line. Got \'" << line << "\', but expected \'MSG_TYPTE DATA\'";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    if(elems[0].compare("MSG_TYPE") !=0 || elems[1].compare("DATA") != 0) 
    {
        log_msg.str("");
        log_msg << "Invalid data. Got \'" << elems[0] << " " << elems[1] << "\', but expected MSG_TYPE DATA";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    elems.clear();

    std::getline(inFile, line);
    ExplodeString(elems, line, ' ');
    if(elems.size() < 2) 
    {
        log_msg.str("");
        log_msg << "Unexpected line. Got \'" << line << "\', but expected \'MSG_ID data...\'";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    if(elems[0].compare("MSG_ID") !=0)
    {
        log_msg.str("");
        log_msg << "Unexpected line. Got \'" << line << "\', but expected MSG_ID";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    if(elems.size() == 2) 
    { 
        imsData.msg_id.id_string = elems[1]; 
        std::cout << "MSG_ID string: " << imsData.msg_id.id_string << std::endl;
    }
    else if(elems.size() == 3) 
    { 
        imsData.msg_id.id_string = elems[1]; 
        imsData.msg_id.source = elems[2];

        log_msg.str("");
        log_msg << "MSG_ID string: " << imsData.msg_id.id_string;
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());

        log_msg.str("");
        log_msg << "MSG_ID source: " << imsData.msg_id.source;
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
    }
    elems.clear();

    std::getline(inFile, line);
    ExplodeString(elems, line, ' ');
    if(elems.size() != 2) 
    {
        log_msg.str("");
        log_msg << "Unexpected line. Got \'" << line << "\', but expected DATA_TYPE data...";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    if(elems[0].compare("DATA_TYPE") !=0) 
    {
        log_msg.str("");
        log_msg << "Unexpected line. Got \'" << line << "\', but expected DATA_TYPE";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        return false;
    }
    else
    {
        if(elems[1].compare("QCPHD") == 0 || elems[1].compare("RMSSOH") == 0)
        {
            imsData.dataType = elems[1];
            log_msg.str("");
            log_msg << "Data type: " << imsData.dataType;
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
        }
        else 
        {
            std::cout << "Unknown data type" << std::endl;
            log_msg.str("");
            log_msg << "Unknown data type: " << elems[1];
            log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
            return false;
        }
    }
    
    elems.clear();

    while (getline(inFile,line))
    {   
        if (line == "#Header 3")
            header = true;
        else if (line.front() == '#' || line == "STOP")
            header = false;
        
        if (header)
        {
            getline(inFile,line);
            std::vector<std::string> sep = split(line);
            if (sep.size()<1) continue;
            imsData.detectorName = sep[0];
            break;
        }
    }

    std::cout << "File header OK" << std::endl;
    log_msg.str("");
    log_msg << "Header check done.";
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
    return true;
}

void IMSHandler::Close()
{
    imsData = {}; //reset struct
    fileOK = false;
    inFile.close();
}

void IMSHandler::ReadRMSSOH()
{
    bool DetEnv = false;
    std::stringstream date;
    while (getline(inFile,line))
    {   
        if (line == "#DetEnv")
            DetEnv = true;
        else if (line.front() == '#' || line == "STOP")
            DetEnv = false;
        
        if (DetEnv)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<11) continue;
            if (stod(sep[3]) < 100) continue; //no high voltage -> skip
            
            
            int yearmonth = sep[8].find('/');
            int monthday = sep[8].rfind('/');
            int year = stoi(sep[8].substr(0,yearmonth));
            int month = stoi(sep[8].substr(yearmonth+1,monthday-yearmonth-1));
            int day = stoi(sep[8].substr(monthday+1));
            
            int hourminute = sep[9].find(':');
            int minutesec = sep[9].rfind(':');
            int hour = stoi(sep[9].substr(0,hourminute));
            int minute = stoi(sep[9].substr(hourminute+1,minutesec-hourminute-1));
            int sec = stoi(sep[9].substr(minutesec+1));
            
            
            time_t rawtime;
            time(&rawtime);
            struct tm* timeinfo = localtime ( &rawtime ); 
            
            timeinfo->tm_year   = year - 1900;
            timeinfo->tm_mon    = month - 1;    //months since January - [0,11]
            timeinfo->tm_mday   = day;          //day of the month - [1,31] 
            timeinfo->tm_hour   = hour;         //hours since midnight - [0,23]
            timeinfo->tm_min    = minute;       //minutes after the hour - [0,59]
            timeinfo->tm_sec    = sec;          //seconds after the minute - [0,59]
            
            imsData.sohData.RoomTemperature.push_back(std::stod(sep[0]));
            imsData.sohData.ShieldStatus.push_back(sep[1].compare("CLOSED") == 0 ? "TRUE" : "FALSE"); //CLOSED = TRUE | OPEN = FALSE
            imsData.sohData.Humidity.push_back(std::stoi(sep[2]));
            imsData.sohData.HighVoltage.push_back(std::stoi(sep[3]));
            imsData.sohData.CrystalTemperature.push_back(std::stoi(sep[4]));
            imsData.sohData.Timestamp.push_back(timegm(timeinfo)); //converted timestamp
            date.str("");
            date << sep[8] << " " << sep[9];
            imsData.sohData.DateTime.push_back(date.str());
            imsData.sohData.MeasurementTime.push_back(std::stoi(sep[10]));

            /*
            timestamps[numofvalues] = timegm(timeinfo);
            roomtemps[numofvalues] = stod(sep[0]);
            humidities[numofvalues] = stod(sep[2]);
            crystaltemps[numofvalues] = stod(sep[4]);
            highvoltages[numofvalues] = stod(sep[3]);
            numofvalues++;
            */
        }
    }
    if(imsData.sohData.HighVoltage.size() < 1)
    {
        fileOK = false;
        log_msg.str("");
        log_msg << "Read or file error. Row count is 0";
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
    }
    else
    {
        fileOK = true;
        log_msg.str("");
        log_msg << "Reading file done." << std::endl << imsData;
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
    }
    
}

void IMSHandler::ReadQCPHD()
{
    bool gspectrumstart = false;
    bool gspectrum = false;
    bool gefficiency = false;
    bool genergy = false;
    bool acqu = false;
    bool calibration = false;
    std::stringstream date;
    while (getline (inFile,line))
    {
        if (line == "#g_Spectrum")
            gspectrumstart = true;
        else if (line.front() == '#' || line == "STOP")
            gspectrum = false;
        
        if (line == "#g_Efficiency")
            gefficiency = true;
        else if (line.front() == '#' || line == "STOP")
            gefficiency = false;
        
        if (line == "#g_Energy")
            genergy = true;
        else if (line.front() == '#' || line == "STOP")
            genergy = false;
        
        if (line == "#Acquisition")
            acqu = true;
        else if (line.front() == '#' || line == "STOP")
            acqu = false;

        if (line == "#Calibration")
            calibration = true;
        else if (line.front() == '#' || line == "STOP")
            calibration = false;
        
        
        if (gspectrum)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<2) continue;
            for(int i=0;i<sep.size()-1;i++)
            {
                imsData.qcData.Spectrum.X.push_back(std::stoi(sep[0])+i);
                imsData.qcData.Spectrum.Y.push_back(std::stoi(sep[i+1]));
                imsData.qcData.Spectrum.Error.push_back(0);
                /*
                xvalues[numofvalues] = std::stoi(sep[0])+i;
                errorvalues[numofvalues] = 0;
                yvalues[numofvalues] = std::stoi(sep[i+1]);
                numofvalues++;
                */
            }        
        }
        
        
        if (gspectrumstart)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<2) continue;
            gspectrumstart = 0;
            gspectrum = 1;
        }
                
        if (gefficiency)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<3) continue;
            imsData.qcData.Efficiency.X.push_back(std::stod(sep[0]));
            imsData.qcData.Efficiency.Y.push_back(std::stod(sep[1]));
            imsData.qcData.Efficiency.Error.push_back(std::stod(sep[2]));
            /*
            effxvalues[numofeffvalues] = std::stod(sep[0]);
            effyvalues[numofeffvalues] = std::stod(sep[1]);
            erroreffvalues[numofeffvalues] = std::stod(sep[2]);
            numofeffvalues++;      
            */
        }
        
        if (genergy)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<3) continue;
            imsData.qcData.Energy.X.push_back(std::stod(sep[0]));
            imsData.qcData.Energy.Y.push_back(std::stod(sep[1]));
            imsData.qcData.Energy.Error.push_back(std::stod(sep[2]));
            /*
            calxvalues[numofcalvalues] = std::stod(sep[0]);
            calyvalues[numofcalvalues] = std::stod(sep[1]);
            numofcalvalues++;  
            */    
        }
        
        if (acqu)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<4) continue;
            date.str("");
            date << sep[0] << " " << sep[1];
            imsData.qcData.AcquisitionStartTime = date.str();
            imsData.qcData.Realtime = std::stod(sep[2]);
            imsData.qcData.AcquisitionLength = stod(sep[3]);
        }

        if (calibration)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<2) continue;
            date.str("");
            date << sep[0] << " " << sep[1];
            imsData.qcData.CalibrationDate = date.str();
        }
    }
    fileOK = true;
    log_msg.str("");
    log_msg << "Reading file done." << std::endl << imsData;
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
}

const IMSData& IMSHandler::GetIMSData()
{
    if(!fileOK)
    {
        imsData = {};
    }
    return imsData;
}

//------------------------------------------------------------------------------------------------
static void ExplodeString(std::vector<std::string>& out, const std::string& s, const char& c)
{
    std::string buff{""};
	
	for(auto n:s)
	{
		if(n != c) buff+=n; 
        else
        {
            if(n == c && buff != "") 
            { 
                out.push_back(buff); buff = ""; 
            }
        }
		
	}
	if(buff != "") out.push_back(buff);
}