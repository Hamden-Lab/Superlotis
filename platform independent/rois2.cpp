#include "picam.h" /* Teledyne Princeton Instruments API Header */
#include "stdio.h"
#include <iostream>
PicamCameraID id;
PicamHandle camera;

#define SAMPLE_REGIONS		4	/* Multiple Region Count */
#define ACQUIRE_TIMEOUT	15000	/* Fifteen second timeout */

void get_roi(PicamHandle camera);
/* Single Region Example Code */
void DoSingleROI(PicamHandle camera);

/* Multiple Region Example Code */
void DoMultipleROIs(PicamHandle camera);

/* Show error code helper routine */
void DisplayError(PicamError errCode);

/* Compute average pixel value helper routine */
piflt ComputeAverage(void *p, piint depth, piint x, piint y, piint pixelOffset);

/******************************************************************************
*	Teledyne Princeton Instruments Region of Interest (ROI) example.
*
*	This example main is broken up into two core parts 
*			a.) Single Region 
*			b.) Multiple region
*
******************************************************************************/
// int main()
// {
// 	// PicamError  errCode = PicamError_None;	/* Error Code		*/
// 	PicamError  err;	/* Error Code		*/
// 	PicamHandle camera;				/* Camera Handle	*/

// 	errCode = Picam_InitializeLibrary();
//     if (err == PicamError_None) {
//         /* Open the first camera */
//         if (Picam_OpenFirstCamera(&camera) == PicamError_None) {
//             Picam_GetCameraID(camera, &id);
//         } else {
//             Picam_ConnectDemoCamera(PicamModel_Pixis100F, "0008675309", &id);
//             Picam_OpenCamera(&id, &camera);
//             printf("No Camera Detected, Creating Demo Camera\n");
//         }

//         /* If the camera open succeeded */
//         if (camera != NULL) { // Check if camera is opened successfully
//             /* Setup and acquire with a single region of interest */
//             DoSingleROI(camera);
// 			get_roi(camera, &region, &x, &y, &width, &height, &x_binning, &y_binning);

// 			// get_roi(camera);
//             /* Setup and acquire with multiple regions of interest */
//             // DoMultipleROIs(camera);

//             /* We are done with the camera so close it */
//             Picam_CloseCamera(camera);
//         } else {
//             DisplayError(errCode); // Show error if camera opening failed
//         }
//     } else {
//         DisplayError(errCode); // Show error if library initialization failed
//     }

//     /* Uninitialize the library */
//     Picam_UninitializeLibrary();
//     return 0; // Return an integer to indicate successful completion
// }

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

/******************************************************************************
*	Teledyne Princeton Instruments Region of Interest (ROI) example.
*
*	Sets up the camera and acquires from a single region of interest.
*
*		------------------------	This example gets the maximum extent and
*		------------------------	sets up a window in the center that is 
*		------xxxxxxxxxxxx------	 1/2 of the maximum height and width.
*		------xxxxxxxxxxxx------
*		------xxxxxxxxxxxx------
*		------xxxxxxxxxxxx------
*		------------------------
*		------------------------
*
******************************************************************************/
// void get_rois(PicamHandle camera, piint *x2, piint *y2, piint *x1, piint *y1)
// {
// 	PicamError err;
//     const PicamRoisConstraint *constraint;

//     // Get dimensional constraints
//     err = Picam_GetParameterRoisConstraint(camera, 
//                                            PicamParameter_Rois, 
//                                            PicamConstraintCategory_Required, 
//                                            &constraint);
//     if (err == PicamError_None)
//     {
//         // Get width and height from constraints
//         piint width = (piint)constraint->width_constraint.maximum;
//         piint height = (piint)constraint->height_constraint.maximum;
// 		piint *x1 = (piint)constraint->x_constraint.minimum;
// 		piint *y1 = (piint)constraint->y_constraint.minimum;
		
// 		Picam_DestroyRoisConstraints(constraint);
// 		err = Picam_GetParameterRoisValue(	camera, 
// 											PicamParameter_Rois, 
// 											&region);
// 		/* Error check */
// 		if (err == PicamError_None)
// 		{
// 			/* Modify the region */
// 			if (region->roi_count == 1) 
// 			{
// 				/* The absolute size of the ROI */
// 				region->roi_array[0].height		= height;
// 				region->roi_array[0].width		= width;

// 				/* The offset into the chip of the ROI (1/4th) */
// 				region->roi_array[0].x			= *x1;
// 				region->roi_array[0].y			= *y1;

// 				/* The vertical and horizontal binning */
// 				region->roi_array[0].x_binning	= 1;
// 				region->roi_array[0].y_binning	= 1;
// 			}


		


//     //     // Clean up constraints after using them
//     //     Picam_DestroyRoisConstraints(constraint);
//     // }
// 		/* Get the orinal ROI */
// // 		err = Picam_GetParameterRoisValue(	camera, 
// // 											PicamParameter_Rois, 
// // 											&region);
// // 		/* Error check */
// // 		if (err == PicamError_None)
// // 		{
// // 			/* Modify the region */
// // 			if (region->roi_count == 1) 
// // 			{
// // 				/* The absolute size of the ROI */
// // 				region->roi_array[0].height		= height;
// // 				region->roi_array[0].width		= width;

// // 				/* The offset into the chip of the ROI (1/4th) */
// // 				region->roi_array[0].x			= width  / 2;
// // 				region->roi_array[0].y			= height / 2;

// // 				/* The vertical and horizontal binning */
// // 				region->roi_array[0].x_binning	= 1;
// // 				region->roi_array[0].y_binning	= 1;
// // 			}
// // }

// void set_rois(PicamHandle camera)
// {


// }

//compute region, based on x1, x2, y1, y2 
//first step: convert x1, y1 to x and y
//convert y1, y2 to width/x_binning and height/y_binning


void get_roi(PicamHandle camera, const PicamRois *region, piint *x, piint *y, piint *width, piint *height, piint *x_binning, piint *y_binning)
{
	PicamError err;		 
	err = Picam_GetParameterRoisValue(camera, 
									PicamParameter_Rois, 
									&region);
	if (err == PicamError_None)
	{
		// for (piint i = 0; i < region->roi_count; ++i)
		// {
		const PicamRoi roi = region->roi_array[0];
		*x = roi.x;
		*y = roi.y;
		*width = roi.width;
		*height = roi.height;
		*x_binning = roi.x_binning;
		*y_binning = roi.y_binning;
		// }

	}
	else
    {
        std::cout << "Failed to get region." << std::endl;
    }
}

// void set_roi(PicamHandle camera, const PicamRois region, piint x, piint y, piint width, piint height, piint x_binning, piint y_binning) 
// {
// 	get_roi(camera, &region, &x, &y, &width, &height, &x_binning, &y_binning);
// 	PicamError err = Picam_SetParameterRoisValue(camera, PicamParameter_Rois, &region);

//     if (err == PicamError_None)
//     {
//         // Print each ROI in the array
//         // for (piint i = 0; i < region.roi_count; ++i)
//         // {
// 		const PicamRoi roi = region->roi_array[0];

// 		// const PicamRoi roi = region.roi_array[0];

// 		// std::cout << "ROI " << i << ": "
// 		// 			<< "x: " << roi.x << ", y: " << roi.y
// 		// 			<< ", width: " << roi.width << ", height: " << roi.height
// 		// 			<< ", x_binning: " << roi.x_binning
// 		// 			<< ", y_binning: " << roi.y_binning << std::endl;
//         // }
//     }

//     // Clean up allocated memory
// }

    // region.roi_count = 1; // Set the number of ROIs you want to define
    // region.roi_array = new PicamRoi[region.roi_count]; // Allocate memory for the ROI array

    // // Initialize the first (and only) ROI
    // region.roi_array[0].x = x;
    // region.roi_array[0].y = y;
    // region.roi_array[0].width = width;
    // region.roi_array[0].height = height;
    // region.roi_array[0].x_binning = x_binning;
    // region.roi_array[0].y_binning = y_binning;

// void set_roi(PicamHandle camera, piint x, piint y, piint width, piint height, piint x_binning, piint y_binning) 
// {
//     PicamError err;
// 	const PicamRois *region;
// 	err = Picam_SetParameterRoisValue(	camera, 
// 									PicamParameter_Rois, 
// 									region);
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

// 	// 	for (piint i = 0; i < region->roi_count; ++i)
// 	// 	{
//     // roi.x = x;
//     // roi.y = y;
//     // roi.width = width;
//     // roi.height = height;
//     // roi.x_binning = x_binning;
//     // roi.y_binning = y_binning;

//     // // Set the ROI
//     // err = Picam_SetParameterRoisValue(camera, PicamParameter_Rois, &roi);
//     // if (err != PicamError_None) {
//     //     std::cout << "ROI set successfully: "
//     //               << "x: " << roi.x << ", y: " << roi.y
//     //               << ", width: " << roi.width << ", height: " << roi.height
//     //               << ", x_binning: " << roi.x_binning
//     //               << ", y_binning: " << roi.y_binning << std::endl;
//     // }
// }

void DoSingleROI(PicamHandle camera)
{
	PicamError					err;			 /* Error Code			*/
	PicamAvailableData			dataFrame;		 /* Data Struct			*/
	PicamAcquisitionErrorsMask	acqErrors;		 /* Errors				*/
	const PicamRois				*region;		 /* Region of interest  */
	const PicamParameter		*paramsFailed;	 /* Failed to commit    */
	piint						failCount;		 /* Count of failed	    */
	const PicamRoisConstraint  *constraint;		 /* Constraints			*/
	
	/* Variables to compute central region in image */
	piint height, width, x1, y1, x_binning, y_binning;

	piint halfHeight, halfWidth, totalWidth, totalHeight;
	piint height_increment, width_increment, height_min, width_min;
	// piflt height_outlier, width_outlier, height_excluded, width_excluded;

	/* Get dimensional constraints */
	err = Picam_GetParameterRoisConstraint(	camera, 
											PicamParameter_Rois, 
											PicamConstraintCategory_Required, 
											&constraint);	
	/* Error check */
	if (err == PicamError_None)
	{		
		/* Get width and height from constraints */
		totalWidth = (piint)constraint->width_constraint.maximum; //need this
		totalHeight= (piint)constraint->height_constraint.maximum; //need this
		std::cout << totalHeight << std::endl;
		std::cout << totalWidth << std::endl;


		// if (constraint->height_constraint.outlying_values_count > 0) {
		// 	height_outlier = constraint->height_constraint.outlying_values_array[0];
		// 	std::cout << height_outlier << std::endl;

		// }
		// if (constraint->width_constraint.outlying_values_count > 0) {
		// 	width_outlier = constraint->width_constraint.outlying_values_array[0];
		// 	std::cout << width_outlier << std::endl;
		// }

		// if (constraint->height_constraint.excluded_values_count > 0) {
		// 	height_excluded = constraint->height_constraint.excluded_values_array[0];
		// 	std::cout << height_excluded << std::endl;
		// }
		// if (constraint->width_constraint.excluded_values_count > 0) {
		// 	width_excluded = constraint->width_constraint.excluded_values_array[0];
		// 	std::cout << width_excluded << std::endl;
		// }
		height_increment = (piint)constraint->height_constraint.increment;
		width_increment = (piint)constraint->width_constraint.increment;

		height_min = (piint)constraint->height_constraint.minimum;
		width_min = (piint)constraint->width_constraint.minimum;

		std::cout << height_increment << std::endl;
		std::cout << width_increment << std::endl;

		std::cout << height_min << std::endl;
		std::cout << width_min << std::endl;
											

		/* Clean up constraints after using constraints */
		Picam_DestroyRoisConstraints(constraint);

		halfWidth	= totalWidth  / 2;
		halfHeight	= totalHeight / 2;	

		/* Get the orinal ROI */
		err = Picam_GetParameterRoisValue(	camera, 
											PicamParameter_Rois, 
											&region);
		if (err == PicamError_None && region->roi_count > 0)
		{
			// Print each ROI in the array
			for (piint i = 0; i < region->roi_count; ++i)
			{
				const PicamRoi roi = region->roi_array[i];
				x1 = roi.x;
				y1 = roi.y;
				height = roi.height;
				width = roi.width;
				x_binning = roi.x_binning;
				y_binning = roi.y_binning;

			}
		/* Error check */
	// 	if (err == PicamError_None)
	// 	{

	// 		/* Modify the region */
	// 		if (region->roi_count == 1) 
	// 		{
	// 			for (piint i = 0; i < region->roi_count; ++i)
    // {
	// 			std::cout << region.height << std::endl;
	// 			std::cout << region.weight << std::endl;
	// 			std::cout << region.x << std::endl;
	// 			std::cout << region.y << std::endl;
	// 			std::cout << region.y_binning << std::endl;
	// 			std::cout << region.x_binning << std::endl;

	// 			/* The absolute size of the ROI */
	// 			region->roi_array[0].height		= halfHeight;
	// 			region->roi_array[0].width		= halfWidth;

	// 			/* The offset into the chip of the ROI (1/4th) */
	// 			region->roi_array[0].x			= halfWidth  / 2;
	// 			region->roi_array[0].y			= halfHeight / 2;

	// 			/* The vertical and horizontal binning */
	// 			region->roi_array[0].x_binning	= 1;
	// 			region->roi_array[0].y_binning	= 1;

	// 		}
			/* Set the region of interest */
			err = Picam_SetParameterRoisValue(	camera, 
												PicamParameter_Rois, 
												region);
			/* Error check */
			if (err == PicamError_None)
			{
				/* Commit ROI to hardware */
				err = Picam_CommitParameters(	camera, 
												&paramsFailed, 
												&failCount);
                Picam_DestroyParameters(paramsFailed);

				/* Error check */
				if (err == PicamError_None)
				{
					/* Acquire 1 frame of data with a timeout */
					if (Picam_Acquire(camera, 1, ACQUIRE_TIMEOUT, &dataFrame, &acqErrors) == PicamError_None) 
					{
						/* Get the bit depth */
						piint depth;
						Picam_GetParameterIntegerValue(	camera, 
														PicamParameter_PixelBitDepth,  
														&depth);
						/* Compute the average over the region */
						double dAverage = ComputeAverage(	dataFrame.initial_readout, 
															depth, 
															halfWidth, 
															halfHeight,
															0);						
						/* Print Average */
						printf("Single: Average for ROI => %.2f \n", (double) dAverage);	
					}
				}				
			}	
			/* Free the regions */
			std::cout << "  Height: " << height << std::endl;
			std::cout << "  Width: " << width << std::endl;
			std::cout << "  X Offset: " << x1 << std::endl;
			std::cout << "  Y Offset: " << y1 << std::endl;
			std::cout << "  X Binning: " << x_binning << std::endl;
			std::cout << "  Y Binning: " << y_binning << std::endl;

			Picam_DestroyRois(region);
		}
	} 	
}

/******************************************************************************
*	Teledyne Princeton Instruments Region of Interest (ROI) example.
*
*	Sets up the camera and acquires from four regions of interest.
*
*		------------------------	This example code will generate four ROIs
*		--1111---------111------	with various sizes and binning.
*		--1111---------111------	
*		---------------111------
*		------------------------
*		--222222--------2222----
*		--222222--------2222----
*		------------------------
*
******************************************************************************/
void DoMultipleROIs(PicamHandle camera)
{
	PicamError					err;			 /* Error Code			*/
	PicamAvailableData			dataFrame;		 /* Data Struct			*/
	PicamAcquisitionErrorsMask	acqErrors;		 /* Errors				*/	
	const PicamParameter		*paramsFailed;	 /* Failed to commit    */
	piint						failCount;		 /* Count of failed	    */

	PicamRois					region;			 /* Region of interest  */
	PicamRoi					sampleRegions[SAMPLE_REGIONS];

	const PicamRoisConstraint  *constraint;		 /* Constraints			*/

	/* Simple structure used to compute N regions	*/
	/* based on overall imager percentages			*/
	struct RegionPercents 
	{
		piflt percentX_start;	/* Offset X position as a percent */
		piflt percentY_start;	/* Offset Y position as a percent */
		piflt percentX_end;		/* Ending X position as a percent */
		piflt percentY_end;		/* Ending Y position as a percent */
		piint xBin;				/* X Binning of this region		  */
		piint yBin;				/* Y Binning of this region		  */
	};
	
	/* 4 regions defined as percentages of imaging area */
	RegionPercents computed[] = {{ 0.10, 0.10, 0.25, 0.25, 1, 1 }, 
								 { 0.65, 0.05, 0.84, 0.35, 1, 1 }, 
								 { 0.15, 0.50, 0.37, 0.90, 1, 2 }, 
								 { 0.55, 0.50, 0.95, 0.90, 1, 2 }};

	/* setup the region object count and pointer */	
	region.roi_count = SAMPLE_REGIONS;
	region.roi_array = sampleRegions;

	/* Variables to compute central region in image */
	piint totalWidth, totalHeight;

	/* If we require symmetrical regions return since we are not setup */
	/* for that with our regions */
	err = Picam_GetParameterRoisConstraint(	camera, 
											PicamParameter_Rois, 
											PicamConstraintCategory_Required, 
											&constraint);	

	/* Error check for width and height */
	if (err == PicamError_None)
	{						
		/* If we require symmetrical regions return since we are not setup */
		/* for that with our regions, they are asymmetrical */
		if ((constraint->rules & PicamRoisConstraintRulesMask_HorizontalSymmetry) ||
			(constraint->rules & PicamRoisConstraintRulesMask_VerticalSymmetry))
		{
			/* cleanup and return */
			Picam_DestroyRoisConstraints(constraint);
			return;
		}

		/* Get width and height from constraints */
		totalWidth  = (piint)constraint->width_constraint.maximum;
		totalHeight = (piint)constraint->height_constraint.maximum;

		/* Clean up constraints after using them */
		Picam_DestroyRoisConstraints(constraint);

		/* Compute the 4 regions based on percentages of the sensor size */
		for (int i=0; i<SAMPLE_REGIONS; i++) {

			/* Sizes (convert from percents to pixels) */
			sampleRegions[i].height = (piint)((computed[i].percentY_end - computed[i].percentY_start) * (piflt)totalHeight);
			sampleRegions[i].width  = (piint)((computed[i].percentX_end - computed[i].percentX_start) * (piflt)totalWidth);

			/* Offsets (convert from percents to pixels) */
			sampleRegions[i].y = (piint)(computed[i].percentY_start * (piflt)totalHeight);
			sampleRegions[i].x = (piint)(computed[i].percentX_start * (piflt)totalWidth);

			/* Binning */
			sampleRegions[i].y_binning = computed[i].yBin;
			sampleRegions[i].x_binning = computed[i].xBin;
		}

		/* Set the region of interest */
		err = Picam_SetParameterRoisValue(camera, PicamParameter_Rois, &region);

		/* Error check */
		if (err == PicamError_None)
		{
			/* Commit ROI to hardware */
			err = Picam_CommitParameters(camera, &paramsFailed, &failCount);
            Picam_DestroyParameters(paramsFailed);

			/* Error check */
			if (err == PicamError_None)
			{
				/* Get the bit depth */
				piint depth;
				Picam_GetParameterIntegerValue(	camera, 
												PicamParameter_PixelBitDepth,  
												&depth);
				/* Acquire 1 frame of data with a timeout */
				if (Picam_Acquire(camera, 1, ACQUIRE_TIMEOUT, &dataFrame, &acqErrors) == PicamError_None)
				{
					piint offset_pixels = 0;
					for (int i=0; i<SAMPLE_REGIONS; i++)
					{
						/* Compute the average over the region */
						double dAverage = ComputeAverage(	dataFrame.initial_readout, 
															depth, 
															sampleRegions[i].width / sampleRegions[i].x_binning, 
															sampleRegions[i].height/ sampleRegions[i].y_binning,
															offset_pixels);

						/* move the offset into the buffer to look at for next roi */
						offset_pixels += (sampleRegions[i].width / sampleRegions[i].x_binning) * (sampleRegions[i].height / sampleRegions[i].y_binning);

						/* Print Average */
						printf("Multiple: Average for ROI %i => %.2f \n",(int) i+1, (double)dAverage);
					}							
				}
			}			
		}					
	} 		
}

/******************************************************************************
*	Teledyne Princeton Instruments Region of Interest (ROI) example.
*	
*	DisplayError helper routine, prints out the message for the associated 
*	error code.
*
******************************************************************************/
void DisplayError(PicamError errCode)
{
	/* Get the error string, the called function allocates the memory */
	const pichar *errString;
	Picam_GetEnumerationString(	PicamEnumeratedType_Error, 
								(piint)errCode, 
								&errString);
	/* Print the error */
	printf("%s\n", errString);

	/* Since the called function allocated the memory it must free it */
	Picam_DestroyString(errString);
}

/******************************************************************************
*
*	Simple helper function to make an average computation from within a buffer
*	Note: The pixelOffset should be the sum of pixels in the regions you are
*	skipping over.
*
******************************************************************************/
piflt ComputeAverage(void *buffer, 
					 piint depth, 
					 piint x, 
					 piint y,
					 piint pixelOffset)
{	
	piint totalPixels = x*y;
	piflt average	  = 0.0;

	switch (depth)
	{				
		case 16: 
			{			
				/* Sum Pixels */
				unsigned short *pUShort = (unsigned short *)buffer + pixelOffset;
				for (int i=0; i<x*y; i++)				
					average += *pUShort++;

				/* Divide by N */
				average /= totalPixels;
			}
			break;
	}
	return average;
}
