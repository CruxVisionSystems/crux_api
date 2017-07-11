#include "crux.h"
#include "crux_parser.h"
#include "crux_stitch.h"

#include <pthread.h>
#include <stdio.h>

static pthread_t mainthread;
static FT_HANDLE camHandles[1];

int main(int argc, char* argv[])
{
   // Unused 
   (void)argc;
   (void)argv;

   Crux_ConnectCamera();

   pthread_join(mainthread, NULL);
   //Crux_Init();
   return 0;
}

void* Crux_NewFrameCallback(struct CruxImage* image)
{
	printf("Got frame\n");

	int i;
	for (i = 0; i < image->width* image->height; i++)
	{
		printf("%02x ", image->data[i]);
	}

	return 0;
}

#define CRUX_RECEIVE_BUFFER_SIZE 68*20

void* _cruxmain(void* ptr)
{
    (void)ptr;

    printf("_cruxmain thread start\n");
    FT_STATUS ftStatus;
    DWORD EventDWord;
    DWORD TxBytes;
    DWORD RxBytes;
    DWORD BytesReceived;
    char RxBuffer[CRUX_RECEIVE_BUFFER_SIZE];
    struct CruxParser parser;
    struct CruxImage image;

    int imgWidth = 120;
    int imgHeight = 64;
    int imgType = CRUX_8B_GRAY;

    CruxParserInit(&parser);
    CruxStitchInit(&image, imgWidth, imgHeight, imgType);

    FT_Purge(camHandles[0], FT_PURGE_RX | FT_PURGE_TX);

    while (1==1)
    {
        FT_GetStatus(camHandles[0], &RxBytes, &TxBytes, &EventDWord); 

        // Look for more bytes if none were sent since the last update.
        if (RxBytes <= 0) continue;

		DWORD amountToRead = (CRUX_RECEIVE_BUFFER_SIZE > RxBytes) ? RxBytes : CRUX_RECEIVE_BUFFER_SIZE;
		ftStatus = FT_Read(camHandles[0], RxBuffer, amountToRead, &BytesReceived);

		if (ftStatus != FT_OK)
		{
			printf("Critical FT_Read error (code %d)\n", ftStatus);
			return (void*)CRUX_GEN_ERROR;
		}

		uint32_t i;
		for (i = 0; i < BytesReceived; i++)
		{
			int result = CruxParseChar(&parser, (uint8_t)RxBuffer[i]);

			// if no packet was found, continue to next byte received
			if (result != CRUX_PACKET_FOUND) continue;

			result = CruxStitchImage(&image, &parser.packet);

			if (result != CRUX_IMAGE_FOUND) continue;

			Crux_NewFrameCallback(&image);

			CruxStitchSetMetaData(&image, imgWidth, imgHeight, imgType);
		}
	}

	CruxStitchDeInit(&image);
}

int Crux_ConnectCamera()
{
    FT_STATUS ftStatus;
    DWORD numDevices = 0;

    // Discover how many FTDI devices are connected
    ftStatus = FT_CreateDeviceInfoList(&numDevices);
    if (ftStatus != FT_OK)
    {
        printf("FT_CreateDeviceInfoList failed (error code %d)\n", ftStatus);
        return CRUX_GEN_ERROR;
    }

    if (numDevices == 0)
    {
        printf("No cameras detected\n");
        return CRUX_GEN_ERROR;
    }

    // Attempt to open camera
    ftStatus = FT_Open(0, &camHandles[0]);
    if (ftStatus != FT_OK)
    {
        printf("Could not open camera (error code %d).  Try sudo.\n", ftStatus);
        return CRUX_GEN_ERROR;
    }

    if (pthread_create(&mainthread, NULL, _cruxmain, NULL))
    {
        printf("Error creating thread\n");
        return CRUX_GEN_ERROR;
    }

    return CRUX_SUCCESS;
}
