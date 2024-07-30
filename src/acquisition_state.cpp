#include <iostream>
#include <vector>
#include <ctime>  // For time functions

#include "picam_advanced.h"

// #defines
#define BUFFER_DEPTH 4
#define TIMEOUT -1  // infinite

// Prototypes for callback functions
PicamError PIL_CALL ReadoutStatusCB(PicamHandle, PicamAcquisitionState,
                                    const PicamAcquisitionStateCounters *,
                                    PicamAcquisitionStateErrorsMask errors);

// Prototypes for helper functions
void PrintEnumString(PicamEnumeratedType, piint);
void PrintError(PicamError);

// Global variables to cache AcquisitionState errors
pibln bErrorOccurred_ReadoutStarted = false;
pibln bErrorOccurred_ReadoutEnded = false;
pi64s ReadoutsStartedCount = 0;
pi64s ReadoutsEndedCount = 0;

void PrintError(PicamError error) {
    const pichar *errorString;
    Picam_GetEnumerationString(PicamEnumeratedType_Error, error, &errorString);
    std::cerr << "PicamError: " << errorString << std::endl;
    Picam_DestroyString(errorString);
}

void PrintEnumString(PicamEnumeratedType type, piint value) {
    const pichar *string;
    Picam_GetEnumerationString(type, value, &string);
    std::cout << string;
    Picam_DestroyString(string);
}

pibool InitializeAndOpen(PicamHandle *hDevice) {
    piint id_cnt = 0;
    const PicamCameraID *id = 0;
    PicamError e = PicamError_None;

    if (Picam_InitializeLibrary() != PicamError_None) {
        std::cout << "Cannot Initialize Picam Library" << std::endl;
        return false;
    }
    if ((e = Picam_GetAvailableCameraIDs(&id, &id_cnt)) != PicamError_None) {
        PrintError(e);
        Picam_UninitializeLibrary();
        return false;
    }
    if (!id_cnt) {
        std::cout << "No Cameras Available" << std::endl;
        Picam_DestroyCameraIDs(id);
        Picam_UninitializeLibrary();
        return false;
    }
    if ((e = PicamAdvanced_OpenCameraDevice(id, hDevice)) != PicamError_None) {
        PrintError(e);
        Picam_DestroyCameraIDs(id);
        Picam_UninitializeLibrary();
        return false;
    }

    Picam_DestroyCameraIDs(id);

    return true;
}

pibool ConfigureExperiment(PicamHandle hDevice, std::vector<pibyte> &dataBuf, pi64s exposureTimeMicroseconds) {
    PicamError e;
    PicamHandle hModel;
    PicamAcquisitionBuffer acqBuf;
    piint bufferDepth = BUFFER_DEPTH;
    piint frameSize = 0;
    pibln committed;
    const PicamParameter *failed_parameters;
    piint failed_parameters_count;
    const PicamCollectionConstraint *constraint;
    piflt adcSpeed = 0.;

    if ((e = Picam_GetParameterIntegerValue(hDevice, PicamParameter_ReadoutStride, &frameSize)) != PicamError_None) {
        PrintError(e);
        return false;
    }
    dataBuf.resize(frameSize * bufferDepth);
    PicamAdvanced_GetCameraModel(hDevice, &hModel);

    // Determine slowest ADC speed for camera
    // Set ADC Quality to Low Noise
    if ((e = Picam_SetParameterIntegerValue(hModel, PicamParameter_AdcQuality, PicamAdcQuality_LowNoise)) != PicamError_None)
        PrintError(e);
    // Query ADC Speed capabilities
    if ((e = Picam_GetParameterCollectionConstraint(hModel, PicamParameter_AdcSpeed, PicamConstraintCategory_Capable, &constraint)) != PicamError_None)
        PrintError(e);
    else {
        adcSpeed = constraint->values_array[0];
        // Choose slowest speed
        for (piint i = 1; i < constraint->values_count; i++) {
            if (constraint->values_array[i] < constraint->values_array[i - 1])
                adcSpeed = constraint->values_array[i];
        }
    }
    Picam_DestroyCollectionConstraints(constraint);

    std::cout << "Setting Adc Speed to " << adcSpeed << " MHz." << std::endl;
    // Set ADC Speed
    if ((e = Picam_SetParameterFloatingPointValue(hModel, PicamParameter_AdcSpeed, adcSpeed)) != PicamError_None)
        PrintError(e);  // Adc speed not changed, but continuing anyway

    // Set exposure time
    if ((e = Picam_SetParameterLargeIntegerValue(hModel, PicamParameter_ExposureTime, exposureTimeMicroseconds)) != PicamError_None)
        PrintError(e);

    // Commit changes to hardware
    if ((e = Picam_AreParametersCommitted(hModel, &committed)) != PicamError_None)
        PrintError(e);
    if ((e = Picam_CommitParameters(hModel, &failed_parameters, &failed_parameters_count)) != PicamError_None)
        PrintError(e);
    // - print any invalid parameters
    if (failed_parameters_count > 0) {
        std::cout << "The following parameters are invalid:" << std::endl;
        for (piint i = 0; i < failed_parameters_count; ++i) {
            std::cout << "    ";
            PrintEnumString(PicamEnumeratedType_Parameter, failed_parameters[i]);
            std::cout << std::endl;
        }
    }
    // - free picam-allocated resources
    Picam_DestroyParameters(failed_parameters);

    acqBuf.memory = &dataBuf[0];
    acqBuf.memory_size = frameSize * bufferDepth;

    if ((e = PicamAdvanced_SetAcquisitionBuffer(hDevice, &acqBuf)) != PicamError_None) {
        PrintError(e);
        return false;
    }

    return true;
}

void Acquire(PicamHandle d, pi64s exposureTimeMicroseconds) {
    Picam_StartAcquisition(d);

    PicamAcquisitionStatus status;
    PicamAvailableData available;
    PicamError err;
    piint dVal = 0;
    pichar D[] = "|/-\\|/-\\";

    std::cout << "Acquiring for " << exposureTimeMicroseconds << " microseconds" << std::endl << std::endl << std::endl;

    clock_t start_time = clock();
    clock_t end_time = start_time + (exposureTimeMicroseconds / 1000000.0) * CLOCKS_PER_SEC;

    do {
        err = Picam_WaitForAcquisitionUpdate(d, TIMEOUT, &available, &status);
        if (status.running) {
            dVal = (dVal + available.readout_count) % 8;
            std::cout << D[dVal] << '\r' << std::flush;
        }

        if (clock() >= end_time) {
            Picam_StopAcquisition(d);
            break;
        }
    } while (status.running || err == PicamError_TimeOutOccurred);
}

void EnableReadoutStatusCallbacks(PicamHandle hDevice, pibln &bStarted, pibln &bEnded) {
    pibln detectable;

    PicamAdvanced_CanRegisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutStarted, &detectable);
    if (!detectable)
        std::cout << "Camera doesn't support AcquisitionState_ReadoutStarted" << std::endl;
    else {
        if (PicamAdvanced_RegisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutStarted, ReadoutStatusCB) == PicamError_None)
            bStarted = true;
    }
    PicamAdvanced_CanRegisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutEnded, &detectable);
    if (!detectable)
        std::cout << "Camera doesn't support AcquisitionState_ReadoutEnded" << std::endl;
    else {
        if (PicamAdvanced_RegisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutEnded, ReadoutStatusCB) == PicamError_None)
            bEnded = true;
    }
}

void DisableReadoutStatusCallbacks(PicamHandle hDevice, pibln started, pibln ended) {
    if (started)
        PicamAdvanced_UnregisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutStarted,
                                                           (PicamAcquisitionStateUpdatedCallback)ReadoutStatusCB);
    if (ended)
        PicamAdvanced_UnregisterForAcquisitionStateUpdated(hDevice, PicamAcquisitionState_ReadoutEnded,
                                                           (PicamAcquisitionStateUpdatedCallback)ReadoutStatusCB);
}

PicamError PIL_CALL ReadoutStatusCB(PicamHandle, PicamAcquisitionState state, const PicamAcquisitionStateCounters *, PicamAcquisitionStateErrorsMask errors) {
    if (state == PicamAcquisitionState_ReadoutStarted) {
        ReadoutsStartedCount++;
        if (errors)
            bErrorOccurred_ReadoutStarted = true;
    } else if (state == PicamAcquisitionState_ReadoutEnded) {
        ReadoutsEndedCount++;
        if (errors)
            bErrorOccurred_ReadoutEnded = true;
    }
    return PicamError_None;
}

int main() {
    std::vector<pibyte> userBuffer;
    PicamHandle hDevice;
    pibln readoutStartedEnabled = false;
    pibln readoutDoneEnabled = false;
    pi64s exposureTimeMicroseconds = 10000000;

    if (!InitializeAndOpen(&hDevice))
        return -1;

    if (!ConfigureExperiment(hDevice, userBuffer, exposureTimeMicroseconds))
        return -1;

    EnableReadoutStatusCallbacks(hDevice, readoutStartedEnabled, readoutDoneEnabled);
    Acquire(hDevice, exposureTimeMicroseconds);
    DisableReadoutStatusCallbacks(hDevice, readoutStartedEnabled, readoutDoneEnabled);

    Picam_CloseCamera(hDevice);
    Picam_UninitializeLibrary();
    return 0;
}
