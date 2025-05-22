/*
This script takes a bias, a dark and then an exposure
Exposure time for the dark and science images is set to 120ms for testing purposes
Number of exposures is set to 10
Added ROIs
*/
#include "stdio.h"
#include "picam.h"
#include "camera.h"
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

// // Global ROI variables
// piint roi_x = 100;      // X position
// piint roi_y = 100;      // Y position
// piint roi_width = 500;  // Width
// piint roi_height = 400; // Height

PicamPtcArgs params;
const PicamRoisConstraint* constraints;
const pichar* string;
const PicamRois* region;
const PicamParameter		*paramsFailed;	 /* Failed to commit    */
piint						failCount;		 /* Count of failed	    */

void PrintEnumString(PicamEnumeratedType type, piint value)
{
    Picam_GetEnumerationString(type, value, &string);
    printf("%s", string);
    Picam_DestroyString(string);
}

void PrintError(PicamError error)
{
    if (error == PicamError_None)
        printf("Succeeded\n");
    else
    {
        printf("Failed (");
        PrintEnumString(PicamEnumeratedType_Error, error);
        printf(")\n");
    }
}

void get_exposure_time(piflt *exposure_time)
{
    printf("Getting current exposure time...\n");
    // *exposure_time = params.exposure_time;
    PicamError err;
    err = Picam_GetParameterFloatingPointValue(params.camera, PicamParameter_ExposureTime, exposure_time);
    if (err != PicamError_None)
    {
        printf("Failed to get exposure time.\n");
        PrintError(err);
    }
    else
    {
        // std::cout << "Exposure time is: " << exposure_time << std::endl;
        // std::cout << "Exposure time is: " << &exposure_time << std::endl;
        std::cout << "Exposure time is: " << params.exposure_time << " ms " << std::endl;
        // printf("Exposure time is: %.2f ms\n", *(double*)exposure_time);
    }
}

void set_exposure_time(piflt exposure_time)
{
    get_exposure_time(&exposure_time);
    params.exposure_time = exposure_time;
    printf("Setting new exposure time...\n");
    PicamError error = Picam_SetParameterFloatingPointValue(params.camera, PicamParameter_ExposureTime, params.exposure_time);
    if (error != PicamError_None)
    {
        printf("Failed to set exposure time.\n");
        PrintError(error);
    }
    else
    {
        std::cout << "Exposure time set to: " << params.exposure_time << " ms " << std::endl;
        // printf("Exposure time set to: %.2f ms\n", exposure_time);
    }
}

void get_shutter(piint *mode)
{
    *mode = params.mode;
    printf("Getting shutter mode...\n");
    PicamError error;
    error = Picam_GetParameterIntegerValue(params.camera, PicamParameter_ShutterTimingMode, mode);
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
    }
    else
    {
        std::cout << "Failed to get shutter timing mode." << std::endl;
    }
}

void set_shutter(piint mode)
{
    PicamError error;
    get_shutter(&mode);
    params.mode = mode;
    printf("Setting shutter mode...\n");
    error = Picam_SetParameterIntegerValue(params.camera, PicamParameter_ShutterTimingMode, params.mode);
    if (error == PicamError_None){
    const char* shutterDescription = "unknown";
    switch (params.mode) {
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
    }
    else
    {
        std::cout << "Failed to set shutter mode." << std::endl;
    }

    // PrintError(error);
}


void get_temp(piflt *temp)
{
    PicamError error;
    std::cout << "Getting sensor temperature..."<< std::endl;
    error = Picam_ReadParameterFloatingPointValue(
        params.camera,
        PicamParameter_SensorTemperatureReading,
        temp);
    PrintError(error);
    *temp = params.temp;
    if (error == PicamError_None)
    {
        std::cout << "Current temperature is " << params.temp << " degrees C" << std::endl;
    }
}

void set_temp(piflt temp)
{
    get_temp(&temp);
    PicamError error;
    // Check sensor temperature status
    std::cout << "Checking sensor temperature status..."<< std::endl;
    PicamSensorTemperatureStatus status;
    error = Picam_ReadParameterIntegerValue(
        params.camera,
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
            std::cout << "Setting sensor temperature..."<< std::endl;
            error = Picam_SetParameterFloatingPointValue(
                params.camera,
                PicamParameter_SensorTemperatureSetPoint,
                params.temp);
            PrintError(error);
        }
        else
        {
            std::cout << "Temperature is not locked. Skipping setting temperature." << std::endl;
        }
        
    }

}

void get_analog_gain(piint *gainValue)
{
    *gainValue = params.gainValue;
    std::cout << "Getting adc analog gain..." << std::endl;
    PicamError error;
    error = Picam_GetParameterIntegerValue(params.camera, PicamParameter_AdcAnalogGain, gainValue);

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
    }
    else
    {
        std::cout << "Failed to get analog gain." << std::endl;
    }
}

/*    PicamAdcAnalogGain_Low    = 1,
    PicamAdcAnalogGain_Medium = 2,
    PicamAdcAnalogGain_High   = 3*/

void set_analog_gain(piint gainValue)
{
    PicamError error;
    gainValue = params.gainValue;
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

    error = Picam_SetParameterIntegerValue(params.camera, PicamParameter_AdcAnalogGain, params.gainValue);


    // std::cout << "Set analog gain: ";
    // error = Picam_SetParameterIntegerValue(camera, PicamParameter_AdcAnalogGain, gainValue);
    // PrintError(error);

    // Store the gain value in params
    if (error == PicamError_None)
    {
        // params.gainValue = gainValue; // Update params with the new gain value
        std::cout << "Checking analog gain value... \nAnalog gain: " << gainDescription << std::endl;
    }
}

// void get_roi(piint *width, piint *height) {
//     std::cout << "Getting rois... " << std::endl;
//     *width = params.width;
//     *height = params.height;
//     PicamError err;

//     // Get and print ROI constraints
//     err = Picam_GetParameterRoisConstraint(params.camera, 
//                                            PicamParameter_Rois, 
//                                            PicamConstraintCategory_Required, 
//                                            &constraints);
//     if (err == PicamError_None) {
//         // printf("ROI Constraints:\n");
//         *width = static_cast<piint>(constraints->width_constraint.maximum);
//         *height = static_cast<piint>(constraints->height_constraint.maximum);
//         std::cout << "Current width: " << *width << std::endl;
//         std::cout << "Current height: " << *height << std::endl;

//         // Clean up constraints after use
//         Picam_DestroyRoisConstraints(constraints);
//             } else {
//         std::cerr << "Error getting ROI constraints: " << err << std::endl;
//     }
// }

// void set_rois(piint width, piint height)
// {
//     PicamError err;
//     params.width = width;
//     params.height = height;

//     get_rois(&width, &height);
//     std::cerr << "Setting rois..."<< std::endl;
//         err = Picam_GetParameterRoisValue(params.camera, PicamParameter_Rois, &region);
//         if (err == PicamError_None)
// 		{
// 			/* Modify the region */
// 			if (region->roi_count == 1) 
// 			{
// 				/* The absolute size of the ROI */
// 				region->roi_array[0].height		= height;
// 				region->roi_array[0].width		= width;

// 				/* The offset into the chip of the ROI (1/4th) */
// 				region->roi_array[0].x			= width  / 2;
// 				region->roi_array[0].y			= height / 2;

// 				/* The vertical and horizontal binning */
// 				region->roi_array[0].x_binning	= 1;
// 				region->roi_array[0].y_binning	= 1;

//             /* Set the region of interest */
// 			err = Picam_SetParameterRoisValue(	params.camera, 
// 												PicamParameter_Rois, 
// 												region);
//             const PicamRoi& roi = region->roi_array[0];

//             std::cout << "New width: " << roi.width << std::endl;
//             std::cout << "New height: " << roi.height << std::endl;
//             std::cout << "New x offset: " << roi.x << std::endl;
//             std::cout << "New y offset: " << roi.y << std::endl;
//             std::cout << "New x binning: " << roi.x_binning << std::endl;
//             std::cout << "New y binning: " << roi.y_binning << std::endl;

// 			/* Error check */
// 			if (err == PicamError_None)
// 			{   
//                 const PicamParameter* paramsFailed;
//                 piint failCount;

// 				/* Commit ROI to hardware */
// 				err = Picam_CommitParameters(   params.camera, 
// 												&paramsFailed, 
// 												&failCount);
//                 Picam_DestroyParameters(paramsFailed);
//             }
//         	Picam_DestroyRois(region);
//             }
//         } else{
//             std::cerr << "Error getting ROI values: " << err << std::endl;
//     }
// }

// void get_roi(piint *roi)
// {
//     *roi = params.roi;
//     // PicamError err;

//     // // Get and print ROI constraints
//     // err = Picam_GetParameterRoisConstraint(params.camera, 
//     //                                        PicamParameter_Rois, 
//     //                                        PicamConstraintCategory_Required, 
//     //                                        &constraints);
//     // if (err == PicamError_None)
//     // {
//     //     std::cout << "Getting region of interest... \nRegion of interest: " << err << std::endl;
//     // }
//     PicamError err;
//     std::cerr << "Getting roi..."<< std::endl;
//     piint *width, *height;
//     err = Picam_GetParameterRoisConstraint(	params.camera, 
// 											PicamParameter_Rois, 
// 											PicamConstraintCategory_Required, 
// 											&constraints);	
//     if (err == PicamError_None)
//     {
//         width = (piint)constraints->width_constraint.maximum;
// 		height = (piint)constraints->height_constraint.maximum;

//         Picam_DestroyRoisConstraints(constraints);

//         /* Modify the region */
//         if (region->roi_count == 1) 
//         {
//             /* The absolute size of the ROI */
//             region->roi_array[0].height		= height;
//             region->roi_array[0].width		= width;

//             /* The offset into the chip of the ROI (1/2th) */
//             region->roi_array[0].x			= width  / 2;
//             region->roi_array[0].y			= height / 2;

//             /* The vertical and horizontal binning */
//             region->roi_array[0].x_binning	= 1;
//             region->roi_array[0].y_binning	= 1;
//         }
//         // err = Picam_SetParameterRoisValue(	params.camera, 
// 		// 										PicamParameter_Rois, 
// 		// 										region);
// 		// 	/* Error check */
// 		// if (err == PicamError_None)
// 		// {
// 		// 		/* Commit ROI to hardware */
// 		// err = Picam_CommitParameters(params.camera, &paramsFailed, &failCount);
//         // Picam_DestroyParameters(paramsFailed);
//         }
    
//     Picam_DestroyRois(region);
// }

void get_roi(piint *roi) {
    std::cout << "Getting roi... " << std::endl;
    *roi = params.roi;
    PicamError err;

    // Get and print ROI constraints
    err = Picam_GetParameterRoisConstraint(params.camera, 
                                           PicamParameter_Rois, 
                                           PicamConstraintCategory_Required, 
                                           &constraints);
    if (err == PicamError_None) {
        // printf("ROI Constraints:\n");
        *width = static_cast<piint>(constraints->width_constraint.maximum);
        *height = static_cast<piint>(constraints->height_constraint.maximum);
        std::cout << "Current width: " << *width << std::endl;
        std::cout << "Current height: " << *height << std::endl;

        // Clean up constraints after use
        Picam_DestroyRoisConstraints(constraints);
            } else {
        std::cerr << "Error getting ROI constraints: " << err << std::endl;
    }
}

void set_roi(piint roi)
{
    get_roi(&roi);
    // std::cout << "Setting region of interest... "<< std::endl;

    // params.roi = roi;
    // PicamError err;
    // err = Picam_SetParameterRoisValue(	params.camera, 
	// 											PicamParameter_Rois, 
	// 											region);
    std::cerr << "Setting roi..."<< std::endl;


}

void get_num_images(int *num_images)
{
    std::cerr << "Getting number of images..." << std::endl;
    *num_images = params.num_images;
    std::cerr << "Number of images: " << *num_images << std::endl;
}

void set_num_images(int num_images)
{
    get_num_images(&num_images);
    std::cerr << "Setting number of images..." << std::endl;
    if (num_images >= 0){
        params.num_images = num_images;}
}

void open_camera()
{
    Picam_InitializeLibrary();
    std::cout << "Open camera" << std::endl;

    if (Picam_OpenFirstCamera(&params.camera) == PicamError_None)
        Picam_GetCameraID(&params.camera, &params.id);
    else
    {
        Picam_ConnectDemoCamera(PicamModel_Pixis100F, "0008675309", &params.id);
        Picam_OpenCamera(&params.id, &params.camera);
        printf("No Camera Detected, Creating Demo Camera\n");
    }

    Picam_GetEnumerationString(PicamEnumeratedType_Model, params.id.model, &string);
    printf("%s", string);
    printf(" (SN:%s) [%s]\n", params.id.serial_number, params.id.sensor_name);
    Picam_DestroyString(string);
    Picam_GetParameterIntegerValue(params.camera, PicamParameter_ReadoutStride, &params.readoutstride);


    // set_rois(1340, 100);
    // set_temp(-50.0);
    // set_analog_gain(2);
    // set_shutter(2);
    // set_exposure_time(5);
    set_shutter(params.mode);
    set_temp(params.temp);
    set_analog_gain(params.gainValue);
    set_exposure_time(params.exposure_time);
    set_num_images(params.num_images);
    set_roi(params.roi);

}

void close_camera()
{
    // PicamHandle camera;
    std::cout << "Close camera" << std::endl;
    Picam_CloseCamera(params.camera);
    Picam_UninitializeLibrary();
}

void commit_params()
{
    pibln committed;
    PicamError error = Picam_AreParametersCommitted(params.camera, &committed);
    if (error != PicamError_None || !committed)
    {
        const PicamParameter* failed_parameters;
        piint failed_parameter_count;
        error = Picam_CommitParameters(params.camera, &failed_parameters, &failed_parameter_count);
        if (error != PicamError_None)
        {
            std::cerr << "Failed to commit parameters. ";
            PrintError(error);
            return;
        }
        if (failed_parameter_count > 0)
        {
            Picam_DestroyParameters(failed_parameters);
        }
    }
}
void expose()
{
    std::cout << "Take exposure" << std::endl;

    // Commit the parameters before acquisition
    commit_params();

    // The acquisition logic
    set_shutter(1); // Set shutter mode to normal


    PicamError error = Picam_Acquire(params.camera, params.num_images, TIME_BETWEEN_READOUTS, &params.data, &params.errors);
    if (error == PicamError_None)
    {
        std::cout << "Successfully took exposure" << std::endl;
        params.num_images++;
        // image_counter++;
        // std::cout << "Image " << image_counter << " taken." << std::endl;
    }
    else
    {
        std::cerr << "Failed to acquire image. ";
        PrintError(error);
    }
}


void dark()
{
    std::cout << "Take dark" << std::endl;
    set_shutter(2); // Set shutter mode to closed
    // piflt exposure_time = EXP_TIME;

    set_exposure_time(params.exposure_time);

    commit_params();

    PicamError error = Picam_Acquire(params.camera, params.num_images, TIME_BETWEEN_READOUTS, &params.data, &params.errors);
    if (error == PicamError_None)
        {
            std::cout << "Successfully took dark" << std::endl;
            params.num_images++;
        }
        else
        {
            std::cerr << "Failed to acquire image. ";
            PrintError(error);
        }
}

void bias()
{
    std::cout << "Take bias" << std::endl;
    set_shutter(3); // Set shutter mode to open
    piflt exposure_time = 0; 
    // set_exposure_time(0);

    set_exposure_time(exposure_time);

    commit_params();

    PicamError error = Picam_Acquire(params.camera, params.num_images, TIME_BETWEEN_READOUTS, &params.data, &params.errors);
    if (error == PicamError_None)
        {
            std::cout << "Successfully took bias" << std::endl;
            params.num_images++;

        }
        else
        {
            std::cerr << "Failed to acquire image. ";
            PrintError(error);
        }
        // get_num_images(params.num_images);

}

// int get_next_filename(char *fname){
//   if(grab_open){
//     return sprintf(fname,"%s%s%06d.fits",params.image_path,params.root_name,params.imagenumber);
//   } else {
//     printf("Camera not open\n");
//     return -1;
//   };
//   return 0;
// }; // get_next_filename

// int get_last_filename(char *fname){
//   if(grab_open){
//     return sprintf(fname,"%s",params.last_fname);
//   } else {
//     printf("Camera not open\n");
//     return -1;
//   };
//   return 0;
// }; // get_last_filename

// int add_header(char *fname){
//   // things for handling the fits file
//   fitsfile *fptr;
//   int iomode, status=0;
//   char dat[255],tim[255],temp[255],camtemp1[255],camtemp2[255];
//   iomode=READWRITE;
  
//   if(! fits_open_file(&fptr, fname, iomode, &status) ){

//     /* Update/Add Keywords */

//     fits_write_date(fptr,&status);
//     fits_update_key(fptr, TFLOAT, "EXPTIME",&params.exptime,"Exposure time (s)",&status);
    
//     fits_update_key(fptr, TINT, "EMGAIN",&params.emgain,"EM Gain",&status);
//     fits_update_key(fptr, TFLOAT, "GAIN1",&params.preamp,"Pre-amplifier gain",&status);
//     fits_update_key(fptr, TINT, "VSS",&params.vss,"Substrate voltage",&status);
//     fits_update_key(fptr, TINT, "IMNO",&params.imagenumber,"Image number",&status);
//     fits_update_key(fptr, TINT, "IMBURST",&params.imageburst,"Image burst",&status);
//     fits_update_key(fptr, TINT, "SHUTTER",&params.shutter,"Shutter state",&status);
//     fits_update_key(fptr,TINT,"ROS",&params.readoutSeq,"Read-out sequence",&status);
//     fits_update_key(fptr,TINT,"CDSOFFSET",&params.cdsoffset,"CDS Offset",&status);

//     if ( ! add_temps(dat,tim,camtemp1,camtemp2) ) { 
//       fits_update_key(fptr,TSTRING,"TEMPDATE",dat,"Temperature reading date",&status);
//       fits_update_key(fptr,TSTRING,"TEMPTIME",tim,"Temperature reading time",&status);
//       fits_update_key(fptr,TSTRING,"TEMPA",camtemp1,"Temperature reading (K)",&status);
//       fits_update_key(fptr,TSTRING,"TEMPB",camtemp2,"Temperature reading (K)",&status);
//     } else {
//       printf("Couldn't add temperatures.\n");
//     };

//     // observation type
//     switch (params.shutter){
//     case 0:
//       sprintf(temp,"OBJECT");
//       break;
//     case -2:
//       if ( params.exptime > 0) {
// 	sprintf(temp,"DARK");
//       } else {
// 	sprintf(temp,"BIAS");
//       }
//       break;
//     default:
//       sprintf(temp,"UNKNOWN");
//       break;
//     }
//     fits_update_key(fptr,TSTRING,"OBSTYPE",temp,"Image type",&status);


//     fits_close_file(fptr, &status);
//     if ( status ) { 
//       printf("Trouble updating FITS file: %s\n",fname);
//       return -1;
//     };
		    
//   } else {
//     printf("Could not open FITS file: %s\n",fname);
//     return -1;
//   };

//   return 0;
// };

int main()
{
    open_camera();
    for (int i = 0; i < params.num_images; ++i)
    {
        bias();
        dark();
        expose();
    }
    close_camera();
}