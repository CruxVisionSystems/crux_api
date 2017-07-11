#ifndef __CRUX_H__
#define __CRUX_H__

#include "ftd2xx.h"

#include <stdint.h>

// Image types
#define CRUX_8B_GRAY 0

// Return codes.  All before CRUX_GEN_ERROR are general return information.
// All after are error codes.
// To test if there is an error, use this statement:
// `if (returnedValue >= CRUX_GEN_ERROR) { // handle error }`
#define CRUX_SUCCESS 0
#define CRUX_PACKET_FOUND 1
#define CRUX_IMAGE_FOUND 1
#define CRUX_GEN_ERROR 100
#define CRUX_OUT_OF_MEMORY 101

// Data structure used for passing image data.
struct CruxImage
{
    // Raw data from the camera
    uint8_t* data;    

    // Type of image saved in data
    // 0 = 8-bit grayscale
    uint8_t type;

    // Number of rows/columns in the image.
    uint16_t width;
    uint16_t height;

    // used for stitching together an image
    uint32_t _pixelCounter;
};

// Blocking function that attempts to connect to a camera
// Returns 0 if a camera was succesfully connected.
int Crux_ConnectCamera();

// Returns information about the camera plugged in 
// including resolution, framerate, and software version
//CruxInfo Crux_GetCameraInfo();

// Callback functions

// Called when a new frame is saved.
void* Crux_NewFrameCallback(struct CruxImage* image);

// Called when a camera status changes 
// Ex. when a camera is unplugged.
//void* Crux_CameraStatus(void* arg);

#endif
