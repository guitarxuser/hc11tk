/*--------------------------------------------------------------------
 *
 *		COMGLOBAL.H
 *		In this module are the type definitions
 *
 *--------------------------------------------------------------------*/

 
/*--------GLOBAL TYPES within this modules-----------*/

/*--------------D E F I N E S-----------------*/


//#define SERIAL_DEV "/dev/ttyUSB1"
#define TO_CLOSE 1
#define TO_OPEN  0
#define SERVER_KEY 10001 /*key for message q*/
#define MAX_LINE 256    /* max char for a message*/

/*------------T Y P D E F S--------------------*/

typedef struct y
{
  int offset_length;
  int status;
}offset_message;

typedef struct 
{
  char buffer[10000];
  int status;
}out_message;

typedef struct 
{
  long mtyp;
  char buffer[MAX_LINE];
}mqu_answer;

int fd; /*global fail descriptor to perform read/write operations via serial IO*/
char receiver_buffer[10000];
char *device;
struct termios options_old;

out_message init_modem_low_level(int closure);
out_message hc11_cmd_md(int fd,const char* command);
out_message hc11_cmd_mm(int fd,const char* memory_address,const char *memory_tag);
out_message send_string(int fd,const char *inp_str);
out_message receive_string(int fd, const char* out_str);
