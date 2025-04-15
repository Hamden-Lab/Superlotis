/*
This script defines camera commands 
*/
#include "stdio.h"
#include "picam.h"
#include "camera.h"
#include "fitsio.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>  // For stat and mkdir
#include <cerrno>      // For errno
#include <cstring>     // For strerror


#define NO_TIMEOUT  -1
#define TIME_BETWEEN_READOUTS 10 //ms
#define NUM_EXPOSURES  2
#define EXP_TIME 5.0 // Default exposure time in milliseconds
#define ACQUIRE_TIMEOUT 15000  // Fifteen-second timeout

PicamPtcArgs params;
PicamHandle camera;
// piint mode;
// piflt temp;
// piint gainValue;
// const PicamRoisConstraint* constraints;
// piint width;
// piint height;
PicamAvailableData data;
PicamAcquisitionErrorsMask errors;
// int num_image;
PicamCameraID id;
piint readoutstride;
const pichar* string;
FILE* pFile;

void PrintData( pibyte* buf, piint numframes, piint framelength )
{
    pi16u  *midpt = NULL;
    pibyte *frameptr = NULL;

    for( piint loop = 0; loop < numframes; loop++ )
    {
        frameptr = buf + ( framelength * loop );
        midpt = (pi16u*)frameptr + ( ( ( framelength/sizeof(pi16u) )/ 2 ) );
        printf( "%5d,%5d,%5d\t%d\n",(int) *(midpt-1),(int) *(midpt),(int) *(midpt+1),(int) loop+1 );
    }
}

int PrintEnumString(PicamEnumeratedType type, piint value)
{
    const pichar* string;
    PicamError error = Picam_GetEnumerationString(type, value, &string);
    if (error == PicamError_None)
    {
        printf("%s", string);
        Picam_DestroyString(string);
        return 0;  // Success
    }
    else
    {
        return -1; // Failure
    }
}

int PrintError(PicamError error)
{
    if (error == PicamError_None){
        printf("Succeeded\n");
        return 0;
    }
    else
    {
        printf("Failed (");
        PrintEnumString(PicamEnumeratedType_Error, error);
        printf(")\n");
        return -1;
    }
}

int get_exposure_time(piflt *exposure_time)
{
    printf("Getting current exposure time...\n");
    PicamError err;
    err = Picam_GetParameterFloatingPointValue(camera, PicamParameter_ExposureTime, &params.exposure_time);
    if (err != PicamError_None)
    {
        printf("Failed to get exposure time.\n");
        PrintError(err);
        return -1;
    }
    else
    {
        std::cout << "Exposure time is: " << *exposure_time << " ms " << std::endl;
        // printf("Exposure time is: %.2f ms\n", *(double*)exposure_time);
        return 0;
    }
}

int set_exposure_time(piflt exposure_time)
{
    get_exposure_time(&exposure_time);
    printf("Setting new exposure time...\n");
    PicamError error = Picam_SetParameterFloatingPointValue(camera, PicamParameter_ExposureTime, params.exposure_time);
    if (error != PicamError_None)
    {
        printf("Failed to set exposure time.\n");
        PrintError(error);
        return -1;
    }
    else
    {
        std::cout << "Exposure time set to: " << exposure_time << " ms " << std::endl;
        return 0;
    }
}

int get_shutter(piint *mode)
{
    printf("Getting shutter mode...\n");
    PicamError error;
    error = Picam_GetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, &params.mode);
    // PrintError(error);

    if (error == PicamError_None)
    {
        const char* modeDescription = "Unknown shutter status";
        switch (*(int*)mode) {
            case 1:
                modeDescription = "normal timing mode";
                break;
            case 2:
                modeDescription = "shutter always closed";
                break;
            case 3:
                modeDescription = "shutter always open";
                break;
            case 4:
                modeDescription = "shutter open before trigger";
                break;
            default:
                break;
        }
        std::cout << modeDescription << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Failed to get shutter timing mode." << std::endl;
        return -1;
    }
}


int set_shutter(piint mode)
{
    PicamError error;
    get_shutter(&mode);
    printf("Setting shutter mode...\n");
    error = Picam_SetParameterIntegerValue(camera, PicamParameter_ShutterTimingMode, params.mode);
    if (error == PicamError_None){
    const char* shutterDescription = "unknown";
    switch (mode) {
        case 1:
            shutterDescription = "normal";
            break;
        case 2:
            shutterDescription = "always closed";
            // printf("closed: ");
            break;
        case 3:
            shutterDescription = "always open";

            // printf("open: ");
            break;
        case 4:
            shutterDescription = "open before trigger";

            // printf("open before trigger: ");
            break;
    }
    std::cout << "New shutter mode: " << shutterDescription << std::endl;
    return 0;
    }
    else
    {
        std::cout << "Failed to set shutter mode." << std::endl;
        return -1;
    }

    // PrintError(error);
}


int get_temp(piflt *temp)
{
    PicamError error;
    std::cout << "Getting sensor temperature..."<< std::endl;
    error = Picam_ReadParameterFloatingPointValue(
        camera,
        PicamParameter_SensorTemperatureReading,
        &params.temp);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "Current temperature is " << *temp << " degrees C" << std::endl;
        return 0;
    }
    else
    {
        return -1;
    }
}
int set_temp(piflt temp)
{
    get_temp(&temp);
    PicamError error;
    // Check sensor temperature status
    std::cout << "Checking sensor temperature status..." << std::endl;
    PicamSensorTemperatureStatus status;
    error = Picam_ReadParameterIntegerValue(
        camera,
        PicamParameter_SensorTemperatureStatus,
        reinterpret_cast<piint*>(&status));
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "Status is: ";
        PrintEnumString(PicamEnumeratedType_SensorTemperatureStatus, status);
        std::cout << std::endl;

        // If temperature is locked, set the temperature set point
        if (status == PicamSensorTemperatureStatus_Locked)
        {
            std::cout << "Setting sensor temperature..." << std::endl;
            error = Picam_SetParameterFloatingPointValue(
                camera,
                PicamParameter_SensorTemperatureSetPoint,
                temp);
            PrintError(error);
            return 0;  // Return success if setting the temperature was successful
        }
        else
        {
            std::cout << "Temperature is not locked. Skipping setting temperature." << std::endl;
            return -1;  // Return -1 if temperature is not locked
        }
    }
    
    // Return an error code if reading the parameter failed
    std::cerr << "Failed to read sensor temperature status." << std::endl;
    return -2;  // Indicate failure in reading the sensor temperature status
}



int get_analog_gain(piint *gainValue)
{
    std::cout << "Getting adc analog gain..." << std::endl;
    PicamError error;
    error = Picam_GetParameterIntegerValue(camera, PicamParameter_AdcAnalogGain, &params.gainValue);

    // Print current analog gain
    if (error == PicamError_None)
    {
        const char* gainDescription = "unknown";
        switch (*(int*)gainValue) {
            case 1:
                gainDescription = "low";
                break;
            case 2:
                gainDescription = "medium";
                break;
            case 3:
                gainDescription = "high";
                break;
        }
        std::cout << "Current analog gain value: " << gainDescription << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Failed to get analog gain." << std::endl;
        return -1;
    }
}

/*    PicamAdcAnalogGain_Low    = 1,
    PicamAdcAnalogGain_Medium = 2,
    PicamAdcAnalogGain_High   = 3*/

int set_analog_gain(piint gainValue)
{
    PicamError error;
    get_analog_gain(&gainValue);
    printf("Setting adc analog gain...\n");
    
    const char* gainDescription = "unknown";
    switch (gainValue) {
        case 1:
            gainDescription = "low";
            break;
        case 2:
            gainDescription = "medium";
            break;
        case 3:
            gainDescription = "high";
            break;
    }
    std::cout << "New analog gain value: " << gainDescription << std::endl;

    // printf("%s\n", gainDescription);

    error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcAnalogGain, params.gainValue);


    // std::cout << "Set analog gain: ";
    // error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcAnalogGain, gainValue);
    // PrintError(error);

    // Store the gain value in params
    if (error == PicamError_None)
    {
        // params.gainValue = gainValue; // Update params with the new gain value
        std::cout << "Checking analog gain value... \nAnalog gain: " << gainDescription << std::endl;
        return 0;
    }
    else
    {
    return -1;
    }
}

int open_camera()
{
    Picam_InitializeLibrary();
    std::cout << "Open camera" << std::endl;

    if (Picam_OpenFirstCamera(&camera) == PicamError_None)
        Picam_GetCameraID(&camera, &id);
    else
    {
        Picam_ConnectDemoCamera(PicamModel_Pixis100F, "0008675309", &id);
        Picam_OpenCamera(&id, &camera);
        printf("No Camera Detected, Creating Demo Camera\n");
    }

    Picam_GetEnumerationString(PicamEnumeratedType_Model, id.model, &string);
    printf("%s", string);
    printf(" (SN:%s) [%s]\n", id.serial_number, id.sensor_name);
    Picam_DestroyString(string);
    Picam_GetParameterIntegerValue(camera, PicamParameter_ReadoutStride, &readoutstride);


    set_temp(-10.0);
    set_analog_gain(2);
    set_shutter(2);
    return 0;

    // sprintf(params.image_path,"./");
    // sprintf(params.root_name,"image");
    // sprintf(params.last_fname," ");
//   params.imagenumber=0;
}

int close_camera()
{
    // PicamHandle camera;
    std::cout << "Close camera" << std::endl;
    Picam_CloseCamera(camera);
    Picam_UninitializeLibrary();
    return 0;
}

int commit_params()
{
    pibln committed;
    PicamError error = Picam_AreParametersCommitted(camera, &committed);
    if (error != PicamError_None || !committed)
    {
        const PicamParameter* failed_parameters;
        piint failed_parameter_count;
        error = Picam_CommitParameters(camera, &failed_parameters, &failed_parameter_count);
        if (error != PicamError_None)
        {
            std::cerr << "Failed to commit parameters. ";
            PrintError(error);
            return -1;
        }
        if (failed_parameter_count > 0)
        {
            Picam_DestroyParameters(failed_parameters);
            return -2;  // Indicate that some parameters failed to commit
        }
    }
    
    // If no errors and parameters are committed
    return 0;  // Indicate success
}


int image(const char *filename)
{
    PicamError error = Picam_Acquire(camera, 2, 6000, &data, &errors);
   if (error == PicamError_None)
    {
        std::cout << "Successfully took frame" << std::endl;
        std::cout << "Filename is:" << filename << std::endl;

        // std::cout << *static_cast<pibyte*>(data.initial_readout) << std::endl;
        PrintData( (pibyte*)data.initial_readout, 2, readoutstride );
		pFile = fopen(filename, "wb" );
		if( pFile )
		{
			if( !fwrite( data.initial_readout, 1, (2*readoutstride), pFile ) )
				printf( "Data file not saved\n" );
			fclose( pFile );
		}
    }
    else
    {
        std::cerr << "Failed: ";
        PrintError(error);
        return -1;
    }
    return 0;
}

int expose(const char *expose_filename)
{
    std::cout << "Take exposure" << std::endl;
    commit_params();

    // The acquisition logic
    set_shutter(1); 

    // Commit the parameters before acquisition

    image(expose_filename);
    return 0;

}

//new:
int burst(int i) {
    for (int j = 1; j <= i; ++j) {
        // Use stringstream to construct the filename
        std::stringstream ss;
        ss << "exposure_file_" << j << ".raw";
        std::string filename = ss.str(); // Get the string from stringstream
        
        // Call the expose function with the constructed filename
        expose(filename.c_str());
    }
    return 0;
}
//end new


int dark(const char *dark_filename)
{
    std::cout << "Take dark" << std::endl;
    commit_params();

    set_shutter(2); // Set shutter mode to closed
    piflt exposure_time = EXP_TIME;

    set_exposure_time(exposure_time);

    image(dark_filename);

    return 0;
}


int bias(const char *bias_filename)
{
    std::cout << "Take bias" << std::endl;
    commit_params();
    set_shutter(3); // Set shutter mode to open
    // piflt exposure_time = 0; 

    set_exposure_time(0);

    image(bias_filename);

    return 0;
}

// int main(){
//     open_camera();
//     burst(5);
//     // for (int i; i<2;++i)
//     // {
//     //     const char *file = "exp_file";
//     //     expose(file);
//     //     }
//     close_camera();
// }
