#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib> // For exit
#include <cstdio>  // For printf
#include "picam.h"
#include <sys/stat.h>
#include <cerrno>   // For errno
#include <cstring>  // For strerror
#include <map>      // For std::map
#include <string>   // For std::string
#include <iomanip>  // For std::setprecision
#include <ctime>    // For std::time_t, std::tm, std::localtime, std::strftime


// Function to create a directory
void createDirectory(const std::string& dir) {
    struct stat info;

    if (stat(dir.c_str(), &info) != 0) {
        // Directory does not exist, try to create it
        if (mkdir(dir.c_str(), 0755) != 0) {
            std::cerr << "Error creating directory " << dir << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Directory " << dir << " created." << std::endl;
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << dir << " exists but is not a directory." << std::endl;
    }
}

// Function to print error code
void PrintError(PicamError error)
{
    if (error == PicamError_None)
        std::cout << "Succeeded" << std::endl;
    else
    {
        std::cout << "Failed (";
        // Print the error string
        const pichar* errorString;
        Picam_GetEnumerationString(PicamEnumeratedType_Error, error, &errorString);
        std::cout << errorString << ")" << std::endl;
        Picam_DestroyString(errorString); // Free the error string
    }
}

void PrintEnumString(PicamEnumeratedType type, piint value)
{
    const pichar* string;
    Picam_GetEnumerationString(type, value, &string);
    std::cout << string;
    Picam_DestroyString(string);
}

// - prints the camera identity
void PrintCameraID(const PicamCameraID& id)
{
    // - print the model
    PrintEnumString(PicamEnumeratedType_Model, id.model);

    // - print the serial number and sensor
    std::cout << " (SN:" << id.serial_number << ")"
              << " ["    << id.sensor_name   << "]" << std::endl;
}


// void SetTemperatureSetPoint(PicamHandle camera, piflt setTempSetPoint)
// {
//     PicamError error;
//     std::cout << "Set sensor temperature set point: ";
//     error = Picam_SetParameterFloatingPointValue(
//         camera,
//         PicamParameter_SensorTemperatureSetPoint,
//         setTempSetPoint);
//     PrintError(error);
// }

piflt SetTemperatureSetPoint(PicamHandle camera, piflt setTempSetPoint)
{
    PicamError error;
    std::cout << "Set sensor temperature set point: ";
    error = Picam_SetParameterFloatingPointValue(
        camera,
        PicamParameter_SensorTemperatureSetPoint,
        setTempSetPoint);
    PrintError(error);

    if (error == PicamError_None)
    {
        piflt newTempSetPoint;
        error = Picam_GetParameterFloatingPointValue(
            camera,
            PicamParameter_SensorTemperatureSetPoint,
            &newTempSetPoint);
        if (error == PicamError_None)
        {
            std::cout << "New sensor temperature set point: " << newTempSetPoint << " degrees C" << std::endl;
            return newTempSetPoint; // Return the new temperature
        }
        else
        {
            std::cerr << "Failed to read back the new temperature set point." << std::endl;
            return setTempSetPoint; // Return the original temperature
        }
    }
    return setTempSetPoint; // Return the original temperature if setting failed
}

void CheckStatus(PicamHandle camera)
{
    PicamError error;
    std::cout << "Read sensor temperature status: ";
    PicamSensorTemperatureStatus status;
    error = Picam_ReadParameterIntegerValue(
        camera,
        PicamParameter_SensorTemperatureStatus,
        reinterpret_cast<piint*>(&status));
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Status is ";
        PrintEnumString(PicamEnumeratedType_SensorTemperatureStatus, status);
        std::cout << std::endl;
    }
}

void ReadTemperature(PicamHandle camera)
{
    PicamError error;
    std::cout << "Read sensor temperature: ";
    piflt temperature;
    error = Picam_ReadParameterFloatingPointValue(
        camera,
        PicamParameter_SensorTemperatureReading,
        &temperature);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Temperature is "
                  << temperature << " degrees C" << std::endl;
    }
}



// Function to write frame data to CSV file
void writeFrameDataToCSV(const std::string& outputDir, int frameIndex, PicamCameraID id, const std::string& exposureStartTime, pi64s tsRes, piflt tempval)
{
    char idChar[256]; // Assuming a maximum length for the ID

    // Convert id.model and id.serial_number to strings
    std::stringstream modelStream;
    modelStream << id.model;
    std::string modelStr = modelStream.str();

    std::string serialStr = id.serial_number;

    // Use sprintf to format the camera ID
    sprintf(idChar, "%s:%s", modelStr.c_str(), serialStr.c_str());

    std::ostringstream oss;
    oss << outputDir << "/frame" << frameIndex << ".csv";
    std::string fileName = oss.str();
    std::ofstream outFile(fileName.c_str()); // Open file for writing

    if (!outFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
    }
    else {
        // Write metadata to CSV file
        outFile << "Camera ID," << idChar << std::endl;
        outFile << "Exposure Start Time," << exposureStartTime << std::endl;
        outFile << "Timestamp Resolution," << tsRes << std::endl;
        outFile << "Camera Temperature" << tempval << std::endl;
        
        outFile.close();
        std::cout << "Timestamp data saved to " << fileName << std::endl;
    }
}

// Function to get the current time as a string
std::string getCurrentTime() {
    std::time_t t = std::time(NULL);

    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
    return oss.str();
}

int main(int argc, char* argv[])
{
    Picam_InitializeLibrary();
    // PicamError error2;


    std::string outputDir = "output";
    createDirectory(outputDir);

    // // Check if the input is "./camera vars"
    // if (argc == 2 && std::string(argv[1]) == "vars")
    // {
    //     // Print the contents of config_file.h
    //     printAllVariables();
    // }

    // Open the first camera if any or create a demo camera
    PicamHandle camera;
    pibln bUseFrameTrack;
    pibln committed;
    PicamCameraID id;
    const pichar* string;
    PicamAvailableData data;
    piint readoutstride = 0;
    piint framesize = 0;
    piflt setTemp;


    // pi64s metadataOffset;
    PicamAcquisitionStatus errors; // Change the type to PicamAcquisitionStatus

    if (Picam_OpenFirstCamera(&camera) == PicamError_None)
        Picam_GetCameraID(camera, &id);
    else
    {
        Picam_ConnectDemoCamera(PicamModel_Sophia2048B, "12345", &id);
        Picam_OpenCamera(&id, &camera);
        std::cout << "No Camera Detected, Creating Demo Camera" << std::endl;
    }
    Picam_GetEnumerationString(PicamEnumeratedType_Model, id.model, &string);
    std::cout << string;
    std::cout << " (SN:" << id.serial_number << ")"
              << " [" << id.sensor_name << "]" << std::endl;
    Picam_DestroyString(string);

    // Set the default exposure time
    piflt exposureTime = 30000; // 30 seconds default exposure
    piflt temp = -35;



    if (argc == 3)
    {
        std::string arg1(argv[1]);
        std::string arg2(argv[2]);

        if (arg1 == "exposure")
    {
        std::string arg2(argv[2]);
        std::istringstream iss(arg2);
        std::cout << "Exposure time is: " << arg2 << std::endl;
        if (!(iss >> exposureTime))
        {
            std::cerr << "Invalid exposure time: " << arg2 << std::endl;
            return 1;
        }

    }
    std::cout << "Exposure Time: " << exposureTime << " ms" << std::endl;
    std::cout << "Temperature: " << temp << " C" << std::endl;

    }

    if (argc == 4)
    {
        // PicamError error;

        std::string arg1(argv[1]);
        std::string arg2(argv[2]);
        std::string arg3(argv[3]);

        if (arg1 == "temp" && arg2 == "lock")
        {
            std::istringstream iss(arg3);
            if (!(iss >> setTemp))
            {
                std::cerr << "Invalid temp: " << arg3 << std::endl;
                return 1;
            }
        }
            // Set temperature set point and lock, and print the new temperature
            piflt newTemp = SetTemperatureSetPoint(camera, setTemp);
            std::cout << "Temp that was set: " << newTemp << " degrees C" << std::endl;
        
        std::cout << "Exposure Time: " << exposureTime << " ms" << std::endl;
        std::cout << "Temperature: " << temp << " C" << std::endl;

        // if (error == PicamError_None)
        // {
        //     std::cout << "Temp that was set: " << setTemp << " degrees C" << std::endl;
        // }
        // else
        // {
        //     std::cout << "Failed to get set temp." << std::endl;
        // }
    }
            


    piint ts_mask_start = PicamTimeStampsMask_ExposureStarted;
    bUseFrameTrack = true;

    Picam_SetParameterIntegerValue(camera, PicamParameter_TimeStamps, ts_mask_start);
    Picam_SetParameterIntegerValue(camera, PicamParameter_TrackFrames, bUseFrameTrack);

    // Commit settings
    Picam_AreParametersCommitted(camera, &committed);
    if (!committed)
    {
        const PicamParameter* failed_parameter_array = NULL;
        piint failed_parameter_count = 0;

        Picam_CommitParameters(camera, &failed_parameter_array, &failed_parameter_count);
        if (failed_parameter_count)
            Picam_DestroyParameters(failed_parameter_array);
        else
        {
            pi64s tsRes;
            piint tsBitDepth;    // bit depth for Time Stamp(s)
            piflt tempval;

            Picam_GetParameterIntegerValue(camera, PicamParameter_ReadoutStride, &readoutstride);
            Picam_GetParameterIntegerValue(camera, PicamParameter_FrameSize, &framesize);
            Picam_GetParameterLargeIntegerValue(camera, PicamParameter_TimeStampResolution, &tsRes);
            Picam_GetParameterIntegerValue(camera, PicamParameter_TimeStampBitDepth, &tsBitDepth);
            Picam_GetParameterFloatingPointValue(camera, PicamParameter_SensorTemperatureStatus, &tempval);

            // std::cout << "Frame                   Exposure Start (us)     Exposure End (us)    Frame #     Status" << std::endl;

            // Read data from the camera
            Picam_StartAcquisition(camera);
            Picam_WaitForAcquisitionUpdate(camera, -1, &data, &errors);
            Picam_StopAcquisition(camera);

            if (data.readout_count > 0)
            {
                for (piint i = 0; i < data.readout_count; i++)
                {
                    // pibyte* buffer = (pibyte*)data.initial_readout + (i * readoutstride);
                    std::cout << "Frame data saved to " << outputDir << "/frame" << i << ".csv" << std::endl;
                        std::string exposureStartTime = getCurrentTime(); // Example function to get current time
                        piint numFramesToSave = 5;
                    for (piint frameIndex = 0; frameIndex < numFramesToSave; ++frameIndex) {
                        // Call writeFrameDataToCSV to save each frame
                        writeFrameDataToCSV(outputDir, frameIndex, id, exposureStartTime, tsRes, tempval);
                    }
                }
            }
        }
    }

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();

    return 0;
}











// //output exposing status (["exposing", "idle", "readout","writing"])
// //while exposing, status = "exposing"
// //while idle, status = "idle"
// //while reading, status = "readout"
// //while writing, status = "writeout"

// /*
// while exposing: change 'status' to 'exposing', and output the 

// */



        
    
    // Handle command line arguments for exposure time and temperature
    // if (argc == 2 || argc == 3)
    // {
    //     std::string arg1(argv[1]);
    //     std::istringstream iss1(arg1);
    //     if (!(iss1 >> exposureTime))
    //     {
    //         std::cerr << "Invalid exposure time: " << arg1 << std::endl;
    //         return 1;
    //     }

    //     if (argc == 3)
    //     {
    //         std::string arg2(argv[2]);
    //         std::istringstream iss2(arg2);
    //         if (!(iss2 >> temp))
    //         {
    //             std::cerr << "Invalid temperature: " << arg2 << std::endl;
    //             return 1;
    //         }
    //     }
    // }
    // else if (argc != 1)
    // {
    //     std::cerr << "Usage: " << argv[0] << " [exposureTime] [temp]" << std::endl;
    //     return 1;
    // }

    // Output the results

    

    // Set the exposure time
    // setExposureTime(camera, exposureTime);

    // Print the exposure time being used
    // std::cout << "Exposure time set to: " << std::fixed << std::setprecision(2) << exposureTime << " ms" << std::endl;

    // Enable metadata
    //piint ts_mask = PicamTimeStampsMask_ExposureStarted | PicamTimeStampsMask_ExposureEnded;



                // if (arg2 == "lock")
            // {
            // piflt setTemp;
            // // piflt temperature;
            // std::istringstream iss(arg3);
            // if (!(iss >> setTemp))
            // {
            //     std::cerr << "Invalid temp: " << arg3 << std::endl;
            //     return 1;
            // }

            // // Set temperature set point and lock
            // SetTemperatureSetPoint(camera, setTemp);
            // CheckStatus(camera);
            // ReadTemperature(camera);

            // error = Picam_GetParameterFloatingPointValue(camera, PicamParameter_SensorTemperatureSetPoint, &setTemp);

            // if (error1 == PicamError_None)
            // {
            //     std::cout << "Temp that was set: " << temperature << " degrees C" << std::endl;
            // }
            // else
            // {
            //     std::cout << "Failed to get set temp." << std::endl;
            // }
            // PicamError error;

