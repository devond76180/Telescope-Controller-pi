#include "parseUtils.h"
#include <stdio.h>
//#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <stdlib.h>		/* atoi */
#include <errno.h>		/* errno */
//#include <time.h>		/* CLOCKS_PER_SEC */
//#include <fcntl.h>		/* open */
//#include <assert.h>		/* assert */
#include <syslog.h>
#include <pthread.h>
//#include <ctype.h>
#include "motionManager.h"
#include "readConfig.h"
#include "serverTypes.h"

#define SERCD_SOCKET int



#define WAIT_CONNECT 0
#define CONNECTED 1
#define INIT_SENT 2


/* Maximum length of temporary strings */
#define TmpStrLen 255

/* Error conditions constants */
#define NoError 0
#define Error 1
#define OpenError -1

/* Base Telnet protocol constants (STD 8) */
#define TNSE ((unsigned char) 240)
#define TNNOP ((unsigned char) 241)
#define TNSB ((unsigned char) 250)
#define TNWILL ((unsigned char) 251)
#define TNWONT ((unsigned char) 252)
#define TNDO ((unsigned char) 253)
#define TNDONT ((unsigned char) 254)
#define TNIAC ((unsigned char) 255)

/* Base Telnet protocol options constants (STD 27, STD 28, STD 29) */
#define TN_TRANSMIT_BINARY ((unsigned char) 0)
#define TN_ECHO ((unsigned char) 1)
#define TN_SUPPRESS_GO_AHEAD ((unsigned char) 3)

/* Base Telnet Com Port Control (CPC) protocol constants (RFC 2217) */
#define TNCOM_PORT_OPTION ((unsigned char) 44)

/* CPC Client to Access Server constants */
#define TNCAS_SIGNATURE ((unsigned char) 0)
#define TNCAS_SET_BAUDRATE ((unsigned char) 1)
#define TNCAS_SET_DATASIZE ((unsigned char) 2)
#define TNCAS_SET_PARITY ((unsigned char) 3)
#define TNCAS_SET_STOPSIZE ((unsigned char) 4)
#define TNCAS_SET_CONTROL ((unsigned char) 5)
#define TNCAS_NOTIFY_LINESTATE ((unsigned char) 6)
#define TNCAS_NOTIFY_MODEMSTATE ((unsigned char) 7)
#define TNCAS_FLOWCONTROL_SUSPEND ((unsigned char) 8)
#define TNCAS_FLOWCONTROL_RESUME ((unsigned char) 9)
#define TNCAS_SET_LINESTATE_MASK ((unsigned char) 10)
#define TNCAS_SET_MODEMSTATE_MASK ((unsigned char) 11)
#define TNCAS_PURGE_DATA ((unsigned char) 12)

/* CPC Access Server to Client constants */
#define TNASC_SIGNATURE ((unsigned char) 100)
#define TNASC_SET_BAUDRATE ((unsigned char) 101)
#define TNASC_SET_DATASIZE ((unsigned char) 102)
#define TNASC_SET_PARITY ((unsigned char) 103)
#define TNASC_SET_STOPSIZE ((unsigned char) 104)
#define TNASC_SET_CONTROL ((unsigned char) 105)
#define TNASC_NOTIFY_LINESTATE ((unsigned char) 106)
#define TNASC_NOTIFY_MODEMSTATE ((unsigned char) 107)
#define TNASC_FLOWCONTROL_SUSPEND ((unsigned char) 108)
#define TNASC_FLOWCONTROL_RESUME ((unsigned char) 109)
#define TNASC_SET_LINESTATE_MASK ((unsigned char) 110)
#define TNASC_SET_MODEMSTATE_MASK ((unsigned char) 111)
#define TNASC_PURGE_DATA ((unsigned char) 112)

/* CPC set parity */
#define TNCOM_PARITY_REQUEST ((unsigned char) 0)
#define TNCOM_NOPARITY ((unsigned char) 1)
#define TNCOM_ODDPARITY ((unsigned char) 2)
#define TNCOM_EVENPARITY ((unsigned char) 3)
#define TNCOM_MARKPARITY ((unsigned char) 4)
#define TNCOM_SPACEPARITY ((unsigned char) 5)

/* CPC set stopsize */
#define TNCOM_STOPSIZE_REQUEST ((unsigned char) 0)
#define TNCOM_ONESTOPBIT ((unsigned char) 1)
#define TNCOM_TWOSTOPBITS ((unsigned char) 2)
#define TNCOM_ONE5STOPBITS ((unsigned char) 3)

/* CPC commands */
#define TNCOM_CMD_FLOW_REQ ((unsigned char) 0)
#define TNCOM_CMD_FLOW_NONE ((unsigned char) 1)
#define TNCOM_CMD_FLOW_XONXOFF ((unsigned char) 2)
#define TNCOM_CMD_FLOW_HARDWARE ((unsigned char) 3)
#define TNCOM_CMD_BREAK_REQ ((unsigned char) 4)
#define TNCOM_CMD_BREAK_ON ((unsigned char) 5)
#define TNCOM_CMD_BREAK_OFF ((unsigned char) 6)
#define TNCOM_CMD_DTR_REQ ((unsigned char) 7)
#define TNCOM_CMD_DTR_ON ((unsigned char) 8)
#define TNCOM_CMD_DTR_OFF ((unsigned char) 9)
#define TNCOM_CMD_RTS_REQ ((unsigned char) 10)
#define TNCOM_CMD_RTS_ON ((unsigned char) 11)
#define TNCOM_CMD_RTS_OFF ((unsigned char) 12)
#define TNCOM_CMD_INFLOW_REQ ((unsigned char) 13)
#define TNCOM_CMD_INFLOW_NONE ((unsigned char) 14)
#define TNCOM_CMD_INFLOW_XONXOFF ((unsigned char) 15)
#define TNCOM_CMD_INFLOW_HARDWARE ((unsigned char) 16)
#define TNCOM_CMD_FLOW_DCD ((unsigned char) 17)
#define TNCOM_CMD_INFLOW_DTR ((unsigned char) 18)
#define TNCOM_CMD_FLOW_DSR ((unsigned char) 19)

/* CPC linestate mask and notifies */
#define TNCOM_LINEMASK_TIMEOUT ((unsigned char) 128)
#define TNCOM_LINEMASK_TRANS_SHIFT_EMTPY ((unsigned char) 64)
#define TNCOM_LINEMASK_TRANS_HOLD_EMPTY ((unsigned char) 32)
#define TNCOM_LINEMASK_BREAK_ERR ((unsigned char) 16)
#define TNCOM_LINEMASK_FRAME_ERR ((unsigned char) 8)
#define TNCOM_LINEMASK_PARITY_ERR ((unsigned char) 4)
#define TNCOM_LINEMASK_OVERRUN_ERR ((unsigned char) 2)
#define TNCOM_LINEMASK_DATA_RD ((unsigned char) 1)

/* CPC modemstate mask and notifies */
#define TNCOM_MODMASK_RLSD ((unsigned char) 128)
#define TNCOM_MODMASK_RING ((unsigned char) 64)
#define TNCOM_MODMASK_DSR ((unsigned char) 32)
#define TNCOM_MODMASK_CTS ((unsigned char) 16)
#define TNCOM_MODMASK_NODELTA (TNCOM_MODMASK_RLSD|TNCOM_MODMASK_RING|TNCOM_MODMASK_DSR|TNCOM_MODMASK_CTS)
#define TNCOM_MODMASK_RLSD_DELTA ((unsigned char) 8)
#define TNCOM_MODMASK_RING_TRAIL ((unsigned char) 4)
#define TNCOM_MODMASK_DSR_DELTA ((unsigned char) 2)
#define TNCOM_MODMASK_CTS_DELTA ((unsigned char) 1)

/* CPC purge data */
#define TNCOM_PURGE_RX ((unsigned char) 1)
#define TNCOM_PURGE_TX ((unsigned char) 2)
#define TNCOM_PURGE_BOTH ((unsigned char) 3)



/* Telnet State Machine */
static struct _tnstate
{
    int sent_will:1;
    int sent_do:1;
    int sent_wont:1;
    int sent_dont:1;
    int is_will:1;
    int is_do:1;
}
tnstate[256];


Boolean precise = False;
Boolean StdErrLogging = False;
SERCD_SOCKET *outSocket, *inSocket;
#define LOG_DEBUG       7
int MaxLogLevel = LOG_DEBUG + 1;


BufferType bufferWrite;



/* get information from socket */
void *HandleSocketRead();

/* Initialize a buffer for operation */
void InitBuffer(BufferType *b);


/* Add a byte to a buffer */
void AddToBuffer(BufferType *b, unsigned char c);


/* Send the specific telnet option to SockFd using Command as command */
void SendTelnetOption(BufferType *b, unsigned char command, char option);

/* Generic log function with log level control. Uses the same log levels
of the syslog(3) system call */
void
LogMsg(int LogLevel, const char *const Msg);

/* Check and act upon read/write result. Uses errno. Returns true on error. */
Boolean
IOResultError(int iobytes, const char *err, const char *eof_err);

/* Drop client connection */
void
DropConnection(SERCD_SOCKET * inSocketFd, SERCD_SOCKET * outSocketFd);

// set the date in the system time
void setDate(int month, int day, int year);

// set the time in the system time
void setTime(int hour, int minute, float seconds) ;

// write buffer to network connection
int  
WriteNet(BufferType *b);

// get the local sideral time first step radec to altaz
void getLocalSideralTime();

