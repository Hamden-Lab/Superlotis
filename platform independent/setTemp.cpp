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

void SetTemperatureSetPoint(PicamHandle camera, piflt setTempSetPoint)
{
    PicamError error;
    std::cout << "Set sensor temperature set point: ";
    error = Picam_SetParameterFloatingPointValue(
        camera,
        PicamParameter_SensorTemperatureSetPoint,
        setTempSetPoint);
    PrintError(error);
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
        ReadTemperature(camera);
    }
    else if (argc == 2)
    {
        std::string arg1(argv[1]);
        if (arg1 == "lock")
        {
            std::cout << "Error: Temperature set point not provided." << std::endl;
            return 1;
        }
        else
        {
            std::cout << "Error: Invalid arguments." << std::endl;
            return 1;
        }
    }
    else if (argc == 3)
    {
        std::string arg1(argv[1]);
        std::string arg2(argv[2]);

        if (arg1 == "lock")
        {
            piflt setTemp;
            std::istringstream iss(arg2);
            if (!(iss >> setTemp))
            {
                std::cerr << "Invalid temp: " << arg2 << std::endl;
                return 1;
            }

            // Set temperature set point and lock
            SetTemperatureSetPoint(camera, setTemp);
            CheckStatus(camera);
            ReadTemperature(camera);
            error = Picam_GetParameterFloatingPointValue(camera, PicamParameter_SensorTemperatureSetPoint, &setTemp);

            if (error == PicamError_None)
            {
                std::cout << "Temp that was set: " << setTemp << std::endl;
            }
            else
            {
                std::cout << "Failed to get set temp." << std::endl;
            }
        }
        else
        {
            std::cout << "Error: Invalid arguments." << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "Error: Too many arguments." << std::endl;
        return 1;
    }

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();

    return 0;
}
