#include "crux_stitch.h"

#include <string.h>
#include <stdlib.h>

int CruxStitchInit(struct CruxImage* image, uint32_t width, uint32_t height, uint8_t type)
{
    if (image == 0) return CRUX_GEN_ERROR;

    // Attempt to allocate memory for our image.
    image->data = (uint8_t*)malloc(sizeof(uint8_t)*width*height);
    if (image->data == 0) return CRUX_OUT_OF_MEMORY;

    return CruxStitchSetMetaData(image, width, height, type);
}

int CruxStitchSetMetaData(struct CruxImage* image, uint32_t width, uint32_t height, uint8_t type)
{
    if (image == 0) return CRUX_GEN_ERROR;

    // Metadata
    image->type = type;
    image->width = width;
    image->height = height;
    image->_pixelCounter = 0;

    return CRUX_SUCCESS;
}

int CruxStitchDeInit(struct CruxImage* image)
{
    if (image == 0) return CRUX_GEN_ERROR;

    free(image->data);

    return CRUX_SUCCESS;
}

int CruxStitchImage(struct CruxImage* image, struct CruxPacket* packet)
{
    if (image == 0 || packet == 0) return CRUX_GEN_ERROR;

    // Determine how much memory to copy into the image array.
    // If the array doesn't have enough memory available left, crop
    // and forget the rest of the packet.
    int bufferSize = image->width * image->height - image->_pixelCounter;
    int packetSize = packet->size;
    int memcopyAmount = (bufferSize > packetSize) ? packetSize : bufferSize;

    /*int i;
    for (i = 0; i < memcopyAmount; i++)
        printf("%02x ", packet->data[i]);

    printf("\n");*/

    // Defensive check
    if (memcopyAmount < 0) return CRUX_GEN_ERROR;

    // Copy packet data to image data
    memcpy(&image->data[image->_pixelCounter], packet->data, memcopyAmount);
    image->_pixelCounter += memcopyAmount;

    // If the buffersize was less then the max packet size,
    // we must of filled the buffer completely.  Return that the image was
    // completed
    if (bufferSize < packetSize) return CRUX_IMAGE_FOUND;

    return CRUX_SUCCESS;
}
