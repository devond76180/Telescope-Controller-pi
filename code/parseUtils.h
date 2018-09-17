#include "serverTypes.h"


DateType parseDate(unsigned char *commandBuffer, int size);

HMSType parseHMS(unsigned char *commandBuffer,int size); 

DMSType parseDMS(unsigned char *commandBuffer,int size);

LatLongType parseLatLongFile(unsigned char *commandBuffer,int size);

LatLongType parseLatLong(unsigned char *commandBuffer,int size);

float parseUtcOffset(unsigned char *commandBuffer,int size); 

