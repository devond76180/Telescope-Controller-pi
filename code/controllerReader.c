#include "controllerReader.h"

/*
int main(int argc, char *argv[])
{
	gamepadReader();
	return 0;
}
*/

int etime = 0;
void *gamepadReader()
{
	int show = 0;  // turn on off debugging
	int showPress = 0;  // turn on off debugging
	int showAdjust = 1;

	debugLogging(show,False,"gpReader\n");
    int fd = open (gamepadFile,O_RDONLY);
    struct js_event e;

    
    while (1)
    {
        read (fd, &e, sizeof(e));
	    if(e.time != etime)
	    {
		    etime = e.time;
			debugLogging(showPress,False,"gamepad %d %d %d %d\n",e.time,e.value,e.type,e.number);
		    if(typeGamepad == FX)
		    {
			    if(e.type == GP_EVENT_BUTTON && e.value == GP_VALUE_BUTTON_PRESS)
			    {
				    switch(e.number)
				    {
					   case FX_NUMBER_BUTTON_START :
						   //enableMotor(False);
						   moveAz360CW();
					   break;
					   case FX_NUMBER_BUTTON_BACK :
						   //enableMotor(True);
						   //moveAz360CCW();
                                                   moveAzCCWStep();
					   break;
				    }
			    }
		        if(isAligned())
		        {
			        debugLogging(show,False,"fx aligned %d %d %d %d\n",e.time,e.value,e.type,e.number);
                    if(e.type == GP_EVENT_AXIS)
				    {					   
					    if(e.number == FX_NUMBER_JOYRIGHT_VERTICAL)
					    {
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							    addUpLarge = True;
							    debugLogging(showAdjust,False,"uplarge\n");
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
							    addDownLarge = True;
							    debugLogging(showAdjust,False,"downlarge\n");
						    }
							else
							{
								addDownLarge = False;
								addUpLarge = False;
							    debugLogging(showAdjust,False,"ud large off\n");
							}
					    }
					    if(e.number == FX_NUMBER_JOYRIGHT_HORIZONTAL)
					    {
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							    addLeftLarge = True;
							    debugLogging(showAdjust,False,"left large\n");
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							    addRightLarge = True;;
							    debugLogging(showAdjust,False,"right large\n");
						    }
							else
							{
								addLeftLarge = False;
								addRightLarge = False;
							    debugLogging(showAdjust,False,"rl large off\n");
							}
					    }
					    if(e.number == FX_NUMBER_JOYLEFT_VERTICAL)
					    {
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							    addUpSmall = True;
							    debugLogging(showAdjust,False,"up smalle\n");
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
								addDownSmall = True;
							    debugLogging(showAdjust,False,"down small\n");
						    } 
							else
							{
								addUpSmall = False;
								addDownSmall = False;
							    debugLogging(showAdjust,False,"ud small off\n");
							}
					    }
					    if(e.number == FX_NUMBER_JOYLEFT_HORIZONTAL)
					    {
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							    addLeftSmall = True;
							    debugLogging(showAdjust,False,"left small\n");
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							    addRightSmall = True;
							    debugLogging(showAdjust,False,"right small\n");
							}
							else
							{
								addLeftSmall = False;
								addRightSmall = False;
							    debugLogging(showAdjust,False,"rl smalloff\n");
							}
					    }
				    }
  		        }
			    else
			    {
				    if(e.type == GP_EVENT_BUTTON )
				    {
					    switch(e.number)
					    {
						    case FX_NUMBER_BUTTON_X :
							    debugLogging(show,False,"FX X\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
									holdAzRatio = xAzRatio;	
									holdAltRatio = xAltRatio;	
								}									
						    break;
						    case FX_NUMBER_BUTTON_A :
							    debugLogging(show,False,"FX A\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   holdAzRatio = aAzRatio;
								   holdAltRatio = aAltRatio;
							    }
						    break;
						    case FX_NUMBER_BUTTON_B :
							    debugLogging(show,False,"FX B\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   holdAzRatio = bAzRatio;
								   holdAltRatio = bAltRatio;
							    }
						    break;
						    case FX_NUMBER_BUTTON_Y :
							    debugLogging(show,False,"FX Y\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   holdAzRatio = yAzRatio;
								   holdAltRatio = yAltRatio;
							    }
							break;
						}
					}
				    if(e.type == GP_EVENT_AXIS)
				    {
					    //if(e.number == FX_NUMBER_THROTTLE_RIGHT)
					    //{
						//	int base = abs(e.value - 32767);
						//	debugLogging(showPress,False,"base %d\n",base);
						//	if (base < 1)  
						//	{
						//		base = 1;
						//	}
						//	holdSlowRatio = abs(yRatio - slowRatio)/65535.0 * base;
						//	debugLogging(showPress,False,"yRatio %d  slowRatio %d %f %d\n",yRatio,slowRatio,abs(yRatio - slowRatio)/65535.0,holdSlowRation);
						//	userMoveRatio = holdSlowRatio;
						//}
				        if(e.number == FX_NUMBER_JOYRIGHT_VERTICAL)
					    {
						    if(e.value > GP_VALUE_AXIS_UP  && e.value < GP_VALUE_AXIS_DOWN)
						    {
							   altUp = False;
							   altDown = False;
						    }
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							   altUp = True;
							   userMoveAltRatio = holdAltRatio;
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
							   altDown = True;
							   userMoveAltRatio = holdAltRatio;
						    }
					    }
					    if(e.number == FX_NUMBER_JOYRIGHT_HORIZONTAL)
					    {
						    if(e.value > GP_VALUE_AXIS_LEFT  && e.value < GP_VALUE_AXIS_RIGHT)
						    {
							   azLeft = False;
							   azRight = False;
						    }
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							   azLeft = True;
							   userMoveAzRatio = holdAzRatio;
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							   azRight = True;
							   userMoveAzRatio = holdAzRatio;
						    }
					    }
				        if(e.number == FX_NUMBER_JOYLEFT_VERTICAL)
					    {
						    if(e.value > GP_VALUE_AXIS_UP  && e.value < GP_VALUE_AXIS_DOWN)
						    {
							   altUp = False;
							   altDown = False;
						    }
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							   altUp = True;
							   userMoveAltRatio = holdSlowAltRatio;
							   
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
							   altDown = True;
							   userMoveAltRatio = holdSlowAltRatio;
						    }
					    }
					    if(e.number == FX_NUMBER_JOYLEFT_HORIZONTAL)
					    {
						    if(e.value > GP_VALUE_AXIS_LEFT  && e.value < GP_VALUE_AXIS_RIGHT)
						    {
							   azLeft = False;
							   azRight = False;
						    }
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							   azLeft = True;
							   userMoveAzRatio = holdSlowAzRatio;
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							   azRight = True;
							   userMoveAzRatio = holdSlowAzRatio;
						    }
					    }
				    }

			    }
		    }
		    else 
		    {
		        if(isAligned())
		        {
			        debugLogging(show,False,"snes aligned %d %d %d %d\n",e.time,e.value,e.type,e.number);
				    if(e.type == GP_EVENT_BUTTON && e.value == GP_VALUE_BUTTON_PRESS)
				    {
					    switch(e.number)
					    {
						    case SNES_NUMBER_BUTTON_X :						   
							   altOffset += altButtonKeySteps;
							   debugLogging(show,False,"altoffset %ld\n",altOffset);
						    break;
						    case SNES_NUMBER_BUTTON_A :
							   azOffset += azButtonKeySteps;
							   debugLogging(show,False,"azoffset %ld\n",azOffset);
						    break;
						    case SNES_NUMBER_BUTTON_B :
							   altOffset -= altButtonKeySteps;
							   debugLogging(show,False,"altoffset %ld\n",altOffset);
						    break;
						    case SNES_NUMBER_BUTTON_Y :
							   azOffset -= azButtonKeySteps;
							   debugLogging(show,False,"azoffset %ld\n",azOffset);
						    break;
						    case SNES_NUMBER_BUTTON_START :
							   enableMotor(False);
						    break;
						    case SNES_NUMBER_BUTTON_SELECT :
							   enableMotor(True);
						    break;
					    }
				    }
				    else if(e.type == GP_EVENT_AXIS)
				    {
					    if(e.number == SNES_NUMBER_DPAD_VERTICAL)
					    {
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							   altOffset += altJoyKeySteps;
							   debugLogging(show,False,"altoffset %ld\n",altOffset);
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
							   altOffset -=altJoyKeySteps;
							   debugLogging(show,False,"altoffset %ld\n",altOffset);
						    }
					    }
					    if(e.number == SNES_NUMBER_DPAD_HORIZONTAL)
					    {
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							   azOffset += azJoyKeySteps;
							   debugLogging(show,False,"azoffset %ld\n",azOffset);
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							   azOffset -=azJoyKeySteps;
							   debugLogging(show,False,"azoffset %ld\n",azOffset);
						    }
					    }
				    }
		        }
		        else 
                {
				    debugLogging(show,False,"snes %d %d %d %d\n",e.time,e.value,e.type,e.number);
				    if(e.type == GP_EVENT_BUTTON )
				    {
					    switch(e.number)
					    {
						    case SNES_NUMBER_BUTTON_X :
							    debugLogging(show,False,"X\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   altUp = True;
							    }
							    else
							    {
								   altUp = False;
							    }
						    break;
						    case SNES_NUMBER_BUTTON_A :
							    debugLogging(show,False,"A\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   azRight = True;
							    }
							    else
							    {
								   azRight = False;
							    }
						    break;
						    case SNES_NUMBER_BUTTON_B :
							    debugLogging(show,False,"B\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   altDown = True;
							    }
							    else
							    {
								   altDown = False;
							    }
						    break;
						    case SNES_NUMBER_BUTTON_Y :
							    debugLogging(show,False,"Y\n");
							    if(e.value == GP_VALUE_BUTTON_PRESS)
							    {
								   azLeft = True;
							    }
							    else
							    {
								   azLeft = False;
							    }
						    break;
						    case SNES_NUMBER_BUTTON_START :
							    enableMotor(False);
						    break;
						    case SNES_NUMBER_BUTTON_SELECT :
							    enableMotor(True);
						    break;
					    }
				    }
				    else if(e.type == GP_EVENT_AXIS)
				    {
					    if(e.number == SNES_NUMBER_DPAD_VERTICAL)
					    {
						    if(e.value > GP_VALUE_AXIS_UP  && e.value < GP_VALUE_AXIS_DOWN)
						    {
							   altSlowUp = False;
							   altSlowDown = False;
						    }
						    if(e.value < GP_VALUE_AXIS_UP)
						    {
							   altSlowUp = True;
						    }
						    else if (e.value > GP_VALUE_AXIS_DOWN)
						    {
							   altSlowDown = True;
						    }
					    }
					    if(e.number == SNES_NUMBER_DPAD_HORIZONTAL)
					    {
						    if(e.value > GP_VALUE_AXIS_LEFT  && e.value < GP_VALUE_AXIS_RIGHT)
						    {
							   azSlowLeft = False;
							   azSlowRight = False;
						    }
						    if(e.value < GP_VALUE_AXIS_LEFT)
						    {
							   azSlowLeft = True;
						    }
						    else if (e.value > GP_VALUE_AXIS_RIGHT)
						    {
							   azSlowRight = True;
						    }
					    }
				    }
			    }
			}
		}	   
    }
}
    
    
    
