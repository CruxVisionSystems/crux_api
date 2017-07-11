#ifndef __CRUX_H__
#define __CRUX_H__

#include "ftdi_lib/ftd2xx.h"

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

    // Which camera source this came from.
    uint8_t id; 

    // Type of image saved in data
    // 0 = 8-bit grayscale
    uint8_t type;

    // Number of rows/columns in the image.
    uint16_t width;
    uint16_t height;

    // used for stitching together an image
    uint32_t _pixelCounter;
};

// Attempt to connect to a camera
// Returns CRUX_SUCCESS if a camera was succesfully connected.
int Crux_ConnectCamera();

// Blocking function that returns when a new frame is sent to a specific id.
struct CruxImage* Crux_ReadFrame(int id);

#endif
