#include "stdio.h"
#include "picam.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>  // For stat and mkdir
#include <cerrno>      // For errno
#include <cstring>     // For strerror

PicamHandle camera;

int get_shutter(piint *mode) {
    printf("Getting shutter mode...\n");
    PicamError error;

    // Call Picam_GetParameterIntegerValue with the correct pointer
    error = Picam_GetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, mode);
    
    if (error == PicamError_None) {
        const char* modeDescription = "Unknown shutter status";
        switch (*mode) { // Dereference mode correctly
            case 1:
                modeDescription = "normal";
                break;
            case 2:
                modeDescription = "closed";
                break;
            case 3:
                modeDescription = "open";
                break;
            case 4:
                modeDescription = "open before trigger";
                break;
            default:
                break;
        }
        std::cout << modeDescription << std::endl;
        return 0;
    } else {
        std::cout << "Failed to get shutter timing mode." << std::endl;
        return -1;
    }
}

int set_shutter(piint mode) {
    PicamError error;

    // Declare a variable to hold the current mode
    piint currentMode;

    // Retrieve the current shutter mode
    if (get_shutter(&currentMode) != 0) {
        return -1; // If getting the current mode fails, return
    }

    printf("Setting shutter mode...\n");
    
    // Set the new mode
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, mode);
    
    if (error == PicamError_None) {
        const char* shutterDescription = "unknown";
        switch (mode) {
            case 1:
                shutterDescription = "normal";
                break;
            case 2:
                shutterDescription = "closed";
                break;
            case 3:
                shutterDescription = "open";
                break;
            case 4:
                shutterDescription = "open before trigger";
                break;
        }
        std::cout << "New shutter mode: " << shutterDescription << std::endl;
        return 0;
    } else {
        std::cout << "Failed to set shutter mode." << std::endl;
        return -1;
    }
}

int main(int argc, char* argv[]) {
    Picam_InitializeLibrary();
    PicamError error;

    // Use the global camera variable
    error = Picam_OpenFirstCamera(&camera);
    if (error == PicamError_None) {
        PicamCameraID id;
        error = Picam_GetCameraID(camera, &id);
        if (error != PicamError_None) {
            std::cout << "Failed to get camera ID." << std::endl;
            return 1;
        }
    } else {
        PicamCameraID id;
        error = Picam_ConnectDemoCamera(PicamModel_Pixis100B, "12345", &id);
        if (error == PicamError_None) {
            error = Picam_OpenCamera(&id, &camera);
            if (error != PicamError_None) {
                std::cout << "Failed to open demo camera." << std::endl;
                return 1;
            }
        } else {
            std::cout << "Failed to connect demo camera." << std::endl;
            return 1;
        }
    }
    
    // Set shutter mode to 10 (if applicable)
    set_shutter(10);

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();
    return 0; // Added return statement for main
}
