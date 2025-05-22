#include "picam.h" /* Teledyne Princeton Instruments API Header */
#include "stdio.h"
#include <iostream>

#define SAMPLE_REGIONS		4	/* Multiple Region Count */
#define ACQUIRE_TIMEOUT	15000	/* Fifteen second timeout */


PicamAcquisitionErrorsMask errors;
PicamCameraID id;
piint readoutstride;
const pichar* string;
PicamError err;
PicamHandle camera;
PicamRois region;
const PicamParameter *paramsFailed;
piint failCount;
PicamRoi roi;


void open_camera()
{
    Picam_InitializeLibrary();
    std::cout << "Open camera" << std::endl;

    if (Picam_OpenFirstCamera(&camera) == PicamError_None)
        Picam_GetCameraID(&camera, &id);
    else
    {
        Picam_ConnectDemoCamera(PicamModel_Pixis100F, "0008675309", &id);
        Picam_OpenCamera(&id, &camera);
        printf("No Camera Detected, Creating Demo Camera\n");
    }

    Picam_GetEnumerationString(PicamEnumeratedType_Model, id.model, &string);
    printf("%s", string);
    printf(" (SN:%s) [%s]\n", id.serial_number, id.sensor_name);
    Picam_DestroyString(string);
    Picam_GetParameterIntegerValue(camera, PicamParameter_ReadoutStride, &readoutstride);


}

// void get_roi(const PicamRois *region)
// {
// 	std::cout << "Getting region of interest..." << std::endl;
// 	err = Picam_GetParameterRoisValue(camera, 
// 									PicamParameter_Rois, 
// 									&region);
// 	if (err == PicamError_None)
// 	{
// 		PicamRoi roi = region->roi_array[0];


// 	}
// 	else
//     {
//         std::cout << "Failed to get region of interest." << std::endl;
//     }
// }

int get_rois(piint *x1, piint *x2, piint *y1, piint *y2)
{
	std::cout << "Getting region of interest..." << std::endl;
	roi = get_region(&region);
	piint x = roi.x;
	piint y = roi.y;
	piint width = roi.width;
	piint height = roi.height;
	piint x_binnig = roi.x_binning;
	piint y_binning = roi.x_binning;

	x1 = x;
	piint x2 = roi.x + (roi.width * roi.x_binning) -1;
	piint y1 = roi.y;
	piint y2 = roi.y + (roi.height * roi.y_binning) -1;
	std::cout << "[x1, x2, y1, y2] =" [x1, x2, y1, y2] << std::endl;
	std::cout << roi.y << std::endl;
	std::cout << roi.width << std::endl;
	std::cout << roi.height << std::endl;
	std::cout << roi.x_binning << std::endl;
	std::cout << roi.y_binning << std::endl;

}

PicamRoi get_region(PicamRois *region)
{
	std::cout << "Getting region of interest..." << std::endl;
	err = Picam_GetParameterRoisValue(camera, 
									PicamParameter_Rois, 
									&region);
	if (err == PicamError_None)
	{
		PicamRoi roi = region->roi_array[0];
	}
	else
    {
        std::cout << "Failed to get region of interest." << std::endl;
    }
	return roi;
}

void set_roi(piint x1, piint x2, piint y1, piint y2) 
{
    std::cout << "Setting region of interest..." << std::endl;
	roi = get_roi(&region);
    PicamError err;
	const PicamRoisConstraint *constraint;	
	err = Picam_GetParameterRoisConstraint(	camera, 
										PicamParameter_Rois, 
										PicamConstraintCategory_Required, 
										&constraint);
	
	if (err = PicamError_None)
	{
        piint width_max = (piint)constraint->width_constraint.maximum;
        piint height_max = (piint)constraint->height_constraint.maximum;
		err = Picam_SetParameterRoisValue(camera, PicamParameter_Rois, &region);
		if (err == PicamError_None)
		{
			// PicamRoi roi = region.roi_array[0];

			// if (roi.width <= width_max)
			// {
			x1 = roi.x;
			x2 = roi.x + (roi.width * roi.x_binning) -1;
			y1 = roi.y;
			y2 = roi.y + (roi.height * roi.y_binning) -1;

			// std::cout << roi.x << std::endl;
			// std::cout << roi.y << std::endl;
			// std::cout << roi.width << std::endl;
			// std::cout << roi.height << std::endl;
			// std::cout << roi.x_binning << std::endl;
			// std::cout << roi.y_binning << std::endl;
			// }

		}
	}
}



// 	}
// }

	// // const PicamRoisConstraint *constraint;	

	// // err = Picam_GetParameterRoisConstraint(	camera, 
	// // 									PicamParameter_Rois, 
	// // 									PicamConstraintCategory_Required, 
	// // 									&constraint);	
	// if (err == PicamError_None)
	// {	PicamRoi roi = region.roi_array[0];
	// 	piint width_max = (piint)constraint->width_constraint.maximum; //need this
	// 	// piint height_max = (piint)constraint->height_constraint.maximum; //need this
	// 	if (width_max <= roi.width)
	// 	{
	// 	err = Picam_SetParameterRoisValue(	camera, 
	// 						PicamParameter_Rois, 
	// 						&region);
		
// 		if (err == PicamError_None)
// 		{
// 			/* Commit ROI to hardware */
// 			err = Picam_CommitParameters(	camera, 
// 											&paramsFailed, 
// 											&failCount);
// 			Picam_DestroyParameters(paramsFailed);

// 	}
// 	}
// 	}
// }



int main(){	
    open_camera();
	PicamRois region; // Assume region is initialized and populated properly


}


//compute region, based on x1, x2, y1, y2 
//first step: convert x1, y1 to x and y
//convert y1, y2 to width/x_binning and height/y_binning


				
// 	err = Picam_SetParameterRoisValue(	camera, 
// 									PicamParameter_Rois, 
// 									&region);
// 	if (err == PicamError_None)
// 	{
// 		// Print each ROI in the array
// 		for (piint i = 0; i < region->roi_count; ++i)
// 		{
// 			const PicamRoi roi = region->roi_array[i];
// 			x = roi.x;
// 			y = roi.y;
// 			height = roi.height;
// 			width = roi.width;
// 			x_binning = roi.x_binning;
// 			y_binning = roi.y_binning;
// 			std::cout << "ROI " << i << ": "
// 			<< "x: " << x << ", y: " << y
// 			<< ", width: " << width << ", height: " << height
// 			<< ", x_binning: " << x_binning
// 			<< ", y_binning: " << y_binning << std::endl;

// 		}
// 	}
// }