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
    imsData.Clear();
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
        imsData.Clear(); //reset struct
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
			log_msg.str("");
			log_msg << "Error while processing file (" << path << ")";
			log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
        }
        
    }
    else
    {
        success = false;
        log_msg.str("");
        log_msg << "Error while opening file (" << path << ")";
        failedFiles.push_back(path);
        log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
    }
    return success;
}

bool IMSHandler::CheckFile()
{
	bool begin = false;
    bool header = false;
    bool found = false; //flag for checking non necessary things
    uint64_t file_pos = 0;
    std::vector<std::string> elems;
    std::string item;
    char delim = ' ';
	
	while (getline(inFile,line))
    {   
		elems.clear();
        ExplodeString(elems, line, ' ');
		if(elems.size() != 2)
		{
			continue;
		}
		if(elems[0].compare("BEGIN") == 0) 
		{
			if(!(elems[1].compare("IMS2.0") == 0 || elems[1].compare("IMS1.0") == 0)) //if no match
			{
				log_msg.str("");
				log_msg << "Error after BEGIN word. Got \'" << elems[1] << "\' instead of IMS1.0 or IMS2.0.";
				log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
			}
			else
			{
				begin = true;
				break;
			}
		}
	}
	if(!begin)
	{
		log_msg.str("");
		log_msg << "Could not find BEGIN word. Probably not a valid IMS file.";
		log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
		return false;
	}
    elems.clear();
    file_pos = inFile.tellg(); //store the current position
	std::cout << "Current position before MSG_TYPE: " << file_pos << std::endl;

    while (getline(inFile,line))
    {   
		elems.clear();
        ExplodeString(elems, line, ' ');
		if(elems.size() != 2)
		{
			continue;
		}
		if(elems[0].compare("MSG_TYPE") == 0) 
        {
            if(elems[1].compare("DATA") == 0)
            {
                file_pos = inFile.tellg();
                found = true;
                break;
            }
            else
            {
                log_msg.str("");
                log_msg << "Invalid data. Got \'" << elems[0] << " " << elems[1] << "\', but expected MSG_TYPE DATA";
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
				found = true;
                break;
            }
        }
	}
	if(!found)
	{
        inFile.seekg(file_pos); //return back to initial position
		log_msg.str("");
		log_msg << "Could not find MSG_TYPE word.";
		log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
	}
    found = false;
    elems.clear();

    while (getline(inFile,line))
    {   
		elems.clear();
        ExplodeString(elems, line, ' ');
		if(elems.size() > 3)
		{
			continue;
		}
		if(elems[0].compare("MSG_ID") == 0) 
        {
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
            else 
            {
                log_msg.str("");
                log_msg << "Invalid data near MSG_ID. Got \'" << line << "\'.";
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
                break;
            }
            file_pos = inFile.tellg();
            found = true;
            break; 
        }
	}
	if(!found)
	{
        inFile.seekg(file_pos); //return back to initial position
		log_msg.str("");
		log_msg << "Could not find MSG_ID word.";
		log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
	}
    found = false;
    elems.clear();

    while (getline(inFile,line))
    {   
		elems.clear();
        ExplodeString(elems, line, ' ');
		if(elems.size() != 2)
		{
			continue;
		}
        if(elems[0].compare("DATA_TYPE") == 0) 
        {
            if(elems[1].compare("QCPHD") == 0 || elems[1].compare("RMSSOH") == 0)
            {
                imsData.dataType = elems[1];
                log_msg.str("");
                log_msg << "Data type: " << imsData.dataType;
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
                file_pos = inFile.tellg();
                found = true;
                break; 
            }
            else 
            {
                log_msg.str("");
                log_msg << "Unknown data type: " << elems[1];
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
                return false;
            }
        }
        
	}
	if(!found)
	{
        inFile.seekg(file_pos); //return back to initial position
		log_msg.str("");
		log_msg << "Could not find DATA_TYPE word. Skipping invalid file.";
		log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str());
	}
    found = false;
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
            try
            {
                imsData.sohData.RoomTemperature.push_back(std::stod(sep[0]));
                imsData.sohData.ShieldStatus.push_back(sep[1].compare("CLOSED") == 0 ? "TRUE" : "FALSE"); //CLOSED = TRUE | OPEN = FALSE
                imsData.sohData.Humidity.push_back(std::stoi(sep[2]));
                imsData.sohData.HighVoltage.push_back(std::stoi(sep[3]));
                imsData.sohData.CrystalTemperature.push_back(std::stoi(sep[4]));
                date.str("");
                date << sep[8] << " " << sep[9];
                imsData.sohData.DateTime.push_back(date.str());
                imsData.sohData.MeasurementTime.push_back(std::stoi(sep[10]));
            }
            catch(const std::invalid_argument& ia)
            {
                fileOK = false;
                log_msg.str("");
                log_msg << "Exception occured on ReadRMSSOH: " << ia.what();
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str().c_str());
                return;
            }
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
    bool certificatestart = false;
    bool certificate = false;
    std::stringstream date;
    uint64_t sum = 0;
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

        if (line == "#Certificate")
            certificatestart = true;
        else if (line.front() == '#' || line == "STOP")
            certificate = false;
        
        
        if (gspectrum)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<2) continue;
            for(int i=0;i<sep.size()-1;i++)
            {
                try
                {
                    imsData.qcData.Spectrum.X.push_back(std::stoi(sep[0])+i);
                    imsData.qcData.Spectrum.Y.push_back(std::stoi(sep[i+1]));
                    imsData.qcData.Spectrum.Error.push_back(0);
                    sum += stoi(sep[i+1]);
                    //if(std::stoi(sep[i+1]) == 0 && thresholdMethod) QCSpectrumZeroCount++;
                    //else if(std::stoi(sep[i+1]) > 0) imsData.qcData.Valid = true; //if at least one is not zero
                }
                catch(const std::invalid_argument& ia)
                {
                    fileOK = false;
                    log_msg.str("");
                    log_msg << "Exception occured on ReadQCPHD (spectrum): " << ia.what();
                    log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str().c_str());
                    return;
                }
                
            }        
        }
        
        if (gspectrumstart)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<2) continue;
            gspectrumstart = 0;
            gspectrum = 1;
        }

        if (certificate)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<10) continue;
            imsData.qcData.Certificate.intensity.IsotopeName.push_back(sep[0]);
            imsData.qcData.Certificate.intensity.HalfLife.push_back(std::stod(sep[1]));
            imsData.qcData.Certificate.intensity.RelativeActivity.push_back(std::stod(sep[3]));
            imsData.qcData.Certificate.intensity.Energy.push_back(std::stod(sep[5]));
            imsData.qcData.Certificate.intensity.EnergyPercentage.push_back(std::stod(sep[6]));
        }
        
        if (certificatestart)
        {
            try
            {
                std::vector<std::string> sep = split(line);
                if (sep.size()<2) continue;
                imsData.qcData.Certificate.AbsoluteActivity = std::stoi(sep[0]);
                date.str("");
                date << sep[1] << " " << sep[2];
                imsData.qcData.Certificate.DateTime = date.str();
                certificatestart = 0;
                certificate = 1;
            }
            catch(const std::invalid_argument& ia)
            {
                fileOK = false;
                log_msg.str("");
                log_msg << "Exception occured on ReadQCPHD (certificatestart): " << ia.what();
                log.Append(DLog::MESSAGE_TYPE::MESSAGE_ERROR, IMS_TAG, log_msg.str().c_str());
                return;
            }
            
        }
                
        if (gefficiency)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<3) continue;
            imsData.qcData.Efficiency.X.push_back(std::stod(sep[0]));
            imsData.qcData.Efficiency.Y.push_back(std::stod(sep[1]));
            imsData.qcData.Efficiency.Error.push_back(std::stod(sep[2]));
        }
        
        if (genergy)
        {
            std::vector<std::string> sep = split(line);
            if (sep.size()<3) continue;
            imsData.qcData.Energy.X.push_back(std::stod(sep[0]));
            imsData.qcData.Energy.Y.push_back(std::stod(sep[1]));
            imsData.qcData.Energy.Error.push_back(std::stod(sep[2]));
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
    if(explicitValidFlag) //override at the end if needed
    {
        imsData.qcData.Valid = explicitValidFlagState;
    }
    else if(sum < sumThreshold) //use threshold method will override the SUM method
    {
        imsData.qcData.Valid = false;
    }
    else
    {
        imsData.qcData.Valid = true;
    }
    
    fileOK = true;
    log_msg.str("");
    log_msg << "Reading file done." << std::endl << imsData;
    log.Append(DLog::MESSAGE_TYPE::MESSAGE_INFO, IMS_TAG, log_msg.str());
}

const IMSData& IMSHandler::GetIMSData()
{
    imsData.fileOK = fileOK; //If file read correctly
    return imsData;
}

void IMSHandler::SetValidFlag(bool state)
{
    explicitValidFlag = true;
    explicitValidFlagState = state;
}

void IMSHandler::DisableValidFlagOverride()
{
    explicitValidFlag = false;
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

void IMSHandler::SetThreshold(unsigned int threshold)
{
    sumThreshold = threshold;
}