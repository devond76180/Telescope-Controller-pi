#include <stdio.h>
#include <unistd.h>    //write
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <wiringPi.h>
#include <math.h>

// phys pin 11
#define ALT_DIR_PIN 11
// phys pin 13
#define ALT_PULSE_PIN 13
// phys pin 15
#define ALT_ENABLE_PIN 15

// phys pin 16
#define ALT_MAX_PIN 16
// phys pin 18
#define ALT_MIN_PIN 18

// phys pin 36
#define AZ_DIR_PIN 36
// phys pin 38
#define AZ_PULSE_PIN 38
// phys pin 40
#define AZ_ENABLE_PIN 40

// phys pin 22
#define SHIFTER_ENABLE_PIN 22

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
/* Standard boolean definition */
typedef enum
{ False, True }
Boolean;

void setAlign(Boolean value);
Boolean isAligned();
void setTracking(Boolean value);
void initMotors();

long double azStepsPerDegree;
long double azStepsPerArcminute;
long double azStepsPerArcSecond;
long double azDegreesPerMicroStep;

long double altStepsPerDegree;
long double altStepsPerArcminute;
long double altStepsPerArcSecond;
long double altDegreesPerMicroStep;

long secondsEpoch1_1_2000;


