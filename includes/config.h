#ifndef CONFIG_H
#define CONFIG_H

#include "picam.h"

// Function prototypes for camera configuration
void setAdcQualityHighSpeed(PicamHandle camera);
void setShutterModeNormal(PicamHandle camera);
void setTemperatureSetPoint(PicamHandle camera);
void setMediumAnalogGain(PicamHandle camera);

/*
   Parameters are defined in camera_config.cpp.
   This header sets:
   - Camera model/camera ID
   - Analog gain
   - Exposure time
   - Shutter timing mode
   - Vacuum status (to be implemented)
   - Cooling fan status (to be implemented)
*/

#endif // CONFIG_H
