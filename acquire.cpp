//Basic Acquisition Sample
//The sample will open the first camera attached
//and acquire 5 frames.  Part 2 of the sample will collect
//1 frame of data each time the function is called, looping
//through 5 times.

#define NUM_FRAMES  2

#include "stdio.h" 
#include "picam.h"
#include <iostream>

PicamHandle camera;
pi64s total_acquired = 0;
const pi64s target_frames = NUM_FRAMES;
piint readoutstride = 0;


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

// Define status enum
enum CameraStatus {
    Exposing = 0,
    Idle = 1,
    Readout = 2
} CameraStatus;

// Define the function
int get_status(pi64s total_acquired, pi64s target_frames) {
    if (total_acquired == 0){
        return Idle;
    }
    else if (total_acquired < target_frames){
        return Exposing;
    }
    else{
        return Readout;
    }
}

const char* status_to_string(piint status) {
    switch (status) {
        case Idle: return "Idle";
        case Exposing: return "Exposing";
        case Readout: return "Readout";
        default: return "Unknown";
    }
}


int main()
{
    Picam_InitializeLibrary();
    printf("lib initialzed\n");

    // - open the first camera if any or create a demo camera
    PicamCameraID id;
    const pichar* string;
    piint status;

    printf("starting...\n");
    if( Picam_OpenFirstCamera( &camera ) == PicamError_None )
        Picam_GetCameraID( camera, &id );
    else
    {
        Picam_ConnectDemoCamera(
            PicamModel_Sophia2048BUV135, 
            "XO30000923",
            &id );
        PicamError err = Picam_OpenCamera( &id, &camera );
        if (err != PicamError_None) {
            printf("Failed to open demo camera\n");
            return -1;
        }
    }

    Picam_GetEnumerationString( PicamEnumeratedType_Model, id.model, &string );
    printf( "%s", string );
    printf( " (SN:%s) [%s]\n", id.serial_number, id.sensor_name );
    Picam_DestroyString( string );

    Picam_GetParameterIntegerValue( camera, PicamParameter_ReadoutStride, &readoutstride );
    printf( "Waiting for %d frames to be collected\n\n", (int) NUM_FRAMES );

    status = get_status(total_acquired, target_frames);
    printf("Status: %s\n", status_to_string(status));

    while (total_acquired < target_frames)
    {
        PicamAvailableData data;
        PicamAcquisitionErrorsMask errors;

        PicamError err = Picam_Acquire(camera, 1, 6000, &data, &errors);
        status = get_status(total_acquired, target_frames);
        
        if (err != PicamError_None)
        {
            printf("Acquisition error after %ld frames\n", total_acquired);
            PrintError(err);
            break;
        }
        else
        {
            total_acquired += data.readout_count;
            printf("Total frames acquired so far: %ld\n", total_acquired);

            // Print the data for this chunk
            PrintData((pibyte*)data.initial_readout, (piint)data.readout_count, readoutstride);
            status = get_status(total_acquired, target_frames);
            printf("Status: %s\n", status_to_string(status));
        }
    }
    status = get_status(total_acquired, target_frames);
    printf("Final Status: %s\n", status_to_string(status));
    
    Picam_CloseCamera( camera );
    Picam_UninitializeLibrary();
}
