#include "serverTypes.h"
#include <stdio.h>
#include <unistd.h>    //write
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <wiringPi.h>
#include <stdlib.h>

// phys pin 36
#define ALT_DIR_PIN 36
// phys pin 38
#define ALT_PULSE_PIN 38
// phys pin 40
#define ALT_ENABLE_PIN 40

// phys pin 16
#define STOP_PIN 16
// phys pin 18
#define RELAY_PIN 32

// phys pin 11
#define AZ_DIR_PIN 11
// phys pin 13
#define AZ_PULSE_PIN 13
// phys pin 15
#define AZ_ENABLE_PIN 15
// phys pin 22
#define SHIFTER_ENABLE_PIN 22

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

void setAlign(Boolean value);
Boolean isAligned();
void setTracking(Boolean value);
void initMotors();
void stepMotors();
void makeTimer(char *name, timer_t *timerID, int expireSec, int expireNSec, int intervalSec, int intervalNSec);

void setGoto(Boolean value);
void enableMotor(Boolean value);
AltAzType getCurrentAltAz();
