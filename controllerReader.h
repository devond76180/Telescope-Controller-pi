#include <stdio.h>
#include <stdlib.h>		/* atoi */
#include <errno.h>		/* errno */
#include <time.h>		/* CLOCKS_PER_SEC */
#include <fcntl.h>		/* open */
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//#include "serverTypes.h"
#include "motionManager.h"

#define GP_EVENT_BUTTON    0x01
#define GP_EVENT_AXIS      0x02
#define GP_EVENT_INIT      0x80

#define GP_VALUE_BUTTON_RELEASE        0x00
#define GP_VALUE_BUTTON_PRESS      0x01


#define GP_VALUE_AXIS_LEFT  -16000
#define GP_VALUE_AXIS_RIGHT 16000


#define GP_VALUE_AXIS_UP     -16000  
#define GP_VALUE_AXIS_DOWN   16000

#define SNES_NUMBER_BUTTON_X   0
#define SNES_NUMBER_BUTTON_A   1
#define SNES_NUMBER_BUTTON_B   2
#define SNES_NUMBER_BUTTON_Y   3
#define SNES_NUMBER_BUTTON_START   9
#define SNES_NUMBER_BUTTON_SELECT   8
#define SNES_NUMBER_BUTTON_LEFT   4
#define SNES_NUMBER_BUTTON_RIGHT   6

#define SNES_NUMBER_DPAD_HORIZONTAL 0
#define SNES_NUMBER_DPAD_VERTICAL  1


#define FX_NUMBER_BUTTON_A   0    
#define FX_NUMBER_BUTTON_B   1    
#define FX_NUMBER_BUTTON_X   2    
#define FX_NUMBER_BUTTON_Y   3

#define FX_NUMBER_BUTTON_LEFT 4
#define FX_NUMBER_BUTTON_RIGHT 5
#define FX_NUMBER_BUTTON_BACK 6
#define FX_NUMBER_BUTTON_START 7
#define FX_NUMBER_BUTTON_JOYLEFT 9
#define FX_NUMBER_BUTTON_JOYRIGHT 10


#define FX_NUMBER_JOYLEFT_HORIZONTAL 0
#define FX_NUMBER_JOYLEFT_VERTICAL 1

#define FX_NUMBER_JOYRIGHT_HORIZONTAL 3
#define FX_NUMBER_JOYRIGHT_VERTICAL 4

#define FX_NUMBER_THROTTLE_LEFT 2
#define FX_NUMBER_THROTTLE_RIGHT 5

#define FX_NUMBER_DPAD_HORIZONTAL 6
#define FX_NUMBER_DPAD_VERTICAL 7


struct js_event {
	__u32 time;
	__s16 value;
	__u8 type;
	__u8 number;
};
