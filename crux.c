#include "crux.h"
#include "crux_parser.h"
#include "crux_stitch.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define CRUX_RECEIVE_BUFFER_SIZE 68*20
#define CRUX_MAX_NUM_CAMERAS 4

static pthread_t mainthread;
static pthread_mutex_t bufferLock;
static FT_HANDLE camHandles[CRUX_MAX_NUM_CAMERAS];

// Triple buffers used for swapping.
static struct CruxImage savedImages[CRUX_MAX_NUM_CAMERAS];
static struct CruxImage savedImages2[CRUX_MAX_NUM_CAMERAS];
static struct CruxImage savedImages3[CRUX_MAX_NUM_CAMERAS];
static volatile int savedImagesCounter[CRUX_MAX_NUM_CAMERAS];

void _cruxUpdateImageBuffer(int id, struct CruxImage* image)
{
	// Swap buffers to allow a new buffer to be written.
	pthread_mutex_lock(&bufferLock);
	uint8_t* tmp = savedImages2[id].data;
	savedImages2[id].data = image[id].data;
	image[id].data = tmp;
    savedImagesCounter[id]++;
    pthread_mutex_unlock(&bufferLock);
}

struct CruxImage* Crux_ReadFrame(int id)
{
	if (id >= CRUX_MAX_NUM_CAMERAS) return 0;

	static volatile int previousImageCounts[CRUX_MAX_NUM_CAMERAS];

	// Wait until a new image is ready.
	while (previousImageCounts[id] >= savedImagesCounter[id]);

	// Swap buffers so that any new data doesn't corrupt our data unexpectedly.
	pthread_mutex_lock(&bufferLock);
	uint8_t* tmp = savedImages3[id].data;
	savedImages3[id].data = savedImages2[id].data;
	savedImages2[id].data = tmp;
	pthread_mutex_unlock(&bufferLock);

	previousImageCounts[id] = savedImagesCounter[id];

	return &savedImages3[id];
}

// Main thread that reads in data from USB and converts it to images
// TODO: Some effort has been put in to allow multiple cameras
//       to be used at once in a single thread, but it's not entirely
//       implemented yet.
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
    struct CruxParser parser[CRUX_MAX_NUM_CAMERAS];

    int imgWidth = 120;
    int imgHeight = 64;
    int imgType = CRUX_8B_GRAY;

    // Initialize memory on buffers
    pthread_mutex_lock(&bufferLock);
    int result = 0;
    result |= CruxParserInit(&parser[0]);
    result |= CruxStitchInit(&savedImages[0], imgWidth, imgHeight, imgType);
    result |= CruxStitchInit(&savedImages2[0], imgWidth, imgHeight, imgType);
    result |= CruxStitchInit(&savedImages3[0], imgWidth, imgHeight, imgType);
    savedImagesCounter[0] = 0;
    pthread_mutex_unlock(&bufferLock);

    // Defensive check, in case something went wrong.
    if (result >= CRUX_GEN_ERROR) return (void*)CRUX_GEN_ERROR;

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
			int result = CruxParseChar(&parser[0], (uint8_t)RxBuffer[i]);

			// if no packet was found, continue to next byte received
			if (result != CRUX_PACKET_FOUND) continue;

			result = CruxStitchImage(&savedImages[0], &parser[0].packet);

			if (result != CRUX_IMAGE_FOUND) continue;

			_cruxUpdateImageBuffer(0, &savedImages[0]);

			CruxStitchSetMetaData(&savedImages[0], imgWidth, imgHeight, imgType);
		}
	}

	CruxStitchDeInit(&savedImages[0]);
}

// TODO: Allow user to select specific FTDI port.
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
