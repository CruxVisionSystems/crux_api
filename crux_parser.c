#include "crux_parser.h"
#include <string.h>

int CruxParserInit(struct CruxParser* parse)
{
    if (parse == 0) return CRUX_GEN_ERROR;

    parse->state = CRUX_STATE_PREAMBLE;
    memset(parse->packet.data, 0, CRUX_BUFFER_SIZE);

    return CRUX_SUCCESS;
}

int CruxParseChar(struct CruxParser* parse, uint8_t c)
{
	//printf("%02X ", c);

    if (parse == 0) return CRUX_GEN_ERROR;
    switch (parse->state)
    {
        case CRUX_STATE_PREAMBLE:
            if (c == 0xFA) parse->state = CRUX_STATE_PREAMBLE2;
            break;

        case CRUX_STATE_PREAMBLE2:
            if (c == 0xDE) parse->state = CRUX_STATE_FUNC;
            else parse->state = CRUX_STATE_PREAMBLE;
            break;

        case CRUX_STATE_FUNC:
            parse->packet.function = c;
            parse->_sum = c;
            parse->state = CRUX_STATE_DATA;
            parse->_data_counter = 0;
            break;

        case CRUX_STATE_DATA:
            parse->packet.data[parse->_data_counter] = c;
            parse->_sum += c;
            parse->_data_counter++;

            //printf("%02x ", c);

            if (parse->_data_counter > CRUX_BUFFER_SIZE-1)
                parse->state = CRUX_STATE_CRC;
                
            break;

        case CRUX_STATE_CRC: 
        	printf("\n");
            parse->state = CRUX_STATE_PREAMBLE;
            parse->packet.checksum = parse->_sum;
            parse->packet.size = CRUX_BUFFER_SIZE;
            if (parse->_sum != c) parse->packet.badchecksum = 1;

            return CRUX_PACKET_FOUND;
    }

    return CRUX_SUCCESS;
}

int CruxParseCompose(uint8_t func, uint8_t* data, uint8_t* output)
{
    if (data == 0) return CRUX_GEN_ERROR; 
    if (output == 0) return CRUX_GEN_ERROR;
    
    uint8_t sum;

    // Preamble 
    output[0] = 0xFA;
    output[1] = 0xDE;

    // Function
    output[2] = func;
    sum = func;

    // Data
    int i;
    for (i = 0; i < CRUX_BUFFER_SIZE; i++)
    {
        output[i+3] = data[i];
        sum += data[i];
    }
    
    // Checksum
    output[3 + CRUX_BUFFER_SIZE] = sum;

    return CRUX_SUCCESS;
}
