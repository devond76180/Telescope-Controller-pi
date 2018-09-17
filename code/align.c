// test 

// ---------------------------------------------------------------------
// Program for pointing a telescope by Toshimi Taki
//
// Original published in Sky & Telescope, February 1989, pages 194-196.
//
// Source here, with errors corrected: 
// http://wwwcdn.skyandtelescope.com/wp-content/uploads/taki.bas
//
// Background on equations is on Taki's site:
// http://www.geocities.jp/toshimi_taki/aim/aim.htm
//
// Conversion to C++/Arduino by Howard Dutton, 10/12/2016
// converted to C and up Devon Davis Sept 1, 2018

#include "align.h"
#define Rad 57.29577951
#define DEBUG

int SHOW_DEBUG = False;
long secondsSinceEpocMidnight2 = 0;
int prevDay;
timer_t timerid;

//static byte alignNumStars = 0;
//static byte alignThisStar = 0;

// Initialize
void alignInit() {
  Z1=0;  // Mount error angle between horizontal axis and vertical axis
  Z2=0;  // Mount error angle between vertical axis and telescope optical axis
  Z3=0;  // Mount error angle, zero point shift (azimuth axis of rotation vs. instrument altitude angle)
  int i;
  int j;
  for(i=0;i<4;i++) {
      for(j=0;j<4;j++) {
          R[i][j] = 1;
          debugLogging(True,False,"R%d%d  %Lf r\n",i,j,R[i][j]);
      }
  }
  //
  t_ready=False;
}

// Status
Boolean alignIsReady() {
  return t_ready;
}

int static stars = 1;

double convertRAtoHA(int secondsOffset) {
    FILE *in1;
    double HA;
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
        
    
    if(secondsSinceEpocMidnight2==0 || prevDay!=day)
    {
		prevDay = day;
	    secondsSinceEpocMidnight2=0;
    
		char command2[50];
		sprintf(command2,"date --date=\"%d/%d/%d UTC\" +%%s",month,day,year);
#ifdef DEBUG 		
		debugLogging(SHOW_DEBUG,False,"%s\n",command2);
#endif
		
		while(secondsSinceEpocMidnight2 == 0)
	    {
#ifdef DEBUG 
			debugLogging(SHOW_DEBUG,False,"do seconds since epoc midnight\n");
#endif
			if((in1 = popen(command2,"r"))!=NULL)
			{
				while(fgets(buff, sizeof(buff),in1) != NULL) 
				{
					sscanf(buff,"%ld",&secondsSinceEpocMidnight2);
#ifdef DEBUG 
					debugLogging(SHOW_DEBUG,False,"command result %s\n",buff);
#endif
				}    
			}
	    }
	}
    long double offsetGmstNoon = dateHour - 12 + dateMinute/60.0f + dateSecond/3600.0f;
    long double partDayGmstNoon = offsetGmstNoon/24;
    long double dayOffset = (secondsSinceEpocMidnight2 - secondsEpoch1_1_2000)/(60.0*60.0*24.0) + partDayGmstNoon;
    //GMST noon sideral time in hms
#ifdef DEBUG 
    long double gmst = 18.697374558+24.06570982441908 * (dayOffset);
    long double gmstMod = fmod(gmst,24);
    int hour = gmstMod;
    int minute = (gmstMod - hour)* 60.0;
    long double second = ((gmstMod - hour) * 60.0 - minute) * 60.0; 
    debugLogging(SHOW_DEBUG,False,"conv inputs %Lf %Lf %Lf\r\n",dayOffset,offsetGmstNoon,partDayGmstNoon);
    debugLogging(SHOW_DEBUG,False,"gmst LST %d:%d:%Lf \r\n",hour,minute,second);
#endif
    //local sideral time offest to current gmt by 12 hours (since reference is noon
    //time then offset local longitiude.
    long double lstLong = 18.697374558+24.06570982441908 * (dayOffset) - longitudeHours;
    long double lstLongMod = fmod(lstLong,24);
    long double lstLongDegrees = lstLongMod *15;
    HA = lstLongDegrees - raDegrees;
    debugLogging(SHOW_DEBUG,False,"lst inputs %Lf %Lf %Lf %Lf\r\n",lstLong,lstLongMod,lstLongDegrees,raDegrees);    
    if(HA < 0)
       HA += 360;
 #ifdef DEBUG 
    long double haHms = HA /15;
	int hours = haHms;
	int minutes = (haHms - hours) * 60.0;
	long double seconds = ((haHms - hours) * 60.0 - minutes) * 60.0;  
    debugLogging(SHOW_DEBUG,False,"HA %Lf %dh %dm %Lfs\n",HA,hours,minutes,seconds);        

#endif
    return HA;
    
}

// no mount error
// I=1 for 1st star, I=2 for 2nd star, both must be initialized before use
// for I>2 additional stars allow for mount modeling
// N=total number of stars for this align
// B=HA(RA), D=Dec, H=Elevation (Alt), F=Azimuth (all in degrees)
void alignSetStar(int I, int N, double RA, double Dec, double Alt, double Az) {

  AlignStars[I-1].HA=RA;  AlignStars[I-1].Dec=Dec;
  AlignStars[I-1].Alt=Alt; AlignStars[I-1].Az=Az;

  // FOR >2 STARS LOG THEM AND USE TO CALCULATE Z1/Z2/Z3 TERMS
  if (I==N) {
    if (I>2) {
      // handles searching for Z3 up to +/- 10 degrees
      alignBestZ3(N,-10.0,10.0,2.0);       // 10x
      alignBestZ3(N,Z3-2.0,Z3+2.0,0.5);    // 8x
      alignBestZ3(N,Z3-0.5,Z3+0.5,0.062);  // 16x
//    bestZ12(N);
    }
  }
  if (I>2) return;

  // HAVE STAR, PREPARE
  // eq 6 and 8  X is matrix LMN
  X[1][I]=cos(Dec/Rad)*cos(RA/Rad);
  debugLogging(SHOW_DEBUG,False,"init X1%d %Lf\r\n",I,X[1][I]);  //x
  X[2][I]=cos(Dec/Rad)*sin(RA/Rad);
  debugLogging(SHOW_DEBUG,False,"init X2%d %Lf\r\n",I,X[2][I]);  //y
  X[3][I]=sin(Dec/Rad);
  debugLogging(SHOW_DEBUG,False,"init X3%d %Lf\r\n",I,X[3][I]);  //z
  
  // eq 7 and 9 Y is matrix lmn
  Az=Az/Rad;
  Alt=(Alt+Z3)/Rad;
  Y[1][I]=Y[1][0]=cos(Az)*cos(Alt);
  Y[2][I]=Y[2][0]=sin(Az)*cos(Alt);
  Y[3][I]=Y[3][0]=sin(Alt);
  
  
  if(I == 1 ) {
      return;
  }
  //
  
  t_ready=True;
  
  // Generate Third init
  //eq 11 M1N2-N1M2
  X[1][3]=X[2][1]*X[3][2]-X[3][1]*X[2][2];
  debugLogging(SHOW_DEBUG,False,"X13 %Lf\r\n",X[1][3]);  //x
  //N1M2 - L1N2
  X[2][3]=X[3][1]*X[1][2]-X[1][1]*X[3][2];
  debugLogging(SHOW_DEBUG,False,"X23 %Lf\r\n",X[2][3]);  //x
  //L1M2 - M1L2
  X[3][3]=X[1][1]*X[2][2]-X[2][1]*X[1][2];
  debugLogging(SHOW_DEBUG,False,"X33 %Lf\r\n",X[3][3]);  //x
  
  // sqrt((M1N2-N1M2)^2 + (N1M2 - L1N2)^2 + (L1M2 - M1L2)^2)
  double A=sqrt(pow(X[1][3],2.0)+pow(X[2][3],2.0)+pow(X[3][3],2.0));
  //debugLogging(SHOW_DEBUG,False,"A %Lf \r\n",A);  //x

  // final step array completes equation 11
  for (I=1; I<=3; I++) {
    X[I][3]=X[I][3]/A;
  }
  
  
  //eq 10 m1n2 - n1m2
  Y[1][3]=Y[2][1]*Y[3][2]-Y[3][1]*Y[2][2];
  debugLogging(SHOW_DEBUG,False,"Y13 %Lf\r\n",Y[1][3]);  //x
  // n1m2 - l1n2
  Y[2][3]=Y[3][1]*Y[1][2]-Y[1][1]*Y[3][2];
  debugLogging(SHOW_DEBUG,False,"Y23 %Lf\r\n",Y[2][3]);  //x
  // l1m2 - m1l2
  Y[3][3]=Y[1][1]*Y[2][2]-Y[2][1]*Y[1][2];
  debugLogging(SHOW_DEBUG,False,"Y33 %Lf\r\n",Y[3][3]);  //x
  A=sqrt(pow(Y[1][3],2.0)+pow(Y[2][3],2.0)+pow(Y[3][3],2.0));
  
  // completes eq 10 
  for (I=1; I<=3; I++) {
    Y[I][3]=Y[I][3]/A;
  }
  //
  // copies LMN into V
  // TRANSFORM MATRIX
  int J;
  for (I=1; I<=3; I++) {
    for (J=1; J<=3; J++) {
      V[I][J]=X[I][J];
    }
  }
  // gets Detminent and puts in E
  alignTDeterSub(V);
  
  //debugLogging(SHOW_DEBUG,False,"E=W %Lf\r\n",W);  //x
  double E=W;
  //
  int M;
  
  for (M=1; M<=3; M++) {
    for (I=1; I<=3; I++) {
      for (J=1; J<=3; J++) {
        V[I][J]=X[I][J];
        debugLogging(SHOW_DEBUG,False,"V%d%d  %Lf X%d%d %Lf\r\n",I,J,V[I][J],I,J,X[I][J]);  //x  
      }
    }
    for (N=1; N<=3; N++) {
      V[1][M]=0;
      V[2][M]=0;
      V[3][M]=0;
      V[N][M]=1;
      alignTDeterSub(V);
      debugLogging(SHOW_DEBUG,False,"W/E %Lf %Lf\r\n",W,E);  //x
      Q[M][N]=W/E;
    }
  }
  //
  for (I=1; I<=3; I++) {
    for (J=1; J<=3; J++) {
      R[I][J]=0;
    }
  }
  int L;
  // used for equ to inst
  for (I=1; I<=3; I++) {
    for (J=1; J<=3; J++) {
      for (L=1; L<=3; L++) {
        R[I][J]=R[I][J]+Y[I][L]*Q[L][J];
        debugLogging(SHOW_DEBUG,False,"Y%d%d  %Lf\r\n",I,L,Y[I][L]);  //x
        debugLogging(SHOW_DEBUG,False,"Q%d%d  %Lf\r\n",L,J,Q[L][J]);  //x
        debugLogging(SHOW_DEBUG,False,"R%d%d  %Lf\r\n",I,J,R[I][J]);  //x

      }
    }
  }
  //  below is used for inst to equ
  for (M=1; M<=3; M++) {
    for (I=1; I<=3; I++) {
      for (J=1; J<=3; J++) {
        V[I][J]=R[I][J];
      }
    }
    alignTDeterSub(V);
    E=W;
    for (N=1; N<=3; N++) {
      V[1][M]=0;
      V[2][M]=0;
      V[3][M]=0;
      V[N][M]=1;
      alignTDeterSub(V);
      Q[M][N]=W/E;
    }
  }
}


// I=1 for 1st star, I=2 for 2nd star, both must be initialized before use
// for I>2 additional stars allow for mount modeling
// N=total number of stars for this align
// B=HA(RA), D=Dec, H=Elevation (Alt), F=Azimuth (all in degrees)
//void alignSetStar(int I, int N, double RA, double Dec, double Alt, double Az) {
//
//  AlignStars[I-1].HA=RA;  AlignStars[I-1].Dec=Dec;
//  AlignStars[I-1].Alt=Alt; AlignStars[I-1].Az=Az;
//
//  // FOR >2 STARS LOG THEM AND USE TO CALCULATE Z1/Z2/Z3 TERMS
//  if (I==N) {
//    t_ready=True;
//    if (I>2) {
//      // handles searching for Z3 up to +/- 10 degrees
//      alignBestZ3(N,-10.0,10.0,2.0);       // 10x
//      alignBestZ3(N,Z3-2.0,Z3+2.0,0.5);    // 8x
//      alignBestZ3(N,Z3-0.5,Z3+0.5,0.062);  // 16x
////    bestZ12(N);
//    }
//  }
//  if (I>2) return;
//
//  // HAVE STAR, PREPARE
//  X[1][I]=cos(Dec/Rad)*cos(RA/Rad);
//  debugLogging(SHOW_DEBUG,False,"X1I %Lf\r\n",X[1][I]);  //x
//  X[2][I]=cos(Dec/Rad)*sin(RA/Rad);
//  debugLogging(SHOW_DEBUG,False,"X2I %Lf\r\n",X[2][I]);  //y
//  X[3][I]=sin(Dec/Rad);
//  debugLogging(SHOW_DEBUG,False,"X3I %Lf\r\n",X[3][I]);  //z
//  Az=Az/Rad;
//  Alt=(Alt+Z3)/Rad;
//  alignTSub1(Az,Alt);
//  Y[1][I]=Y[1][0];
//  Y[2][I]=Y[2][0];
//  Y[3][I]=Y[3][0];
//  //
//  X[1][3]=X[2][1]*X[3][2]-X[3][1]*X[2][2];
//  debugLogging(SHOW_DEBUG,False,"X13 %Lf\r\n",X[1][3]);  //x
//  X[2][3]=X[3][1]*X[1][2]-X[1][1]*X[3][2];
//  debugLogging(SHOW_DEBUG,False,"X23 %Lf\r\n",X[2][3]);  //x
//  X[3][3]=X[1][1]*X[2][2]-X[2][1]*X[1][2];
//  debugLogging(SHOW_DEBUG,False,"X33 %Lf\r\n",X[3][3]);  //x
//  double A=sqrt(pow(X[1][3],2.0)+pow(X[2][3],2.0)+pow(X[3][3],2.0));
//  
//  
//  for (I=1; I<=3; I++) {
//    X[I][3]=X[I][3]/A;
//  }
//  //
//  Y[1][3]=Y[2][1]*Y[3][2]-Y[3][1]*Y[2][2];
//  debugLogging(SHOW_DEBUG,False,"Y13 %Lf\r\n",Y[1][3]);  //x
//  Y[2][3]=Y[3][1]*Y[1][2]-Y[1][1]*Y[3][2];
//  debugLogging(SHOW_DEBUG,False,"Y23 %Lf\r\n",Y[2][3]);  //x
//  Y[3][3]=Y[1][1]*Y[2][2]-Y[2][1]*Y[1][2];
//  debugLogging(SHOW_DEBUG,False,"Y33 %Lf\r\n",Y[3][3]);  //x
//  A=sqrt(pow(Y[1][3],2.0)+pow(Y[2][3],2.0)+pow(Y[3][3],2.0));
//  for (I=1; I<=3; I++) {
//    Y[I][3]=Y[I][3]/A;
//  }
//  //
//  //
//  // TRANSFORM MATRIX
//  int J;
//  for (I=1; I<=3; I++) {
//    for (J=1; J<=3; J++) {
//      V[I][J]=X[I][J];
//    }
//  }
//  alignTDeterSub();
//  double E=W;
//  //
//  int M;
//  
//  for (M=1; M<=3; M++) {
//    for (I=1; I<=3; I++) {
//      for (J=1; J<=3; J++) {
//        V[I][J]=X[I][J];
//      }
//    }
//    for (N=1; N<=3; N++) {
//      V[1][M]=0;
//      V[2][M]=0;
//      V[3][M]=0;
//      V[N][M]=1;
//      alignTDeterSub();
//      Q[M][N]=W/E;
//    }
//  }
//  //
//  for (I=1; I<=3; I++) {
//    for (J=1; J<=3; J++) {
//      R[I][J]=0;
//    }
//  }
//  int L;
//  for (I=1; I<=3; I++) {
//    for (J=1; J<=3; J++) {
//      for (L=1; L<=3; L++) {
//        R[I][J]=R[I][J]+Y[I][L]*Q[L][J];
//        debugLogging(SHOW_DEBUG,False,"R%d%d  %Lf\r\n",I,J,R[I][J]);  //x
//
//      }
//    }
//  }
//  //
//  for (M=1; M<=3; M++) {
//    for (I=1; I<=3; I++) {
//      for (J=1; J<=3; J++) {
//        V[I][J]=R[I][J];
//      }
//    }
//    alignTDeterSub();
//    E=W;
//    for (N=1; N<=3; N++) {
//      V[1][M]=0;
//      V[2][M]=0;
//      V[3][M]=0;
//      V[N][M]=1;
//      alignTDeterSub();
//      Q[M][N]=W/E;
//    }
//  }
//}

void alignAddStar2(double RA, double Dec, AltAzType altAz) {
    debugLogging(True,False,"addstar2 %Lf %Lf\r\n",altAz.alt,altAz.az);  //x    
    alignSetStar(stars,stars,RA,Dec,altAz.alt,altAz.az);
    stars++;
}

void alignAddStar(double RA, double Dec, double alt, double az) {
    debugLogging(True,False,"addstar %Lf %Lf\r\n",alt,az);  //x    
    alignSetStar(stars,stars,RA,Dec,alt,az);
    stars++;
}

//  without mount error
// CONVERT EQUATORIAL --> TELESCOPE
// B=HA, D=Dec, H=Elevation(Alt), F=Azimuth(Az) (all in degrees)
AltAzType alignEquToInstr2(double HA, double Dec) {
    
    SHOW_DEBUG = True;
    AltAzType result;
    double alt = 0;
    double az = 0;
    double RaDecMatrix[4][4];
    double AltAzMatrix[4][4];
    debugLogging(SHOW_DEBUG,False,"alignEquToInstr2 %Lf %Lf\r\n",HA,Dec);  //x
    RaDecMatrix[1][1]=cos(Dec/Rad)*cos(HA/Rad);
    debugLogging(SHOW_DEBUG,False,"RaDecMatrix11 %Lf\r\n",RaDecMatrix[1][1]);  //x
    RaDecMatrix[2][1]=cos(Dec/Rad)*sin(HA/Rad);
    debugLogging(SHOW_DEBUG,False,"RaDecMatrix21 %Lf\r\n",RaDecMatrix[2][1]);  //y
    RaDecMatrix[3][1]=sin(Dec/Rad); 
    debugLogging(SHOW_DEBUG,False,"RaDecMatrix31 %Lf\r\n",RaDecMatrix[3][1]);  //z
    AltAzMatrix[1][1]=0;
    AltAzMatrix[2][1]=0;
    AltAzMatrix[3][1]=0;
    int I;
    int J;
    // ????
    for (I=1; I<=3; I++) {
      for (J=1; J<=3; J++) {
        AltAzMatrix[I][1]=AltAzMatrix[I][1]+R[I][J]*RaDecMatrix[J][1];
       debugLogging(SHOW_DEBUG,False,"alignEquToInstr2 R%d%d  %Lf RaDecMatrix%d%d %Lf  AltAzMatrix%d1 %Lf\r\n",I,J,R[I][J],J,I,RaDecMatrix[J][I], I,AltAzMatrix[I][1]);
      }
    }
    
    debugLogging(SHOW_DEBUG,False,"altaz1 %Lf %Lf\r\n",alt,az);  //x
    alignTAngleSub(&az,&alt,AltAzMatrix);
    debugLogging(SHOW_DEBUG,False,"altaz2 %Lf %Lf\r\n",alt,az);  //x
//   az=az/Rad;
//    alt=alt/Rad;
//    debugLogging(SHOW_DEBUG,False,"altaz3 %Lf %Lf\r\n",alt,az);  //x
//    Y[1][1]=cos(az)*cos(alt)
//    Y[2][1]=sin(az)*cos(alt)
//    Y[3][1]=sin(alt);
//    debugLogging(SHOW_DEBUG,False,"altaz4 %Lf %Lf\r\n",alt,az);  //x
//    alignTAngleSub(&az,&alt);
//    debugLogging(SHOW_DEBUG,False,"altaz5 %Lf %Lf\r\n",alt,az);  //x
    result.alt=alt;
    while(az < 0) az+=360;
    while(az >= 360) az -= 360;
    result.az = az;
    SHOW_DEBUG = False;
    return result; 
}


//
// CONVERT EQUATORIAL --> TELESCOPE
// B=HA, D=Dec, H=Elevation(Alt), F=Azimuth(Az) (all in degrees)
//AltAzType alignEquToInstr2(double HA, double Dec) {
//    
//    AltAzType result;
//    double alt;
//    double az;
//    X[1][1]=cos(Dec/Rad)*cos(HA/Rad);
//    debugLogging(SHOW_DEBUG,False,"X11 %Lf\r\n",X[1][1]);  //x
//    X[2][1]=cos(Dec/Rad)*sin(HA/Rad);
//    debugLogging(SHOW_DEBUG,False,"X21 %Lf\r\n",X[2][1]);  //y
//    X[3][1]=sin(Dec/Rad);
//    debugLogging(SHOW_DEBUG,False,"X31 %Lf\r\n",X[3][1]);  //z
//    Y[1][1]=0;
//    Y[2][1]=0;
//    Y[3][1]=0;
//    int I;
//    int J;
//
//    for (I=1; I<=3; I++) {
//      for (J=1; J<=3; J++) {
//        Y[I][1]=Y[I][1]+R[I][J]*X[J][1];
//       debugLogging(True,False,"R%d%d  %Lf X%d%d %Lf  Y%d1 %Lf\r\n",I,J,R[I][J],J,I,X[J][I], I,Y[I][1]);
//      }
//    }
//    
//    debugLogging(SHOW_DEBUG,False,"altaz1 %Lf %Lf\r\n",alt,az);  //x
//    alignTAngleSub(&az,&alt);
//    debugLogging(SHOW_DEBUG,False,"altaz2 %Lf %Lf\r\n",alt,az);  //x
//    az=az/Rad;
//    alt=alt/Rad;
//    debugLogging(SHOW_DEBUG,False,"altaz3 %Lf %Lf\r\n",alt,az);  //x
//    alignTSub2(az,alt);
//    debugLogging(SHOW_DEBUG,False,"altaz4 %Lf %Lf\r\n",alt,az);  //x
//    alignTAngleSub(&az,&alt);
//    debugLogging(SHOW_DEBUG,False,"altaz5 %Lf %Lf\r\n",alt,az);  //x
//    result.alt=alt-Z3;
//    result.az = az;
//  
//}


//
// CONVERT EQUATORIAL --> TELESCOPE
// B=HA, D=Dec, H=Elevation(Alt), F=Azimuth(Az) (all in degrees)
void alignEquToInstr(double HA, double Dec, double *Alt,  double *Az) {
    
    X[1][1]=cos(Dec/Rad)*cos(HA/Rad);
    //debugLogging(SHOW_DEBUG,False,"X11 %Lf\r\n",X[1][1]);  //x
    X[2][1]=cos(Dec/Rad)*sin(HA/Rad);
    //debugLogging(SHOW_DEBUG,False,"X21 %Lf\r\n",X[2][1]);  //y
    X[3][1]=sin(Dec/Rad);
    //debugLogging(SHOW_DEBUG,False,"X31 %Lf\r\n",X[3][1]);  //z
    Y[1][1]=0;
    Y[2][1]=0;
    Y[3][1]=0;
    int I;
    int J;

    for (I=1; I<=3; I++) {
      for (J=1; J<=3; J++) {
        Y[I][1]=Y[I][1]+R[I][J]*X[J][1];
       //debugLogging(SHOW_DEBUG,False,"R%d%d  %Lf X%d%d %Lf  Y%d1 %Lf\r\n",I,J,R[I][J],J,I,X[J][I], I,Y[I][1]);
      }
    }
    
    //debugLogging(SHOW_DEBUG,False,"altaz1 %Lf %Lf\r\n",*Alt,*Az);  //x
    alignTAngleSub(Az,Alt,Y);
    //debugLogging(SHOW_DEBUG,False,"altaz2 %Lf %Lf\r\n",*Alt,*Az);  //x
    *Az=*Az/Rad;
    *Alt=*Alt/Rad;
    //debugLogging(SHOW_DEBUG,False,"altaz3 %Lf %Lf\r\n",*Alt,*Az);  //x
    alignTSub2(*Az,*Alt,Y);
    //debugLogging(SHOW_DEBUG,False,"altaz4 %Lf %Lf\r\n",*Alt,*Az);  //x
    alignTAngleSub(Az,Alt,Y);
    //debugLogging(SHOW_DEBUG,False,"altaz5 %Lf %Lf\r\n",*Alt,*Az);  //x
    *Alt=*Alt-Z3;
  
}

// convert equatorial coordinates to horizon
// this takes approx. 1.4mS on a 16MHz Mega2560
AltAzType equToHor2(double HA, double Dec) {
    AltAzType result;
    double alt;
    double az;
    while (HA<0.0)    HA=HA+360.0;
    while (HA>=360.0) HA=HA-360.0;

    HA =HA/Rad;
    Dec=Dec/Rad;
    double SinAlt = (sin(Dec) * sinLat) + (cos(Dec) * cosLat * cos(HA));  
    alt   = asin(SinAlt);
    double t1=sin(HA);
    double t2=cos(HA)*sinLat-tan(Dec)*cosLat;
    az=atan2(t1,t2)*Rad;
    result.az=az+180.0;
    result.alt = alt*Rad;
    return result;
    
}

//
// CONVERT TELESCOPE --> EQUATORIAL
// H=Elevation(Alt), F=Azimuth(Az), B=HA(RA), D=Dec (all in degrees)
void alignInstrToEqu(double Alt, double Az, double *HA, double *Dec) {
  Az=Az/Rad;
  Alt=(Alt+Z3)/Rad;
  alignTSub1(Az,Alt,Y);
  X[1][1]=Y[1][0];
  X[2][1]=Y[2][0];
  X[3][1]=Y[3][0];
  Y[1][1]=0;
  Y[2][1]=0;
  Y[3][1]=0;
  int I;
  int J;
  for (I=1; I<=3; I++) {
    for (J=1; J<=3; J++) {
      Y[I][1]=Y[I][1]+Q[I][J]*X[J][1];
    }
  }
  alignTAngleSub(&Az,&Alt,Y);
  Az=Az-round(Az/360.0)*360.0;  // was INT()
  *HA=Az;
  *Dec=Alt;
}

//
// DETERMINANT SUBROUTINE
void alignTDeterSub(double V[][4]) {
    //debugLogging(SHOW_DEBUG,False,"W1 %Lf %Lf %Lf %Lf %Lf %Lf %Lf \r\n",W,V[1][1],V[2][2],V[3][3],V[1][2],V[2][3],V[3][1]);  //x
    W=  V[1][1]*V[2][2]*V[3][3]+V[1][2]*V[2][3]*V[3][1];
    //debugLogging(SHOW_DEBUG,False,"W2 %Lf\r\n",W);  //x
    W=W+V[1][3]*V[3][2]*V[2][1];
    //debugLogging(SHOW_DEBUG,False,"W3 %Lf\r\n",W);  //x
    W=W-V[1][3]*V[2][2]*V[3][1]-V[1][1]*V[3][2]*V[2][3];
    //debugLogging(SHOW_DEBUG,False,"W4 %Lf\r\n",W);  //x
    W=W-V[1][2]*V[2][1]*V[3][3];
    //debugLogging(SHOW_DEBUG,False,"W5 %Lf\r\n",W);  //x
}

//  routine is bascially chacking 
// ANGLE SUBROUTINE
void alignTAngleSub(double *Az, double *Alt, double Y[][4]) {
  debugLogging(SHOW_DEBUG,False,"altaz tsub %Lf %Lf\r\n",*Alt,*Az);  //x
  double C=sqrt(Y[1][1]*Y[1][1]+Y[2][1]*Y[2][1]);
  if ((C==0) && (Y[3][1]>0)) *Alt=90.0;
  if ((C==0) && (Y[3][1]<0)) *Alt=-90.0;
  if (C!=0) *Alt=atan(Y[3][1]/C)*Rad;
  //
  if (C==0) *Az=1000.0;
  if ((C!=0) && (Y[1][1]==0) && (Y[2][1]>0)) *Az=90.0;
  if ((C!=0) && (Y[1][1]==0) && (Y[2][1]<0)) *Az=270.0;
  if (Y[1][1]>0) *Az=atan(Y[2][1]/Y[1][1])*Rad;
  if (Y[1][1]<0) *Az=atan(Y[2][1]/Y[1][1])*Rad+180.0;
  *Az=*Az-round(*Az/360.0)*360.0; // was INT()
  debugLogging(SHOW_DEBUG,False,"altaz 2tsub %Lf %Lf\r\n",*Alt,*Az);  //x
}

//
// SUBROUTINE
void alignTSub1(double Az, double Alt, double Y[][4]) {
  debugLogging(SHOW_DEBUG,False,"altaz tsub1 %Lf %Lf\r\n",Alt,Az);  //x
  Y[1][0]=cos(Az)*cos(Alt)-sin(Az)*(Z2/Rad);
  Y[1][0]=Y[1][0]+sin(Az)*sin(Alt)*(Z1/Rad);
  Y[2][0]=sin(Az)*cos(Alt)+cos(Az)*(Z2/Rad);
  Y[2][0]=Y[2][0]-cos(Az)*sin(Alt)*(Z1/Rad);
  Y[3][0]=sin(Alt);
  debugLogging(SHOW_DEBUG,False,"altaz 2tsub1 %Lf %Lf\r\n",Alt,Az);  //x
}

//
// SUBROUTINE
void alignTSub2(double Az, double Alt, double Y[][4]) {
  debugLogging(SHOW_DEBUG,False,"altaz tsub2 %Lf %Lf\r\n",Alt,Az);  //x
  Y[1][1]=cos(Az)*cos(Alt)+sin(Az)*(Z2/Rad);
  Y[1][1]=Y[1][1]-sin(Az)*sin(Alt)*(Z1/Rad);
  Y[2][1]=sin(Az)*cos(Alt)-cos(Az)*(Z2/Rad);
  Y[2][1]=Y[2][1]+cos(Az)*sin(Alt)*(Z1/Rad);
  Y[3][1]=sin(Alt);
  debugLogging(SHOW_DEBUG,False,"altaz 2tsub2 %Lf %Lf\r\n",Alt,Az);  //x
}


double angDist(double h, double d, double h1, double d1) {
  return acos(sin(d/Rad)*sin(d1/Rad)+cos(d/Rad)*cos(d1/Rad)*cos((h1-h)/Rad))*Rad;
}

//
// BESTZ3
// nrange to range is the search area in degrees
// incr is the increment distance across that range in +/- degrees
void alignBestZ3(int N, double nrange, double range, double incr) {
  double HA1,Dec1;
  double HA2,Dec2;
  double BestZ3=180.0;
  double BestDist=180.0;

  // search
  int J;
  int K;

  for (Z3=nrange; Z3<=range; Z3+=incr) {
    // for each star
    
    for (J=0; J<N; J++) {
      alignInstrToEqu(AlignStars[J].Alt,AlignStars[J].Az,&HA1,&Dec1); 

      double Dist=0;
      for (K=0; K<N; K++) {
        if (J!=K) {
          // Star J to Star K dist catalog vs. aligned

          // catalog
          double Dist1=angDist(AlignStars[J].HA,AlignStars[J].Dec,AlignStars[K].HA,AlignStars[K].Dec);

          // aligned
          alignInstrToEqu(AlignStars[K].Alt,AlignStars[K].Az,&HA2,&Dec2);
          double Dist2=angDist(HA1,Dec1,HA2,Dec2);

          Dist+=abs(Dist1-Dist2);
        }
      }
      Dist=Dist/N; // average distance for this set
      if (Dist<BestDist) { BestZ3=Z3; BestDist=Dist; }
    }
  }

  Z3=BestZ3;
}



