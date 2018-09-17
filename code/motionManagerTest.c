#include "motionManagerTest.h"

//#define TRACK_SIDEREAL   15
//#define TRACK_LUNAR
//#define TRACK_SOLAR
//#define MOVE_1
//#define MOVE_2
//#define MOVE_3
//#define MOVE_4

int invertedPulse = 1;
int invertAltDirection = 0;
int invertAzDirection = 0;

typedef void (*sighandler_t) (int);

typedef struct {
	long double alt;
	long double az;
} 
AltAzType;

typedef struct {
	long double altMotorPosition;
	long double azMotorPosition;
} 
AltAzMotorPositionType;


Boolean doGoto = True;
Boolean doTracking = False;
Boolean align = True;

//long double alt = 90;
//long double az = 0;
long double altMotorPosition = 0;
long double azMotorPosition = 0;

long double altGotoPosition = 0;
long double azGotoPosition = 0;

//int trackRate = 0;
long double altSlewRate;
long double azSlewRate;
long double altSlewRateMSec;
long double azSlewRateMSec;
long double azSlew;
long double altSlew;
int azSlewDirection;
int altSlewDirection;
int azSlewDirectionPrev;
int altSlewDirectionPrev;

timer_t timerid;
sigset_t mask;
struct itimerspec its;
struct sigevent sev;

int count = 0;

AltAzMotorPositionType convertToMotorPostion(AltAzType altAz)
{
	AltAzMotorPositionType result;
	result.altMotorPosition = altAz.alt * altStepsPerDegree;
	result.azMotorPosition = altAz.az * azStepsPerDegree;
	return result;
}






void setupGoto()
{
	azGotoPosition = 100000;
	altGotoPosition = 100000;
	long double deltaAz = (azMotorPosition - azGotoPosition);  
	long double deltaAlt = (altMotorPosition - altGotoPosition); 
	azSlewDirection = 0;
	altSlewDirection = 0;
	if(deltaAz < 0) 
	{
		azSlewDirection = -1.0;
		deltaAz = fabs(deltaAz);
	}
	else if (deltaAz > 0)
	{
		azSlewDirection = 1.0;
	}
	if(deltaAlt < 0)
	{
		altSlewDirection = -1.0;
		deltaAlt= fabs(deltaAlt);
	}
	else if(deltaAlt > 0)
	{
		altSlewDirection = 1.0;
	}
}

void setGoto(Boolean value) 
{
	if(!align)
	   return;
	if(doGoto!= value)
	{
		doGoto = value;
		if(doGoto)
		{
			doTracking = False;
			setupGoto();
			/* block timer temporarily */
			sigemptyset (&mask);
			sigaddset(&mask,SIG);
			sigprocmask(SIG_SETMASK, &mask, NULL);
						
			/* start timer */
			its.it_value.tv_sec = 1;
			its.it_value.tv_nsec = 00000;
			its.it_interval.tv_sec = 1;
			its.it_interval.tv_nsec = 00000;
			
			timer_settime(timerid, 0, &its, NULL);
			
			/* unblock */
			sigprocmask(SIG_UNBLOCK,&mask, NULL);
		}
	}
}

void moveAltMotor(int direction)
{
	if(direction != altSlewDirectionPrev)
	{
		digitalWrite(ALT_PULSE_PIN,direction ^ invertAltDirection);
		altSlewDirectionPrev = direction;	
	    delayMicroseconds(3);
	}
	digitalWrite(ALT_PULSE_PIN,1 ^ invertedPulse);
	delayMicroseconds(3);
	digitalWrite(ALT_PULSE_PIN,0 ^ invertedPulse);	
}

void moveAzMotor(int direction)
{
	if(direction != azSlewDirectionPrev)
	{
		digitalWrite(AZ_PULSE_PIN,direction ^ invertAzDirection);
		azSlewDirectionPrev = direction;	
	    delayMicroseconds(3);
	}
	digitalWrite(AZ_PULSE_PIN,1 ^ invertedPulse);
	delayMicroseconds(3);
	digitalWrite(AZ_PULSE_PIN,0 ^ invertedPulse);	
}






void stepMotorsMax()
{
    Boolean doneAlt = True;
    Boolean doneAz = False;
	if(altSlewDirection > 0)
	{
		if(floorl(altMotorPosition) < floorl(altGotoPosition) )
		{
		    doneAlt = False;
		    altMotorPosition += altSlewDirection;
	    }  
	} 
	else 
	{
		if(floorl(altMotorPosition) > floorl(altGotoPosition) )
		{
		    doneAlt = False;
		    altMotorPosition += altSlewDirection;
	    }  
	}
	if(azSlewDirection > 0)
	{
		if(floorl(azMotorPosition) < floorl(azGotoPosition) )
		{
		    doneAz = False;
		    azMotorPosition += azSlewDirection;
	    }  
	} 
	else 
	{
		if(floorl(azMotorPosition) > floorl(azGotoPosition) )
		{
		    doneAz = False;
		    azMotorPosition += azSlewDirection;
	    }  
	}
	if(doneAlt && doneAz)
	{
		
		/* block timer */
		sigemptyset (&mask);
		sigaddset(&mask,SIG);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		
	}
}


void stepMotors()
{
	stepMotorsMax();
}

void initMotors()
{
	// timer_delete(timerid);
	
	// 1 ms timer setup
	
	struct sigaction sa;
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = stepMotors;
	sigemptyset(&sa.sa_mask);
	sigaction(SIG,&sa, NULL);
	
	/* block timer temporarily */
	sigemptyset (&mask);
	sigaddset(&mask,SIG);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	
	/* create timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	timer_create(CLOCKID,&sev,&timerid);
		
    azSlewDirection = 0;
    altSlewDirection = 0;
	digitalWrite(ALT_PULSE_PIN,altSlewDirection);
	digitalWrite(AZ_PULSE_PIN,azSlewDirection);
	delayMicroseconds(3);
	altSlewDirectionPrev = altSlewDirection;	
	azSlewDirectionPrev = azSlewDirection;	


    azSlewDirectionPrev = 0;
    altSlewDirectionPrev = 0;
    
    printf("wiring \n");
    wiringPiSetupPhys();
    pinMode(ALT_DIR_PIN,OUTPUT);
    pinMode(ALT_PULSE_PIN,OUTPUT);
    pinMode(ALT_ENABLE_PIN,OUTPUT);
     
    pinMode(ALT_MAX_PIN,INPUT);
    pinMode(ALT_MIN_PIN,INPUT);


    pinMode(AZ_DIR_PIN,OUTPUT);
    pinMode(AZ_PULSE_PIN,OUTPUT);
    pinMode(AZ_ENABLE_PIN,OUTPUT);
    
    pinMode(SHIFTER_ENABLE_PIN,OUTPUT);
   
    pullUpDnControl(SHIFTER_ENABLE_PIN,PUD_DOWN);
    pullUpDnControl(ALT_MAX_PIN,PUD_UP);
    pullUpDnControl(ALT_MIN_PIN,PUD_UP);
    
 	digitalWrite(AZ_PULSE_PIN,1 ^ invertedPulse);
   
  	digitalWrite(SHIFTER_ENABLE_PIN,1);
   
    
}
    
int main(int argc, char * argv[])
{
	initMotors();
	azMotorPosition = 0;
	altMotorPosition = 0;
    digitalWrite(ALT_ENABLE_PIN,1);
    digitalWrite(ALT_DIR_PIN,1);
   while(1)
    {
		digitalWrite(ALT_PULSE_PIN,0);
		usleep(1);
		digitalWrite(ALT_PULSE_PIN,1);
		usleep(1);
		
	}
    return 0;
} 
    
