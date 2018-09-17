/*
    C socket server example
*/
/* TODO
 * start button does not stop tracking
 * HA matches and az very close but alt is off.
 * 
 * maybe if not connecting wait 1 minute and try again.
 * handle writenet failure better.
 * 
*/ 


#include "server.h"


/* Check and act upon read/write result. Uses errno. Returns true on error. */
Boolean
IOResultError(int iobytes, const char *err, const char *eof_err)
{
    switch (iobytes) {
    case -1:
	if (errno != EWOULDBLOCK) {
	    return True;
	}
	break;
    case 0:
	return True;
	break;
    }
    return False;
}


/* Drop client connection */
void
DropConnection(SERCD_SOCKET * inSocketFd, SERCD_SOCKET * outSocketFd)
{
    if (inSocketFd) {
	close(*inSocketFd);
    }

    if (outSocketFd) {
	close(*outSocketFd);
    }
}

// set the date in the system time
void setDate(int month, int day, int year) 
{
   time_t mytime = time(0);
   struct tm* tm_ptr = localtime(&mytime);
   
   if((tm_ptr->tm_hour + utcOffset) > 24)
      day--;
   if(tm_ptr)
   {
      tm_ptr->tm_mon = month -1;
      tm_ptr->tm_mday = day;
      tm_ptr->tm_year = year + (2000 - 1900);
      const struct timeval tv = {mktime(tm_ptr),0};
      settimeofday(&tv,0);
   }
}

// set the time in the system time
void setTime(int hour, int minute, float seconds) 
{
   time_t mytime = time(0);
   struct tm* tm_ptr = localtime(&mytime);
   
   if(tm_ptr)
   {
      tm_ptr->tm_hour = hour;
      tm_ptr->tm_min = minute;
      tm_ptr->tm_sec = seconds;
      const struct timeval tv = {mktime(tm_ptr),0};
      settimeofday(&tv,0);
   }
}

// write buffer to network connection
int  
WriteNet(BufferType *b) 
{
   /* Write to network */
   ssize_t iobytes;
   iobytes = write(*outSocket, b->buffer, b->endPos);
   if (IOResultError(iobytes, "Error writing to network", "EOF to network")) {
      DropConnection(inSocket, outSocket);
      inSocket = outSocket = NULL;
      return -1;
   }
   else {
      b->endPos = 0;
      return 0;
   }
}



/* Add a byte to a buffer. */
void
AddToBuffer(BufferType *b, unsigned char c)
{

    b->buffer[b->endPos] = c;
    b->endPos = (b->endPos + 1);
}


/* Setup sockets for low latency and automatic keepalive; doesn't
 * check if anything fails because failure doesn't prevent correct
 * functioning but only provides slightly worse behaviour
 */
void
SetSocketOptions(SERCD_SOCKET insocket, SERCD_SOCKET outsocket)
{
    /* Socket setup flag */
    int SockParmEnable = 1;

    setsockopt(insocket, SOL_SOCKET, SO_KEEPALIVE, (char *) &SockParmEnable,
	       sizeof(SockParmEnable));
    setsockopt(insocket, SOL_SOCKET, SO_OOBINLINE, (char *) &SockParmEnable,
	       sizeof(SockParmEnable));
    setsockopt(outsocket, SOL_SOCKET, SO_KEEPALIVE, (char *) &SockParmEnable,
	       sizeof(SockParmEnable));
}

/* Setup sockets for low latency and automatic keepalive; doesn't
 * check if anything fails because failure doesn't prevent correct
 * functioning but only provides slightly worse behaviour
 */
void
SetSocketOption(SERCD_SOCKET socket)
{
    /* Socket setup flag */
    int SockParmEnable = 1;

    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *) &SockParmEnable,
	       sizeof(SockParmEnable));
    setsockopt(socket, SOL_SOCKET, SO_OOBINLINE, (char *) &SockParmEnable,
	       sizeof(SockParmEnable));
}


/* Send the specific telnet option to SockFd using Command as command */
#define SendTelnetOption_bytes 3
void
SendTelnetOption(BufferType *b, unsigned char command, char option)
{
    unsigned char iac = TNIAC;

    AddToBuffer(b, iac);
    AddToBuffer(b, command);
    AddToBuffer(b, option);
}

/* Send initial Telnet negotiations to the client */
#define SendTelnetInitialOptions_bytes (SendTelnetOption_bytes*3)
void
SendTelnetInitialOptions(BufferType *b)
{
    SendTelnetOption(b, TNWILL, TN_TRANSMIT_BINARY);
    tnstate[TN_TRANSMIT_BINARY].sent_will = 1;
    SendTelnetOption(b, TNDO, TN_TRANSMIT_BINARY);
    tnstate[TN_TRANSMIT_BINARY].sent_do = 1;
    SendTelnetOption(b, TNWILL, TN_ECHO);
    tnstate[TN_ECHO].sent_will = 1;
    SendTelnetOption(b, TNWILL, TN_SUPPRESS_GO_AHEAD);
    tnstate[TN_SUPPRESS_GO_AHEAD].sent_will = 1;
    SendTelnetOption(b, TNDO, TN_SUPPRESS_GO_AHEAD);
    tnstate[TN_SUPPRESS_GO_AHEAD].sent_do = 1;
    SendTelnetOption(b, TNDO, TNCOM_PORT_OPTION);
    tnstate[TNCOM_PORT_OPTION].sent_do = 1;
}


void commandGet(unsigned char *commandBuffer,int size) 
{
    if(size > 2) 
    {
       switch (*(commandBuffer+2))
       {
          case '0' :   // alignment menu entry 0
          break;
          case '1' :   // alignment menu entry 1
          break;
          case '2' :   // alignment menu entry 1
          break;
          case 'A' :   // get altitude
          break;
          case 'a' :   // get local time
          break;
          case 'b' :   // browse brighter magnitude limit
          break;
          case 'C' :   // curent date
          break;
          case 'c' :   // calendar format
          break;
          case 'D' :   // get declination
                //printf("DEC %d",precise);
                if(precise) 
                { 
                   //printf("dp %+02d*%02d'%02.0f#\n",declination.degrees,declination.minutes,declination.seconds);
                   sprintf(bufferWrite.buffer,"%+02d*%02d'%02.0f#",declinationNew.degrees,declinationNew.minutes,declinationNew.seconds);
                   bufferWrite.endPos = 10;
                   WriteNet(&bufferWrite);
                }
                else
                { 
                   //printf("dn %+02d*%02d#\n",declination.degrees,declination.minutes);
                   sprintf(bufferWrite.buffer,"%+02d*%02d#",declinationNew.degrees,declinationNew.minutes);
                   bufferWrite.endPos = 7;
                   WriteNet(&bufferWrite);
                }
                //getLocalSideralTime();
          break;
          case 'd' :   // current object/target declination
          break;
          case 'F' : 
          break;
          case 'f' : 
          break;
          case 'G' : 
          break;
          case 'g' : 
          break;
          case 'h' : 
          break;
          case 'L' : 
          break;
          case 'l' : 
          break;
          case 'M' : 
          break;
          case 'N' : 
          break;
          case 'O' : 
          break;
          case 'o' : 
          break;
          case 'P' : 
          break;
          case 'q' : 
          break;
          case 'R' :
                if(precise) 
                { 
                   //printf("rp %02d:%02d:%02.0f# \n",ra.hours,ra.minutes,ra.seconds);
                   sprintf(bufferWrite.buffer,"%02d:%02d:%02.0f#",raNew.hours,raNew.minutes,raNew.seconds);
                   bufferWrite.endPos = 9;
                   WriteNet(&bufferWrite);
                }
                else
                { 
                   //printf("rn %02d:%02.1f#\n",ra.hours,ra.minutes+ra.seconds/60.0);
                   sprintf(bufferWrite.buffer,"%02d:%02.1f#",raNew.hours,raNew.minutes+raNew.seconds/60.0);
                   bufferWrite.endPos = 8;
                   WriteNet(&bufferWrite);
                }
          break;
          case 'r' : 
          break;
          case 'S' : 
          break;
          case 's' : 
          break;
          case 'T' : 
          break;
          case 't' : 
          break;
          case 'V' : 
          break;
          case 'y' : 
          break;
          case 'Z' : 
          break;
       }
    }
}

void setSync() 
{
	
}

void commandAlign(unsigned char *commandBuffer, int size) 
{
    printf("Align\r\n");
    fflush(stdout);
}

void commandHomePosition(unsigned char *commandBuffer, int size) 
{
}

void commandMovement(unsigned char *commandBuffer, int size) 
{
	if(size > 2) 
    {
       switch (*(commandBuffer+2))
       {
		    case 'e' : 
				if(!isAligned())
				{
					azRight = True;
					userMoveAzRatio = holdAzRatio;
				}
		    break;
		    case 'n' : 
				if(!isAligned())
				{
					altUp = True;
					userMoveAltRatio = holdAltRatio;
				}
		    break;
		    case 's' : 
				if(!isAligned())
				{
					altDown = True;
					userMoveAltRatio = holdAltRatio;
				}
		    break;
		    case 'S' : 
		        if(isAligned())
		        {
				    if(!moving) 
					{
						doneMoving = False;
						setGoto(True);
			        }
		        }
		   break;
		   case 'w' : 
				if(!isAligned())
				{
					azLeft = True;
					userMoveAzRatio = holdAzRatio;
				}
		    break;
	   }
   }

}


void commandMovementStop(unsigned char *commandBuffer, int size) 
{
	if(size > 2) 
    {
       switch (*(commandBuffer+2))
       {
		    case '#' : 
		      if(!isAligned())
			  {
				  altUp = False;
				  altDown = False;
				  azRight = False;
				  azLeft = False;
			  }
		    case 'e' : 
		      if(!isAligned())
			  {
				  azRight = False;
			  }
		    break;
		    case 'n' : 
		      if(!isAligned())
			  {
				  altUp = False;
			  }
		    break;
		    case 's' : 
		      if(!isAligned())
			  {
				  altDown = False;
			  }
		    break;
		   case 'w' : 
		      if(!isAligned())
			  {
				  azLeft = False;
			  }
		    break;
	   }
   }
}

void commandSlewRate(unsigned char *commandBuffer, int size) 
{
	if(size > 2) 
    {
       switch (*(commandBuffer+2))
       {
		   
		    case 'C' : // second slowest
				holdAltRatio = aAltRatio;
				holdAzRatio = aAzRatio;
		    break;
		    case 'G' : // slowest
				holdAltRatio = xAltRatio;
				holdAzRatio = xAzRatio;
		    break;
		    case 'M' : // second fastest
				holdAltRatio = bAltRatio;
				holdAzRatio = bAzRatio;
		    break;
		    case 'S' : // fastest
				holdAltRatio = bAltRatio;
				holdAzRatio = bAzRatio;
		    break;
	   }
   }
}

void commandSet(unsigned char *commandBuffer, int size) 
{
    HMSType currentTime;
    DateType currentDate;

    if(size > 2) 
    {
        switch (*(commandBuffer+2))
        {
            case 'a' :   // set altitude
            break;
            case 'b' :   // set Brighter limit
            break;
            case 'B' :   // set buad rate ignore
            break;
            case 'C' :   // set Date
                currentDate = parseDate(commandBuffer,size);
                printf("Date %d/%d/%d\n",currentDate.month,currentDate.day,currentDate.year);
                setDate(currentDate.month,currentDate.day,currentDate.year);
                sprintf(bufferWrite.buffer,"Updating        planetary data. #");
                bufferWrite.endPos = 33;
                WriteNet(&bufferWrite);
                sleep(1);
                sprintf(bufferWrite.buffer,"                                #");
                bufferWrite.endPos = 33;
                WriteNet(&bufferWrite);          
            break;
            case 'd' :   // set declination
                declination.seconds = 0.0;  // clear seconds
                declination = parseDMS(commandBuffer,size);
                declinationDegrees = declination.degrees + declination.minutes/60.0f+declination.seconds/3600.0f;
                printf("declination %d:%d:%f \n",declination.degrees,declination.minutes,declination.seconds);
                sprintf(bufferWrite.buffer,"1#");
                bufferWrite.endPos = 2;
                WriteNet(&bufferWrite);
            break;
            case 'E' :   // set altitude
            break;
            case 'e' :   // set altitude
            break;
            case 'f' :   // set altitude
            break;
            case 'F' :   // set altitude
            break;
            case 'g' :   // set longitude
                longitude = parseLatLong(commandBuffer,size);
                longitudeDegrees = longitude.degrees + longitude.minutes/60.0f;
                longitudeHours = longitudeDegrees/15;
                printf("longitude %d:%d\n",longitude.degrees,longitude.minutes);
                sprintf(bufferWrite.buffer,"1#");
                bufferWrite.endPos = 2;
                WriteNet(&bufferWrite);
                FILE *latlong = fopen("latlong.txt","w");
                fprintf(latlong,"%d:%d\n",longitude.degrees,longitude.minutes);
                fprintf(latlong,"%d:%d\n",latitude.degrees,latitude.minutes);	
                fclose(latlong);
            break;
            case 'G' :   // set UTC
                utcOffset = parseUtcOffset(commandBuffer,size);
                printf("utc offset %f\n",utcOffset);
                sprintf(bufferWrite.buffer,"1#");
                bufferWrite.endPos = 2;
                WriteNet(&bufferWrite);
            break;
            case 'h' :   // set altitude
            break;
            case 'l' :   // set altitude
            break;
            case 'L' :   // set time
                   currentTime = parseHMS(commandBuffer,size);
                   setTime(currentTime.hours,currentTime.minutes,currentTime.seconds);
                   sprintf(bufferWrite.buffer,"1#");
                   bufferWrite.endPos = 2;
                   WriteNet(&bufferWrite);
            break;
            case 'M' :   // set altitude
            break;
            case 'N' :   // set altitude
            break;
            case 'o' :   // set altitude
            break;
            case 'O' :   // set altitude
            break;
            case 'P' :   // set altitude
            break;
            case 'q' :   // set altitude
            break;
            case 'r' :   // set RA
                  ra = parseHMS(commandBuffer,size);
                  raHours = ra.hours + ra.minutes/60.0f + ra.seconds/3600.0f;
                  raDegrees = (ra.hours + ra.minutes/60.0f + ra.seconds/3600.0f)*15;
                  printf("ra %d:%d:%f\n",ra.hours,ra.minutes,ra.seconds);
                  sprintf(bufferWrite.buffer,"1#");
                  bufferWrite.endPos = 2;
                  WriteNet(&bufferWrite);
            break;
            case 's' :   // set altitude
            break;
            case 'S' :   // set altitude
            break;
            case 't' :   // set latitiude
                latitude = parseLatLong(commandBuffer,size);
                latitudeDegrees = latitude.degrees + latitude.minutes/60.0f;
                sinLat = sin(latitudeDegrees * C_PI180);
                cosLat = cos(latitudeDegrees * C_PI180);
                C_LATCALC = (90 - latitudeDegrees) * C_PI180;
                printf("latitude %d:%d\n",latitude.degrees,latitude.minutes);
                sprintf(bufferWrite.buffer,"1#");
                bufferWrite.endPos = 2;
                WriteNet(&bufferWrite);
           break;
           case 'T' :   // set altitude
           break;
           case 'w' :   // set altitude
           break;
           case 'y' :   // set altitude
           break;
           case 'z' :   // set azimuth
           break;
        }
   }

}

void setLatitude(LatLongType lLatitude){
    latitude = lLatitude;
    latitudeDegrees = latitude.degrees + latitude.minutes/60.0f;
    sinLat = sin(latitudeDegrees * C_PI180);
    cosLat = cos(latitudeDegrees * C_PI180);
    C_LATCALC = (90 - latitudeDegrees) * C_PI180;
    printf("latitude %d:%d\n",latitude.degrees,latitude.minutes);
    
}

void testSetLongitude(LatLongType lLongitude) {
    longitude = lLongitude;
    longitudeDegrees = longitude.degrees + longitude.minutes/60.0f;
    longitudeHours = longitudeDegrees/15;
    printf("longitude %d:%d\n",longitude.degrees,longitude.minutes);
    
}

void testSetTime(HMSType currentTime) {
    setTime(currentTime.hours,currentTime.minutes,currentTime.seconds);   
}

void testSetDate(DateType currentDate) {
    printf("Date %d/%d/%d\n",currentDate.month,currentDate.day,currentDate.year);
    setDate(currentDate.month,currentDate.day,currentDate.year);
    sprintf(bufferWrite.buffer,"Updating        planetary data. #");
}

void testSetUTC (float lUtcOffset) {
    utcOffset = lUtcOffset;
    printf("utc offset %f\n",utcOffset);
    
}

void testSetRA (HMSType lRA) {
    ra = lRA;
    raHours = ra.hours + ra.minutes/60.0f + ra.seconds/3600.0f;
    raDegrees = (ra.hours + ra.minutes/60.0f + ra.seconds/3600.0f)*15;
    printf("ra %d:%d:%f\n",ra.hours,ra.minutes,ra.seconds);  
}

void testSetDeclination (DMSType lDeclination) {
    declination.seconds = 0.0;
    declination = lDeclination;
    declinationDegrees = declination.degrees + declination.minutes/60.0f + declination.seconds/3600.0f;
    printf("declination %d:%d:%f \n",declination.degrees,declination.minutes,declination.seconds);    
}

void processRequest(BufferType buffer) 
{
    unsigned char *commandBuffer = buffer.buffer;
    int size = buffer.endPos;
    // check fo ACK (0x06)
    printf("Command %s\n",commandBuffer); 
    if(*(commandBuffer+0) == 0x06) 
    {
    }
    if(size > 1) 
    {
       switch (*(commandBuffer+1))
       {
          case 'A' :   //Alignment Commands
          commandAlign(commandBuffer,size);
          break;
          case '$' :   // Backlash $B or smart drive $Q return nothing ignore
          break;
          case 'B' :   //Retical ignore
          break;
          //THIS IS ALIGN COMMAND
          case 'C' :   // Sync Control return "M31 EX GAL MAG 3.5 SZ!78.0'#"  
             if(*(commandBuffer+2) == 'M') 
             {
                 // set align with add starts to alignment
                setAlign(True);
                printf("CM response\n");
                sprintf(bufferWrite.buffer," M31 EX GAL MAG 3.5 SZ178.0'#");
                bufferWrite.endPos = 29;
                WriteNet(&bufferWrite);
            }
          break;
          case 'D' :   // distance bars ditance bars string one bar until slew complete then null
          break;
          case 'f' :   // fan command return nothing ignore for now
          break;
          case 'F' :   // focuser control return nothing ignore for now
          break;
          case 'g' :   // gps ignore
          break;
          case 'G' :   // get info
             commandGet(commandBuffer,size);
          break;
          case 'h' :   // :h?# home position rest ignore
             commandHomePosition(commandBuffer,size);
          break;
          case 'H' :   // time format toggle 12 24 hour time format
          break;
          case 'I' :   // init telescope maybe?
          break;
          case 'L' :   // Object Library really does nothing Skysafri should do this
          break;
          case 'M' :   // movement commands
             commandMovement(commandBuffer,size);
          break;
          case 'P' :   // precision toggle
             precise = ~precise;
             if(precise) 
             {
                sprintf(bufferWrite.buffer,"%s","HIGH PRESICION");
                bufferWrite.endPos = 15;
                WriteNet(&bufferWrite);
             }
             else
             {
                sprintf(bufferWrite.buffer,"%s","LOW PRESICION");
                bufferWrite.endPos = 14;
                WriteNet(&bufferWrite);
            }
          break;
          case 'Q' :   //halt movement commands
              commandMovementStop(commandBuffer,size);
         break;
          case 'r' :   // Field derotator
          break;
          case 'R' :   // slew rate commands
              commandSlewRate(commandBuffer,size);
          break;
          case 'S' :   // set commands
             commandSet(commandBuffer,size);
          break;
          case 'T' :   // tracking commands
          break;
          case 'U' :   // precision toggle
             //printf("presicion toggle");
             precise = ~precise;
          break;
          case 'W' :   // site select
          break;
       }
    }
}


BufferType buffer1;
BufferType buffer2;
BufferType buffer3;
int error = 0;

int main(int argc, char * argv[])
{

    moving = False;
    doneMoving = False;
    altUp = False;
    altDown = False;
    azLeft = False;
    azRight = False;
	azStepsPerDegree = 0;
	altStepsPerDegree = 0;
	invertAltDirection = 0;
	invertAzDirection = 0;
	invertAltEnable = 0;
	invertAzEnable = 0;
	invertAltPulse = 0;
	invertAzPulse = 0;
    holdAltRatio =1;
	yAltRatio = 1;
	xAltRatio = 1;
	bAltRatio = 1;
	aAltRatio = 1;

    holdAzRatio =1;
	yAzRatio = 1;
	xAzRatio = 1;
	bAzRatio = 1;
	aAzRatio = 1;

	// alt az conversion constants
	C_PI180 = M_PI/180;
	C_180PI = 180/M_PI;
	C_LATCALC = 90*(M_PI/180);

    azBigTooth = 1;
    azSmallTooth = 1;
    azStepsPerRev = 1;
    
    altBigTooth = 1;
    altSmallTooth = 1;
    altStepsPerRev = 1;


    int ret = nice(-20);
	
    debugLog = fopen("/var/log/debuf.txt","a");
  


	char *command1;
	command1 = "date --date=\"1/1/2000 UTC\" +%s";

    
	FILE *in1;
	char buff[512];
	if((in1 = popen(command1,"r"))!=NULL)
	{
		while(fgets(buff, sizeof(buff),in1) != NULL) 
		{
			sscanf(buff,"%ld",&secondsEpoch1_1_2000);
	    }
	}
    
    fclose(in1);
    
    readConfig();
    
    FILE *latlong = fopen("/etc/tc/latlong.txt","r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if((latlong != NULL))
    {
		if((read = getline(&line,&len,latlong)) != -1)
		{
			 longitude = parseLatLongFile(line,len);
			 longitudeDegrees = longitude.degrees + longitude.minutes/60.0f;
			 longitudeHours = longitudeDegrees/15;
			 //printf("long %d : %d",longitude.degrees,longitude.minutes);			
		}
		if((read = getline(&line,&len,latlong)) != -1)
		{
			 latitude = parseLatLongFile(line,len);
			 latitudeDegrees = latitude.degrees + latitude.minutes/60.0f;
		     C_LATCALC = (90 - latitudeDegrees) * C_PI180;
			 //printf("lat %d : %d",latitude.degrees,latitude.minutes);			
		}		
		fclose(latlong);
	}
	
    initMotors();

    azSteps360 = azStepsPerRev*(azBigTooth*1.0/azSmallTooth);
    printf("azSteps360 %d\r\n",azSteps360);
    float correction = correctionPercentAz/100;
    azSteps360 += azSteps360 * correction;
    printf("azSteps360 %d correction %f\r\n",azSteps360,correction);
    azStepsPerDegree = azSteps360/360.0;
    azStepsPerArcminute = azStepsPerDegree/60.0;
    azStepsPerArcSecond = azStepsPerArcminute/60.0;
    
    correction = correctionPercentAlt/100;
    long altSteps360 = altStepsPerRev*(altBigTooth*1.0/altSmallTooth);
    printf("altSteps360 %d\r\n",altSteps360);
    altSteps360 += altSteps360 * correction;
    printf("altSteps360 %d correction %f\r\n",altSteps360,correction);
    altStepsPerDegree = altSteps360/360.0;
    altStepsPerArcminute = altStepsPerDegree/60.0;
    altStepsPerArcSecond = altStepsPerArcminute/60.0;
    
    printf("az %d %d %d %0.15f %0.15f %0.15f\n",azStepsPerRev,azBigTooth,azSmallTooth,azStepsPerDegree,azStepsPerArcminute,azStepsPerArcSecond);
    printf("alt %d %d %d %0.15f %0.15f %0.15f\n",altStepsPerRev,altBigTooth,altSmallTooth,altStepsPerDegree,altStepsPerArcminute,altStepsPerArcSecond);
   
    free(line);
	
	if(yAltRatio > 1)
	{
		holdAltRatio = yAltRatio;
	}
    holdSlowAltRatio = slowAltRatio;
	
	if(yAzRatio > 1)
	{
		holdAzRatio = yAzRatio;
	}
    holdSlowAzRatio = slowAzRatio;
	
    altitude.degrees = 90;
    altitude.minutes = 0;
    altitude.seconds = 0;
    
    azimuth.degrees = 90;
    azimuth.minutes = 0;
    azimuth.seconds = 0;
    buffer1.endPos = 0;
    buffer2.endPos = 0;
    buffer3.endPos = 0;
    int gamepadThreadStatus;
    pthread_t gamepadThread;
    if( (gamepadThreadStatus = pthread_create(&gamepadThread, NULL, &gamepadReader,NULL)) )
    {
       printf("Receive Thread Creation Failed %d\n",gamepadThreadStatus);
    }
   
    int threadStatus;
    pthread_t receiveThread;
    if( (threadStatus = pthread_create(&receiveThread, NULL, &HandleSocketRead,NULL)) )
    {
       printf("Receive Thread Creation Failed %d\n",threadStatus);
    }
    alignInit();
    HMSType time;
    time.hours = 6;
    time.minutes = 30;
    time.seconds = 30;
    testSetTime(time);
    DateType date;
    date.day = 11;
    date.month = 9;
    date.year = 2018;
    testSetDate(date);
    testSetUTC(-6);
    float fLat = 33.022232;
    LatLongType latitude;
    latitude.degrees = 33;
    latitude.minutes = 1.33392;
    setLatitude(latitude);
    
    float fLongitude = -97.077921;
    LatLongType longitude;
    longitude.degrees = -97;
    longitude.minutes = 4.67526;
    testSetLongitude(longitude);
    //Mirfak
    HMSType RA;
    RA.hours = 03;
    RA.minutes = 25;
    RA.seconds = 39.99;
    testSetRA(RA);
    
    DMSType Declination;
    Declination.degrees = 49;
    Declination.minutes = 55;
    Declination.seconds = 27.8;
    testSetDeclination(Declination);
    setGoto(True);
    
    sleep(10);

    setAlign(True);
    azOffset = 10;
    altOffset = 10;
    
    //Capella
    RA.hours = 05;
    RA.minutes = 18;
    RA.seconds = 03.08;
    testSetRA(RA);
    Declination.degrees = 46;
    Declination.minutes = 00;
    Declination.seconds = 56.7;
    testSetDeclination(Declination);
    
    setAlign(True);
    
    //setAlign(True);
    // Poluxx
    RA.hours = 07;
    RA.minutes = 46;
    RA.seconds = 27.21;
    testSetRA(RA);
    Declination.degrees = 27;
    Declination.minutes = 58;
    Declination.seconds = 45.8;
    testSetDeclination(Declination);
    
    //setAlign(True);
    
    
    while(1) {
       if(buffer1.endPos > 0)
       {
		  //printf("buffer1");
          processRequest(buffer1);
          buffer1.endPos = 0;
       }
       if(buffer2.endPos > 0)
       {
 		  //printf("buffer2");
          processRequest(buffer2);
          buffer2.endPos = 0;
       }
       if(buffer3.endPos > 0)
       {
 		  //printf("buffer3");
          processRequest(buffer3);
          buffer3.endPos = 0;
       }
    }
    return 0;
}

void *HandleSocketRead()
{
    int socketDesc , c , readSize;
    SERCD_SOCKET clientSock; 
    struct sockaddr_in server , client;
    char clientMessage[BUFFER_SIZE];
	Boolean resetSocket = True;
	int option = 1;
	Boolean acceptfailed = False;
	int failedCount = 0;
    
	while(True)
	{
		if(resetSocket)
		{
			resetSocket = False;
			//Create socket
			socketDesc = socket(AF_INET , SOCK_STREAM , 0);
			if (socketDesc == -1)
			{
				printf("Could not create socket");
			}
			puts("Socket created");
			 
			//Prepare the sockaddr_in structure
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons( 4030 );
			 
			if(setsockopt(socketDesc,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
			{

				printf("setsockopt failed\n");
				close(socketDesc);
				exit(2);

		    }
			//Bind
			if( bind(socketDesc,(struct sockaddr *)&server , sizeof(server)) < 0)
			{
				//print the error message
				perror("bind failed. Error");
				error = 1;
				debugLogging(1,False,"%s","bind failed\n");
				return 0;
			}
			//Listen
			listen(socketDesc , 3);
			 
			//Accept and incoming connection
			puts("Waiting for incoming connections...");
			c = sizeof(struct sockaddr_in);
		}
       //accept connection from an incoming client
       clientSock = accept(socketDesc, (struct sockaddr *)&client, (socklen_t*)&c);
       //SetSocketOption(clientSock);
       if (clientSock < 0)
       {
		  failedCount++;
		  if(!acceptfailed)
		  {
			 debugLogging(1,False,"accept failed ","");
		  }
		  acceptfailed = True;
       }
       else
       {
		   if(acceptfailed)
		   {
			 debugLogging(1,False,"accept failed %d times",failedCount);
			 failedCount = 0;
		   }   
		   acceptfailed = False;
		   //puts("Connection accepted");
		   outSocket = inSocket = &clientSock;
		   //Receive a message from client
		   while( (readSize = recv(clientSock , clientMessage , 2000 , 0)) > 0 )
		   {
			   clientMessage[readSize] = 0;
			   //debugLogging(1,False,"%s\n",clientMessage);
			   unsigned char *localBuffer;
			   int *localBufferSize;
			   if(buffer1.endPos == 0) 
			   {
				   localBuffer = buffer1.buffer;
				   localBufferSize = &(buffer1.endPos);
			   } 
			   else if(buffer2.endPos == 0) 
			   {
				   localBuffer = buffer2.buffer;
				   localBufferSize = &(buffer2.endPos);
			   }
			   else 
			   {
				   localBuffer = buffer3.buffer;
				   localBufferSize = &(buffer3.endPos);
			   }
			   int index;
			   for(index = 0;index < readSize;index++) 
			   {
				  localBuffer[index] = clientMessage[index];
			   }
			   *localBufferSize = readSize;
		   }
		   if(readSize == 0)
		   {
			  //debugLogging(1,False,"Cdc\n","");
			  fflush(stdout);
			  //close(clientSock);
		   }
		   else if(readSize == -1)
		   {
			  debugLogging(1,False,"read error %d\n",readSize);
			  debugLogging(1,True,"read error %d\n",readSize);
			  //error = errno;
			  //close(clientSock);
			  //resetSocket = True;
			  //while(True);
		  }
      }
    }
    return 0;
}


