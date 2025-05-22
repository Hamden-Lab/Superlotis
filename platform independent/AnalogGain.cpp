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

void setHighAnalogGain(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set high analog gain: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera, 
            PicamParameter_AdcAnalogGain, 
            PicamAdcAnalogGain_High);
    PrintError(error);
}


void setMediumAnalogGain(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set medium analog gain: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera, 
            PicamParameter_AdcAnalogGain, 
            PicamAdcAnalogGain_Medium);
    PrintError(error);
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

int main(int argc, char* argv[]) {
    PicamError error;
    PicamHandle camera;
    PicamCameraID id;
    piflt adcAnalogGain = 0;


    Picam_InitializeLibrary();
    //piint setAnalogGain;


    //open first camera or create demo camera
    if( Picam_OpenFirstCamera( &camera ) == PicamError_None )
        Picam_GetCameraID( camera, &id );
    else
    {
        Picam_ConnectDemoCamera(
            PicamModel_Pixis100B,
            "12345",
            &id );
        Picam_OpenCamera( &id, &camera );
    };

    //get analog gain
    if (argc == 1){
        // std::cout << "Get analog gain: ";
        // error = Picam_GetParameterIntegerValue( camera, 
        //         PicamParameter_AdcAnalogGain, &adcAnalogGain);
        // std::cout << "Get adc quality: ";
        // error = Picam_GetParameterIntegerValue( camera, 
        //         PicamParameter_AdcAnalogGain, &adcAnalogGain);
        std::cout << "Get adc speed: ";
        error = Picam_GetParameterFloatingPointValue( camera, 
                PicamParameter_AdcAnalogGain, &adcAnalogGain);
        //print current analog gain
        if (error == PicamError_None)
        {
            std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else
        {
            std::cout << "Failed to get analog gain." << std::endl;
        }
    }
    //set analog gain
    if (argc == 2) 
    {
        std::string arg( argv[1] );
        if( arg == "low" ){
            setLowAnalogGain(camera);
            std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else if( arg == "medium" ){
            setMediumAnalogGain(camera);
            std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else if( arg == "high" ){
            setHighAnalogGain(camera);
            std::cout << "Analog gain value: " << adcAnalogGain << std::endl;
        }
        else{
            std::cout << "Failed to set analog gain." << std::endl;
        }
        
    }

    Picam_CloseCamera( camera );

    Picam_UninitializeLibrary();
    }