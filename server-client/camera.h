#ifndef CAMERA_H_
#define CAMERA_H_
#include "picam.h"
#include <string.h>  
#include "fitsio.h"
// #include <cstdio>
// #include <iostream>

#define COMMAND_LINE_BUILD 1
#define STRING_LENGTH 255

#define NO_TIMEOUT             -1
#define TIME_BETWEEN_READOUTS  10  
#define NUM_EXPOSURES          2
#define EXP_TIME               5.0  
#define ACQUIRE_TIMEOUT        15000 
#define OK                     0
#define ERR                    -1
#define ELEMENT_SIZE 2 //for dtype = uint16, this is a camera property
#define WIDTH 2048
#define HEIGHT 2048

// structure for camera parameters
typedef struct PicamPtcArgs {
//   PicamHandle camera;
  piint mode;
  piflt temp;
  piint gain;
  piflt exposure_time;
  const char* filename;
  const char* fits_filename;
  PicamAvailableData data;
  PicamAcquisitionErrorsMask errors;
  PicamCameraID id;
  piint readoutstride;
  const pichar* string;
  FILE* pFile;
  PicamHandle camera;
  // piint res_width; //resolution width: 2048
  // piint res_height; //resolution width: 2048

}PicamPtcArgs;

//print error and get status functions
void PrintData( pibyte* buf, piint numframes, piint framelength);

int PrintEnumString(PicamEnumeratedType type, piint value);
int PrintError(PicamError error); 

int get_exposure_time(piflt *exposure_time);
int get_shutter(piint *mode);
int get_temp(piflt *temp);
int get_analog_gain(piint *gain);

int set_exposure_time(piflt exposure_time);
int set_shutter(piint mode);
int set_temp(piflt temp);
int set_analog_gain(piint gain);
int status(piflt exposure_time, piint gainValue, piint mode, piflt temp);

int open_camera();
int close_camera();
int commit_params();
int image(const char *filename);

int expose(const char *expose_filename);
int dark(const char *dark_filename);
int bias(const char *bias_filename);
int burst(int i); //new 

//handling output from image()
int resize_raw(const char* filename);
int convert_raw_to_fits(const char *filename, const char *fits_filename);


// int save_as_fits(const std::string& output_file, const std::vector<uint16_t>& data, piint *width, piint *height);


// int add_header(char *fname);
// int get_last_filename(char *fname);
// int get_next_filename(char *fname);

// void createDirectory(const std::string& dir);


// **** 161102 END



#endif // CAMERA_H_