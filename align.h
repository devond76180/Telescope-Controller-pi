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
// Converted to C.  Removed Eq mount stuff Sept 1, 2018


#ifndef ALIGN
#define ALIGN

#include "serverTypes.h"
//#include "globals.h"
#include "astro.h"
//#include <sys/time.h>
//#include <unistd.h>    //write
#include <time.h>


typedef struct {
  double HA;
  double Dec;
  double Alt;
  double Az;
} align_coord_t;

void alignInit();
Boolean alignIsReady();
void alignAddStar2(double RA, double Dec, AltAzType altAz);
void alignAddStar(double RA, double Dec, double alt, double az);
void alignEquToInstr(double HA, double Dec, double *Alt, double *Az);
AltAzType alignEquToInstr2(double HA, double Dec);
AltAzType equToHor2(double HA, double Dec);
double convertRAtoHA(int secondOffset);
//void alignInstrToEqu(double Alt, double Az, double *RA, double *Dec);

double Z1;
double Z2;
double Z3;
Boolean t_ready;
double W;
double Q[4][4];
double V[4][4];
double R[4][4];
double X[4][4];
double Y[4][4];

align_coord_t AlignStars[10];

void alignTDeterSub(double V[][4]);
void alignTAngleSub(double *Az, double *Alt, double Y[][4]);
void alignTSub1(double Az, double Alt, double Y[][4]);
void alignTSub2(double Az, double Alt, double Y[][4]);

void alignBestZ3(int I, double nrange, double range, double incr);

#endif

