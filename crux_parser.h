#ifndef __CRUX_PARSER_H__
#define __CRUX_PARSER_H__

#include <stdint.h>
#include "crux.h"
#include <stdio.h>

#define CRUX_BUFFER_SIZE 64

#define CRUX_STATE_PREAMBLE 0
#define CRUX_STATE_PREAMBLE2 1
#define CRUX_STATE_FUNC 2
#define CRUX_STATE_DATA 3
#define CRUX_STATE_CRC 4

// A completed packet
struct CruxPacket
{
    uint8_t function;
    uint8_t data[CRUX_BUFFER_SIZE];
    uint8_t checksum;
    uint32_t size;

    // 1 if bad checksum
    // 0 if data is valid
    uint8_t badchecksum;
};

struct CruxParser
{
    uint8_t state;

    uint8_t _sum;
    uint16_t _data_counter;

    struct CruxPacket packet;
};

int CruxParserInit(struct CruxParser* parse);

int CruxParseChar(struct CruxParser* parse, uint8_t c);

int CruxParseCompose(uint8_t func, uint8_t* data, uint8_t* output);

#endif
