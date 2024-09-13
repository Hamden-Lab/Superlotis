/*
  This header file contains the structure definitions and function
  declarations needed to operate the Sophia camera with the Picam software.
  Created July 2024.
*/

#ifndef CAMERA_H_
#define CAMERA_H_
#include "picam.h"
#include <string>   // For std::string


// #define SIMULATE 0

// legact from NUVU software stuff
#define COMMAND_LINE_BUILD 1

#define STRING_LENGTH 255

// structure for camera parameters
typedef struct PicamPtcArgs {
//   PicamHandle camera;
  piint mode;
  piflt temp;
  piint gainValue;
  piflt exposure_time;
//   PicamAvailableData data;
//   int num_images;
//   piint roi;
//   piint readoutstride;
//   PicamCameraID id;
//   PicamAcquisitionErrorsMask errors;

  // piflt expTime;
  // Global counter for the number of images

}PicamPtcArgs;

//print error and get status functions
int PrintEnumString(PicamEnumeratedType type, piint value);
int PrintError(PicamError error); 

int get_exposure_time(piflt *exposure_time);
int get_shutter(piint *mode);
int get_temp(piflt *temp);
int get_analog_gain(piint *gainValue);

int set_exposure_time(piflt exposure_time);
int set_shutter(piint mode);
int set_temp(piflt temp);
int set_analog_gain(piint gainValue);

int open_camera();
int close_camera();
int commit_params();

int expose();
int dark();
int bias();
// int add_header(char *fname);
// int get_last_filename(char *fname);
// int get_next_filename(char *fname);

// void createDirectory(const std::string& dir);


// **** 161102 END



#endif // CAMERA_H_