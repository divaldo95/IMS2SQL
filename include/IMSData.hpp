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
    inline void Clear()
    {
        id_string = "";
        source = "";
    }
};

struct Intensity
{
    std::vector<std::string> IsotopeName;
    std::vector<double> HalfLife;
    std::vector<double> RelativeActivity;
    std::vector<double> Energy;
    std::vector<double> EnergyPercentage;
    inline void Clear()
    {
        IsotopeName.clear();
        HalfLife.clear();
        RelativeActivity.clear();
        Energy.clear();
        EnergyPercentage.clear();
    }
};

struct QCCertificate
{
    int AbsoluteActivity;
    std::string DateTime;
    Intensity intensity;
    inline void Clear()
    {
        AbsoluteActivity = 0;
        DateTime = "";
        intensity.Clear();
    }
};

struct gSpectrum
{
    std::vector<int> X;
    std::vector<int> Y;
    std::vector<int> Error;
    inline void Clear()
    {
        X.clear();
        Y.clear();
        Error.clear();
    }
};

struct gEfficiency
{
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Error;
    inline void Clear()
    {
        X.clear();
        Y.clear();
        Error.clear();
    }
};

struct gEnergy
{
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Error;
    inline void Clear()
    {
        X.clear();
        Y.clear();
        Error.clear();
    }
};

struct SOHData
{
    std::vector<double> RoomTemperature;
    std::vector<std::string> ShieldStatus;
    std::vector<int> Humidity;
    std::vector<int> HighVoltage;
    std::vector<int> CrystalTemperature;
    std::vector<std::string> DateTime;
    std::vector<int> MeasurementTime;
    bool Valid;
    inline void Clear()
    {
        RoomTemperature.clear();
        ShieldStatus.clear();
        Humidity.clear();
        HighVoltage.clear();
        CrystalTemperature.clear();
        DateTime.clear();
        MeasurementTime.clear();
        Valid = true; //couldn't validate on uploading so explicitly set to true 
    }
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
    QCCertificate Certificate;
    bool Valid;
    inline void Clear()
    {
        AcquisitionLength = 0;
        AcquisitionStartTime = "";
        Realtime = 0;
        CalibrationDate = "";
        Spectrum.Clear();
        Efficiency.Clear();
        Energy.Clear();
        Certificate.Clear();
        Valid = false;
    }
};

struct IMSData
{
    std::string detectorName;
    std::string relativepath;
    std::string absolutepath;
    std::string filename;
    bool fileOK;
    MSG_ID msg_id;
    std::string dataType;
    SOHData sohData;
    QCData qcData;
    inline void Clear()
    {
        detectorName = "";
        relativepath = "";
        absolutepath = "";
        filename = "";
        fileOK = false;
        msg_id.Clear();
        dataType = "";
        sohData.Clear();
        qcData.Clear();
    }
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