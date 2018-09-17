#include "parseUtils.h"
#include <stdio.h>



DateType parseDate(unsigned char *commandBuffer, int size)
{
    DateType result;
    unsigned char temp[10];
     *(commandBuffer+size) = 0x00;
    sscanf(commandBuffer,"%c%c%c%d%c%d%c%d",&temp[0],&temp[1],&temp[2],
                          &result.month,&temp[3],
                          &result.day,&temp[4],
                          &result.year);
    return result; 
}


HMSType parseHMS(unsigned char *commandBuffer,int size) 
{
    HMSType result;
    unsigned char temp[10];
    sscanf(commandBuffer,"%c%c%c%d%c%d%c%f",&temp[0],&temp[1],&temp[2],
                            &(result.hours),&temp[3],
                            &(result.minutes),&temp[4],
                            &(result.seconds));
    return result;
}

DMSType parseDMS(unsigned char *commandBuffer,int size) 
{
    DMSType result;
    *(commandBuffer+size) = 0x00;
    unsigned char temp[10];
    sscanf(commandBuffer,"%c%c%c%d%c%d%c%f",&temp[0],&temp[1],&temp[2],
                          &(result.degrees),&temp[3],
                          &(result.minutes),&temp[4],
                          &(result.seconds));
   return result;
}


LatLongType parseLatLongFile(unsigned char *commandBuffer,int size) 
{
    LatLongType result;
    unsigned char temp[3];
    sscanf(commandBuffer,"%d%c%d",&(result.degrees),&temp[3],
                          &(result.minutes));

    return result;
}

LatLongType parseLatLong(unsigned char *commandBuffer,int size) 
{
    LatLongType result;
    unsigned char temp[3];
    *(commandBuffer+size) = 0x00;
    sscanf(commandBuffer,"%c%c%c%d%c%d",&temp[0],&temp[1],&temp[2],
                          &(result.degrees),&temp[3],
                          &(result.minutes));

    return result;
}

float parseUtcOffset(unsigned char *commandBuffer,int size) 
{
    float result = 0;
    unsigned char temp[6];
     *(commandBuffer+size) = 0x00;
    sscanf(commandBuffer,"%c%c%c%f",&temp[0],&temp[1],&temp[2],
                          &result);
   
    return result; 
}



