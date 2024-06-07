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

void setShutterModeNormal(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to normal: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_Normal
        );
    PrintError(error);

}

void setShutterModeAlwaysClosed(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to always closed: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_AlwaysClosed
        );
    PrintError(error);

}

void setShutterModeAlwaysOpen(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to always open: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_AlwaysOpen
        );
    PrintError(error);

}

void setShutterModeOpenBeforeTrigger(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to open before trigger: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_OpenBeforeTrigger
        );
    PrintError(error);

}

int main(int argc, char* argv[])
{
    Picam_InitializeLibrary();
    PicamError error;
    PicamHandle camera;
    PicamCameraID id;
    piint shutterStatus;

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
        };
    };

    //std::cout << "Get shutter status: ";
    if (argc == 1)
    { error = Picam_GetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, &shutterStatus);
        
        if (error == PicamError_None)
        {
            if (shutterStatus == 1)
            {
                std::cout << "normal timing mode" << std::endl;
            }
            else if (shutterStatus == 2)
            {
                std::cout << "shutter always closed" << std::endl;
            }
            else if (shutterStatus == 3)
            {
                std::cout << "shutter always open" << std::endl;
            }
            else if (shutterStatus == 4)
            {
                std::cout << "shutter open before trigger" << std::endl;
            }
            else
            {
                std::cout << "Unknown shutter status: " << shutterStatus << std::endl;
            }
        }
    }
        // else
        // {
        //     std::cout << "Failed to get shutter status value. ";
        //     PrintError(error);
        
        // };
        if (argc == 2)
        {
        std::string arg( argv[1] );
        if( arg == "shutter always open" ){
            setShutterModeAlwaysOpen(camera);
        }
        else if( arg == "shutter always closed" ){
            setShutterModeAlwaysClosed(camera);
            // std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else if( arg == "shutter open before trigger" ){
            setShutterModeOpenBeforeTrigger(camera);
            // std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else if( arg == "normal timing mode" ){
            setShutterModeNormal(camera);
            // std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else{
            std::cout << "Failed to set shutter status." << std::endl;
        }
        

        }

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();
}


