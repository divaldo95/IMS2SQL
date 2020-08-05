START TRANSACTION;
SET autocommit = 0;
CREATE DATABASE IF NOT EXISTS Decay;
USE Decay;

CREATE TABLE `Detectors` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`Name` varchar(255) NOT NULL UNIQUE,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `ImportedFiles` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`DetectorID` INT NOT NULL,
	`FilePath` varchar(255) NOT NULL,
	`FileName` varchar(255) NOT NULL,
	`ImportDate` DATETIME NOT NULL DEFAULT NOW(),
	`ConvertDate` DATETIME NOT NULL,
	`ConvertedBy` varchar(255) NOT NULL,
	`DataType` varchar(255) NOT NULL,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `SOH` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`DetectorID` INT NOT NULL,
	`FileID` INT NOT NULL,
	`RoomTemperature` FLOAT NOT NULL,
	`ShieldStatus` BOOLEAN NOT NULL,
	`Humidity` INT NOT NULL,
	`HighVoltage` INT NOT NULL,
	`CrystalTemperature` INT NOT NULL,
	`Timestamp` TIMESTAMP NOT NULL,
	`DateTime` DATETIME NOT NULL,
	`MeasurementTime` INT NOT NULL,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `QC` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`DetectorID` INT NOT NULL,
	`FileID` INT NOT NULL,
	`AcquisitionStartTime` DATETIME NOT NULL,
	`Realtime` FLOAT NOT NULL,
	`AcquisitionLength` FLOAT NOT NULL,
	`CalibrationDate` DATETIME NOT NULL,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `QCEnergy` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`QCID` INT NOT NULL,
	`X` FLOAT NOT NULL,
	`Y` FLOAT NOT NULL,
	`Error` FLOAT NOT NULL,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `QCEfficiency` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`QCID` INT NOT NULL,
	`X` FLOAT NOT NULL,
	`Y` FLOAT NOT NULL,
	`Error` FLOAT NOT NULL,
	PRIMARY KEY (`ID`)
);

CREATE TABLE `QCSpectrum` (
	`ID` INT NOT NULL AUTO_INCREMENT,
	`QCID` INT NOT NULL,
	`X` FLOAT NOT NULL,
	`Y` FLOAT NOT NULL,
	`Error` FLOAT NOT NULL,
	PRIMARY KEY (`ID`)
);

ALTER TABLE `ImportedFiles` ADD CONSTRAINT `ImportedFiles_fk0` FOREIGN KEY (`DetectorID`) REFERENCES `Detectors`(`ID`);

ALTER TABLE `SOH` ADD CONSTRAINT `SOH_fk0` FOREIGN KEY (`DetectorID`) REFERENCES `Detectors`(`ID`);

ALTER TABLE `SOH` ADD CONSTRAINT `SOH_fk1` FOREIGN KEY (`FileID`) REFERENCES `ImportedFiles`(`ID`);

ALTER TABLE `QC` ADD CONSTRAINT `QC_fk0` FOREIGN KEY (`DetectorID`) REFERENCES `Detectors`(`ID`);

ALTER TABLE `QC` ADD CONSTRAINT `QC_fk1` FOREIGN KEY (`FileID`) REFERENCES `ImportedFiles`(`ID`);

ALTER TABLE `QCEnergy` ADD CONSTRAINT `QCEnergy_fk0` FOREIGN KEY (`QCID`) REFERENCES `QC`(`ID`);

ALTER TABLE `QCEfficiency` ADD CONSTRAINT `QCEfficiency_fk0` FOREIGN KEY (`QCID`) REFERENCES `QC`(`ID`);

ALTER TABLE `QCSpectrum` ADD CONSTRAINT `QCSpectrum_fk0` FOREIGN KEY (`QCID`) REFERENCES `QC`(`ID`);

COMMIT;
