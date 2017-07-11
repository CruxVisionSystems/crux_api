#ifndef __CRUX_STITCH_H__
#define __CRUX_STITCH_H__

#include "crux.h"
#include "crux_parser.h"
#include <stdint.h>

int CruxStitchInit(struct CruxImage* image, uint32_t width, uint32_t height, uint8_t type);
int CruxStitchSetMetaData(struct CruxImage* image, uint32_t width, uint32_t height, uint8_t type);
int CruxStitchDeInit(struct CruxImage* image);
int CruxStitchImage(struct CruxImage* image, struct CruxPacket* packet);

#endif
