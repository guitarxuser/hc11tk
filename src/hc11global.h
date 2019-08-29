/*--------------------------------------------------------------------
 *
 *		HC11GLOBALS.H
 *		In this module are the type definitions
 *
 *--------------------------------------------------------------------*/


/*--------GLOBAL TYPES within this modules-----------*/

typedef struct 
{
  int offset_length;
  int status;
}offset_message;

typedef struct 
{
  char buffer[10000];
  int status;
}out_message;

int fd,serial; /*global fail descriptor to perform read/write operations via serial IO*/
int my_delay(unsigned long mikros);
out_message init_modem_low_level();
out_message hc11_cmd_md(int fd,const char* command);
out_message hc11_cmd_mm(int fd,const char* memory_address,const char *memory_tag);
