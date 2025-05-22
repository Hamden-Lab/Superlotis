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


void setLowAnalogGain(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set low analog gain: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera, 
            PicamParameter_AdcAnalogGain, 
            PicamAdcAnalogGain_Low);
    PrintError(error);
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
    setLowAnalogGain(camera);
    // std::cout << "Set temp" <<std::endl;
    // SetTemperature(camera, temp_val, lock);
    // std::cout << std::endl;
    // ReadTemperature( camera, lock );


    Picam_CloseCamera( camera );

    Picam_UninitializeLibrary();
    }
