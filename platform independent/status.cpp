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


void getShutterStatus(PicamHandle camera)
{
    piint shutterStatus;
    PicamError error;
    std::cout << "Get shutter status: ";
    error = 
        Picam_GetParameterIntegerValue(
            camera, 
            PicamParameter_ShutterTimingMode, 
            &shutterStatus);
    PrintError(error);
}

void getTempStatus(PicamHandle camera)
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

void getVacuumStatus(PicamHandle camera)
{
    PicamError error;
    std::cout << "Get vacuum status: ";
    piint vacStatus;
    error = Picam_ReadParameterIntegerValue(
        camera,
        PicamParameter_VacuumStatus,
        &vacStatus);
    PrintError(error);
}

void getCoolingStatus(PicamHandle camera)
{
    PicamError error;
    piint coolingStatus;
    std::cout << "Get cooling fan status: ";
    error = Picam_ReadParameterIntegerValue(
        camera,
        PicamParameter_CoolingFanStatus,
        &coolingStatus);
    PrintError(error);
}

int main()
{    Picam_InitializeLibrary();

    // - open the first camera if any or create a demo camera
    PicamHandle camera;
    PicamCameraID id;
    if( Picam_OpenFirstCamera( &camera ) == PicamError_None )
        Picam_GetCameraID( camera, &id );
    else
    {
        Picam_ConnectDemoCamera(
            PicamModel_Pixis100B,
            "12345",
            &id );
        Picam_OpenCamera( &id, &camera );
    }
    getShutterStatus(camera);
    getTempStatus(camera);
    getVacuumStatus(camera);
    getCoolingStatus(camera);
    // std::cout << "Set temp" <<std::endl;
    // SetTemperature(camera, temp_val, lock);
    // std::cout << std::endl;
    // ReadTemperature( camera, lock );


    Picam_CloseCamera( camera );

    Picam_UninitializeLibrary();
}






/*        // Retrieve and print vacuum status
        error = Picam_GetParameterIntegerValue(camera, PicamParameter_VacuumStatus, &vacuumStatus);
        if (error == PicamError_None)
        {
            std::cout << "Vacuum status: ";
            if (vacuumStatus == 1)
                std::cout << "Sufficient";
            else if (vacuumStatus == 2)
                std::cout << "Low";
            else
                std::cout << "Unknown";
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Failed to get vacuum status. ";
            PrintError(error);
        }

        // Retrieve and print cooling fan status
        error = Picam_GetParameterIntegerValue(camera, PicamParameter_CoolingFanStatus, &coolingFanStatus);
        if (error == PicamError_None)
        {
            std::cout << "Cooling fan status: ";
            if (coolingFanStatus == 1)
                std::cout << "Off";
            else if (coolingFanStatus == 2)
                std::cout << "On";
            else if (coolingFanStatus == 3)
                std::cout << "Forced On";
            else
                std::cout << "Unknown";
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Failed to get cooling fan status. ";
            PrintError(error);
        }
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string parameter(argv[i]);
            if (parameter == "shutterStatus")
            {
                error = Picam_GetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, &shutterStatus);
                if (error == PicamError_None)
                {
                    std::cout << "Shutter timing mode: ";
                    PrintEnumString(PicamEnumeratedType_ShutterTimingMode, shutterStatus);
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "Failed to get shutter status. ";
                    PrintError(error);
                }
            }
            else if (parameter == "vacuumStatus")
            {
                error = Picam_GetParameterIntegerValue(camera, PicamParameter_VacuumStatus, &vacuumStatus);
                if (error == PicamError_None)
                {
                    std::cout << "Vacuum status: ";
                    if (vacuumStatus == 1)
                        std::cout << "Sufficient";
                    else if (vacuumStatus == 2)
                        std::cout << "Low";
                    else
                        std::cout << "Unknown";
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "Failed to get vacuum status. ";
                    PrintError(error);
                }
            }
            else if (parameter == "coolingFanStatus")
            {
                error = Picam_GetParameterIntegerValue(camera, PicamParameter_CoolingFanStatus, &coolingFanStatus);
                if (error == PicamError_None)
                {
                    std::cout << "Cooling fan status: ";
                    if (coolingFanStatus == 1)
                        std::cout << "Off";
                    else if (coolingFanStatus == 2)
                        std::cout << "On";
                    else if (coolingFanStatus == 3)
                        std::cout << "Forced On";
                    else
                        std::cout << "Unknown";
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "Failed to get cooling fan status. ";
                    PrintError(error);
                }
            }
            else
            {
                std::cout << "Unknown parameter: " << parameter << std::endl;
            }
        }
    }

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();*/

/*A series of if statements to troubleshoot any problems*/
