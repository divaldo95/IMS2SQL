/*
 * IMSData.hpp
 * Created by David Baranyai
 * 2020.07.20
 */

#ifndef IMSData_hpp
#define IMSData_hpp

#include <iostream>
#include <vector>

struct MSG_ID
{
    std::string id_string;
    std::string source;
};

struct gSpectrum
{
    std::vector<int> X;
    std::vector<int> Y;
    std::vector<int> Error;
};

struct gEfficiency
{
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Error;
};

struct gEnergy
{
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Error;
};

struct SOHData
{
    std::vector<double> RoomTemperature;
    std::vector<std::string> ShieldStatus;
    std::vector<int> Humidity;
    std::vector<int> HighVoltage;
    std::vector<int> CrystalTemperature;
    std::vector<int> Timestamp;
    std::vector<std::string> DateTime;
    std::vector<int> MeasurementTime;
};

struct QCData
{
    std::string AcquisitionStartTime;
    double Realtime;
    double AcquisitionLength;
    std::string CalibrationDate;
    gSpectrum Spectrum;
    gEfficiency Efficiency;
    gEnergy Energy;
};

struct IMSData
{
    std::string detectorName;
    std::string relativepath;
    std::string absolutepath;
    std::string filename;
    MSG_ID msg_id;
    std::string dataType;
    SOHData sohData;
    QCData qcData;
};

inline std::ostream& operator<<(std::ostream& os, const IMSData& imsData)
{
    os << "IMSData: \nDetector name: " << imsData.detectorName << std::endl
        << "Relative Path: " << imsData.relativepath << std::endl
        << "Absolute Path: " << imsData.absolutepath << std::endl
        << "File name: " << imsData.filename << std::endl
        << "Data type: " << imsData.dataType << std::endl;
    if(imsData.dataType.compare("QCPHD") == 0)
    {
        os << "QC::Aquisition start time: " << imsData.qcData.AcquisitionStartTime << std::endl
            << "QC::Realtime: " << imsData.qcData.Realtime << std::endl
            << "QC::Acquisition length: " << imsData.qcData.AcquisitionLength << std::endl
            << "QC::Calibration date: " << imsData.qcData.CalibrationDate << std::endl
            << "QC::Spectrum row count: " << imsData.qcData.Spectrum.X.size() << std::endl
            << "QC::Efficiency row count: " << imsData.qcData.Efficiency.X.size() << std::endl
            << "QC::Energy row count: " << imsData.qcData.Energy.X.size();
    }
    else if(imsData.dataType.compare("RMSSOH") == 0)
    {
        os << "SOH::SOH row count: " << imsData.sohData.RoomTemperature.size();
    }
    else 
    {
        os << "Unknown data type.";
    }
    return os;
}

#endif //IMSData_hpp