#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

#include "bmpf.h"

int bmp_validate(FILE *bmpFile);
int bmp_get_version(int headerSize);
float bmp_get_padding(int width, int bitrate);
BMPINFO bmp_get_info(FILE *bmpFile);
void bmp_to_array(BITMAP bmp, unsigned char array[]);
void bmp_from_array(BITMAP bmp, unsigned char array[], RGBQUAD palette[]);