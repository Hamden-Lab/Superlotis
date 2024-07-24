#include <string>
#include <iostream>
#include <exception>
#include "picam.h"  // Picam library header

// Function declarations
void PrintError(PicamError error);
void setSensorTemperatureSetPoint(PicamHandle camera, piflt temperature);
piflt getSensorTemperatureSetPoint(PicamHandle camera);

int main(int argc, char* argv[]) {
    try {
        bool setTemp = false;
        float temperature = 0.0;

        if (argc == 2) {
            std::string arg = argv[1];
            if (arg.rfind("temp=", 0) == 0) {
                setTemp = true;
                temperature = std::stof(arg.substr(5)); // Extracts the temperature value after 'temp='
            } else {
                std::cout << "Invalid argument. Use the format temp=<temperature>" << std::endl;
                return 1;
            }
        } else if (argc != 1) {
            std::cout << "Usage: " << argv[0] << " [temp=<temperature>]" << std::endl;
            return 1;
        }

        PicamError error;
        PicamHandle camera;
        PicamCameraID id;

        std::cout << "Initializing Picam library..." << std::endl;
        error = Picam_InitializeLibrary();
        if (error != PicamError_None) {
            std::cout << "Failed to initialize Picam library. ";
            PrintError(error);
            return 1;
        }

        std::cout << "Opening first camera..." << std::endl;
        // Open first camera or create demo camera
        error = Picam_OpenFirstCamera(&camera);
        if (error == PicamError_None) {
            error = Picam_GetCameraID(camera, &id);
            if (error != PicamError_None) {
                std::cout << "Failed to get camera ID. ";
                PrintError(error);
                return 1;
            }
        } else {
            std::cout << "Opening demo camera..." << std::endl;
            error = Picam_ConnectDemoCamera(PicamModel_Pixis100B, "12345", &id);
            if (error == PicamError_None) {
                error = Picam_OpenCamera(&id, &camera);
                if (error != PicamError_None) {
                    std::cout << "Failed to open demo camera. ";
                    PrintError(error);
                    return 1;
                } else {
                    std::cout << "Demo camera setup successful." << std::endl;
                }
            } else {
                std::cout << "Failed to connect demo camera. ";
                PrintError(error);
                return 1;
            }
        }

        // Execute the function based on the command-line argument
        if (setTemp) {
            setSensorTemperatureSetPoint(camera, temperature);
        } else {
            piflt currentTemperature = getSensorTemperatureSetPoint(camera);
            std::cout << "Current sensor temperature set point: " << currentTemperature << "°C" << std::endl;
        }

        // Close the camera
        std::cout << "Closing camera..." << std::endl;
        error = Picam_CloseCamera(camera);
        if (error != PicamError_None) {
            std::cout << "Failed to close the camera: ";
            PrintError(error);
        }

        // Uninitialize the library
        std::cout << "Uninitializing Picam library..." << std::endl;
        Picam_UninitializeLibrary();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
    }

    return 0;
}

void PrintError(PicamError error) {
    const char* errorString;
    Picam_GetEnumerationString(PicamEnumeratedType_Error, error, &errorString);
    std::cout << errorString << std::endl;
}

void setSensorTemperatureSetPoint(PicamHandle camera, piflt temperature) {
    PicamError error;
    std::cout << "Setting sensor temperature set point to " << temperature << "°C: ";
    error = Picam_SetParameterFloatingPointValue(camera, PicamParameter_SensorTemperatureSetPoint, temperature);
    if (error != PicamError_None) {
        std::cout << "Failed to set sensor temperature set point. ";
        PrintError(error);
    } else {
        std::cout << "Sensor temperature set point set successfully." << std::endl;
    }
}

piflt getSensorTemperatureSetPoint(PicamHandle camera) {
    piflt temperature;
    PicamError error;
    std::cout << "Getting current sensor temperature set point: ";
    error = Picam_GetParameterFloatingPointValue(camera, PicamParameter_SensorTemperatureSetPoint, &temperature);
    if (error != PicamError_None) {
        std::cout << "Failed to get sensor temperature set point. ";
        PrintError(error);
        return -1.0; // Return an invalid temperature in case of error(still messeing with this part)
    }
    std::cout << temperature << "°C" << std::endl;
    return temperature;
}
