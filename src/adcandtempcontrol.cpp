#include <string>
#include <iostream>
#include <exception>
#include "picam.h"  // Picam library header

// Function declarations
void PrintError(PicamError error);
void setEm_AdcQuality(PicamHandle camera);
void setHighCapacity_AdcQuality(PicamHandle camera);
void setHighSpeed_AdcQuality(PicamHandle camera);
void setLowNoise_AdcQuality(PicamHandle camera);
int getEm_AdcQuality(PicamHandle camera);
void setSensorTemperatureSetPoint(PicamHandle camera, piflt temperature);
piflt getSensorTemperatureSetPoint(PicamHandle camera);

int main(int argc, char* argv[]) {
    try {
        bool setAdcQuality = false;
        bool setTemp = false;
        std::string quality;
        piflt temperature = 0.0;
        piflt currentTemperature = 0.0;

        // Parse command-line arguments
        if (argc == 4) {
            std::string arg1 = argv[1];
            std::string arg2 = argv[2];
            std::string arg3 = argv[3];
            if (arg1 == "quality" && arg2 == "=") {
                quality = arg3;
                setAdcQuality = true;
            } else if (arg1 == "temp" && arg2 == "=") {
                temperature = std::stof(arg3); //Converts string to floating point
                setTemp = true;
            } else {
                std::cout << "Invalid argument. Use the format quality = <adc_quality> or temp = <temperature>" << std::endl;
                return 1;
            }
        } else if (argc != 1) {
            std::cout << "Usage: " << argv[0] << " [quality = <adc_quality>] or [temp = <temperature>]" << std::endl;
            return 1;
        } //what if i want to set both qualities at once does our camera support it?


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

        // Set or get ADC quality or temperature based on the command-line argument
        if (setTemp) {
            setSensorTemperatureSetPoint(camera, temperature);
        } else if (setAdcQuality) {
            currentTemperature = getSensorTemperatureSetPoint(camera);
            if (currentTemperature > 50.0) {
                std::cout << "Error: Cannot set ADC quality when temperature is above 50°C." << std::endl;
                return 1;
            }

            if (quality == "EM") {
                setEm_AdcQuality(camera);
            } else if (quality == "HighCapacity") {
                setHighCapacity_AdcQuality(camera);
            } else if (quality == "HighSpeed") {
                setHighSpeed_AdcQuality(camera);
            } else if (quality == "LowNoise") {
                setLowNoise_AdcQuality(camera);
            } else {
                std::cout << "Invalid ADC quality. Use one of: EM, HighCapacity, HighSpeed, LowNoise." << std::endl;
                return 1;
            }
        } else {
            int adcQuality = getEm_AdcQuality(camera); //well the issue is that we can't just get temp or just adc quality
            currentTemperature = getSensorTemperatureSetPoint(camera);
            std::cout << "Current EM ADC quality: " << adcQuality << std::endl;
            std::cout << "Current Sensor Temperature: " << currentTemperature << std::endl;
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

void setEm_AdcQuality(PicamHandle camera) {
    PicamError error;
    std::cout << "Setting EM ADC quality: ";
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcQuality, PicamAdcQuality_ElectronMultiplied);
    PrintError(error);
}

void setHighCapacity_AdcQuality(PicamHandle camera) {
    PicamError error;
    std::cout << "Setting High Capacity ADC quality: ";
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcQuality, PicamAdcQuality_HighCapacity);
    PrintError(error);
}

void setHighSpeed_AdcQuality(PicamHandle camera) {
    PicamError error;
    std::cout << "Setting High Speed ADC quality: ";
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcQuality, PicamAdcQuality_HighSpeed);
    PrintError(error);
}

void setLowNoise_AdcQuality(PicamHandle camera) {
    PicamError error;
    std::cout << "Setting Low Noise ADC quality: ";
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcQuality, PicamAdcQuality_LowNoise);
    PrintError(error);
}

int getEm_AdcQuality(PicamHandle camera) {
    piint adcQuality;
    PicamError error = Picam_GetParameterIntegerValue(camera, PicamParameter_AdcQuality, &adcQuality);
    if (error != PicamError_None) {
        std::cout << "Failed to get EM ADC quality. ";
        PrintError(error);
        return -1;
    }
    return adcQuality;
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
        return -1.0; // Return an invalid temperature in case of error
    }
    std::cout << temperature << "°C" << std::endl;
    return temperature;
}
     