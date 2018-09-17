#include "motionManager.h"
#include "align.h"

//#define TRACK_SIDEREAL   15
//#define TRACK_LUNAR
//#define TRACK_SOLAR
//#define MOVE_1
//#define MOVE_2
//#define MOVE_3
//#define MOVE_4

#define DEBUG

#define SHOW_ALIGN 0
#define SHOW_GET_ALT_AZ 1
#define SHOW_RESET_SLEW 0
#define SHOW_ANGLE 0
#define SHOW_GET_ALT_AZ_HA 0

typedef void (*sighandler_t) (int);



typedef struct {
	long double altMotorPosition;
	long double azMotorPosition;
} 
AltAzMotorPositionType;


// this is as viewed from back of motor
// CW is right Az and up alt
// CCW is left Az and down alt
// + delta moves up and right
// - delta moves down and left
// 1 CCW - value moving down
// 0 CW  + value moving up
// 1 CCW - value moving left
// 0 CW  + value moving right
typedef enum
{ CW, CCW }
Direction;


Boolean doGoto = False;
Boolean doTracking = False;
Boolean align = False;
Boolean userMove = False;

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
long double deltaAz;
long double deltaAlt;

Direction azSlewDirection;
Direction altSlewDirection;
Direction azSlewDirectionPrev;
Direction altSlewDirectionPrev;
//TODO remove with getAltAzOrig
long secondsSinceEpocMidnight = 0;
int prevDay;
timer_t timerid;


//timer_t trackTimerid;
//timer_t gotoTimerid;
//timer_t userTimerid;
sigset_t mask;
struct itimerspec its;
struct sigevent sev;
//struct sigevent trackSev;
//struct sigevent gotoSev;
//struct sigevent userSev;

int count = 0;

int azCount = 0;

void blockTimer()
{
	/* block timer temporarily */
	sigemptyset (&mask);
	sigaddset(&mask,SIG);
	if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	{
		printf("block digprocmask fail\n");
		
	}	
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if(timer_settime(timerid,0,&its,NULL) == -1)
	{
		printf("block disable timer fail\n");
	}
}

// void block?UserTimer()
// {
	// /* block timer temporarily */
	// sigemptyset (&mask);
	// sigaddset(&mask,SIG);
	// if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	// {
		// printf("block digprocmask fail\n");
		
	// }	
	// its.it_value.tv_sec = 0;
	// its.it_value.tv_nsec = 0;
	// its.it_interval.tv_sec = 0;
	// its.it_interval.tv_nsec = 0;

	// if(timer_settime(userTimerid,0,&its,NULL) == -1)
	// {
		// printf("block disable timer fail\n");
	// }
// }

// void blockGotoTimer()
// {
	// /* block timer temporarily */
	// sigemptyset (&mask);
	// sigaddset(&mask,SIG);
	// if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	// {
		// printf("block digprocmask fail\n");
		
	// }	
	// its.it_value.tv_sec = 0;
	// its.it_value.tv_nsec = 0;
	// its.it_interval.tv_sec = 0;
	// its.it_interval.tv_nsec = 0;

	// if(timer_settime(gotoTimerid,0,&its,NULL) == -1)
	// {
		// printf("block disable timer fail\n");
	// }
// }

// void blockTrackTimer()
// {
	// /* block timer temporarily */
	// sigemptyset (&mask);
	// sigaddset(&mask,SIG);
	// if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
	// {
		// printf("block digprocmask fail\n");
		
	// }	
	// its.it_value.tv_sec = 0;
	// its.it_value.tv_nsec = 0;
	// its.it_interval.tv_sec = 0;
	// its.it_interval.tv_nsec = 0;

	// if(timer_settime(trackTimerid,0,&its,NULL) == -1)
	// {
		// printf("block disable timer fail\n");
	// }
// }

AltAzType convertToAltAz(AltAzMotorPositionType mp) {
    AltAzType result;
    result.alt = mp.altMotorPosition / altStepsPerDegree;
    result.az = mp.azMotorPosition / azStepsPerDegree;
}

AltAzType getCurrentAltAz () {
    AltAzType result;
    AltAzMotorPositionType mp;
    mp.altMotorPosition = altMotorPosition + altOffset;
    mp.azMotorPosition = azMotorPosition + azOffset;
    result = convertToAltAz(mp);
    return result;
}


AltAzMotorPositionType convertToMotorPostion(AltAzType altAz)
{
	AltAzMotorPositionType result;
	result.altMotorPosition = altAz.alt * altStepsPerDegree;
	result.azMotorPosition = altAz.az * azStepsPerDegree;
	return result;
}

//long double convertRAtoHA(int secondsOffset) {
//    FILE *in1;
//    char buff[512];
//    //convert degrees long into hour long  divide by 15 360/24
//    // get current time
//    time_t mytime = time(0);
//    struct tm* tm_ptr = gmtime(&mytime);
//    int dateHour = tm_ptr->tm_hour;
//    int dateMinute = tm_ptr->tm_min;
//    int dateSecond = tm_ptr->tm_sec + secondsOffset;
//    int day = tm_ptr->tm_mday;
//    int month = tm_ptr->tm_mon+1;
//    int year = tm_ptr->tm_year+1900;
//        
//    
//    if(secondsSinceEpocMidnight==0 || prevDay!=day)
//    {
//		prevDay = day;
//	    secondsSinceEpocMidnight=0;
//    
//		char command2[50];
//		sprintf(command2,"date --date=\"%d/%d/%d UTC\" +%%s",month,day,year);
//#ifdef DEBUG 		
//		debugLogging(SHOW_GET_ALT_AZ_HA,False,"%s\n",command2);
//#endif
//		
//		while(secondsSinceEpocMidnight == 0)
//	    {
//#ifdef DEBUG 
//			debugLogging(SHOW_GET_ALT_AZ,False,"do seconds since epoc midnight\n");
//#endif
//			if((in1 = popen(command2,"r"))!=NULL)
//			{
//				while(fgets(buff, sizeof(buff),in1) != NULL) 
//				{
//					sscanf(buff,"%ld",&secondsSinceEpocMidnight);
//#ifdef DEBUG 
//					debugLogging(SHOW_GET_ALT_AZ_HA,False,"command result %s\n",buff);
//#endif
//				}    
//			}
//	    }
//	}
//    long double offsetGmstNoon = dateHour - 12 + dateMinute/60.0f + dateSecond/3600.0f;
//    long double partDayGmstNoon = offsetGmstNoon/24;
//    long double dayOffset = (secondsSinceEpocMidnight - secondsEpoch1_1_2000)/(60.0*60.0*24.0) + partDayGmstNoon;
//    //GMST noon sideral time in hms
//#ifdef DEBUG 
//    long double gmst = 18.697374558+24.06570982441908 * (dayOffset);
//    long double gmstMod = fmod(gmst,24);
//    int hour = gmstMod;
//    int minute = (gmstMod - hour)* 60.0;
//    long double second = ((gmstMod - hour) * 60.0 - minute) * 60.0; 
//    debugLogging(SHOW_GET_ALT_AZ_HA,False,"conv inputs %Lf %Lf %Lf\r\n",dayOffset,offsetGmstNoon,partDayGmstNoon);
//    debugLogging(SHOW_GET_ALT_AZ_HA,False,"gmst LST %d:%d:%Lf \r\n",hour,minute,second);
//#endif
//    //local sideral time offest to current gmt by 12 hours (since reference is noon
//    //time then offset local longitiude.
//    long double lstLong = 18.697374558+24.06570982441908 * (dayOffset) - longitudeHours;
//    long double lstLongMod = fmod(lstLong,24);
//    long double lstLongDegrees = lstLongMod *15;
//    long double HA = lstLongDegrees - raDegrees;
//    debugLogging(SHOW_GET_ALT_AZ_HA,False,"lst inputs %Lf %Lf %Lf %Lf\r\n",lstLong,lstLongMod,lstLongDegrees,raDegrees);    
//    if(HA < 0)
//       HA += 360;
// #ifdef DEBUG 
//    long double haHms = HA /15;
//	int hours = haHms;
//	int minutes = (haHms - hours) * 60.0;
//	long double seconds = ((haHms - hours) * 60.0 - minutes) * 60.0;  
//    debugLogging(SHOW_GET_ALT_AZ,False,"HA %Lf %dh %dm %Lfs\n",HA,hours,minutes,seconds);        
//
//#endif
//    return HA;
//    
//}

AltAzType getAltAz(double HA)
{
	
    AltAzType result;
    long double c1 = HA * C_PI180;
    long double c2 = cosl(declinationDegrees * C_PI180);
    long double x = cosl(c1) * c2; // X[1][1]
    long double y = sinl(c1) * c2; // X[2][1]
    long double z = sinl(declinationDegrees * C_PI180); //X[3][1]
    debugLogging(False,False,"1 conv HA to altaz %Lf %Lf %Lf %Lf %Lf\n",c1,c2,x,y,z);
    long double c3 = cosl(C_LATCALC);
    long double c4 = sinl(C_LATCALC);
    long double xhor = x * c3 - z * c4;
    long double yhor = y;
    long double zhor = x * c4 + z * c3;
    debugLogging(False,False,"2 conv HA to altaz %Lf %Lf %Lf %Lf %Lf\n",c3,c4,xhor,yhor,zhor);
    
    result.az = atan2(yhor,xhor) * (C_180PI) + 180;
    result.alt= asin(zhor) * C_180PI;
    
#ifdef DEBUG 
    int azDegree = result.az;
    int azMinute = (result.az - azDegree) * 60;  
    float azSecond = ((result.az - azDegree) * 60 - azMinute) * 60.0;
    debugLogging(False,False,"convAz %d:%d:%Lf %Lf \n",azDegree,azMinute,azSecond,result.az);
    int altDegree = result.alt;
    int altMinute = (result.alt - altDegree) * 60;  
    float altSecond = ((result.alt - altDegree) * 60 - altMinute) * 60.0;
    debugLogging(False,False,"convAlt %d:%d:%Lf %Lf \n",altDegree,altMinute,altSecond,result.alt);
#endif
    return result;
}

AltAzType getAltAzOrig(int secondsOffset)
{
	
 	FILE *in1;
 	AltAzType result;
	char buff[512];
    //convert degrees long into hour long  divide by 15 360/24
    // get current time
    time_t mytime = time(0);
    struct tm* tm_ptr = gmtime(&mytime);
    int dateHour = tm_ptr->tm_hour;
    int dateMinute = tm_ptr->tm_min;
    int dateSecond = tm_ptr->tm_sec + secondsOffset;
    int day = tm_ptr->tm_mday;
    int month = tm_ptr->tm_mon+1;
    int year = tm_ptr->tm_year+1900;
        
    
    if(secondsSinceEpocMidnight==0 || prevDay!=day)
    {
		prevDay = day;
	    secondsSinceEpocMidnight=0;
    
		char command2[50];
		sprintf(command2,"date --date=\"%d/%d/%d UTC\" +%%s",month,day,year);
#ifdef DEBUG 		
		debugLogging(SHOW_GET_ALT_AZ_HA,False,"%s\n",command2);
#endif
		
		while(secondsSinceEpocMidnight == 0)
	    {
#ifdef DEBUG 
			debugLogging(SHOW_GET_ALT_AZ,False,"do seconds since epoc midnight\n");
#endif
			if((in1 = popen(command2,"r"))!=NULL)
			{
				while(fgets(buff, sizeof(buff),in1) != NULL) 
				{
					sscanf(buff,"%ld",&secondsSinceEpocMidnight);
#ifdef DEBUG 
					debugLogging(SHOW_GET_ALT_AZ_HA,False,"command result %s\n",buff);
#endif
				}    
			}
	    }
	}
    long double offsetGmstNoon = dateHour - 12 + dateMinute/60.0f + dateSecond/3600.0f;
    long double partDayGmstNoon = offsetGmstNoon/24;
    long double dayOffset = (secondsSinceEpocMidnight - secondsEpoch1_1_2000)/(60.0*60.0*24.0) + partDayGmstNoon;
    //GMST noon sideral time in hms
#ifdef DEBUG 
    long double gmst = 18.697374558+24.06570982441908 * (dayOffset);
    long double gmstMod = fmod(gmst,24);
    int hour = gmstMod;
    int minute = (gmstMod - hour)* 60.0;
    long double second = ((gmstMod - hour) * 60.0 - minute) * 60.0; 
    debugLogging(SHOW_GET_ALT_AZ_HA,False,"conv inputs %Lf %Lf %Lf \r\n",dayOffset,offsetGmstNoon,partDayGmstNoon);
    debugLogging(SHOW_GET_ALT_AZ_HA,False,"gmst LST %d:%d:%Lf \r\n",hour,minute,second);
#endif
    //local sideral time offest to current gmt by 12 hours (since reference is noon
    //time then offset local longitiude.
    long double lstLong = 18.697374558+24.06570982441908 * (dayOffset) - longitudeHours;
    long double lstLongMod = fmod(lstLong,24);
    long double lstLongDegrees = lstLongMod *15;
    long double HA = lstLongDegrees - raDegrees;
    debugLogging(SHOW_GET_ALT_AZ_HA,False,"lst inputs %Lf %Lf %Lf %Lf\r\n",lstLong,lstLongMod,lstLongDegrees,raDegrees);    
    if(HA < 0)
       HA += 360;
 #ifdef DEBUG 
    long double haHms = HA /15;
	int hours = haHms;
	int minutes = (haHms - hours) * 60.0;
	long double seconds = ((haHms - hours) * 60.0 - minutes) * 60.0;  
    debugLogging(SHOW_GET_ALT_AZ,False,"HA %Lf %dh %dm %Lfs\n",HA,hours,minutes,seconds);        

#endif
    debugLogging(True,False,"orig HA %Lf ",HA);       
    long double c1 = HA * C_PI180;
    long double c2 = cos(declinationDegrees * C_PI180);
    long double x = cos(c1) * c2;
    long double y = sin(c1) * c2;
    long double z = sinl(declinationDegrees * C_PI180);
    
    long double c3 = cosl(C_LATCALC);
    long double c4 = sinl(C_LATCALC);
    long double xhor = x * c3 - z * c4;
    long double yhor = y;
    long double zhor = x * c4 + z * c3;
    
    result.az = atan2l(yhor,xhor) * (C_180PI) + 180;
    result.alt= asinl(zhor) * C_180PI;
    
#ifdef DEBUG 
    int azDegree = result.az;
    int azMinute = (result.az - azDegree) * 60;  
    float azSecond = ((result.az - azDegree) * 60 - azMinute) * 60.0;
    debugLogging(SHOW_GET_ALT_AZ,False,"convAz %d:%d:%Lf %Lf \n",azDegree,azMinute,azSecond,result.az);
    int altDegree = result.alt;
    int altMinute = (result.alt - altDegree) * 60;  
    float altSecond = ((result.alt - altDegree) * 60 - altMinute) * 60.0;
    debugLogging(SHOW_GET_ALT_AZ,False,"convAlt %d:%d:%Lf %Lf \n",altDegree,altMinute,altSecond,result.alt);
#endif
    return result;
}


void resetSlew()
{
#ifdef DEBUG 
	
	int showFile = 0;  // turn on off debugging
	int showSlewAdjust = 0;
	int showTrackUpdate = 0;
	debugLogging(SHOW_RESET_SLEW,True,"reset slew\n");
#endif
	azCount = 0;
	count = 0;
        debugLogging(True,True,"at convert\r\n");
        fflush(stdout);
        AltAzType altAz;
        double HA = convertRAtoHA(1);
        //debugLogging(True,False,"at convert\r\n");
        //fflush(stdout);
        if (alignIsReady()) {
            altAz = alignEquToInstr2(HA,declinationDegrees);
        } else {
            altAz = equToHor2(HA,declinationDegrees);
        }
        debugLogging(True,True,"reset slew new altaz %Lf %Lf\r\n",altAz.alt,altAz.az);       
	altAz = getAltAzOrig(1);
        debugLogging(True,True,"reset slew old altaz %Lf %Lf\r\n",altAz.alt,altAz.az);     
	AltAzMotorPositionType locationPlus = convertToMotorPostion(altAz);
	// also equals slew rate steps per second
    if(addLeftLarge)
	{
		azOffset += azButtonKeySteps;
	}
	if(addRightLarge)
	{
		azOffset -= azButtonKeySteps;
	}
    if(addLeftSmall)
	{
		azOffset += azJoyKeySteps;
	}
	if(addRightSmall)
	{
		azOffset -= azJoyKeySteps;
	}
#ifdef DEBUG     
	debugLogging(showSlewAdjust,True,"adjust %Lf \n",locationPlus.azMotorPosition );
#endif
    locationPlus.azMotorPosition += azOffset;
#ifdef DEBUG     
	debugLogging(showSlewAdjust,True,"adjust2 %Lf \n",locationPlus.azMotorPosition );
#endif
	long double deltaAz = locationPlus.azMotorPosition - azMotorPosition; 

	if(fabs(deltaAz) > azSteps360/2)
	{
		if(deltaAz < 0)
		{
			azMotorPosition -= azSteps360;
			deltaAz = locationPlus.azMotorPosition - azMotorPosition;
#ifdef DEBUG     
 	        debugLogging(SHOW_RESET_SLEW,False,"delatAz1 %Lf\n",deltaAz);	   
	        debugLogging(showFile,True,"delatAz1 %Lf\n",deltaAz);	
#endif
		}
		else
		{ 
			azMotorPosition += azSteps360;			
			deltaAz = locationPlus.azMotorPosition - azMotorPosition;
#ifdef DEBUG     
	        debugLogging(SHOW_RESET_SLEW,False,"delatAz2 %Lf\n",deltaAz);	   
	        debugLogging(showFile,True,"delatAz2 %Lf\n",deltaAz);	
#endif
		}
	} 
    if(addUpLarge)
	{
		altOffset += altButtonKeySteps;
	}
	if(addDownLarge)
	{
		altOffset -= altButtonKeySteps;
	}
    if(addUpSmall)
	{
		altOffset += altJoyKeySteps;
	}
	if(addDownSmall)
	{
		altOffset -= altJoyKeySteps;
	}
	locationPlus.altMotorPosition += altOffset;
	long double deltaAlt = (locationPlus.altMotorPosition - altMotorPosition);
#ifdef DEBUG     
	int degrees = altAz.alt;
	int minutes = (altAz.alt - degrees) * 60.0;
	long double seconds = ((altAz.alt - degrees) * 60.0 - minutes) * 60.0;  
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);	
    debugLogging(showTrackUpdate,False,"time %d %d \n",tv.tv_sec,tv.tv_usec);
	debugLogging(SHOW_ANGLE,False,"motorAZ angle %d:%d:%Lf\n",degrees,minutes,seconds);	  
	debugLogging(showTrackUpdate,False,"motorAZ P %Lf %Lf %Lf %Lf\n",altAz.az,azMotorPosition,locationPlus.azMotorPosition,deltaAz);	   
	debugLogging(showTrackUpdate,False,"motorALT P %Lf %Lf %Lf %Lf\n",altAz.alt,altMotorPosition,locationPlus.altMotorPosition,deltaAlt);   
	debugLogging(showFile,True,"motorAZ P %Lf %Lf %Lf %Lf\n",altAz.az,azMotorPosition,locationPlus.azMotorPosition,deltaAz);	  
	debugLogging(showFile,True,"motorALT P %Lf %Lf %Lf %Lf\n",altAz.alt,altMotorPosition,locationPlus.altMotorPosition,deltaAlt); 
#endif
	if(deltaAz < 0) 
	{
		azSlewDirection = CCW;
	}
	else 
	{
		azSlewDirection = CW;
	}
	if(deltaAlt < 0)
	{
		altSlewDirection = CCW;
	}
	else 
	{
		altSlewDirection = CW;
	}
	azSlewRateMSec = deltaAz/1000.0f;
	altSlewRateMSec = deltaAlt/1000.0f;
	azSlew = 0.0;
	altSlew = 0.0;
#ifdef DEBUG     
	debugLogging(SHOW_RESET_SLEW,False,"slews %Lf %Lf\n",azSlewRateMSec,altSlewRateMSec);
#endif
}


Boolean isAligned() 
{
#ifdef DEBUG     
    debugLogging(SHOW_ALIGN,False,"is align %d\n",align);
#endif
	
	return align;
}

void setAlign(Boolean value) 
{

    //TODO need to check for multiple runs
    
#ifdef DEBUG     
    debugLogging(SHOW_ALIGN,False,"set align %d\n",value);
#endif
    if(value) 
    {
        //azOffset = 0;
        //altOffset = 0;
        blockTimer();
        //blockUserTimer();
        //blockGotoTimer();
        align = True;
        userMove = False;
#ifdef DEBUG     
        debugLogging(SHOW_ALIGN,False,"align true %d\n",userMove);
#endif
		// set align before call enableMotor otherwise with start useMove
	enableMotor(True);
        AltAzType altAz;
        double HA = convertRAtoHA(0);
        debugLogging(True,False," set align HA %Lf Dec %Lf\r\n",HA,declinationDegrees);       
	altAz = getAltAzOrig(0);
        debugLogging(True,False," set align orig altaz %Lf %Lf\r\n",altAz.alt,altAz.az);     
        altAz = getAltAz(HA);
        debugLogging(True,False," set align old altaz %Lf %Lf\r\n",altAz.alt,altAz.az);     
        double alt;
        double az;
//        //debugLogging(True,False,"at convert\r\n");
//        //fflush(stdout);
//        if (alignIsReady()) {
//            altAz = alignEquToInstr2(HA,declinationDegrees);
//        } else {
//            altAz = equToHor2(HA,declinationDegrees);
//        }
        debugLogging(True,False," set align new altaz %Lf %Lf\r\n",altAz.alt,altAz.az); 
        AltAzType adjustedAltAz;
        if(altOffset != 0 || azOffset != 0) {
            adjustedAltAz.alt = altAz.alt + altOffset/altStepsPerDegree;
            adjustedAltAz.az = altAz.az + azOffset/azStepsPerDegree;
        } else {
            adjustedAltAz.alt = altAz.alt;
            adjustedAltAz.az = altAz.az;
        }
        debugLogging(True,False," set align goto altaz %Lf %Lf\r\n",adjustedAltAz.alt,adjustedAltAz.az);   
        alignAddStar(HA,declinationDegrees,adjustedAltAz.alt,adjustedAltAz.az);
        debugLogging(True,False," set align afteradd altaz %Lf %Lf\r\n",altAz.alt,altAz.az);       
        if (alignIsReady()) {
            altAz = alignEquToInstr2(HA,declinationDegrees);
        } else {
            altAz = equToHor2(HA,declinationDegrees);
        }
        debugLogging(True,False," set align afteradd2 altaz %Lf %Lf\r\n",altAz.alt,altAz.az);       
        fflush(stdout);	
	//altAz = getAltAzOrig(0);
        //debugLogging(True,False,"old altaz %Lf %Lf\r\n",altAz.alt,altAz.az);     
        fflush(stdout);	
        //AltAzType altAz = getAltAz(convertRAtoHA(0));
        AltAzMotorPositionType align = convertToMotorPostion(altAz);
        altMotorPosition = align.altMotorPosition;
        azMotorPosition = align.azMotorPosition;
#ifdef DEBUG     
        debugLogging(SHOW_ALIGN,False,"align alt %Lf %Lf\n",altMotorPosition,altAz.alt);
        debugLogging(SHOW_ALIGN,False,"align az %Lf %Lf\n",azMotorPosition,altAz.az);
#endif
    }
    else
    {
#ifdef DEBUG     
        debugLogging(SHOW_ALIGN,False,"clear align\n");
#endif
        align = False;
	    
    }
}



void setupGoto()
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging
#endif
	
	altSlewDirectionPrev = -1;
	azSlewDirectionPrev = -1;
#ifdef DEBUG     
	debugLogging(show,False,"setup goto\n");
#endif
        debugLogging(True,False,"setup goto at convert\r\n");
        fflush(stdout);
        AltAzType altAz;
        double HA = convertRAtoHA(0);
        double alt;
        double az;
        //debugLogging(True,False,"at convert\r\n");
        //fflush(stdout);
        if (alignIsReady()) {
            altAz = alignEquToInstr2(HA,declinationDegrees);
        } else {
            altAz = equToHor2(HA,declinationDegrees);
        }
        debugLogging(True,False,"setup goto new altaz %Lf %Lf\r\n",altAz.alt,altAz.az);       
	altAz = getAltAzOrig(0);
        debugLogging(True,False,"setup goto old altaz %Lf %Lf\r\n",altAz.alt,altAz.az);     
        fflush(stdout);	
        AltAzMotorPositionType locationTo = convertToMotorPostion(altAz);
	// also equals slew rate steps per second
	azGotoPosition = locationTo.azMotorPosition + azOffset;
	altGotoPosition = locationTo.altMotorPosition + altOffset;
	deltaAz = (azGotoPosition - azMotorPosition);
	
	if(fabs(deltaAz) > azSteps360/2)
	{
#ifdef DEBUG     
		debugLogging(show,False,"AZ > half way\n");
#endif
		if(deltaAz < 0.0)
		{
			deltaAz += azSteps360;
#ifdef DEBUG     
		    debugLogging(show,False,"deltaAZ %Lf \n",deltaAz);
#endif
		}
		else 
		{ 
			deltaAz -= azSteps360;
#ifdef DEBUG     
		    debugLogging(show,False,"deltaAZ %Lf \n",deltaAz);
#endif
		}
	}
	deltaAlt = (altGotoPosition - altMotorPosition); 
#ifdef DEBUG     
    debugLogging(show,False,"goto deltaAZ   %Lf %Lf %Lf %Lf\n",altAz.az,deltaAz,azMotorPosition,azGotoPosition);
    debugLogging(show,False,"goto deltaAlt  %Lf %Lf %Lf %Lf \n",altAz.alt,deltaAlt,altMotorPosition,altGotoPosition);
#endif
	azSlewDirection = CW;
	altSlewDirection = CW;
	if(deltaAz < 0) 
	{
		azSlewDirection = CCW;
		deltaAz = fabs(deltaAz);
	}
	else if (deltaAz > 0)
	{
		azSlewDirection = CW;
	}
	if(deltaAlt < 0)
	{
		altSlewDirection = CCW;
		deltaAlt= fabs(deltaAlt);
	}
	else if(deltaAlt > 0)
	{
		altSlewDirection = CW;
	}
#ifdef DEBUG     
    debugLogging(show,False,"goto azDir   %d \n",azSlewDirection);
    debugLogging(show,False,"goto altDir  %d \n",altSlewDirection);
#endif

}

// if not aligned do nothing
// if not change value to nothing
// if different assign value, start tracking
void setTracking(Boolean value) 
{
	if(!align)
		return;
	if(doTracking != value)
	{
		doTracking = value;
		if(doTracking)
		{
			doGoto = False;
			
		    blockTimer();
			//blockUserTimer();		
			//blockGotoTimer();		
			/* start timer */
			its.it_value.tv_sec = 0;
			its.it_value.tv_nsec = 1000000;
			its.it_interval.tv_sec = 0;
			its.it_interval.tv_nsec = 1000000;
			
			//if(timer_settime(trackTimerid, 0, &its, NULL)==-1)
			if(timer_settime(timerid, 0, &its, NULL)==-1)
			{
				printf("setTracking enable timer fail\n");
			}
#ifdef DEBUG     
	debugLogging(1,False,"set Tracking Reset Slew \n","");
#endif
			resetSlew();
			/* unblock */
			if(sigprocmask(SIG_UNBLOCK,&mask, NULL) == -1)
			{
				printf("setTracking sigprockmask fail\n");
			}
			
		}
	}

}

// must be aligned to do anyting
// if not change value to nothing
// if different assign value, go goto.  Goto finsishes by switching to tracking

void setGoto(Boolean value) 
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging
#endif	
	doneMoving = False;
	moving = True;
#ifdef DEBUG     
	debugLogging(show,False,"set goto %d %d %d\n",align,doGoto,value);
#endif
	if(!align) 
	{	    
	    declinationNew.degrees = declination.degrees;
		declinationNew.minutes = declination.minutes;
		declinationNew.seconds= declination.seconds;
		raNew.hours = ra.hours;
		raNew.minutes = ra.minutes;
		raNew.seconds = ra.seconds;
	   return;
    }
	if(doGoto!= value)
	{
		doGoto = value;
		if(doGoto)
		{
			doTracking = False;
			setupGoto();
			/* block timer temporarily */
			blockTimer();
			//blockTrackTimer();
			//blockUserTimer();
						
			/* start timer */
			its.it_value.tv_sec = 0;
			its.it_value.tv_nsec = gotoTimer;
			its.it_interval.tv_sec = 0;
			its.it_interval.tv_nsec = gotoTimer;
			
			//if(timer_settime(trackTimerid, 0, &its, NULL)==-1)
			if(timer_settime(timerid, 0, &its, NULL)==-1)
			{
				printf("setGoto enable timer fail\n");
			}
			
			/* unblock */
			if(sigprocmask(SIG_UNBLOCK,&mask, NULL)==-1)
			{
				printf("setGoto sigprocmask fail\n");
			}
		}
		else
		{
			declinationNew.degrees = declination.degrees;
			declinationNew.minutes = declination.minutes;
			declinationNew.seconds= declination.seconds;
			raNew.hours = ra.hours;
			raNew.minutes = ra.minutes;
			raNew.seconds = ra.seconds;
		}
	}
	else
	{
		declinationNew.degrees = declination.degrees;
		declinationNew.minutes = declination.minutes;
		declinationNew.seconds= declination.seconds;
		raNew.hours = ra.hours;
		raNew.minutes = ra.minutes;
		raNew.seconds = ra.seconds;
	}

	
}

// must be aligned to do anyting
// if not change value to nothing
// if different assign value, go goto.  Goto finsishes by switching to tracking

void setUserMove() 
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging
	debugLogging(show,False,"setUserMove \n");
#endif
	if(align) 
	{	    
	   return;
    }
#ifdef DEBUG     
	debugLogging(show,False,"setUserMove 2 %d\n",setupTimer);
#endif
    userMove = True;
    /* block timer temporarily */
	blockTimer();
	//blockTrackTimer();
	//blockGotoTimer();
				
	/* start timer */
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = setupTimer;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = setupTimer;
	
	//if(timer_settime(userTimerid, 0, &its, NULL)==-1)
	if(timer_settime(timerid, 0, &its, NULL)==-1)
	{
		printf("setUserMove enable timer fail\n");
	}
	
	/* unblock */
	if(sigprocmask(SIG_UNBLOCK,&mask, NULL)==-1)
	{
		printf("setUserMove sigprocmask fail\n");
	}
}

void moveAltMotor(Direction direction)
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging
	int showChange = 1;  // turn on off debugging
#endif

#ifdef DEBUG
     debugLogging(show,False,"altm %d ",direction);
#endif
	if(direction != altSlewDirectionPrev)
	{
#ifdef DEBUG
     debugLogging(showChange,False,"alt move change direction from %d to %d  %d\n",altSlewDirectionPrev,direction,altSlewDirection);
#endif
		digitalWrite(ALT_DIR_PIN,direction ^ invertAltDirection);
		altSlewDirectionPrev = direction;	
	    delayMicroseconds(10);
	}
	digitalWrite(ALT_PULSE_PIN,1 ^ invertAltPulse);
	delayMicroseconds(10);
	digitalWrite(ALT_PULSE_PIN,0 ^ invertAltPulse);	
}

void moveAzMotor(Direction direction)
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging
	int showChange = 1;  // turn on off debugging
#endif	
	
#ifdef DEBUG     
    debugLogging(show,False,"azm %d ",direction);
#endif
	if(direction != azSlewDirectionPrev)
	{
#ifdef DEBUG
     debugLogging(showChange,False,"az move change direction from %d to %d  %d\n",azSlewDirectionPrev,direction,azSlewDirection);
#endif
		digitalWrite(AZ_DIR_PIN,direction ^ invertAzDirection);
		azSlewDirectionPrev = direction;	
	    delayMicroseconds(10);
	}
	digitalWrite(AZ_PULSE_PIN,1 ^ invertAzPulse);
	delayMicroseconds(10);
	digitalWrite(AZ_PULSE_PIN,0 ^ invertAzPulse);	
}




void stepMotorsSidereal()
{
#ifdef DEBUG 
	int showAz = 0;  // turn on off debugging
	int showAzMove = 0;  // turn on off debugging
	int showAlt = 0;  // turn on off debugging
	int showAltMove = 0;  // turn on off debugging
	int showFile = 0;  // turn on off debugging
#endif

#ifdef DEBUG     
			debugLogging(showAz,False,"az pre %Lf %Lf\n",azMotorPosition,azSlewRateMSec);		    
			debugLogging(showFile,True,"az1- %Lf %Lf %Lf\n",azMotorPosition,azSlew,azSlewRateMSec);		
#endif
    int azMpPrev = azMotorPosition;   
    azMotorPosition += azSlewRateMSec;
	int azMpCurrent = azMotorPosition;
 #ifdef DEBUG     
			debugLogging(showAz,False,"az post %Lf %Lf %d %d\n",azMotorPosition,azSlewRateMSec,azMpPrev,azMpCurrent);		    
			debugLogging(showFile,True,"az2- %Lf %Lf %Lf\n",azMotorPosition,azSlew,azSlewRateMSec);		
#endif
    if(azMpPrev != azMpCurrent)
	{
        azCount++;
		moveAzMotor(azSlewDirection);
#ifdef DEBUG 	
		if(azSlewRateMSec < 0.00) 
		{
    
			debugLogging(showAzMove,False,"az- %Lf %Lf %d %d\n",azMotorPosition,azSlewRateMSec,azMpPrev,azMpCurrent);		    
			debugLogging(showFile,True,"az- %Lf %Lf \n",azMotorPosition,azSlewRateMSec);		            
	    }		
	    else
	    {    
			debugLogging(showAzMove,False,"az+ %Lf %Lf %d %d\n",azMotorPosition,azSlewRateMSec,azMpPrev,azMpCurrent);		   
			debugLogging(showFile,True,"az+ %Lf %Lf \n",azMotorPosition,azSlew);		
		}
#endif		
	}

#ifdef DEBUG     
			debugLogging(showAlt,False,"alt pre %Lf %Lf\n",altMotorPosition,altSlewRateMSec);
#endif
    int altMpPrev = altMotorPosition;   
	altMotorPosition += altSlewRateMSec;
	int altMpCurrent = altMotorPosition;
#ifdef DEBUG     
			debugLogging(showAlt,False,"alt post %Lf %Lf %d %d\n",altMotorPosition,altSlewRateMSec,altMpPrev,altMpCurrent);
#endif

    if(altMpPrev != altMpCurrent)
	{
		moveAltMotor(altSlewDirection);
#ifdef DEBUG 
		if(altSlewRateMSec < 0.00) 
		{    
			debugLogging(showAltMove,False,"alt %Lf %Lf %d %d\n",altMotorPosition,altSlewRateMSec,altMpPrev,altMpCurrent);
		}
		else
		{     
			debugLogging(showAltMove,False,"alt- %Lf %Lf %d %d\n",altMotorPosition,altSlewRateMSec,altMpPrev,altMpCurrent);
		}
#endif
	}
	
	count++;
#ifdef DEBUG     
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);	
    debugLogging(0,False,"time count %d %d %d\n",tv.tv_sec,tv.tv_usec,count);
	debugLogging(0,False,"c%d ",count);
#endif
	if(count > 1000) 
	{
		count = 0;
#ifdef DEBUG     
	debugLogging(0,False,"step side\n","");
#endif
		resetSlew();
	}
}


void stepMotorsMax()
{
#ifdef DEBUG
	int show = 0;  // turn on off debugging
#endif	

    Boolean doneAlt = True;
    Boolean doneAz = True;
#ifdef DEBUG     
    debugLogging(show,False,"alt step %Lf %Lf %d\n",altMotorPosition,altGotoPosition,altSlewDirection);
#endif

    // correct going past 360/0 reset Az motor position.
    if(azMotorPosition > azSteps360)
    {
#ifdef DEBUG     
		debugLogging(show,False,"fmod1 %Lf %ld \n",azMotorPosition,azSteps360);
#endif
		azMotorPosition = fmod(azMotorPosition,azSteps360);
#ifdef DEBUG     
		debugLogging(show,False,"fmod2 %Lf %ld \n",azMotorPosition,azSteps360);
#endif
	}
	if(azMotorPosition < 0.0)
	{
#ifdef DEBUG     
		debugLogging(show,False,"fmod3 %Lf %ld \n",azMotorPosition,azSteps360);
#endif
		azMotorPosition += azSteps360;
#ifdef DEBUG     
		debugLogging(show,False,"fmod4 %Lf %ld \n",azMotorPosition,azSteps360);
#endif
	}
	
	//delta Alt is total change 
	if(altSlewDirection == CCW)
	{
		deltaAlt--;
		if(deltaAlt > 0) 
		{
		    doneAlt = False;
#ifdef DEBUG     
            debugLogging(show,False,"alt step 1 %Lf \n",altMotorPosition);
#endif
		    altMotorPosition =  altMotorPosition - 1.0;
#ifdef DEBUG     
            debugLogging(show,False,"alt step 2 %Lf \n",altMotorPosition);
#endif
		    moveAltMotor(altSlewDirection);
	    }  
	} 
	else 
	{
		deltaAlt--;
		if(deltaAlt > 0)
		{
		    doneAlt = False;
#ifdef DEBUG     
            debugLogging(show,False,"alt step 3 %Lf \n",altMotorPosition);
#endif
		    altMotorPosition = altMotorPosition + 1.0;
#ifdef DEBUG     
            debugLogging(show,False,"alt step 4 %Lf \n",altMotorPosition);
#endif
		    moveAltMotor(altSlewDirection);
	    }  
	}
#ifdef DEBUG     
    debugLogging(show,False,"az step %Lf %Lf %d\n",azMotorPosition,azGotoPosition,azSlewDirection);
#endif
	if(azSlewDirection == CCW)
	{
		deltaAz--;
		if(deltaAz > 0 )
		{
		    doneAz = False;
		    azMotorPosition = azMotorPosition - 1.0;
		    moveAzMotor(azSlewDirection);
	    }  
	} 
	else 
	{
		deltaAz--;
		if(deltaAz > 0)
		{
		    doneAz = False;
		    azMotorPosition = azMotorPosition + 1.0;
		    moveAzMotor(azSlewDirection);
	    }  
	}
#ifdef DEBUG     
	debugLogging(show,False,"done ?  %d %d\n",doneAlt,doneAz);
#endif
	if(doneAlt && doneAz)
	{
#ifdef DEBUG     
		debugLogging(show,False,"done goto\n");
#endif
		/* block timer */
        blockTimer();			
        //blockUserTimer();			
        //blockGotoTimer();			
		doGoto = False;
		doneMoving = True;
		moving = False;
		declinationNew.degrees = declination.degrees;
		declinationNew.minutes = declination.minutes;
		declinationNew.seconds= declination.seconds;
		raNew.hours = ra.hours;
		raNew.minutes = ra.minutes;
		raNew.seconds = ra.seconds;
 		
 		setTracking(True);
	}
}

int userAltMoveCount = 0;
int userAzMoveCount = 0;
void stepUserMove()
{
#ifdef DEBUG
	int show = 0; // turn on off debugging
		debugLogging(show,False,"step user\n");
#endif	
	
	if(altUp)
	{
		userAltMoveCount++;
 		if(userAltMoveCount == userMoveAltRatio)
	    {
			userAltMoveCount = 0;
		    moveAltMotor(CW);
		}
	}
	else if(altDown)
	{
		userAltMoveCount++;
		if(userAltMoveCount == userMoveAltRatio)
	    {
			userAltMoveCount = 0;
		    moveAltMotor(CCW);
		}
	}
	else 
	{
		userAltMoveCount = 0;
	}
	if(azLeft)
	{
		userAzMoveCount++;
		if(userAzMoveCount == userMoveAzRatio)
	    {
			userAzMoveCount = 0;
		    moveAzMotor(CCW);
		}
	}
	else if(azRight)
	{
		userAzMoveCount++;
		if(userAzMoveCount == userMoveAzRatio)
	    {
			userAzMoveCount = 0;
		    moveAzMotor(CW);
		}
	}
	else
	{
		userAzMoveCount = 0;
	}
}

void stepMotors()
{
#ifdef DEBUG  
	int show = 0;  // turn on off debugging
   	debugLogging(show,False,"setp motor %d %d %d\n",doGoto,doTracking,userMove);
#endif

	if(doGoto)
	{
		stepMotorsMax();
	}
	else if(doTracking)
	{
		stepMotorsSidereal();
	}
	else if(userMove)
	{
#ifdef DEBUG  
   	    debugLogging(show,False,"setp user move\n");
#endif
		stepUserMove();
	}
	
}

void moveStepsAltCW(long steps)  {
	long count = 0;
    Direction direction = CW;
	for(;count < steps;count++)
	{
		moveAltMotor(direction);
		sleep(100);
	}
}

void moveStepsAltCCW(long steps) {
	long count = 0;
	Direction direction = CCW;
	for(;count < steps;count++)
	{
		moveAltMotor(direction);
		sleep(100);
	}
}

void moveStepsAzCW(long steps)  {
	long count = 0;
    Direction direction = CW;
	for(;count < steps;count++)
	{
		moveAzMotor(CW);
        delayMicroseconds(5000);
	}
}

void moveStepsAzCCW(long steps) {
	long count = 0;
	Direction direction = CCW;
	for(;count < steps;count++)
	{
		moveAzMotor(CCW);
		delayMicroseconds(5000);
	}
}

void moveAz360CCW() {
	moveStepsAzCCW(52*400);
}

void moveAz360CW() {
	moveStepsAzCW((52*400)-36);
}

void moveAzCCWStep() {
       moveAzMotor(CCW);
}

void initMotors()
{	
#ifdef DEBUG  
	int show = 0;  // turn on off debugging
   	debugLogging(show,False,"Motor init\n");
#endif
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
	if(sigprocmask(SIG_SETMASK, &mask, NULL)==-1)
	{
		printf("setTaracking enable timer fail");
	}

	
	/* create timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if(timer_create(CLOCKID,&sev,&timerid)==-1)
	{
		 printf("initMotors enable goto timer fail\n");
	}


	// gotoSev.sigev_notify = SIGEV_SIGNAL;
	// gotoSev.sigev_signo = SIG;
	// gotoSev.sigev_value.sival_ptr = &gotoTimerid;
	// if(timer_create(CLOCKID,&gotoSev,&gotoTimerid)==-1)
	// {
		// printf("initMotors enable goto timer fail\n");
	// }

	// trackSev.sigev_notify = SIGEV_SIGNAL;
	// trackSev.sigev_signo = SIG;
	// trackSev.sigev_value.sival_ptr = &trackTimerid;
	// if(timer_create(CLOCKID,&trackSev,&trackTimerid)==-1)
	// {
		// printf("initMotors enable track timer fail\n");
	// }

	// userSev.sigev_notify = SIGEV_SIGNAL;
	// userSev.sigev_signo = SIG;
	// userSev.sigev_value.sival_ptr = &userTimerid;
	// if(timer_create(CLOCKID,&userSev,&userTimerid)==-1)
	// {
		// printf("initMotors enable user timer fail\n");
	// }
	
    azSlewDirection = 0;
    altSlewDirection = 0;
	altSlewDirectionPrev = altSlewDirection;	
	azSlewDirectionPrev = azSlewDirection;	


    azSlewDirectionPrev = 0;
    altSlewDirectionPrev = 0;
    
#ifdef DEBUG     
    debugLogging(show,False,"wiring \n");
#endif
    wiringPiSetupPhys();
	
	
	pinMode(STOP_PIN,INPUT);
	pinMode(RELAY_PIN,OUTPUT);
	
  	digitalWrite(RELAY_PIN,1);
	
    pinMode(ALT_DIR_PIN,OUTPUT);
    pinMode(ALT_PULSE_PIN,OUTPUT);
    pinMode(ALT_ENABLE_PIN,OUTPUT);     

    pinMode(AZ_DIR_PIN,OUTPUT);
    pinMode(AZ_PULSE_PIN,OUTPUT);
    pinMode(AZ_ENABLE_PIN,OUTPUT);
    
    pinMode(SHIFTER_ENABLE_PIN,OUTPUT);
   
    pullUpDnControl(SHIFTER_ENABLE_PIN,PUD_DOWN);
    
    pullUpDnControl(SHIFTER_ENABLE_PIN,PUD_DOWN);
   
  	digitalWrite(AZ_DIR_PIN,azSlewDirectionPrev ^ invertAzDirection);
  	digitalWrite(ALT_DIR_PIN,altSlewDirectionPrev ^ invertAltDirection);

	digitalWrite(AZ_PULSE_PIN,1 ^ invertAzPulse);
  	digitalWrite(ALT_PULSE_PIN,1 ^ invertAltPulse);
   
  	digitalWrite(SHIFTER_ENABLE_PIN,1);
  	
#ifdef DEBUG     
  	debugLogging(show,False,"test %d %d\n", invertAltEnable,1^ invertAltEnable);
#endif
    enableMotor(True);
#ifdef DEBUG     
   	debugLogging(show,False,"Motor init\n");
#endif
   
}

void enableMotor(Boolean value)
{
#ifdef DEBUG 
	int show = 0;  // turn on off debugging    
    debugLogging(show,False,"enable Motor \n");
#endif

	if(value)
	{
		digitalWrite(ALT_ENABLE_PIN,1 ^ invertAltEnable);
		digitalWrite(AZ_ENABLE_PIN,1 ^ invertAzEnable);
        debugLogging(show,False,"enable Motor %d \n",align);
		if(!align)
		{
            debugLogging(show,False,"enable Motor userMove\n");
			userMove = True;
			setUserMove();
		}
	} 
	else
	{
        debugLogging(show,False,"disable Motor %d \n",align);		
		setAlign(False);
		doTracking=False;
		doneMoving = True;
		moving = False;
		digitalWrite(ALT_ENABLE_PIN,0 ^ invertAltEnable);
		digitalWrite(AZ_ENABLE_PIN,0 ^ invertAzEnable);
	}
	

}
    
    
    
