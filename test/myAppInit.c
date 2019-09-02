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
 * RCS: @(#) $Id: myAppInit.c,v 1.7 2001/12/22 20:47:13 root Exp $
 */
#include <tcl.h>    /* tcl includes*/
#include <tk.h>
#include "hc11global.h"
#ifdef TCL_XT_TEST
#include <X11/Intrinsic.h>
#endif

#include <stdio.h>   /* Standard input/output definitions */ 
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

//#include <tcl8.6/tcl.h>    /* tcl includes*/
/*#include "hc11calls.c"*/
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

/*Functions*/
 out_message init_modem(int fd)   /* I - Serial port file */
{
  out_message modem_response; 
  modem_response = init_modem_low_level(fd);
  return (modem_response);
}
/*****************************************************************/
/*                                                               */
/*This Function reading the memory start address for memory dump */
/*                                                               */
/****************************************************************/
 out_message get_mem_dump(int fd,Tcl_Interp *interp)   /* I - Serial port file */
{
  out_message modem_response; 
  const char *c_mem_address_ptr;
  char c_mem_address_array[4];

  c_mem_address_ptr=c_mem_address_array;

  Tcl_LinkVar(interp,"tcl_mem_address",(char*) &c_mem_address_ptr,TCL_LINK_STRING);
  c_mem_address_ptr = Tcl_GetVar(interp,"tcl_mem_read_address",TCL_LEAVE_ERR_MSG);
  modem_response = hc11_cmd_md(fd,c_mem_address_ptr);
  Tcl_UnlinkVar(interp,"tcl_mem_address");
  return (modem_response);
}
/*****************************************************************/
/*                                                               */
/*This Function is calling the memory modify address  function    */
/*                                                               */
/****************************************************************/
 out_message get_mem_mod(int fd,Tcl_Interp *interp)   /* I - Serial port file */
{
  out_message modem_response; 
  const char *c_mem_address_ptr;
  const char *c_mem_tag_ptr;
  char c_mem_address_array[4];
  char c_mem_tag_array[2];

  c_mem_address_ptr=c_mem_address_array;
  c_mem_tag_ptr=c_mem_tag_array; 
  /*c_mem_address_ptr="4000"; initialize the address array*/

  /*  Tcl_LinkVar(interp,"tcl_mem_address",(char*) &c_mem_address_ptr,TCL_LINK_STRING);*/
  c_mem_address_ptr = Tcl_GetVar(interp,"tcl_mem_read_address",TCL_LEAVE_ERR_MSG);
  c_mem_tag_ptr = Tcl_GetVar(interp,"tcl_mem_tag",TCL_LEAVE_ERR_MSG);
  modem_response = hc11_cmd_mm(fd,c_mem_address_ptr,c_mem_tag_ptr);
  /*Tcl_UnlinkVar(interp,"tcl_mem_address");*/
  return (modem_response);
}
/*****************************************************************/
/*                                                               */
/*This Function initializes the board                            */
/*                                                               */
/****************************************************************/

int call_init_modem(ClientData clientData,Tcl_Interp *interp,
                    int argc,char *argv[])
{

  out_message get_modem_mess;
  char* modem_mess_buffer;

  modem_mess_buffer=get_modem_mess.buffer; 
  get_modem_mess = init_modem(fd);
    if(get_modem_mess.status == FALSE)
    {
    	Tcl_GetStringResult(interp);
     // interp->result = "1";
    }
  else
    {
    //  interp->result = "0";
	  Tcl_GetStringResult(interp);
      }
  Tcl_LinkVar(interp,"tcl_modems_resp",(char*) &modem_mess_buffer,TCL_LINK_STRING);
  return TCL_OK;

}
/*****************************************************************/
/*                                                               */
/*This is showing the memory dump at the assigned address       */
/*                                                               */
/****************************************************************/
int call_get_mem_dump(ClientData clientData,Tcl_Interp *interp,
                    int argc,char *argv[])
{

  out_message get_modem_mess;
  char* modem_mess_buffer;

  // Tcl_LinkVar(interp,"tcl_modems_resp",(char*) &modem_mess_buffer,TCL_LINK_STRING);
  modem_mess_buffer=get_modem_mess.buffer; 
  get_modem_mess = get_mem_dump(fd,interp);
  if(get_modem_mess.status == FALSE)
    {
     // interp->result = "1";
	  Tcl_GetStringResult(interp);
    }
  else
    {
      //interp->result = "0";
	  Tcl_GetStringResult(interp);
    }
  Tcl_LinkVar(interp,"tcl_mem_dump_resp",(char*) &modem_mess_buffer,TCL_LINK_STRING);
  return TCL_OK;

}

/*****************************************************************/
/*                                                               */
/*This is modifying the memory dump at the assigned address      */
/*                                                               */
/****************************************************************/
int call_get_mem_mod(ClientData clientData,Tcl_Interp *interp,
                    int argc,char *argv[])
{

  out_message get_modem_mess;
  char* modem_mess_buffer;

  modem_mess_buffer=get_modem_mess.buffer; 
  get_modem_mess = get_mem_mod(fd,interp);
  if(get_modem_mess.status == FALSE)
    {
     // interp->result = "1";
      Tcl_GetStringResult(interp);
    }
  else
    {
     // interp->result = "0";
      Tcl_GetStringResult(interp);
    }
  Tcl_LinkVar(interp,"tcl_mem_mod_resp",(char*) &modem_mess_buffer,TCL_LINK_STRING);
  return TCL_OK;

}
/*Unlik the created links between tcl and tk*/

int call_unlink_init(ClientData clientData,Tcl_Interp *interp,
		int argc,char *argv[])
{
  Tcl_UnlinkVar(interp,"tcl_modems_resp");
  return TCL_OK;  
}

int call_unlink_mem_dump(ClientData clientData,Tcl_Interp *interp,
		int argc,char *argv[])
{
  Tcl_UnlinkVar(interp,"tcl_mem_dump_resp");
  return TCL_OK;  
}

int call_unlink_mem_mod(ClientData clientData,Tcl_Interp *interp,
		int argc,char *argv[])
{
  Tcl_UnlinkVar(interp,"tcl_mem_mod_resp");
  return TCL_OK;  
}

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

//extern int matherr();
//int *tclDummyMathPtr = (int *) matherr;

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
  Tcl_CreateCommand(interp,"modeminit",(Tcl_CmdProc *)call_init_modem,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp,"unlink_init",(Tcl_CmdProc *)call_unlink_init,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp,"mem_dump",(Tcl_CmdProc *)call_get_mem_dump,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp,"unlink_mem_dump",(Tcl_CmdProc *)call_unlink_mem_dump,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp,"mem_modify",(Tcl_CmdProc *)call_get_mem_mod,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp,"unlink_mem_modify",(Tcl_CmdProc *)call_unlink_mem_mod,(ClientData) NULL,(Tcl_CmdDeleteProc *) NULL);

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
