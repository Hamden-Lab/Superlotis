/*
This script defines camera commands 
*/
#include "stdio.h"
#include "picam.h"
#include "camera.h"
#include "fitsio.h"
#include "fitsio2.h"
#include <iostream>
#include <sstream>
#include <sys/stat.h>  // For stat and mkdir
#include <cerrno>      // For errno
#include <cstring>     // For strerror
#include <cstdio>

#define NO_TIMEOUT  -1
#define TIME_BETWEEN_READOUTS 10 //ms
#define NUM_EXPOSURES  2
#define EXP_TIME 5.0 // Default exposure time in milliseconds
#define ACQUIRE_TIMEOUT 15000  // Fifteen-second timeout

#define OK 0
#define ERR -1

PicamPtcArgs params;

typedef struct    /* structure containing mem file structure */ 
{
    char **memaddrptr;   /* Pointer to memory address pointer; */
                         /* This may or may not point to memaddr. */
    char *memaddr;       /* Pointer to starting memory address; may */
                         /* not always be used, so use *memaddrptr instead */
    size_t *memsizeptr;  /* Pointer to the size of the memory allocation. */
                         /* This may or may not point to memsize. */
    size_t memsize;      /* Size of the memory allocation; this may not */
                         /* always be used, so use *memsizeptr instead. */
    size_t deltasize;    /* Suggested increment for reallocating memory */
    void *(*mem_realloc)(void *p, size_t newsize);  /* realloc function */
    LONGLONG currentpos;   /* current file position, relative to start */
    LONGLONG fitsfilesize; /* size of the FITS file (always <= *memsizeptr) */
    FILE *fileptr;      /* pointer to compressed output disk file */
} memdriver;
static memdriver memTable[NMAXFILES];  /* allocate mem file handle tables */

// const PicamRoisConstraint* constraints;
// int num_image;


//converting raw file output from image() into a fits file, using cfitsio
//uses mem_rawfile_open, defined in drvrmem.c
int generate_fits(const char *filename, int rwmode, int &handle)
{
    std::cout << "Convert raw file to fits";
    int status;
    // char filename[] = "/bin/exposure_file.raw[b512,512]"
    status = mem_rawfile_open(const_cast<char *>(filename), rwmode, &handle);

    if (status == 0) {
        printf("Raw file successfully opened and loaded into memory.\n");
        // Perform further operations on the FITS data in memory using the handle
    } else {
        printf("Failed to open the raw file. Error code: %d\n", status);
    }

    // Clean up: Free the memory associated with the handle
    mem_close_free(handle);

    return 0;
}

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
    // const pichar* string;
    PicamError error = Picam_GetEnumerationString(type, value, &params.string);
    if (error == PicamError_None)
    {
        printf("%s", params.string);
        Picam_DestroyString(params.string);
        return OK;  // Success
    }
    else
    {
        return ERR; // Failure
    }
}

int PrintError(PicamError error)
{
    if (error == PicamError_None){
        printf("Succeeded\n");
        return OK;
    }
    else
    {
        printf("Failed (");
        PrintEnumString(PicamEnumeratedType_Error, error);
        printf(")\n");
        return ERR;
    }
}

int get_exposure_time(piflt *exposure_time)
{
    printf("Getting current exposure time...\n");
    PicamError err;
    err = Picam_GetParameterFloatingPointValue(params.camera, PicamParameter_ExposureTime, &params.exposure_time);
    if (err != PicamError_None)
    {
        printf("Failed to get exposure time.\n");
        PrintError(err);
        return ERR;
    }
    else
    {
        std::cout << "Exposure time is: " << *exposure_time << " ms " << std::endl;
        // printf("Exposure time is: %.2f ms\n", *(double*)exposure_time);
        return OK;
    }
}


int set_exposure_time(piflt exposure_time)
{
    // get_exposure_time(&exposure_time);
    printf("Setting new exposure time...\n");
    PicamError error = Picam_SetParameterFloatingPointValue(params.camera, PicamParameter_ExposureTime, params.exposure_time);
    if (error != PicamError_None)
    {
        printf("Failed to set exposure time.\n");
        PrintError(error);
        return ERR;
    }
    else
    {
        std::cout << "Exposure time set to: " << exposure_time << " ms " << std::endl;
        return OK;
    }
}

int get_shutter(piint *mode)
{
    printf("Getting shutter mode...\n");
    PicamError error;
    error = Picam_GetParameterIntegerValue(params.camera, PicamParameter_ShutterTimingMode, &params.mode);
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
        return OK;
    }
    else
    {
        std::cout << "Failed to get shutter timing mode." << std::endl;
        return ERR;
    }
}


int set_shutter(piint mode)
{
    PicamError error;
    get_shutter(&mode);
    printf("Setting shutter mode...\n");
    error = Picam_SetParameterIntegerValue(params.camera, PicamParameter_ShutterTimingMode, params.mode);
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
    return OK;
    }
    else
    {
        std::cout << "Failed to set shutter mode." << std::endl;
        return ERR;
    }

    // PrintError(error);
}


int get_temp(piflt *temp)
{
    PicamError error;
    std::cout << "Getting sensor temperature..."<< std::endl;
    error = Picam_ReadParameterFloatingPointValue(
        params.camera,
        PicamParameter_SensorTemperatureReading,
        &params.temp);
    PrintError(error);
    if (error == PicamError_None)
    {
        std::cout << "Current temperature is " << *temp << " degrees C" << std::endl;
        return OK;
    }
    else
    {
        return ERR;
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
            std::cout << "Setting sensor temperature..." << std::endl;
            error = Picam_SetParameterFloatingPointValue(
                params.camera,
                PicamParameter_SensorTemperatureSetPoint,
                temp);
            PrintError(error);
            return OK;  // Return success if setting the temperature was successful
        }
        else
        {
            std::cout << "Temperature is not locked. Skipping setting temperature." << std::endl;
            return ERR;  // Return -1 if temperature is not locked
        }
    }
    
    // Return an error code if reading the parameter failed
    std::cerr << "Failed to read sensor temperature status." << std::endl;
    return -2;  // Indicate failure in reading the sensor temperature status
}



int get_analog_gain(piint *gain)
{
    std::cout << "Getting adc analog gain..." << std::endl;
    PicamError error;
    error = Picam_GetParameterIntegerValue(params.camera, PicamParameter_AdcAnalogGain, &params.gain);

    // Print current analog gain
    if (error == PicamError_None)
    {
        const char* gainDescription = "unknown";
        switch (*(int*)gain) {
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
        return OK;
    }
    else
    {
        std::cout << "Failed to get analog gain." << std::endl;
        return ERR;
    }
}

/*    PicamAdcAnalogGain_Low    = 1,
    PicamAdcAnalogGain_Medium = 2,
    PicamAdcAnalogGain_High   = 3*/

int set_analog_gain(piint gain)
{
    PicamError error;
    get_analog_gain(&gain);
    printf("Setting adc analog gain...\n");
    
    const char* gainDescription = "unknown";
    switch (gain) {
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

    error = Picam_SetParameterIntegerValue(params.camera, PicamParameter_AdcAnalogGain, params.gain);


    // std::cout << "Set analog gain: ";
    // error = Picam_SetParameterIntegerValue(params.camera, PicamParameter_AdcAnalogGain, gain);
    // PrintError(error);

    // Store the gain value in params
    if (error == PicamError_None)
    {
        // params.gain = gain; // Update params with the new gain value
        std::cout << "Checking analog gain value... \nAnalog gain: " << gainDescription << std::endl;
        return OK;
    }
    else
    {
    return ERR;
    }
}

int open_camera()
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

    Picam_GetEnumerationString(PicamEnumeratedType_Model, params.id.model, &params.string);
    printf("%s", params.string);
    printf(" (SN:%s) [%s]\n", params.id.serial_number, params.id.sensor_name);
    Picam_DestroyString(params.string);
    Picam_GetParameterIntegerValue(params.camera, PicamParameter_ReadoutStride, &params.readoutstride);


    set_temp(-10.0);
    set_analog_gain(2);
    set_shutter(2);
    return OK;

    // sprintf(params.image_path,"./");
    // sprintf(params.root_name,"image");
    // sprintf(params.last_fname," ");
//   params.imagenumber=0;
}

int close_camera()
{
    // PicamHandle camera;
    std::cout << "Close camera" << std::endl;
    Picam_CloseCamera(params.camera);
    Picam_UninitializeLibrary();
    return OK;
}

int commit_params()
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
            return ERR;
        }
        if (failed_parameter_count > 0)
        {
            Picam_DestroyParameters(failed_parameters);
            return -2;  // Indicate that some parameters failed to commit
        }
    }
    
    // If no params.errors and parameters are committed
    return OK;  // Indicate success
}


int image(const char *filename)
{
    PicamError error = Picam_Acquire(params.camera, 2, 6000, &params.data, &params.errors);
   if (error == PicamError_None)
    {
        std::cout << "Successfully took frame" << std::endl;
        std::cout << "Filename is:" << filename << std::endl;

        // std::cout << *static_cast<pibyte*>(data.initial_readout) << std::endl;
        PrintData( (pibyte*)params.data.initial_readout, 2, params.readoutstride );
		params.pFile = fopen(filename, "wb" );
		if( params.pFile )
		{
			if( !fwrite(params.data.initial_readout, 1, (2*params.readoutstride), params.pFile ) )
				printf( "Data file not saved\n" );
			fclose( params.pFile );
		}
    }
    else
    {
        std::cerr << "Failed: ";
        PrintError(error);
        return ERR;
    }
    return OK;
}

int expose(const char *expose_filename)
{
    std::cout << "Take exposure" << std::endl;
    commit_params();

    // The acquisition logic
    set_shutter(1); 

    // Commit the parameters before acquisition

    image(expose_filename);
    return OK;

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
    return OK;
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

    return OK;
}


int bias(const char *bias_filename)
{
    std::cout << "Take bias" << std::endl;
    commit_params();
    set_shutter(3); // Set shutter mode to open
    // piflt exposure_time = 0; 

    set_exposure_time(0);

    image(bias_filename);

    return OK;
}


int mem_rawfile_open(char *filename, int rwmode, int *hdl)
/*
  This routine creates an empty memory buffer, writes a minimal
  image header, then copies the image data from the raw file into
  memory.  It will byteswap the pixel values if the raw array
  is in little endian byte order.
*/
{
    FILE *diskfile;
    fitsfile *fptr;
    short *sptr;
    int status, endian, datatype, bytePerPix, naxis;
    long dim[5] = {1,1,1,1,1}, ii, nvals, offset = 0;
    size_t filesize = 0, datasize;
    char rootfile[FLEN_FILENAME], *cptr = 0, *cptr2 = 0;
    void *ptr;

    if (rwmode != READONLY)
    {
        ffpmsg(
  "cannot open raw binary file with WRITE access (mem_rawfile_open)");
        ffpmsg(filename);
        return(READONLY_FILE);
    }

    cptr = strchr(filename, '[');   /* search for opening bracket [ */

    if (!cptr)
    {
        ffpmsg("binary file name missing '[' character (mem_rawfile_open)");
        ffpmsg(filename);
        return(URL_PARSE_ERROR);
    }

    *rootfile = '\0';
    strncat(rootfile, filename, cptr - filename);  /* store the rootname */

    cptr++;

    while (*cptr == ' ')
       cptr++;    /* skip leading blanks */

    /* Get the Data Type of the Image */

    if (*cptr == 'b' || *cptr == 'B')
    {
      datatype = BYTE_IMG;
      bytePerPix = 1;
    }
    else if (*cptr == 'i' || *cptr == 'I')
    {
      datatype = SHORT_IMG;
      bytePerPix = 2;
    }
    else if (*cptr == 'u' || *cptr == 'U')
    {
      datatype = USHORT_IMG;
      bytePerPix = 2;

    }
    else if (*cptr == 'j' || *cptr == 'J')
    {
      datatype = LONG_IMG;
      bytePerPix = 4;
    }  
    else if (*cptr == 'r' || *cptr == 'R' || *cptr == 'f' || *cptr == 'F')
    {
      datatype = FLOAT_IMG;
      bytePerPix = 4;
    }    
    else if (*cptr == 'd' || *cptr == 'D')
    {
      datatype = DOUBLE_IMG;
      bytePerPix = 8;
    }
    else
    {
        ffpmsg("error in raw binary file datatype (mem_rawfile_open)");
        ffpmsg(filename);
        return(URL_PARSE_ERROR);
    }

    cptr++;

    /* get Endian: Big or Little; default is same as the local machine */
    
    if (*cptr == 'b' || *cptr == 'B')
    {
        endian = 0;
        cptr++;
    }
    else if (*cptr == 'l' || *cptr == 'L')
    {
        endian = 1;
        cptr++;
    }
    else
        endian = BYTESWAPPED; /* byteswapped machines are little endian */

    /* read each dimension (up to 5) */

    naxis = 1;
    dim[0] = strtol(cptr, &cptr2, 10);
    
    if (cptr2 && *cptr2 == ',')
    {
      naxis = 2;
      dim[1] = strtol(cptr2+1, &cptr, 10);

      if (cptr && *cptr == ',')
      {
        naxis = 3;
        dim[2] = strtol(cptr+1, &cptr2, 10);

        if (cptr2 && *cptr2 == ',')
        {
          naxis = 4;
          dim[3] = strtol(cptr2+1, &cptr, 10);

          if (cptr && *cptr == ',')
            naxis = 5;
            dim[4] = strtol(cptr+1, &cptr2, 10);
        }
      }
    }

    cptr = maxvalue(cptr, cptr2);

    if (*cptr == ':')   /* read starting offset value */
        offset = strtol(cptr+1, 0, 10);

    nvals = dim[0] * dim[1] * dim[2] * dim[3] * dim[4];
    datasize = nvals * bytePerPix;
    filesize = nvals * bytePerPix + 2880;
    filesize = ((filesize - 1) / 2880 + 1) * 2880; 

    /* open the raw binary disk file */
    status = file_openfile(rootfile, READONLY, &diskfile);
    if (status)
    {
        ffpmsg("failed to open raw  binary file (mem_rawfile_open)");
        ffpmsg(rootfile);
        return(status);
    }

    /* create a memory file with corrct size for the FITS converted raw file */
    status = mem_createmem(filesize, hdl);
    if (status)
    {
        ffpmsg("failed to create memory file (mem_rawfile_open)");
        fclose(diskfile);
        return(status);
    }

    /* open this piece of memory as a new FITS file */
    ffimem(&fptr, (void **) memTable[*hdl].memaddrptr, &filesize, 0, 0, &status);

    /* write the required header keywords */
    ffcrim(fptr, datatype, naxis, dim, &status);

    /* close the FITS file, but keep the memory allocated */
    ffclos(fptr, &status);

    if (status > 0)
    {
        ffpmsg("failed to write basic image header (mem_rawfile_open)");
        fclose(diskfile);
        mem_close_free(*hdl);   /* free up the memory */
        return(status);
    }

    if (offset > 0)
       fseek(diskfile, offset, 0);   /* offset to start of the data */

    /* read the raw data into memory */
    ptr = *memTable[*hdl].memaddrptr + 2880;

    if (fread((char *) ptr, 1, datasize, diskfile) != datasize)
      status = READ_ERROR;

    fclose(diskfile);  /* close the raw binary disk file */

    if (status)
    {
        mem_close_free(*hdl);   /* free up the memory */
        ffpmsg("failed to copy raw file data into memory (mem_rawfile_open)");
        return(status);
    }

    if (datatype == USHORT_IMG)  /* have to subtract 32768 from each unsigned */
    {                            /* value to conform to FITS convention. More */
                                 /* efficient way to do this is to just flip  */
                                 /* the most significant bit.                 */

      sptr = (short *) ptr;

      if (endian == BYTESWAPPED)  /* working with native format */
      {
        for (ii = 0; ii < nvals; ii++, sptr++)
        {
          *sptr =  ( *sptr ) ^ 0x8000;
        }
      }
      else  /* pixels are byteswapped WRT the native format */
      {
        for (ii = 0; ii < nvals; ii++, sptr++)
        {
          *sptr =  ( *sptr ) ^ 0x80;
        }
      }
    }

    if (endian)  /* swap the bytes if array is in little endian byte order */
    {
      if (datatype == SHORT_IMG || datatype == USHORT_IMG)
      {
        ffswap2( (short *) ptr, nvals);
      }
      else if (datatype == LONG_IMG || datatype == FLOAT_IMG)
      {
        ffswap4( (INT32BIT *) ptr, nvals);
      }

      else if (datatype == DOUBLE_IMG)
      {
        ffswap8( (double *) ptr, nvals);
      }
    }

    memTable[*hdl].currentpos = 0;           /* save starting position */
    memTable[*hdl].fitsfilesize=filesize;    /* and initial file size  */

    return(0);
}
    
    

int mem_close_free(int handle)
/*
  close the file and free the memory.
*/
{
    free( *(memTable[handle].memaddrptr) );

    memTable[handle].memaddrptr = 0;
    memTable[handle].memaddr = 0;
    return(0);
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
