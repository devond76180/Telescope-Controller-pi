#ifndef SERVER_TYPES
#define SERVER_TYPES
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define byte uint8_t

enum gamepadType
{
FX,
SNES
};


enum gamepadType typeGamepad;

/* Standard boolean definition */
typedef enum
{ False, True }
Boolean;


/* Buffer size */
#define BUFFER_SIZE 2048

typedef struct 
{
   int day;
   int month;
   int year;
}
DateType;

typedef struct 
{
   int hours;
   int minutes;
   float seconds;
}
HMSType;

typedef struct 
{
   int degrees;
   int minutes;
   float seconds;
}
DMSType;


/* Buffer structure */
typedef struct
{
    unsigned char buffer[BUFFER_SIZE];
    unsigned int endPos;
}
BufferType;


typedef struct
{
   int degrees;
   int minutes;
}
LatLongType;

long secondsEpoch1_1_2000;

DMSType altitude;
DMSType declination;
DMSType declinationNew;
HMSType ra;
HMSType raNew;
float raHours;
float raDegrees;
float declinationDegrees;
DMSType azimuth;

LatLongType latitude;
LatLongType longitude;
float longitudeDegrees;
float longitudeHours;
float latitudeDegrees;
float utcOffset;

long double azStepsPerDegree;
long double azStepsPerArcminute;
long double azStepsPerArcSecond;
long double azDegreesPerMicroStep;

long double altStepsPerDegree;
long double altStepsPerArcminute;
long double altStepsPerArcSecond;
long double altDegreesPerMicroStep;

long altOffset;
long azOffset;

int invertAltPulse;
int invertAzPulse;
int invertAltDirection;
int invertAzDirection;
int invertAltEnable;
int invertAzEnable;

// alt az conversion constants
double C_PI180;
double C_180PI;
double C_LATCALC;
double sinLat;
double cosLat;

long azBigTooth;
long azSmallTooth;
long azStepsPerRev;
long azSteps360;

long altBigTooth;
long altSmallTooth;
long altStepsPerRev;

Boolean doneMoving;
Boolean moving;

Boolean azLeft;
Boolean azRight;
Boolean altUp;
Boolean altDown;
Boolean azSlowLeft;
Boolean azSlowRight;
Boolean altSlowUp;
Boolean altSlowDown;

char gamepadFile[50];
int gotoTimer;
int setupTimer;
int altJoyKeySteps;
int azJoyKeySteps;
int altButtonKeySteps;
int azButtonKeySteps;

Boolean addLeftSmall;
Boolean addRightSmall;
Boolean addUpSmall;
Boolean addDownSmall;
Boolean addLeftLarge;
Boolean addRightLarge;
Boolean addUpLarge;
Boolean addDownLarge;



int slowAltRatio;
int xAltRatio;
int yAltRatio;
int aAltRatio;
int bAltRatio;
int holdAltRatio;
int holdSlowAltRatio;
int userMoveAltRatio;
float correctionPercentAlt;

int slowAzRatio;
int xAzRatio;
int yAzRatio;
int aAzRatio;
int bAzRatio;
int holdAzRatio;
int holdSlowAzRatio;
int userMoveAzRatio;
float correctionPercentAz; 
FILE *debugLog;

void debugLogging(int level, Boolean toFile,char *fmt, ...);

void *gamepadReader();

typedef struct {
	double alt;
	double az;
} 
AltAzType;


#endif
