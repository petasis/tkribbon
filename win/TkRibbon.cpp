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

#include "TkRibbon.h"
#include "resource.h"

TCL_DECLARE_MUTEX(globalMutex)

LRESULT CALLBACK ToplevelWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildWndProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC TkRibbon_TkToplevel_WndProc = NULL;
WNDPROC TkRibbon_TkChild_WndProc    = NULL;

static Tk_OptionSpec widgetOptionSpecs[] = {
  {TK_OPTION_STRING, "-command", "command", "Command",
      NULL, Tk_Offset(TkRibbon, commandPtr), -1,
      TK_OPTION_NULL_OK, 0, 0},
  {TK_OPTION_END, NULL, NULL, NULL, NULL, 0, -1, 0, 0, 0}
};


static void TkRibbon_DestroyObjects(TkRibbon *ribbonPtr);

int TkRibbon_ErrorMsg(Tcl_Interp *interp, HRESULT hr) {
  if (!interp) return TCL_ERROR;
  LPVOID lpMsgBuf = NULL;
  if(HRESULT_FACILITY(hr) == FACILITY_WINDOWS) hr = HRESULT_CODE(hr);
  FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    hr,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPWSTR) &lpMsgBuf,
    0, NULL );
  Tcl_SetObjResult(interp, Tcl_NewUnicodeObj((Tcl_UniChar *)lpMsgBuf, -1));
  LocalFree(lpMsgBuf);
  return TCL_ERROR;
}; /* TkRibbon_ErrorMsg */

#include "TkRibbonProperties.cpp"

static int TkRibbon_Attach(TkRibbon *ribbonPtr) {
  // return TCL_OK;
  HRESULT hr;
  if (!ribbonPtr)           return TCL_ERROR;
  if (!ribbonPtr->toplevel) return TCL_ERROR;
  /* Ensure the window is mapped, so the wrapper window has been created */
  Tk_MapWindow(ribbonPtr->toplevel);

  HWND hWnd     = Tk_GetHWND(Tk_WindowId(ribbonPtr->toplevel));
  HWND toplevel = GetAncestor(hWnd, GA_PARENT);

  if (toplevel == 0) {
    Tcl_SetResult(ribbonPtr->interp, "invalid wrapper window", TCL_STATIC);
    return TCL_ERROR;
  }

  /* Ensure that our proc pointers are ok... */
  Tcl_MutexLock(&globalMutex);
  if (ChildWndProc == NULL) {
    TkRibbon_TkChild_WndProc = (WNDPROC) GetWindowLongPtr(hWnd, GWLP_WNDPROC);
  }
  if (TkRibbon_TkToplevel_WndProc == NULL) {
    TkRibbon_TkToplevel_WndProc = (WNDPROC)
             GetWindowLongPtr(toplevel, GWLP_WNDPROC);
  }
  Tcl_MutexUnlock(&globalMutex);

  /* Change the callbacks for these windows... */
  // SetWindowLongPtr(hWnd,     GWLP_WNDPROC, (LONG_PTR) ChildWndProc);
  SetWindowLongPtr(toplevel, GWLP_WNDPROC, (LONG_PTR) ToplevelWndProc);

  // SetWindowLong(toplevel, GWL_STYLE, GetWindowLong(toplevel, GWL_STYLE) |
  //   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

  ribbonPtr->window  = hWnd;
  ribbonPtr->wrapper = toplevel;
 
  hr = InitializeFramework(ribbonPtr);
  if (FAILED(hr)) return TkRibbon_ErrorMsg(ribbonPtr->interp, hr);
  return TCL_OK;
}; /* TkRibbon_Attach */

static void TkRibbonCmdDeletedProc(ClientData clientData) {
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  if (ribbonPtr) {
    TkRibbon_DestroyObjects(ribbonPtr);
    if (ribbonPtr->dll_handle) FreeLibrary(ribbonPtr->dll_handle);
    ckfree((char *) ribbonPtr);
  }
}; /* TkRibbonCmdDeletedProc */

#define TKRIBBON_HAS_ONLY_TWO_ARGUMENTS \
  if (objc != 2) {Tcl_WrongNumArgs(interp, 2, objv, "");\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_HAS_TWO_OR_THREE_ARGUMENTS(x) \
  if (objc != 2 && objc != 3) {Tcl_WrongNumArgs(interp, 2, objv, x);\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_HAS_FIVE_OR_SIX_ARGUMENTS(x) \
  if (objc != 5 && objc != 6) {Tcl_WrongNumArgs(interp, 2, objv, x);\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_HAS_ONLY_THREE_ARGUMENTS(x) \
  if (objc != 3) {Tcl_WrongNumArgs(interp, 2, objv, x);\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_HAS_ONLY_FOUR_ARGUMENTS(x) \
  if (objc != 4) {Tcl_WrongNumArgs(interp, 2, objv, x);\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_HAS_ONLY_FIVE_ARGUMENTS(x) \
  if (objc != 5) {Tcl_WrongNumArgs(interp, 2, objv, x);\
                  result = TCL_ERROR; goto done;}
#define TKRIBBON_ENSURE_FRAMEWORK \
  if (ribbonPtr->framework == NULL) {\
   Tcl_SetResult(interp, "uninitialised framework", TCL_STATIC);\
   return TCL_ERROR;}

static int TkRibbonRibbonObjCmd(ClientData clientData, Tcl_Interp *interp,
                                int objc, Tcl_Obj * const objv[]) {
  static const char *const ribbonOptions[] = {
    "attach",
    "load_resources", "load_ui",
    "get_property", "set_property",
    "invalidate_state", "invalidate_value", "invalidate_property",
    "invalidate_all",
    "modes", "modes_add", "modes_remove",
    NULL
  };
  enum options {
    RIBBON_ATTACH,    RIBBON_LOAD_RESOURCES, RIBBON_LOAD_UI,
    RIBBON_GETPROP,   RIBBON_SETPROP,
    RIBBON_INV_STATE, RIBBON_INV_VAL, RIBBON_INV_PROP, RIBBON_INV_ALL,
    RIBBON_MODES,     RIBBON_MODES_ADD, RIBBON_MODES_REMOVE
  };
  int result = TCL_OK, index, i;
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  UI_INVALIDATIONS flags = UI_INVALIDATIONS_ALLPROPERTIES;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1], (const char **) ribbonOptions,
                          "option", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum options) index) {
    case RIBBON_ATTACH:
      TKRIBBON_HAS_ONLY_TWO_ARGUMENTS;
      result = TkRibbon_Attach(ribbonPtr);
      break;
    case RIBBON_LOAD_RESOURCES: {
      TKRIBBON_HAS_ONLY_THREE_ARGUMENTS("dll_containing_ribbons");
      ribbonPtr->dll_handle    = LoadLibrary(Tcl_GetString(objv[2]));
      if (ribbonPtr->dll_handle == NULL) {
        return TkRibbon_ErrorMsg(interp, GetLastError());
      }
    }
      break;
    case RIBBON_LOAD_UI: {
      TKRIBBON_HAS_ONLY_FOUR_ARGUMENTS("module ribbon_name");
      TKRIBBON_ENSURE_FRAMEWORK;
      HMODULE module = GetModuleHandle(Tcl_GetString(objv[2]));
      if (module == NULL) return TkRibbon_ErrorMsg(interp, GetLastError());
      HRESULT hr = ribbonPtr->framework->LoadUI(module, (LPCWSTR)
                                                Tcl_GetUnicode(objv[3]));
      if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);
    }
      break;
    case RIBBON_GETPROP: {
      TKRIBBON_HAS_ONLY_FOUR_ARGUMENTS("property id");
      return TkRibbonGetProperty(interp, ribbonPtr, objv[2], objv[3]);
    }
    case RIBBON_SETPROP: {
      if (objc < 5 || objc > 7) {
        Tcl_WrongNumArgs(interp, 2, objv,
           "property id value ?images? ?categories?");
        result = TCL_ERROR;
        goto done;
      }
      if (objc == 7) {
        return TkRibbonSetProperty(interp, ribbonPtr, objv[2], objv[3],
                                                      objv[4], objv[5],
                                                      objv[6]);
      } else if (objc == 6) {
        return TkRibbonSetProperty(interp, ribbonPtr, objv[2], objv[3],
                                                      objv[4], objv[5]);
      }
      return TkRibbonSetProperty(interp, ribbonPtr, objv[2], objv[3], objv[4]);
    }
    case RIBBON_INV_STATE:
      flags = UI_INVALIDATIONS_STATE;         goto invalidate;
    case RIBBON_INV_VAL:
      flags = UI_INVALIDATIONS_VALUE;         goto invalidate;
    case RIBBON_INV_PROP:
      flags = UI_INVALIDATIONS_PROPERTY;      goto invalidate;
    case RIBBON_INV_ALL:
      flags = UI_INVALIDATIONS_ALLPROPERTIES; goto invalidate;
    case RIBBON_MODES: {
      TKRIBBON_HAS_TWO_OR_THREE_ARGUMENTS("?modes?");
      if (objc == 3) {
        TKRIBBON_ENSURE_FRAMEWORK;
        if (Tcl_GetLongFromObj(interp, objv[2], &ribbonPtr->modes) != TCL_OK) {
          return TCL_ERROR;
        }
        ribbonPtr->framework->SetModes(ribbonPtr->modes);
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(ribbonPtr->modes));
      break;
    }
    case RIBBON_MODES_ADD:
      TKRIBBON_HAS_ONLY_THREE_ARGUMENTS("mode");
      if (Tcl_GetIntFromObj(interp, objv[2], &i) != TCL_OK) return TCL_ERROR;
      TKRIBBON_ENSURE_FRAMEWORK;
      ribbonPtr->modes |= UI_MAKEAPPMODE(i);
      ribbonPtr->framework->SetModes(ribbonPtr->modes);
      break;
    case RIBBON_MODES_REMOVE:
      TKRIBBON_HAS_ONLY_THREE_ARGUMENTS("mode");
      if (Tcl_GetIntFromObj(interp, objv[2], &i) != TCL_OK) return TCL_ERROR;
      TKRIBBON_ENSURE_FRAMEWORK;
      ribbonPtr->modes ^= UI_MAKEAPPMODE(i);
      ribbonPtr->framework->SetModes(ribbonPtr->modes);
      break;
  }
done:
  return result;
invalidate: {
    long id;
    PROPERTYKEY key;
    TKRIBBON_HAS_ONLY_FOUR_ARGUMENTS("property id");
    TKRIBBON_ENSURE_FRAMEWORK;
    if (Tcl_GetLongFromObj(interp, objv[3], &id) != TCL_OK) return TCL_ERROR;
    if (Tk_Ribbon_String2Property(interp, objv[2], NULL, &key) != TCL_OK) {
      return TCL_ERROR;
    }
    HRESULT hr = ribbonPtr->framework->InvalidateUICommand(id, flags, &key);
    if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);
    ribbonPtr->framework->FlushPendingInvalidations();
  }
  return TCL_OK;
}; /* TkRibbonRibbonObjCmd */

void TkRibbonWidgetWorldChanged(ClientData clientData) {
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  printf("TkRibbonWidgetWorldChanged:\n");
}; /* TkRibbonWidgetWorldChanged */

static Tk_ClassProcs TkRibbonClass = {
  sizeof(Tk_ClassProcs),         /* size */
  TkRibbonWidgetWorldChanged,    /* worldChangedProc */
  NULL,                          /* createProc */
  NULL                           /* modalProc */
};

#if 0
static void TkRibbonEventProc(ClientData clientData, XEvent *eventPtr) {
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  UINT ribbonHeight;
  char *msg = "";
  switch (eventPtr->type) {
    case ConfigureNotify:
      ribbonPtr->width  = eventPtr->xconfigure.width;
      ribbonPtr->height = eventPtr->xconfigure.height;
      printf("ConfigureNotify: x=%d, y=%d, w=%d, h=%d\n",
              eventPtr->xconfigure.x, eventPtr->xconfigure.y,
              ribbonPtr->width, ribbonPtr->height);
      if (SUCCEEDED(GetRibbonHeight(ribbonPtr, &ribbonHeight))) {
        RECT rect;
        GetClientRect(ribbonPtr->wrapper, &rect);
        rect.top = ribbonHeight;
        InvalidateRect(ribbonPtr->wrapper, &rect, TRUE);
      }
      return;
    case Expose:
      msg = "Expose";
      if (SUCCEEDED(GetRibbonHeight(ribbonPtr, &ribbonHeight))) {
        RECT rect;
        GetClientRect(ribbonPtr->wrapper, &rect);
        rect.top = ribbonHeight;
        InvalidateRect(ribbonPtr->wrapper, &rect, TRUE);
      }
      break;
    case DestroyNotify:
      msg = "DestroyNotify";
      break;
    case MapNotify:
      msg = "MapNotify";
      break;
    case UnmapNotify:
      msg = "UnmapNotify";
      break;
    case ReparentNotify:
      msg = "ReparentNotify";
      break;
    case GravityNotify:
      msg = "GravityNotify";
      break;
    case CirculateNotify:
      msg = "CirculateNotify";
      break;
    case MapRequest:
      msg = "MapRequest";
      break;
  }
  printf("TkRibbonEventProc: %s %p\n", msg, ribbonPtr);
}; /* TkRibbonEventProc */
#endif

static void TkRibbon_InitialiseObjects(TkRibbon *ribbonPtr) {
  ribbonPtr->cmd_objects[0] = Tcl_NewStringObj("event",    -1);
  ribbonPtr->cmd_objects[1] = Tcl_NewStringObj("generate", -1);
  ribbonPtr->cmd_objects[2] = Tcl_NewStringObj(
                                  Tk_PathName(ribbonPtr->tkwin), -1);
  ribbonPtr->cmd_objects[3] = NULL;
  ribbonPtr->cmd_objects[4] = Tcl_NewStringObj("-data", -1);
  ribbonPtr->cmd_objects[5] = NULL;

  ribbonPtr->cmd_types[TK_RIBBON_STR_onExecute] =
                      Tcl_NewStringObj("<<onExecute>>",          -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onPreview] =
                      Tcl_NewStringObj("<<onPreview>>",          -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onCancelPreview] =
                      Tcl_NewStringObj("<<onCancelPreview>>",    -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onCreateUICommand] =
                      Tcl_NewStringObj("<<onCreateUICommand>>",  -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onViewChanged] =
                      Tcl_NewStringObj("<<onViewChanged>>",      -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onDestroyUICommand] =
                      Tcl_NewStringObj("<<onDestroyUICommand>>", -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_onUpdateProperty] =
                      Tcl_NewStringObj("<<onUpdateProperty>>",   -1);
  ribbonPtr->cmd_types[TK_RIBBON_STR_EMPTY] =
                      Tcl_NewStringObj("",                        0);
  for (unsigned int i = 0; i < TK_RIBBON_STORED_COMMAND_OBJECTS; ++i) {
    if (ribbonPtr->cmd_objects[i]) Tcl_IncrRefCount(ribbonPtr->cmd_objects[i]);
  }
  for (unsigned int i = 0; i < TK_RIBBON_STORED_STRING_OBJECTS; ++i) {
    if (ribbonPtr->cmd_types[i])   Tcl_IncrRefCount(ribbonPtr->cmd_types[i]);
  }
}; /* TkRibbon_InitialiseObjects */

static void TkRibbon_DestroyObjects(TkRibbon *ribbonPtr) {
  for (unsigned int i = 0; i < TK_RIBBON_STORED_COMMAND_OBJECTS; ++i) {
    if (ribbonPtr->cmd_objects[i]) Tcl_DecrRefCount(ribbonPtr->cmd_objects[i]);
    ribbonPtr->cmd_objects[i] = NULL;
  }
  for (unsigned int i = 0; i < TK_RIBBON_STORED_STRING_OBJECTS; ++i) {
    if (ribbonPtr->cmd_types[i])   Tcl_DecrRefCount(ribbonPtr->cmd_types[i]);
    ribbonPtr->cmd_types[i] = NULL;
  }
}; /* TkRibbon_DestroyObjects */

static int ConfigureWidget(Tcl_Interp *interp, register TkRibbon *ribbonPtr,
                           int objc, Tcl_Obj *const objv[]) {
  int status;
  Tk_SavedOptions savedOptions;
  status = Tk_SetOptions(interp, (char *) ribbonPtr, ribbonPtr->optionTable,
                         objc, objv, ribbonPtr->tkwin, &savedOptions, NULL);
  if (status != TCL_OK) return status;
  return TCL_OK;
}; /* ConfigureWidget */

static int TkRibbon_CreateObjCmd(ClientData clientData, Tcl_Interp *interp,
                                 int objc, Tcl_Obj *objv[]) {
  Tk_Window  main, tkwin, toplevel;
  TkRibbon  *ribbonPtr;

  if (objc < 3) {
    Tcl_WrongNumArgs(interp, 1, objv, "pathName toplevel ?-option value ...?");
    return TCL_ERROR;
  }

  main = Tk_MainWindow(interp);
  if (main == NULL) {
    Tcl_SetResult(interp, "cannot get interpreter's main window", TCL_STATIC);
    return TCL_ERROR;
  }

  /* The provided window must be a toplevel! */
  toplevel = Tk_NameToWindow(interp, Tcl_GetString(objv[2]), main);
  if (toplevel == NULL) return TCL_ERROR;
  if (!Tk_IsTopLevel(toplevel)) {
    Tcl_SetResult(interp, "provided window must be a toplevel", TCL_STATIC);
    return TCL_ERROR;
  }

  /* Create our "fake" window... (that will never be drawn, but it will
   * occupy space - the same of course as the windows ribbon) */
  tkwin = Tk_CreateWindowFromPath(interp, main, Tcl_GetString(objv[1]), NULL);
  if (tkwin == NULL) return TCL_ERROR;

  ribbonPtr = (TkRibbon *) ckalloc(sizeof(TkRibbon));
  memset(ribbonPtr, 0, sizeof(TkRibbon));

  ribbonPtr->interp      = interp;
  ribbonPtr->tkwin       = tkwin;
  ribbonPtr->toplevel    = toplevel;
  ribbonPtr->width       = TK_RIBBON_DEFAULT_WIDTH;
  ribbonPtr->height      = TK_RIBBON_DEFAULT_HEIGHT;
  ribbonPtr->modes       = UI_MAKEAPPMODE(0);
  ribbonPtr->objectCmd   = Tcl_CreateObjCommand(interp, Tcl_GetString(objv[1]),
                                                TkRibbonRibbonObjCmd, ribbonPtr,
                                                TkRibbonCmdDeletedProc);
  /* Set the TkRibbon class... */
  Tk_SetClass(tkwin, "TkRibbon");
  Tk_SetClassProcs(tkwin, &TkRibbonClass, ribbonPtr);

  /*
   * Create the option table for this widget class. If it has already been
   * created, the cached pointer will be returned.
   */
  ribbonPtr->optionTable = Tk_CreateOptionTable(interp, widgetOptionSpecs);
  if (Tk_InitOptions(interp, (char *) ribbonPtr, ribbonPtr->optionTable, tkwin)
            != TCL_OK) {
    Tk_DestroyWindow(tkwin);
    return TCL_ERROR;
  }

  if (ConfigureWidget(interp, ribbonPtr, objc - 3, objv + 3) != TCL_OK) {
    Tk_DestroyWindow(tkwin);
    return TCL_ERROR;
  }

  Tk_SetMinimumRequestSize(tkwin, ribbonPtr->width, ribbonPtr->height);
  Tk_GeometryRequest(      tkwin, ribbonPtr->width, ribbonPtr->height);
  TkRibbon_InitialiseObjects(ribbonPtr);

#if 0
  /* Register an event handler... */
  Tk_CreateEventHandler(tkwin, ExposureMask |
                               StructureNotifyMask | /* Map/Unmap/Configure */
                               SubstructureRedirectMask,
                        TkRibbonEventProc, ribbonPtr);
#endif
  Tcl_SetObjResult(interp, objv[1]);

  return TCL_OK;
}; /* TkRibbon_CreateObjCmd */

/*
 * TkRibbon_InitialisePlatform:
 * Note that this function is called only once, when the DLL has been loaded
 * for the first time!
 */
extern "C"
int TkRibbon_InitialisePlatform(Tcl_Interp *interp) {
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr)) {
    return TCL_ERROR;
  }
  return TCL_OK;
}; /* TkRibbon_InitialisePlatform */

extern "C"
int TkRibbon_FinalisePlatform(ClientData clientData) {
  CoUninitialize();
  return TCL_OK;
}; /* TkRibbon_FinalisePlatform */

extern "C"
int TkRibbon_RegisterPlatformCmds(Tcl_Interp *interp) {
  /* Register the package commands... */
  if (Tcl_CreateObjCommand(interp, "tkribbon::platform::create",
          (Tcl_ObjCmdProc*) TkRibbon_CreateObjCmd,
          (ClientData) NULL, NULL) == NULL) return TCL_ERROR;
  return TCL_OK;
}; /* TkRibbon_RegisterPlatformCmds */

LRESULT CALLBACK ToplevelWndProc(HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_WINDOWPOSCHANGED:
    case WM_WINDOWPOSCHANGING:
      DefWindowProc(hWnd, message, wParam, lParam);
      break;
  }
  return CallWindowProc(TkRibbon_TkToplevel_WndProc,
                        hWnd, message, wParam, lParam);
}; /* ToplevelWndProc */

LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam) {
  return CallWindowProc(TkRibbon_TkChild_WndProc,
                        hWnd, message, wParam, lParam);
}; /* ChildWndProc */
