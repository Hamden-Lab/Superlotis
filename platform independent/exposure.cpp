
#include <string>
#include <iostream>
#include <sstream>
#include "picam.h"

/**********CAMERA START UP**********/
/*Sets PicamParameters, using functions from picam.h*/

// - prints any picam enum
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
              << " [" << id.sensor_name << "]" << std::endl;
}

// - prints error code
void PrintError(PicamError error)
{
    if (error == PicamError_None)
        std::cout << "Succeeded" << std::endl;
    else
    {
        std::cout << "Failed (";
        PrintEnumString(PicamEnumeratedType_Error, error);
        std::cout << ")" << std::endl;
    }
}

void setExposureTime(PicamHandle camera, piflt setExpTime)
{
    PicamError error;
    std::cout << "Set exposure time to 120 ms: ";
    error = 
        Picam_SetParameterFloatingPointValue(
            camera,
            PicamParameter_ExposureTime,
            setExpTime );
    PrintError( error );

    //error = Picam_SetParameterFloatingPointValue(PicamParameter_ExposureTime
}

//generate the time stamp when exposure starts (instead of after)
void genTimeStamp(PicamHandle camera)
{
    PicamError error;
    std::cout << "Generate the time stamp when exposure starts";
    error = 
        Picam_SetParameterFloatingPointValue
        (
            camera,
            PicamParameter_TimeStamps,
            PicamTimeStampsMask_ExposureStarted
        );
    PrintError(error);

}


//in main: PicamTimeStampsMask:  The time will be stamped when exposure starts

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " time=<exposure_time_ms>" << std::endl;
        return 1;
    }

    std::string arg(argv[1]);
    if (arg.find("time=") != 0)
    {
        std::cerr << "Invalid argument format. Expected format: time=<exposure_time_ms>" << std::endl;
        return 1;
    }

    piflt exposureTime;
    std::istringstream iss(arg.substr(5)); // Extract the number after "time="
    if (!(iss >> exposureTime))
    {
        std::cerr << "Invalid exposure time: " << arg.substr(5) << std::endl;
        return 1;
    }

    Picam_InitializeLibrary();
    PicamError error;
    PicamHandle camera;
    PicamCameraID id;

    // Open first camera or create demo camera
    error = Picam_OpenFirstCamera(&camera);
    if (error == PicamError_None)
    {
        error = Picam_GetCameraID(camera, &id);
        if (error != PicamError_None)
        {
            std::cout << "Failed to get camera ID. ";
            PrintError(error);
            return 1;
        }
    }
    else
    {
        error = Picam_ConnectDemoCamera(PicamModel_Pixis100B, "12345", &id);
        if (error == PicamError_None)
        {
            error = Picam_OpenCamera(&id, &camera);
            if (error != PicamError_None)
            {
                std::cout << "Failed to open demo camera. ";
                PrintError(error);
                return 1;
            }
        }
        else
        {
            std::cout << "Failed to connect demo camera. ";
            PrintError(error);
            return 1;
        }
    }

    setExposureTime(camera, exposureTime);

    piflt expTime;
    std::cout << "Get exposure time: ";
    error = Picam_GetParameterFloatingPointValue(camera, PicamParameter_ExposureTime, &expTime);

    // Print current exposure time
    if (error == PicamError_None)
    {
        std::cout << "Exposure time value: " << expTime << std::endl;
    }
    else
    {
        std::cout << "Failed to get exposure time." << std::endl;
    }

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();

    return 0;
}