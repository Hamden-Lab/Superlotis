#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib> // For exit
#include <cstdio>  // For printf
#include <sys/stat.h>
#include <cerrno>   // For errno
#include <cstring>  // For strerror
#include <map>      // For std::map
#include <string>   // For std::string
#include <vector>

#define NUM_FRAMES 5
#define NO_TIMEOUT  -1 //infinate

#include "picam.h" //picam header
#include "camera.h" //header for params 


// //globals 
// PicamPtcArgs params;
unsigned char open_cam = 0;
//std::string outputDir = "/opt/PrincetonInstruments/picam/samples/projects/gcc/objlin/x86_64/debug/output";



// Function to print error code
void PrintError(PicamError error)
{
    if (error == PicamError_None)
        std::cout << "Succeeded" << std::endl;
    else
    {
        std::cout << "Failed (";
        // Print the error string
        const pichar* errorString;
        Picam_GetEnumerationString(PicamEnumeratedType_Error, error, &errorString);
        std::cout << errorString << ")" << std::endl;
        Picam_DestroyString(errorString); // Free the error string
    }
}

void PrintData( pibyte* buf, piint numframes, piint framelength )
{
    pi16u  *midpt = NULL;
    pibyte *frameptr = NULL;

    for( piint loop = 0; loop < numframes; loop++ )
    {
        frameptr = buf + ( framelength * loop );
        midpt = (pi16u*)frameptr + ( ( ( framelength/sizeof(pi16u) )/ 2 ) );
        printf( "%5d,%5d,%5d\t%d\n", (int) *(midpt-1), (int) *(midpt),  (int) *(midpt+1), (int) loop+1 );
    }
}

// Function to create a directory
void createDirectory(const std::string& dir) {
    struct stat info;

    if (stat(dir.c_str(), &info) != 0) {
        // Directory does not exist, try to create it
        if (mkdir(dir.c_str(), 0755) != 0) {
            std::cerr << "Error creating directory " << dir << ": " << strerror(errno) << std::endl;
        } else {
            std::cout << "Directory " << dir << " created." << std::endl;
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << dir << " exists but is not a directory." << std::endl;
    }
}
pibool openCamera( PicamHandle *hDevice )
{
    piint id_cnt = 0;
    const PicamCameraID* id = 0;
    PicamError e = PicamError_None;

    if( Picam_InitializeLibrary() != PicamError_None )
    {
        std::cout << "Cannot Initialize Picam Library" << std::endl;
        return false;
    }
    if( ( e = Picam_GetAvailableCameraIDs( &id, &id_cnt ) ) != PicamError_None )
    {
        PrintError( e );
        Picam_UninitializeLibrary();
        return false;
    }
    if( !id_cnt )
    {
        PicamCameraID demo_id;
        std::cout << "No Cameras Available, connecting a demo camera." << std::endl;
                
    e = Picam_ConnectDemoCamera(PicamModel_Pixis100F, "0008675309", &demo_id);
    if (e != PicamError_None)
    {
        PrintError( e );
        Picam_UninitializeLibrary();
        return false;
    }
    e = Picam_OpenCamera( &demo_id, hDevice );
    if( e != PicamError_None )
    {
        PrintError( e );
        Picam_UninitializeLibrary();
        return false;
    }
    }
}

void closeCamera()
{
    PicamAdvanced_CloseCameraDevice( hDevice );

    Picam_UninitializeLibrary();
}


void setExposureTime(PicamHandle camera)
{
    piflt setExpTime;  
    PicamError error;
    std::cout << "Set exposure time to 120 ms: ";
    error = 
        Picam_SetParameterFloatingPointValue(
            camera,
            PicamParameter_ExposureTime,
            setExpTime );
    PrintError( error );

    //error = Picam_SetParameterFloatingPointValue(PicamParameter_ExposureTime

    if (error == PicamError_None)
    {
        std::cout << "Exposure time value: " << adcAnalogGain << std::endl;
    }
    else
    {
        std::cout << "Failed to set exposure time." << std::endl;
    }
    //error = Picam_SetParameterFloatingPointValue(PicamParameter_ExposureTime
}

void getExposureTime(PicamHandle camera)
{
    piflt expTime;  
    PicamError error;
    std::cout << "Get exposure time: ";
    error = Picam_GetParameterFloatingPointValue(
        camera, 
        PicamParameter_ExposureTime, 
        &expTime);
    PrintError( error );
    //print current analog gain
}

void setAnalogGain(PicamHandle camera)
{
    PicamError error;
    piint analogGain;
    std::cout << "Set analog gain: ";
    error = Picam_SetParameterIntegerValue(
        camera, 
        PicamParameter_AdcAnalogGain, 
        analogGain);
    PrintError(error);

    // Store the gain value in params
    if (error == PicamError_None)
    {
        // params.gainValue = gainValue; // Update params with the new gain value
        std::cout << "Stored analog gain value: " << analogGain << " in params." << std::endl;
    }
}

void getAnalogGain(PicamHandle camera)
{
    PicamError error;
    piint val;
    std::cout << "Getting analog gain...";
    error = Picam_GetParameterIntegerValue(
        camera, 
        PicamParameter_AdcAnalogGain, 
        &val);

    // Print current analog gain
    if (error == PicamError_None)
    {
        std::cout << "Analog gain value: " << *val << std::endl;
    }
    else
    {
        std::cout << "Failed to get analog gain." << std::endl;
    }
}

void setSetTemp(PicamHandle camera)
{
    PicamError error;
    piflt setTempSetPoint;
    std::cout << "Set sensor temperature set point: ";
    error = Picam_SetParameterFloatingPointValue(
        camera,
        PicamParameter_SensorTemperatureSetPoint,
        setTempSetPoint);
    PrintError(error);
}

/*sensor temperature status: PicamSensorTemperatureStatus */
void getSetTemp(PicamHandle camera)
{
    PicamError error;
    piflt setTemp;
    std::cout << "Read sensor temperature status: ";
    PicamSensorTemperatureStatus status;
    error = Picam_GetParameterFloatingPointValue(
        camera, 
        PicamParameter_SensorTemperatureSetPoint, 
        &setTemp);

    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Status is ";
        PrintEnumString(PicamEnumeratedType_SensorTemperatureStatus, status);
        std::cout << std::endl;
    }
}

void readFrameSize(PicamHandle camera)
{
    PicamError error;
    std::cout << "Read frame size: ";
    piint fsize;
    error = Picam_GetParameterIntegerValue(
        camera,
        PicamParameter_FrameSize,
        &fsize);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "    " << "Frame size is "
                  << fsize << std::endl;
    }
}

// void setShutterDelay(PicamHandle camera)
// {
//     //PicamParameter_ShutterDelayResolution
// }
void setExposureNumber(PicamHandle camera, int number)
{    
    PicamError error;
    if (open_cam){
        if(number >=0){
    imageNumber = number;
    } else{
        std::cout << "Invalid exposure number"<< number << std::endl;
    }

} else {
    std::cout << "Camera not open" << std::endl;
}
}

void getExposureNumber(PicamHandle camera, int *number)
{    
    PicamError error;
    if (open_cam){
        if(number >=0){
    *number =imageNumber;
    }
    } else {
    std::cout << "Camera not open" << std::endl;
}
}


void setShutterModeAlwaysOpen(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to always open: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_AlwaysOpen
        );
    PrintError(error);

}


void setShutterModeAlwaysClosed(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to always closed: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_AlwaysClosed
        );
    PrintError(error);

}


void setShutterModeOpenBeforeTrigger(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to open before trigger: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_OpenBeforeTrigger
        );
    PrintError(error);

}

void setShutterModeNormal(PicamHandle camera)
{
    PicamError error;
    std::cout << "Set shutter mode to normal: ";
    error = 
        Picam_SetParameterIntegerValue(
            camera,
            PicamParameter_ShutterTimingMode,
            PicamShutterTimingMode_Normal
        );
    PrintError(error);

}

int set_imagename_root(char *root){
  if (grab_open){
    sprintf(root_name,"%s",root);
  } else {
    printf("Camera not open\n");
    return -1;
  };
  return 0;
}; // set_imagename_root

int get_imagename_root(char *root){
  return sprintf(root,"%s",root_name);
}; // get_imagename_root

int set_imagename_path(char *path){
  if (grab_open){
    sprintf(image_path,"%s/",path);
  } else { 
    printf("Camera not open\n");
    return -1;
  };
  return 0;
}; // set_imagename_path

int get_imagename_path(char *path){
  return sprintf(path, "%s",image_path);
  
}; // get_imagename_path

int get_next_filename(char *fname){
  if(grab_open){
    return sprintf(fname,"%s%s%06d.fits",image_path,root_name,imagenumber);
  } else {
    printf("Camera not open\n");
    return -1;
  };
  return 0;
}; // get_next_filename

int get_last_filename(char *fname){
  if(grab_open){
    return sprintf(fname,"%s",last_fname);
  } else {
    printf("Camera not open\n");
    return -1;
  };
  return 0;
}; // get_last_filename


// int set_exposure_number
//params.imagenumber

// int get_exposure_number


// int get_trigger
//external trigger that starts/controls data acquision
//picamtriggercoupling, picamtriggerdetermination, picamtriggerresponse
//picamtriggersource, picamtriggerstatus, picamtriggertermination, picamparemtertriggerdelay
void exposure (PicamHandle d ){
    Picam_StartAcquisition( d ); 

    PicamAcquisitionStatus status;
    PicamAvailableData available;
    PicamError err;
    piint dVal = 0;
    pichar D[]="|/-\\|/-\\";

    std::cout << "Acquiring " << NUM_FRAMES << " frames" << std::endl << std::endl << std::endl;
    do
    {
        err = Picam_WaitForAcquisitionUpdate( d, NO_TIMEOUT, &available, &status );
        if( status.running )
        {
            dVal = ( dVal + available.readout_count ) % 8;
            std::cout << D[dVal] << '\r' << std::flush;
        }
    }
    while( status.running || err == PicamError_TimeOutOccurred );
}





//set shutter timing mode to always closed at the end

int bias
int dark

int close_shutter

int open_shutter


int add_header




// Function to write frame data to CSV file
void writeFrameDataToCSV(const std::string& outputDir, int frameIndex, PicamCameraID id, const std::string& exposureStartTime, pi64s tsRes)
{
    char idChar[256]; // Assuming a maximum length for the ID

    // Convert id.model and id.serial_number to strings
    std::stringstream modelStream;
    modelStream << id.model;
    std::string modelStr = modelStream.str();

    std::string serialStr = id.serial_number;

    // Use sprintf to format the camera ID
    sprintf(idChar, "%s:%s", modelStr.c_str(), serialStr.c_str());

    std::ostringstream oss;
    oss << outputDir << "/frame" << frameIndex << ".csv";
    std::string fileName = oss.str();
    std::ofstream outFile(fileName.c_str()); // Open file for writing

    if (!outFile) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
    }
    else {
        // Write metadata to CSV file
        outFile << "Camera ID," << idChar << std::endl;
        outFile << "Exposure Start Time," << exposureStartTime << std::endl;
        outFile << "Timestamp Resolution," << tsRes << std::endl;
        
        outFile.close();
        std::cout << "Timestamp data saved to " << fileName << std::endl;
    }
}

// //output exposing status (["exposing", "idle", "readout","writing"])
// //while exposing, status = "exposing"
// //while idle, status = "idle"
// //while reading, status = "readout"
// //while writing, status = "writeout"
//ReadoutAcquisionState
//PicamAcquisitionStateCounters
//PicamAcquisitionBuffer
//PicamAcquisitionUpdatedCallback()
//PicamAcquisitionStateUpdatedCallback()
//PicamAdvanced_CanRegisterForAcquisitionStateUpdated()
//PicamAdvanced_RegisterForAcquisitionStateUpdated()
//PicamReadoutControlMode
//if ReadoutAcquisionState=PicamAcquisitionState_ReadoutStarted, then status = 
//PicamOutputSignal: PicamOutputSignal_NotReadingOut, PicamOutputSignal_ReadingOut
// while exposing: change 'status' to 'exposing', and output the 

// */



//Set the default exposure time
    // piflt exposureTime = 120000; // 120 seconds default exposure



// void openCamera(){
// //     /*assign values to relavent params*/
// //     //picam specific functions
// //     params.camera; //camera object
// //     params.adcAnalogGain = 2 //1 = low, 2 = medium, 3 = high
// //     params.shutterStatus =  2 //1 = not connected, 2 = connected, 3 = overheated
// //     params.adcQuality =  1 //1 = low noise, 2 = high capacity, 4 = high speed, electron multiplied = 3
// //     params.tempSetPoint = -50 //celcius
// //     params.expTime = 120.0 //exposure time
// //     params.frameSize = 0;//

// //     params.vacuum
// //     params.date //file creation date
// //     params.date_obs //exposure start time
// //     //additional params
// //     sprintf(params.image_path,"./");
// //     sprintf(params.root_name,"image");
// //     sprintf(params.last_fname," ");
// //     params.imageNumber = 0 //number of exposures
// //     params.maxFiles = 1000 //max number of files
// //     params.output_dir = "output"
    
// // //////////////////////////////////////////

//     /*initialize library*/
//     // Picam_InitializeLibrary();
//     createDirectory(outputDir);

//     // // Check if the input is "./camera vars"
//     // if (argc == 2 && std::string(argv[1]) == "vars")
//     // {
//     //     // Print the contents of config_file.h
//     //     printAllVariables();
//     // }

//     /*get shutter status:
//     if shutter is not connected, then connect it

//     time stamp is when exposure starts:
//     PicamTimeStampsMask, PicamTimeStampsMask_ExposureStarted

//     PicamReadoutControlMode

//     set trigger

//     */
//    int imageNumber = 0;
//    open_cam = 1;
// }