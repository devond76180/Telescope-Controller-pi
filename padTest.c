#include <stdio.h>
#include <stdlib.h>		/* atoi */
#include <errno.h>		/* errno */
#include <time.h>		/* CLOCKS_PER_SEC */
#include <fcntl.h>		/* open */
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define JS_EVENT_BUTTON    0x01
#define JS_EVENT_AXIS      0x02
#define JS_EVENT_INIT      0x80

#define JS_VALUE_BUTTON_RELEASE        0x00
#define JS_VALUE_BUTTON_PRESS      0x01


#define JS_VALUE_AXIS_LEFT  -32767
#define JS_VALUE_AXIS_RIGHT 32676

#define JS_VALUE_AXIS_UP     -32767  
#define JS_VALUE_AXIS_DOWN   32767

#define JS_NUMBER_BUTTON_Y   0
#define JS_NUMBER_BUTTON_A   1
#define JS_NUMBER_BUTTON_B   2
#define JS_NUMBER_BUTTON_X   3
#define JS_NUMBER_BUTTON_START   9
#define JS_NUMBER_BUTTON_SELECT   8
#define JS_NUMBER_BUTTON_LEFT   4
#define JS_NUMBER_BUTTON_RIGHT   6

#define JS_NUMBER_AXIS_HORIZONTAL 0
#define JS_NUMBER_AXIS_VERTICAL  1

struct js_event {
	__u32 time;
	__s16 value;
	__u8 type;
	__u8 number;
};




int main(int argc, char * argv[])
{
    int fd = open ("/dev/input/js0",O_RDONLY);
    struct js_event e;
    while (1)
    {
       read (fd, &e, sizeof(e));
       printf("%d %d %d %d\n",e.time,e.value,e.type,e.number);
    }
}
    
    
    
