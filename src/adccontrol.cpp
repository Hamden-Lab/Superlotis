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

int main(int argc, char* argv[]) {
    try {
        bool setAdcQuality = false;
        std::string quality;

        if (argc == 4) {
            std::string arg1 = argv[1];
            std::string arg2 = argv[2];
            std::string arg3 = argv[3];
            if (arg1 == "quality" && arg2 == "=") {
                quality = arg3;
                setAdcQuality = true;
            } else {
                std::cout << "Invalid argument. Use the format quality = <adc_quality>" << std::endl;
                return 1;
            }
        } else if (argc != 1) {
            std::cout << "Usage: " << argv[0] << " [quality = <adc_quality>]" << std::endl;
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

        // Set or get ADC quality based on the command-line argument
        if (setAdcQuality) {
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
            int adcQuality = getEm_AdcQuality(camera);
            std::cout << "Current EM ADC quality: " << adcQuality << std::endl;
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
