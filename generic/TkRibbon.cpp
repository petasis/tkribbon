/*
 * This file is part of the TkRibbon package, a Tcl/Tk extension that embeds
 * a Windows Ribbon as a Tk widget.
 *
 * Copyright (C) 2010 by:
 * Georgios P. Petasis, petasis@iit.demokritos.gr,
 * P.O. Box 8055, Aspropirgos, 19300, Attiki, Greece. All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "TkRibbon_Widget.h"

TCL_DECLARE_MUTEX(globalMutex)

static int initialised = 0;

static char initScript[] =
  "namespace eval tkribbon { variable version " PACKAGE_VERSION " };"
  "tcl_findLibrary tkribbon $tkribbon::version "
  "$tkribbon::version tkribbon.tcl TKGECKO_LIBRARY tkribbon::library;";

extern "C"
int TkRibbon_RibbonObjCmd(ClientData clientData, Tcl_Interp *interp,
                          int objc, Tcl_Obj *objv[]);
extern "C"
int TkRibbon_InitialisePlatform(Tcl_Interp *interp);
extern "C"
int TkRibbon_FinalisePlatform(ClientData clientData);
extern "C"
int TkRibbon_RegisterPlatformCmds(Tcl_Interp *interp);

static void TkRibbon_Finalise(ClientData clientData) {
  Tcl_MutexLock(&globalMutex);
  Tcl_DeleteExitHandler(TkRibbon_Finalise, NULL);
  TkRibbon_FinalisePlatform(clientData);
  Tcl_MutexUnlock(&globalMutex);
}; /* TkRibbon_Finalise */

extern "C" int DLLEXPORT
Tkribbon_Init(Tcl_Interp *interp) {
  if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) return TCL_ERROR;
  if (Tk_InitStubs(interp,  TK_VERSION,  0) == NULL) return TCL_ERROR;

  Tcl_MutexLock(&globalMutex);
  if (!initialised) {
    if (TkRibbon_InitialisePlatform(interp) != TCL_OK) {
      Tcl_MutexUnlock(&globalMutex);
      return TCL_ERROR;
    }
    initialised = 0;
    /* Place an exit handler to properly shutdown the extension... */
    Tcl_CreateExitHandler(TkRibbon_Finalise, NULL);
  }
  Tcl_MutexUnlock(&globalMutex);

  /* Register the package commands... */
  // if (Tcl_CreateObjCommand(interp, "tkribbon::ribbon",
  //         (Tcl_ObjCmdProc*) TkRibbon_RibbonObjCmd,
  //         (ClientData) NULL, NULL) == NULL) goto command_error;

  if (TkRibbon_RegisterPlatformCmds(interp) != TCL_OK) goto command_error;

  /* Load the library scripts... */
  if (Tcl_Eval(interp, initScript) != TCL_OK) {
    return TCL_ERROR;
  }
  
  Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION);
  return TCL_OK;
command_error:
  Tcl_SetResult(interp, (char *) "could not register commands!", TCL_STATIC);
  return TCL_ERROR;
}; /* Tkribbon_Init */
