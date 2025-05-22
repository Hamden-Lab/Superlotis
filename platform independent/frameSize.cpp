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
              << " ["    << id.sensor_name   << "]" << std::endl;
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

void readFrameSize(PicamHandle camera)
{
    PicamError error;
    std::cout << "Read frame size: ";
    piint fsize;
    error = Picam_GetParameterIntegerValue(
        camera,
        PicamParameter_FrameSize,
        &fsize);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Frame suze is "
                  << fsize << std::endl;
    }
}

void setFrameSize(PicamHandle camera, piint frameSize)
{
    PicamError error;
    std::cout << "Read frame size: ";
    error = Picam_SetParameterIntegerValue(
        camera,
        PicamParameter_FrameSize,
        frameSize);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Frame suze is "
                  << frameSize << std::endl;
    }

}


int main(int argc, char* argv[])
{
    PicamError error;
    PicamHandle camera;
    PicamCameraID id;
    // piflt temp;
    // pibool lock = false;

    Picam_InitializeLibrary();

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
    
    if (argc == 1)
    {
        // Only read the current temperature
        readFrameSize(camera);
    }
    if (argc == 2)
    {
            std::string arg( argv[1] );
            piint frameSize;

            std::istringstream iss(arg.substr(0));
            if (!(iss >> frameSize))
        {
            std::cerr << "Invalid frame size: " << arg.substr(0) << std::endl;
            return 1;
        }

        setFrameSize(camera, frameSize);

            // Print current exposure time
        if (error == PicamError_None)
        {
            std::cout << "Exposure time value: " << frameSize << std::endl;
        }
        else
        {
            std::cout << "Failed to get exposure time." << std::endl;
    }
    }





    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();

    return 0;
}