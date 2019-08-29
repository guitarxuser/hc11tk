/* 
 * tkAppInit.c --
 *
 *	Provides a default version of the Tcl_AppInit procedure for
 *	use in wish and similar Tk-based applications.
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: tkAppInit.c,v 1.3 1999/02/04 20:57:18 stanton Exp $
 */

#include "tk.h"

#ifdef TCL_XT_TEST
#include <X11/Intrinsic.h>
#endif

#include <stdio.h>   /* Standard input/output definitions */ 
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <tcl/tcl.h>
/*-----------Defines-------------*/

#define TRUE  0
#define FALSE -1
#define ENUF(msg, value) { perror(msg); exit(value); }
/*--------------------------------------------------------------------
 *
 *		
 *		 Functions to initialize the modem
 *
 *
 *--------------------------------------------------------------------*/
/*--------GLOBAL TYPES-----------*/
      struct out_message
	{
	  char buffer[255];
          int status;
      };
                /* O - 0 = MODEM ok, -1 = MODEM bad */
/*--------------------------------*/

struct out_message init_modem(int fd)   /* I - Serial port file */
    {
      char buffer[255];  /* Input buffer */
      char *bufptr;      /* Current char in buffer */
      int  nbytes;       /* Number of bytes read */
      int  tries;        /* Number of tries so far */
      int    mod_answ,code;

      struct out_message modem_response; 
      struct termios options;
      for (tries = 0; tries < 3; tries ++)
      {
    /* open the port */
    fd = open("/dev/cua1", O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(fd, F_SETFL, 0);

    /* get the current options */
    tcgetattr(fd, &options);

    /* set raw input, 1 second timeout */
    options.c_cflag     |= (CLOCAL | CREAD);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    /* set the options */
    tcsetattr(fd, TCSANOW, &options);
       /* send an AT command followed by a CR */
        if (write(fd, "AT\r", 3) < 3)
          continue;

       /* read characters into our string buffer until we get a CR or NL */
        bufptr = buffer;
        while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
        {
          bufptr += nbytes;
          if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
            break;
        }

       /* nul terminate the string and see if we got an OK response */
        *bufptr = '\0';
	  fprintf(stdout,"buffer=%s",buffer);
        if (strncmp(buffer, "OK", 2) == 0)
	  modem_response.status = TRUE;
	strcpy(modem_response.buffer,buffer);
        return (modem_response);
      }
      modem_response.status = FALSE;
      return (modem_response);
    }

int call_init_modem(ClientData clientData,Tcl_Interp *interp,
                    int argc,char *argv[])
 {
   int fd;
   struct out_message get_modem_mess;
   char* modem_mess_buffer;
   modem_mess_buffer=get_modem_mess.buffer; 
   get_modem_mess = init_modem(fd);
   if(get_modem_mess.status == FALSE)
     {
      interp->result = "1";
     }
   else
     {
       interp->result = "0";
     }
    Tcl_LinkVar(interp,"tcl_modems_resp",(char*) &modem_mess_buffer,TCL_LINK_STRING);
   return TCL_OK;
 }
/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

extern int matherr();
int *tclDummyMathPtr = (int *) matherr;

#ifdef TK_TEST
extern int		Tktest_Init _ANSI_ARGS_((Tcl_Interp *interp));
#endif /* TK_TEST */

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(argc, argv)
    int argc;			/* Number of command-line arguments. */
    char **argv;		/* Values of command-line arguments. */
{
    Tk_Main(argc, argv, Tcl_AppInit);
    return 0;			/* Needed only to prevent compiler warning. */
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);
#ifdef TK_TEST
    if (Tktest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "Tktest", Tktest_Init,
            (Tcl_PackageInitProc *) NULL);
#endif /* TK_TEST */


    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */
    Tcl_CreateCommand(interp,"modeminit",call_init_modem,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_rcFileName", "~/.mywishrc", TCL_GLOBAL_ONLY);
    return TCL_OK;
}
