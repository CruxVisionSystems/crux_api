#include "crux.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    // Unused
    (void)argc;
    (void)argv;

    // Connect & Start crux camera
    int result = Crux_ConnectCamera();

    if (result >= CRUX_GEN_ERROR)
    {
    	printf("Could not connect to a camera.  Exiting.\n");
    	return 0;
    }

    while (1==1)
    {
    	// Block until the next frame is found.
    	struct CruxImage* img = Crux_ReadFrame(0);

    	printf("Received frame!\n");

    	// Print data
    	/*int i;
    	for (i = 0; i < img->width*img->height; i++)
			printf("%02x ", img->data[i]);

    	printf("\n");*/
    }
    return 0;
}
