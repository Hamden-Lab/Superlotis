///////////////////////////////////////////////////////////////////
//Sets camera parameters, and takes test images.
//This code sets unchanged parameters:
// -rate of analog gain
// -ADC (analog -> digital conversion) speed
// -ADC quality
// -readout control mode

// It also includes the following dynamic functions:
// -setTemp, which controls the camera temperature
// -getTemp, which gets the camera temperature
// -setExposure, which sets the exposure type and time
// -getExposure, which returns the current exposure type and time
// -shutterOpen, which enables and opens the shutter
// -shutterClose, which disables and closes the shutter
// -getExposingStatus, which returns [“exposing”, “idle”, “readout”, and “writing”
// -setExposingStatus, which sets the status
// -vacuum (?)
// -acquireImage, which sets the camera to acquire 5 readouts

// Next, the camera creates a directory for file output, and sends images to that location.
// It does the exposure, using the acquireImage function, and then writes the image to a FITs file. 
// The output FITs file is also given header information(?).
///////////////////////////////////////////////////////////////////


#include <string>
#include <iostream>
#include "picam.h"

/**********CAMERA START UP**********/
/*Sets PicamParameters, using functions from picam.h*/

void SetParameters(PicamHandle camera){
    PicamError error;

    // - Set the ADC analog gain to low gain
    std::cout << "Set low analog gain: ";
    error =
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_AdcAnalogGain,
            PicamAdcAnalogGain_Low ); // There is also PicamAdcAnalogGain_High, and PicamAdcAnalogGain_Medium
    PrintError( error )

    // - Sets ADC quality
    std::cout << "Optimize for low noise";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_AdcQuality,
            PicamAdcQuality_LowNoise);
    PrintError( error );
    /*PicamAdcQuality_ElectronMultiplied --> Provides electron multiplication.
    PicamAdcQuality_HighCapacity --> Optimized for sensing high levels of radiation.
    PicamAdcQuality_HighSpeed --> Provides faster readout speeds.
    PicamAdcQuality_LowNoise --> Optimized for the lowest noise.*/

    std::count << "Set ADC speed";
    const piflt adc_speed = 10; //does this value make sense?
    error = 
        PicamValueType_FloatingPoint(
            PicamParameter_AdcSpeed,
            adc_speed
        )

    // - Sets the readout parameters
    std::cout << "Sets readout control mode: ";
    error =
        Picam_SetParameterIntegerValue(
            camera,
            PicamReadoutControlMode,
            PicamReadoutControlMode_FullFrame);
    PrintError( error );
    /*PicamReadoutControlMode_Dif --> The Dual Imaging Feature where the sensor acquires two frames rapidly and then reads them both out.
    PicamReadoutControlMode_ExposeDuringReadout --> The sensor is reading out a frame while exposing the next frame.
    PicamReadoutControlMode_FrameTransfer --> The sensor is reading out a frame while exposing the next frame.
    PicamReadoutControlMode_FullFrame --> The sensor is read one frame at a time.
    PicamReadoutControlMode_Interline --> The sensor is reading out a frame while exposing the next frame.
    PicamReadoutControlMode_Kinetics --> The sensor rapidly stores multiple frames and then reads those out.
    PicamReadoutControlMode_RollingShutter --> The sensor is reading out a row while exposing the next row.
    PicamReadoutControlMode_SeNsR --> The sensor accumulates frames by alternating between two different phases
    PicamReadoutControlMode_SpectraKinetics --> Same as kinetics, but optimized to capture a larger burst of spectral frames.*/

}


//taken from configure.cpp:
// - reads the temperature and temperature status directly from hardware
//   and waits for temperature to lock if requested
void ReadTemperature( PicamHandle camera, pibool lock )
{
    PicamError error;

    // - read temperature
    std::cout << "Read sensor temperature: ";
    piflt temperature;
    error =
        Picam_ReadParameterFloatingPointValue(
            camera,
            PicamParameter_SensorTemperatureReading,
            &temperature );
    PrintError( error );
    if( error == PicamError_None )
    {
        std::cout << "    " << "Temperature is "
                  << temperature << " degrees C" << std::endl;
    }

    // - read temperature status
    std::cout << "Read sensor temperature status: ";
    PicamSensorTemperatureStatus status;
    error =
        Picam_ReadParameterIntegerValue(
            camera,
            PicamParameter_SensorTemperatureStatus,
            reinterpret_cast<piint*>( &status ) );
    PrintError( error );
    if( error == PicamError_None )
    {
        std::cout << "    " << "Status is ";
        PrintEnumString( PicamEnumeratedType_SensorTemperatureStatus, status );
        std::cout << std::endl;
    }

    // - wait indefinitely for temperature to lock if requested
    if( lock )
    {
        std::cout << "Waiting for temperature lock: ";
        error =
            Picam_WaitForStatusParameterValue(
                camera,
                PicamParameter_SensorTemperatureStatus,
                PicamSensorTemperatureStatus_Locked,
                -1 );
        PrintError( error );
    }
}


// - Set shutter to open when exposure is happening
void openShutterDuringExposure(PicamHandle camera){
    std::cout << "Open shutter when exposure is happening: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamShutterTimingMode,
            PicamShutterTimingMode_Normal
        )
}

// - Set shutter to be always open
int openShutter(PicamHandle camera){
    std::cout << "Keep shutter open";
    error = 
        Picam_SetParameterIntegerValue(
            camera, 
            PicamShutterTiming_Mode,
            PicamShutterTimingMode_AlwaysOpen
        )
}

// - Set shutter to be always closed
void closeShutter(PicamHandle camera){
    std::cout << "Keep shutter closed";
    error = 
        Picam_SetParameterIntegerValue(
            camera, 
            PicamShutterTiming_Mode,
            PicamShutterTimingMode_AlwaysClosed
        )

}

int main(){
    pibln *shutterOn = NULL;
    
}


// void TemperatureStatus(PicamHandle camera){
//     std::cont << "Set temperature status to locked, so that status can be evaluated"
//     error = 
//         /*Set temperature status to locked*/
//         Picam_SetParameterIntegerValue( 
//             //not sure if this should be PSetParameterIntegerValue. 
//             //i'm thinking that the value associated with PicamSensorTemperatureStatus_Locked is a boolean. booleans can be evaluated by SetParameterIntegerValue
//             camera,
//             PicamSensorTemperatureStatus,
//             PicamSensorTemperatureStatus_Locked
//         )
//         /*Return the status of the current sensor temperature, for the current camera*/
//         Picam_SetParameterFloatingPointValue(
//             /*PICAM_API PicamEMCalibration_ReadSensorTemperatureReading(
//                 PicamHandle calibration,
//                 piflt* value);
//                 calibration = Handle of the camera for which the status of the sensor temperature is to be determined.
//                 I'm assuming that PicamHandle calibration = camera
//                 piflt* value = pointer to calibration date
//             */
//             // 
//             camera,
//             PicamSensorTemperatureStatus* value);

        
    
//     // - PicamEMCalibration_ReadSensorTemperatureStatus: returns the status of the current sensor temperature for a specified camera.
//     // - Set temperature status to unlocked again

// }

//taken from configure.cpp
void ReadTemperature( PicamHandle camera, pibool lock )
{
    PicamError error;

    // - read temperature
    std::cout << "Read sensor temperature: ";
    piflt temperature;
    error =
        Picam_ReadParameterFloatingPointValue(
            camera,
            PicamParameter_SensorTemperatureReading,
            &temperature );
    PrintError( error );
    if( error == PicamError_None )
    {
        std::cout << "    " << "Temperature is "
                  << temperature << " degrees C" << std::endl;
    }

    // - read temperature status
    std::cout << "Read sensor temperature status: ";
    PicamSensorTemperatureStatus status;
    error =
        Picam_ReadParameterIntegerValue(
            camera,
            PicamParameter_SensorTemperatureStatus,
            reinterpret_cast<piint*>( &status ) );
    PrintError( error );
    if( error == PicamError_None )
    {
        std::cout << "    " << "Status is ";
        PrintEnumString( PicamEnumeratedType_SensorTemperatureStatus, status );
        std::cout << std::endl;
    }

    // - wait indefinitely for temperature to lock if requested
    if( lock )
    {
        std::cout << "Waiting for temperature lock: ";
        error =
            Picam_WaitForStatusParameterValue(
                camera,
                PicamParameter_SensorTemperatureStatus,
                PicamSensorTemperatureStatus_Locked,
                -1 );
        PrintError( error );
    }

    // - set temperature status

}


void TakePictures(PicamHamdle camera){
    // - Set the exposure time

    // - Set the number of frames to take 
    std::cout << "Set 5 readouts: ";
    const pi64s readout_count = 5; //pi64s = 64-bit signed integer, is that why we use Picam_SetParameterLargeIntegerValue?
    error =
        Picam_SetParameterLargeIntegerValue(
            camera,
            PicamParameter_ReadoutCount,
            readout_count );
    PrintError( error );


}


Picam_SetParameterIntegerValue


//Read the vacuum status

// - Read the ADC parameters: Picam_GetParameterIntegerValue

//PicamAdcAnalogGain gain_status;

//PicamAdcQuality quality_status;

/*Sets demo camera, if camera is not available*/

// - Sets the camera model
        Picam_ConnectDemoCamera(
            PicamModel_SophiaXO2048B,
            "0008675309", //idk the id, how do i get this information?
            &id );
        Picam_OpenCamera( &id, &camera );
// - Prints the camera model (code taken from configure.cpp)

// - Open first camera if exists, or creates a demo camera (code taken from acquire.cpp)

/*Write 5 test files*/

/**********GET CAMERA RUNNING**********/

/*Set the camera exposure times*/

// - Send the file outputs (might be in save_data.cpp)

/*Do the exposure*/

// - Write fits files



/**********EXPORT OUTPUT FITS FILE**********/
















void SetModel(){
}

// - 






void SetParameters(){
//PicamParameter is the set of user-accessible hardware parameters.


}


//picamconnect demo camera