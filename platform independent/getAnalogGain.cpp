#include <string>
#include <iostream>
#include <sstream>
#include "picam.h"


/**********CAMERA START UP**********/
/*Sets PicamParameters, using functions from picam.h*/

// - prints any picam enum
void PrintEnumString( PicamEnumeratedType type, piint value )
{
    const pichar* string;
    Picam_GetEnumerationString( type, value, &string );
    std::cout << string;
    Picam_DestroyString( string );
}

// - prints the camera identity
void PrintCameraID( const PicamCameraID& id )
{
    // - print the model
    PrintEnumString( PicamEnumeratedType_Model, id.model );

    // - print the serial number and sensor
    std::cout << " (SN:" << id.serial_number << ")"
              << " ["    << id.sensor_name   << "]" << std::endl;
}

// - prints error code
void PrintError( PicamError error )
{
    if( error == PicamError_None )
        std::cout << "Succeeded" << std::endl;
    else
    {
        std::cout << "Failed (";
        PrintEnumString( PicamEnumeratedType_Error, error );
        std::cout << ")" << std::endl;
    }
}

/* The `getAnalogGain` function is retrieving the analog gain value from the camera specified by
the `PicamHandle` parameter. It uses the `Picam_GetParameterIntegerValue` function to get the
analog gain value for the `PicamParameter_AdcAnalogGain` parameter. The function then prints
the result of the operation by calling the `PrintError` function to display whether the
operation succeeded or failed. */
void getAnalogGain(PicamHandle camera)
{
    PicamError error;
    piint adcAnalogGain = 0;
    std::cout << "Get analog gain: ";
    error = Picam_GetParameterIntegerValue( camera, 
            PicamParameter_AdcAnalogGain, &adcAnalogGain);

    if (error == PicamError_None)
    {
        std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
    }
    else
    {
        std::cout << "Failed to get analog gain." << std::endl;
    }
}


int main() {
    Picam_InitializeLibrary();

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
    getAnalogGain(camera);

    Picam_CloseCamera( camera );

    Picam_UninitializeLibrary();
    }
