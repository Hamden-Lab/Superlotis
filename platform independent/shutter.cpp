#include "stdio.h"
#include "picam.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>  // For stat and mkdir
#include <cerrno>      // For errno
#include <cstring>     // For strerror

PicamHandle camera;

void PrintError( PicamError e )
{
    const pichar *s;
    Picam_GetEnumerationString( PicamEnumeratedType_Error, e, &s );
    std::cout << s << std::endl;
    Picam_DestroyString( s );
}

// int active_shutter(piint *shutter_type){

// }

//PicamOutputSignal
//pg 82 manual


int get_ext_shutter_status(piint *ext_status){
    // printf("Getting external shutter status...\n");
    PicamError err;
    err = Picam_GetParameterIntegerValue(camera, PicamParameter_ExternalShutterStatus, ext_status);
    return err;
}

int get_ext_shutter_type(piint *ext_type){
    PicamError err;
    err = Picam_GetParameterIntegerValue(camera, PicamParameter_ExternalShutterType, ext_type);
    return err;
}

int get_int_shutter_status(piint *int_status){
    PicamError err;
    err = Picam_GetParameterIntegerValue(camera, PicamParameter_InternalShutterStatus, int_status);
    return err;
}

int get_int_shutter_type(piint *int_type){
    PicamError err;
    err = Picam_GetParameterIntegerValue(camera, PicamParameter_InternalShutterType, int_type);
    return err;
}

int get_shutter(piint *mode) {
    // printf("Getting shutter mode...\n");
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

int main() {
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
        std::cout << "Demo cam." << std::endl;
        return 1;

        // PicamCameraID id;
        // error = Picam_ConnectDemoCamera(PicamModel_Pixis100B, "12345", &id);
        // if (error == PicamError_None) {
        //     error = Picam_OpenCamera(&id, &camera);
        //     if (error != PicamError_None) {
        //         std::cout << "Failed to open demo camera." << std::endl;
        //         return 1;
        //     }
        // } else {
        //     std::cout << "Failed to connect demo camera." << std::endl;
        //     return 1;
        // }
    }
    
    // Set shutter mode to 1 
    set_shutter(2);

    // piint ext_status;
    // get_ext_shutter_status(&ext_status);
    // std::cout << "External shutter status value: " << ext_status << std::endl;

    // piint ext_type;
    // get_ext_shutter_type(&ext_type);
    // std::cout << "External shutter type value: " << ext_type << std::endl;

    // piint int_status;
    // get_int_shutter_status(&int_status);
    // std::cout << "Internal shutter status value: " << int_status << std::endl;

    // piint int_type;
    // get_int_shutter_type(&int_type);
    // std::cout << "Internal shutter type value: " << int_type << std::endl;

    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();
    return 0; // Added return statement for main
}
