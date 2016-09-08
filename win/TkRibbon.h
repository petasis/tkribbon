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

#pragma once

#include "tcl.h"
#include "tk.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>        // Windows header files
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
//#include <Objbase.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <tkPlatDecls.h>
#ifdef __cplusplus
}
#endif

#define TK_RIBBON_STORED_COMMAND_OBJECTS 6
#define TK_RIBBON_STORED_STRING_OBJECTS 16
typedef struct TkRibbon {
  Tcl_Interp     *interp;         /* Interpreter associated with the object. */
  Tcl_Command     objectCmd;      /* Token for object's command.             */
  Tk_Window       tkwin;          /* The fake widget that will be beneath
                                     the ribbon. */
  Tk_Window       toplevel;       /* The toplevel the Ribbon is attached.    */

  HWND            window;
  HWND            wrapper;

  IUIFramework   *framework;      /* Reference to the Ribbon framework.      */
  IUIApplication *application;    /* Reference to the Application object.    */
  IUIImageFromBitmap
                 *imageFactory;   /* Reference to the image factory object.  */

  int             height;
  int             width;

  Tcl_Obj        *cmd_objects[TK_RIBBON_STORED_COMMAND_OBJECTS];
  Tcl_Obj        *cmd_types[TK_RIBBON_STORED_STRING_OBJECTS];
  Tcl_Obj        *commandPtr;
  Tk_OptionTable  optionTable;
  long            modes;
  HINSTANCE       dll_handle;
} TkRibbon;

#include "RibbonFramework.h"
#include "Application.h"
#include "CommandHandler.h"
#include "PropertySet.h"

#define TK_RIBBON_MIN_HEIGHT        0
#define TK_RIBBON_MIN_WIDTH         0
#define TK_RIBBON_DEFAULT_HEIGHT  146
#define TK_RIBBON_DEFAULT_WIDTH   350

#define TK_RIBBON_STR_onExecute            0
#define TK_RIBBON_STR_onPreview            1
#define TK_RIBBON_STR_onCancelPreview      2
#define TK_RIBBON_STR_onCreateUICommand    3
#define TK_RIBBON_STR_onViewChanged        4
#define TK_RIBBON_STR_onDestroyUICommand   5
#define TK_RIBBON_STR_onUpdateProperty     6
#define TK_RIBBON_STR_EMPTY                7
#define TK_RIBBON_STR(x) ribbonPtr->cmd_types[x]

enum TkRibbonTypes {
  TK_RIBBON_TYPE_BOOLEAN,
  TK_RIBBON_TYPE_UINT,
  TK_RIBBON_TYPE_INT,
  TK_RIBBON_TYPE_DECIMAL,
  TK_RIBBON_TYPE_WSTR,
  TK_RIBBON_TYPE_UICOLLECTION,
  TK_RIBBON_TYPE_UICOLLECTION_SIMPLE,
  TK_RIBBON_TYPE_UNSUPPORTED
};

extern const char* Tk_Ribbon_FindProperty(const PROPERTYKEY key,
                                          enum TkRibbonTypes *type);
extern int Tk_Ribbon_Tcl2Variant(TkRibbon *ribbonPtr, Tcl_Interp *interp,
                                 enum TkRibbonTypes type,
                                 Tcl_Obj *value_obj,
                                 Tcl_Obj *images_obj,
                                 Tcl_Obj *categories_obj,
                                 REFPROPERTYKEY key, PROPVARIANT *var);
extern Tcl_Obj *Tk_Ribbon_Variant2Tcl(Tcl_Interp *interp,
                                 enum TkRibbonTypes type,
                                 const PROPVARIANT *var);
