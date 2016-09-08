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
#include "default.h"

/*
 * This is a convenience macro used to initialize a thread local storage ptr.
 */
#ifndef TCL_TSD_INIT
#define TCL_TSD_INIT(keyPtr) \
  (ThreadSpecificData *)Tcl_GetThreadData((keyPtr), sizeof(ThreadSpecificData))
#endif /* TCL_TSD_INIT */


/* ======================================================================
 * +++ TkRibbon widget:
 */

static const Tk_OptionSpec ribbonOptSpec[] = {
    {TK_OPTION_BORDER, "-background", "background", "Background",
        DEF_RIBBON_BG_COLOR, -1, Tk_Offset(TkRibbon, border),
        TK_OPTION_NULL_OK, (ClientData) DEF_RIBBON_BG_MONO, 0},
    {TK_OPTION_SYNONYM, "-bg", NULL, NULL,
        NULL, 0, -1, 0, (ClientData) "-background", 0},
    {TK_OPTION_STRING, "-colormap", "colormap", "Colormap",
        DEF_RIBBON_COLORMAP, -1, Tk_Offset(TkRibbon, colormapName),
        TK_OPTION_NULL_OK, 0, 0},
    {TK_OPTION_CURSOR, "-cursor", "cursor", "Cursor",
        DEF_RIBBON_CURSOR, -1, Tk_Offset(TkRibbon, cursor),
        TK_OPTION_NULL_OK, 0, 0},
    {TK_OPTION_PIXELS, "-height", "height", "Height",
        DEF_RIBBON_HEIGHT, -1, Tk_Offset(TkRibbon, height), 0, 0, 0},
    {TK_OPTION_COLOR, "-highlightbackground", "highlightBackground",
        "HighlightBackground", DEF_RIBBON_HIGHLIGHT_BG, -1,
        Tk_Offset(TkRibbon, highlightBgColorPtr), 0, 0, 0},
    {TK_OPTION_COLOR, "-highlightcolor", "highlightColor", "HighlightColor",
        DEF_RIBBON_HIGHLIGHT, -1, Tk_Offset(TkRibbon, highlightColorPtr),
        0, 0, 0},
    {TK_OPTION_PIXELS, "-highlightthickness", "highlightThickness",
        "HighlightThickness", DEF_RIBBON_HIGHLIGHT_WIDTH, -1,
        Tk_Offset(TkRibbon, highlightWidth), 0, 0, 0},
    {TK_OPTION_PIXELS, "-padx", "padX", "Pad",
        DEF_RIBBON_PADX, Tk_Offset(TkRibbon, padXPtr),
        Tk_Offset(TkRibbon, padX), 0, 0, 0},
    {TK_OPTION_PIXELS, "-pady", "padY", "Pad",
        DEF_RIBBON_PADY, Tk_Offset(TkRibbon, padYPtr),
        Tk_Offset(TkRibbon, padY), 0, 0, 0},
    {TK_OPTION_STRING, "-takefocus", "takeFocus", "TakeFocus",
        DEF_RIBBON_TAKE_FOCUS, -1, Tk_Offset(TkRibbon, takeFocus),
        TK_OPTION_NULL_OK, 0, 0},
    {TK_OPTION_STRING, "-visual", "visual", "Visual",
        DEF_RIBBON_VISUAL, -1, Tk_Offset(TkRibbon, visualName),
        TK_OPTION_NULL_OK, 0, 0},
    {TK_OPTION_PIXELS, "-width", "width", "Width",
        DEF_RIBBON_WIDTH, -1, Tk_Offset(TkRibbon, width), 0, 0, 0},
    {TK_OPTION_SYNONYM, "-bd", NULL, NULL,
        NULL, 0, -1, 0, (ClientData) "-borderwidth", 0},
    {TK_OPTION_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
        DEF_RIBBON_BORDER_WIDTH, -1, Tk_Offset(TkRibbon, borderWidth), 0, 0, 0},
    {TK_OPTION_STRING, "-class", "class", "Class",
        DEF_RIBBON_CLASS, -1, Tk_Offset(TkRibbon, className), 0, 0, 0},
    {TK_OPTION_RELIEF, "-relief", "relief", "Relief",
        DEF_RIBBON_RELIEF, -1, Tk_Offset(TkRibbon, relief), 0, 0, 0},
    {TK_OPTION_END, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0}
};

/*
 * Forward declarations for functions defined later in this file:
 */

static void ComputeTkRibbonGeometry(TkRibbon *ribbonPtr);
static int  ConfigureTkRibbon(Tcl_Interp *interp, TkRibbon *ribbonPtr,
              int objc, Tcl_Obj *const objv[]);
static void DestroyTkRibbon(char *memPtr);
static void DestroyTkRibbonPartly(TkRibbon *ribbonPtr);
static void DisplayTkRibbon(ClientData clientData);
static void TkRibbonCmdDeletedProc(ClientData clientData);
static void TkRibbonEventProc(ClientData clientData, XEvent *eventPtr);
static int  TkRibbonWidgetObjCmd(ClientData clientData,
              Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static void TkRibbonWidgetWorldChanged(ClientData instanceData);
static void MapTkRibbon(ClientData clientData);

typedef struct ThreadSpecificData {
  TkRibbon *firstWidget;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;


/*
 * The structure below defines ribbon class behavior by means of functions that
 * can be invoked from generic window code.
 */
static Tk_ClassProcs ribbonClass = {
  sizeof(Tk_ClassProcs),           /* size */
  TkRibbonWidgetWorldChanged     /* worldChangedProc */
};

extern "C"
int TkRibbon_RibbonObjCmd(ClientData clientData, Tcl_Interp *interp,
                          int objc, Tcl_Obj *const objv[]) {
  Tk_Window tkwin, newWin;
  TkRibbon *ribbonPtr;
  Tk_OptionTable optionTable;
  const char *className, *visualName, *colormapName;
  int i, c, length, depth;
  const char *arg;
  unsigned int mask;
  Colormap colormap;
  Visual *visual;
  // Ribbon* mRibbon = NULL, *threadRibbon = (Ribbon *) clientData;
#if defined(TkpMakeContainer) && defined(TkpUseWindow) && \
    defined(TkpPrintWindowId)
  char buf[TCL_INTEGER_SPACE];
#endif
  ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
  REPORT_FUNCTION_CALL_ENTER;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "pathName ?-option value ...?");
    return TCL_ERROR;
  }

  /*
   * Create the option table for this widget class. If it has already been
   * created, the cached pointer will be returned.
   */

  optionTable = Tk_CreateOptionTable(interp, ribbonOptSpec);

  /*
   * Pre-process the argument list. Scan through it to find any "-class",
   * "-visual", and "-colormap" options. These arguments need to
   * be processed specially, before the window is configured using the usual
   * Tk mechanisms.
   */

  className = colormapName = visualName = NULL;
  colormap = None;
  for (i = 2; i < objc; i += 2) {
    arg = Tcl_GetStringFromObj(objv[i], &length);
    if (length < 2) {
      continue;
    }
    c = arg[1];
    if ((c == 'c') && (length >= 3)
      && (strncmp(arg, "-class", (unsigned) length) == 0)) {
      className = Tcl_GetString(objv[i+1]);
    } else if ((c == 'c')
      && (strncmp(arg, "-colormap", (unsigned) length) == 0)) {
      colormapName = Tcl_GetString(objv[i+1]);
    } else if ((c == 'v')
      && (strncmp(arg, "-visual", (unsigned) length) == 0)) {
      visualName = Tcl_GetString(objv[i+1]);
    }
  }

  /*
   * Main window associated with interpreter. If we're called by Tk_Init to
   * create a new application, then this is NULL.
   */

  tkwin = Tk_MainWindow(interp);
  if (tkwin == NULL) goto error;
  newWin = Tk_CreateWindowFromPath(interp, tkwin, Tcl_GetString(objv[1]), NULL);
  if (newWin == NULL) goto error;

  if (className == NULL) {
    className = Tk_GetOption(newWin, "class", "Class");
    if (className == NULL) {
      className = DEF_RIBBON_CLASS;
    }
  }
  Tk_SetClass(newWin, className);

  if (visualName == NULL) {
    visualName = Tk_GetOption(newWin, "visual", "Visual");
  }
  if (colormapName == NULL) {
    colormapName = Tk_GetOption(newWin, "colormap", "Colormap");
  }
  if ((colormapName != NULL) && (*colormapName == 0)) {
    colormapName = NULL;
  }
  if (visualName != NULL) {
    visual = Tk_GetVisual(interp, newWin, visualName, &depth,
        (colormapName == NULL) ? &colormap : NULL);
    if (visual == NULL) {
      goto error;
    }
    Tk_SetWindowVisual(newWin, visual, depth, colormap);
  }
  if (colormapName != NULL) {
    colormap = Tk_GetColormap(interp, newWin, colormapName);
    if (colormap == None) {
      goto error;
    }
    Tk_SetWindowColormap(newWin, colormap);
  }

  /*
   * Create the widget record, process configuration options, and create
   * event handlers. Then fill in a few additional fields in the widget
   * record from the special options.
   */
  ribbonPtr = (TkRibbon *) ckalloc(sizeof(TkRibbon));
  memset(ribbonPtr, 0, sizeof(TkRibbon));

  ribbonPtr->tkwin             = newWin;
  ribbonPtr->display           = Tk_Display(newWin);
  ribbonPtr->interp            = interp;
  ribbonPtr->widgetCmd         = Tcl_CreateObjCommand(interp,
              Tk_PathName(newWin), TkRibbonWidgetObjCmd, ribbonPtr,
              TkRibbonCmdDeletedProc);
  ribbonPtr->optionTable       = optionTable;
  ribbonPtr->colormap          = colormap;
  ribbonPtr->relief            = TK_RELIEF_FLAT;
  ribbonPtr->cursor            = None;

  /*
   * Store backreference to ribbon widget in window structure.
   */

  Tk_SetClassProcs(newWin, &ribbonClass, ribbonPtr);

  mask = ExposureMask | StructureNotifyMask | FocusChangeMask;
  Tk_CreateEventHandler(newWin, mask, TkRibbonEventProc, ribbonPtr);
  if (Tk_InitOptions(interp, (char *) ribbonPtr, optionTable, newWin)
                                                  != TCL_OK) goto error;
  if (ConfigureTkRibbon(interp, ribbonPtr, objc-2, objv+2) != TCL_OK)
                                                             goto error;

  /*
   * The widget seems ok. Create the ribbon object...
   */
  Tk_MakeWindowExist(ribbonPtr->tkwin);
  Tk_MapWindow(ribbonPtr->tkwin);

  Tcl_SetResult(interp, Tk_PathName(newWin), TCL_VOLATILE);
  ribbonPtr->ribbonPtr = NULL;
  /* Add the widget to the list of all widgets... */
  ribbonPtr->nextWidgetPtr = tsdPtr->firstWidget;
  tsdPtr->firstWidget = ribbonPtr;
  REPORT_FUNCTION_CALL_LEAVE;
  return TCL_OK;

error:
  if (newWin != NULL) {
    Tk_DestroyWindow(newWin);
  }
  if (ribbonPtr) ckfree((char *) ribbonPtr);
  return TCL_ERROR;
}; /* TkRibbon_TkRibbonObjCmd */

/*
 *--------------------------------------------------------------
 *
 * TkRibbonWidgetObjCmd --
 *
 *    This function is invoked to process the Tcl command that corresponds
 *    to a ribbon widget. See the user documentation for details on what it
 *    does.
 *
 * Results:
 *    A standard Tcl result.
 *
 * Side effects:
 *    See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int
TkRibbonWidgetObjCmd(
    ClientData clientData,        /* Information about ribbon widget. */
    Tcl_Interp *interp,           /* Current interpreter. */
    int objc,                     /* Number of arguments. */
    Tcl_Obj *const objv[])        /* Argument objects. */
{
  static const char *const ribbonOptions[] = {
    "back",
    "cget",           "can_go_back",      "can_go_forward",   "configure",
    "document",
    "expose",
    "focus_activate", "focus_deactivate", "forward",
    "hide",           "load",             "linkmsg",
    "navigate",       "parse",            "reload",
    "save",           "setup",            "show",
    "special",        "statusmsg",        "stop",
    "title",          "uri",              "visibility",        NULL
  };
  enum options {
    RIBBON_BACK,
    RIBBON_CGET,     RIBBON_CAN_GO_B,   RIBBON_CAN_GO_F,   RIBBON_CONFIGURE,
    RIBBON_DOCUMENT,
    RIBBON_EXPOSE,
    RIBBON_FOCUS_AC, RIBBON_FOCUS_DE,   RIBBON_FORWARD,
    RIBBON_HIDE,     RIBBON_LOAD,       RIBBON_LINK_MSG,
    RIBBON_NAVIGATE, RIBBON_PARSE,      RIBBON_RELOAD,
    RIBBON_SAVE,     RIBBON_SETUP,      RIBBON_SHOW,
    RIBBON_SPECIAL,  RIBBON_STATUS_MSG, RIBBON_STOP,
    RIBBON_TITLE,    RIBBON_URI,        RIBBON_VISIBILITY
  };
#if 0
  register TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  int result = TCL_OK, index;
  int c, i, length;
  Tcl_Obj *objPtr;
  // Ribbon *mRibbon = NULL;
  const char *base_uri = "file://", *mime_type = "text/html", *arg, *data;
  long len;
  Params p;
  REPORT_FUNCTION_CALL_ENTER;
  memset(&p, 0, sizeof(Params));

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
    return TCL_ERROR;
  }
  if (Tcl_GetIndexFromObj(interp, objv[1], (const char **) ribbonOptions,
                          "option", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }
  
  Tcl_ResetResult(interp);
  
#define TKGECKO_HAS_ONLY_TWO_ARGUMENTS \
  if (objc != 2) {Tcl_WrongNumArgs(interp, 2, objv, "");\
                  result = TCL_ERROR; goto done;}
#define TKGECKO_HAS_ONLY_TWO_ARGUMENTS_OPTIONAL_FLAGS \
  if (objc != 2&& objc != 3) {Tcl_WrongNumArgs(interp, 2, objv, "?flags?");\
                  result = TCL_ERROR; goto done;} \
  if (objc == 3) {\
    result = Tcl_GetLongFromObj(interp, objv[2], (long *) &p.flags);\
    if (result != TCL_OK) goto done;\
  }

  Tcl_Preserve(ribbonPtr);
  
  if (ribbonPtr) mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
  
  switch ((enum options) index) {
    case RIBBON_EXPOSE:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "expose");
      break;
    case RIBBON_HIDE:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "hide");
      break;
    case RIBBON_SHOW:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "show");
      break;
    case RIBBON_LOAD:
      p.flags = nsIWebNavigation::LOAD_FLAGS_NONE;
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS_OPTIONAL_FLAGS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "load_uri");
      break;
    case RIBBON_RELOAD:
      p.flags = nsIWebNavigation::LOAD_FLAGS_NONE;
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS_OPTIONAL_FLAGS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "reload");
      break;
    case RIBBON_STOP:
      p.flags = nsIWebNavigation::STOP_ALL;
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS_OPTIONAL_FLAGS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "stop");
      break;
    case RIBBON_CAN_GO_B:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) {
        mRibbon->QueueEvent(&p, "can_go_back");
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(p.result_int));
      }
      break;
    case RIBBON_CAN_GO_F:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) {
        mRibbon->QueueEvent(&p, "can_go_forward");
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(p.result_int));
      }
      break;
    case RIBBON_BACK:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "go_back");
      break;
    case RIBBON_FORWARD:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) result = mRibbon->QueueEvent(&p, "go_forward");
      break;
    case RIBBON_LINK_MSG:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) Tcl_SetResult(interp, (char *) mRibbon->GetLinkMessage(),
                                                             TCL_VOLATILE);
      break;
    case RIBBON_STATUS_MSG:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) Tcl_SetResult(interp, (char *) mRibbon->GetJsStatus(),
                                                             TCL_VOLATILE);
      break;
    case RIBBON_TITLE:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) Tcl_SetResult(interp, (char *) mRibbon->GetTitle(),
                                                             TCL_VOLATILE);
      break;
    case RIBBON_DOCUMENT:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) {
        DOM *mDom = new DOM(mRibbon);
        if (!mDom) {
          Tcl_SetResult(interp, (char *) "out of memmory!", TCL_STATIC);
          result = TCL_ERROR;
          goto done;
        }
        if (mDom->document == NULL) {
          Tcl_SetResult(interp, (char *) "unable to retrieve DOM document!",
                                                             TCL_STATIC);
          result = TCL_ERROR;
          goto done;
        }
        /* We have to imitate what SWIG would have been created as a new
         * object, in order to integrate it with the rest of the DOM class...
         * 29/10/2009: This is a newly allocated object, pass ownership to
         * SWIG (by using a true flag - last argument)...
         */
        Tcl_SetObjResult(interp, SWIG_NewInstanceObj(
                SWIG_as_voidptr(mDom), SWIGTYPE_p_DOM , 1));
      }
      break;
    case RIBBON_PARSE:
      for (i = 2; i < objc; i++) {
        arg = Tcl_GetStringFromObj(objv[i], &length);
        c = arg[0];
        if (c == '-') {
          /* This is an option... */
          ++i;
          if (i == objc) {
            /* We are missing arguments! */
            Tcl_WrongNumArgs(interp, 2, objv,
                             "?-base base_uri? ?-mime mime_type? ?--? data");
            result = TCL_ERROR;
            goto done;
          }
          c = arg[1];
          if ((c == 'b' ) && (length >= 2)
                  && (strncmp(arg, "-base", (unsigned)length) == 0)) {
            /* -base base_uri */
            base_uri = Tcl_GetString(objv[i]);
          } else if ((c == 'm' ) && (length >= 2)
                  && (strncmp(arg, "-mime", (unsigned)length) == 0)) {
            /* -mime mime_type */
            mime_type = Tcl_GetString(objv[i]);
          } else if ((c == '-' ) && (length == 2)) {
            /* -- data */
            data = Tcl_GetStringFromObj(objv[i], &length);
            len = length;
            break;
          }
        } else {
          /* We have found the last element... */
          if (i != objc-1) {
            Tcl_WrongNumArgs(interp, 2, objv,
                             "?-base base_uri? ?-mime mime_type? ?--? data");
            result = TCL_ERROR;
            goto done;
          }
          data = arg; len = length;
          break;
        }
      }; /* for (i = 2; i < objc; i++) */
      /* We have all data to call Parse! */
      if (mRibbon) {
        p.str  = data;
        p.len  = len;
        p.str2 = base_uri;
        p.str3 = mime_type;
        result  = mRibbon->QueueEvent(&p, "parse");
      }
      break;
    case RIBBON_SAVE:
      p.dir      = NULL;
      p.mime     = NULL;
      p.encFlags = 0;
      p.perFlags = 0;
      p.col      = 80;
      for (i = 2; i < objc; i++) {
        arg = Tcl_GetStringFromObj(objv[i], &length);
        c = arg[0];
        if (c == '-') {
          /* This is an option... */
          ++i;
          if (i == objc) {
            /* We are missing arguments! */
            Tcl_WrongNumArgs(interp, 2, objv,
             "?-data_dir data_dir? ?-mime mime_type? ?-flags flags? "
             "?-pflags persist_flags? ?-col wrap_col? ?--? uri");
            result = TCL_ERROR;
            goto done;
          }
          c = arg[1];
          if ((c == 'd' ) && (length >= 2)
                  && (strncmp(arg, "-data_dir", (unsigned)length) == 0)) {
            /* -data_dir data_dir */
            p.dir = Tcl_GetString(objv[i]);
          } else if ((c == 'c' ) && (length >= 2)
                  && (strncmp(arg, "-col", (unsigned) length) == 0)) {
            /* -col column */
            if (Tcl_GetLongFromObj(interp, objv[i], (long *) &p.col) != TCL_OK) {
              goto done;
            }
          } else if ((c == 'f' ) && (length >= 2)
                  && (strncmp(arg, "-flags", (unsigned) length) == 0)) {
            /* -flags flags */
            if (Tcl_GetLongFromObj(interp, objv[i], (long *) &p.encFlags) != TCL_OK) {
              goto done;
            }
          } else if ((c == 'p' ) && (length >= 2)
                  && (strncmp(arg, "-pflags", (unsigned) length) == 0)) {
            /* -pflags persist_flags */
            if (Tcl_GetLongFromObj(interp, objv[i], (long *) &p.perFlags) != TCL_OK) {
              goto done;
            }
          } else if ((c == 'm' ) && (length >= 2)
                  && (strncmp(arg, "-mime", (unsigned)length) == 0)) {
            /* -mime mime_type */
            p.mime = Tcl_GetString(objv[i]);
          } else if ((c == '-' ) && (length == 2)) {
            /* -- data */
            p.uri = Tcl_GetStringFromObj(objv[i], &length);
            break;
          }
        } else {
          /* We have found the last element... */
          if (i != objc-1) {
            Tcl_WrongNumArgs(interp, 2, objv,
              "?-data_dir data_dir? ?-mime mime_type? ?-flags flags? "
              "?-pflags persist_flags? ?-col wrap_col? ?--? uri");
            result = TCL_ERROR;
            goto done;
          }
          p.uri = arg;
          break;
        }
      }; /* for (i = 2; i < objc; i++) */
      /* We have all data to call SaveDocument! */
      if (mRibbon) {
        result  = mRibbon->QueueEvent(&p, "save_document");
      }
      break;
    case RIBBON_SETUP:
      if (objc != 4) {
        Tcl_WrongNumArgs(interp, 2, objv, "nsIWebRibbonSetup::Id boolean");
        result = TCL_ERROR;
        goto done;
      }
      result = Tcl_GetLongFromObj(interp, objv[2], (long *) &p.id);
      if (result != TCL_OK) goto done;
      result = Tcl_GetBooleanFromObj(interp, objv[3], &c);
      if (result != TCL_OK) goto done;
      p.value = c;
      mRibbon->QueueEvent(&p, "set_property");
      break;
    case RIBBON_URI:
      if (objc !=2 && objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "?uri?");
        result = TCL_ERROR;
        goto done;
      }
      if (mRibbon) {
        if (objc == 3) {
          p.str = Tcl_GetString(objv[2]);
          result = mRibbon->QueueEvent(&p, "set_uri");
        }
        result = mRibbon->QueueEvent(&p, "get_uri");
        Tcl_SetResult(interp, (char *) p.str, TCL_VOLATILE);
      }
      break;
    case RIBBON_VISIBILITY:
      if (objc !=2 && objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "?boolean?");
        result = TCL_ERROR;
        goto done;
      }
      if (mRibbon) {
        if (objc == 3) {
          result = Tcl_GetBooleanFromObj(interp, objv[2], &c);
          p.flags = c;
          mRibbon->QueueEvent(&p, "set_visibility");
        }
        mRibbon->QueueEvent(&p, "get_visibility");
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(p.flags));
      }
      break;
    case RIBBON_FOCUS_AC:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) {
        mRibbon->QueueEvent(&p, "focus_activate");
      }
      break;
    case RIBBON_FOCUS_DE:
      TKGECKO_HAS_ONLY_TWO_ARGUMENTS;
      if (mRibbon) {
        mRibbon->QueueEvent(&p, "focus_deactivate");
      }
      break;
    case RIBBON_NAVIGATE:
      p.flags = nsIWebNavigation::LOAD_FLAGS_NONE;
      if (objc != 3 && objc != 4) {
        Tcl_WrongNumArgs(interp, 2, objv, "uri ?flags?");
        result = TCL_ERROR;
        goto done;
      }
      if (objc == 4) {
        result = Tcl_GetLongFromObj(interp, objv[3], (long *) &p.flags);
        if (result != TCL_OK) goto done;
      }
      if (mRibbon) {
        p.str   = Tcl_GetString(objv[2]);
        result  = mRibbon->QueueEvent(&p, "set_uri");
        result  = mRibbon->QueueEvent(&p, "load_uri");
        result  = mRibbon->QueueEvent(&p, "get_uri");
        Tcl_SetResult(interp, (char *) p.str, TCL_VOLATILE);
      }
      break;
    case RIBBON_SPECIAL:
      if (objc != 3) {Tcl_WrongNumArgs(interp, 2, objv, "action");\
                      result = TCL_ERROR; goto done;}
      if (mRibbon) result = mRibbon->QueueEvent(&p, Tcl_GetString(objv[2]));
      break;
    case RIBBON_CGET:
      if (objc != 3) {
        Tcl_WrongNumArgs(interp, 2, objv, "option");
        result = TCL_ERROR;
        goto done;
      }
      objPtr = Tk_GetOptionValue(interp, (char *) ribbonPtr,
              ribbonPtr->optionTable, objv[2], ribbonPtr->tkwin);
      if (objPtr == NULL) {
        result = TCL_ERROR;
        goto done;
      }
      Tcl_SetObjResult(interp, objPtr);
      break;
    case RIBBON_CONFIGURE:
      if (objc <= 3) {
        objPtr = Tk_GetOptionInfo(interp, (char *) ribbonPtr,
                ribbonPtr->optionTable, (objc == 3) ? objv[2] : NULL,
                ribbonPtr->tkwin);
        if (objPtr == NULL) {
          result = TCL_ERROR;
          goto done;
        }
        Tcl_SetObjResult(interp, objPtr);
      } else {
        /*
         * Don't allow the options -class, -colormap, or -visual to be changed.
         */

        for (i = 2; i < objc; i++) {
          char *arg = Tcl_GetStringFromObj(objv[i], &length);
          if (length < 2) {
              continue;
          }
          c = arg[1];
          if (((c == 'c') && (length >= 2)
                  && (strncmp(arg, "-class", (unsigned)length) == 0))
              || ((c == 'c') && (length >= 3)
                  && (strncmp(arg, "-colormap", (unsigned)length) == 0))
              || ((c == 'v')
                  && (strncmp(arg, "-visual", (unsigned)length) == 0))) {
            Tcl_AppendResult(interp, "can't modify ", arg,
                                     " option after widget is created", NULL);
            result = TCL_ERROR;
            goto done;
          }
        }
        result = ConfigureTkRibbon(interp, ribbonPtr, objc-2, objv+2);
      }
      break;
  }

done:
  Tcl_Release(ribbonPtr);
  REPORT_FUNCTION_CALL_LEAVE;
  return result;

#endif
  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyTkRibbon --
 *
 *    This function is invoked by Tcl_EventuallyFree or Tcl_Release to clean
 *    up the internal structure of a ribbon at a safe time (when no-one is
 *    using it anymore).
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Everything associated with the ribbon is freed up.
 *
 *----------------------------------------------------------------------
 */
static void
DestroyTkRibbon(
    char *memPtr)                /* Info about ribbon widget. */
{
  register TkRibbon *ribbonPtr = (TkRibbon *) memPtr;
  register TkRibbon *prevWidgetPtr;
  ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
  REPORT_FUNCTION_CALL_ENTER;

  if (ribbonPtr && ribbonPtr->ribbonPtr) {
    Ribbon* mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
    // TODO
  }

  if (ribbonPtr->colormap != None) {
    Tk_FreeColormap(ribbonPtr->display, ribbonPtr->colormap);
  }
  /* Remove the widget from the list of all widgets... */
  if (ribbonPtr == tsdPtr->firstWidget) {
    tsdPtr->firstWidget = ribbonPtr->nextWidgetPtr;
  } else {
    for (prevWidgetPtr = tsdPtr->firstWidget;
         prevWidgetPtr && (prevWidgetPtr->nextWidgetPtr != ribbonPtr);
         prevWidgetPtr = prevWidgetPtr->nextWidgetPtr) {
      /* Empty loop body. */
    }
    if (prevWidgetPtr == NULL) {
      Tcl_Panic("DestroyTkRibbon: damaged widget list");
    }
    prevWidgetPtr->nextWidgetPtr = ribbonPtr->nextWidgetPtr;
  }
  ribbonPtr->nextWidgetPtr = NULL;
  ckfree((char *) ribbonPtr);
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyTkRibbonPartly --
 *
 *    This function is invoked to clean up everything that needs tkwin to be
 *    defined when deleted. During the destruction process tkwin is always
 *    set to NULL and this function must be called before that happens.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Some things associated with the ribbon are freed up.
 *
 *----------------------------------------------------------------------
 */
static void
DestroyTkRibbonPartly(
    TkRibbon *ribbonPtr)                /* Info about ribbon widget. */
{
  REPORT_FUNCTION_CALL_ENTER;
  Ribbon *mRibbon = NULL;
  if (ribbonPtr) mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
  if (mRibbon) {
    // mRibbon->QueueEvent("stop");
    // mRibbon->QueueEvent("destroy");
  }
  Tk_FreeConfigOptions((char *) ribbonPtr, ribbonPtr->optionTable,
                       ribbonPtr->tkwin);
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * Tkgecko_ThreadFinalize --
 *
 *    This function is invoked to clean up all widgets that are currently
 *    active in this thread.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    All Tk widgets are detached from Gecko.
 *
 *----------------------------------------------------------------------
 */
void Tkgecko_ThreadFinalize(ClientData clientData) {
  register TkRibbon *ribbonPtr;
  ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
  REPORT_FUNCTION_CALL_ENTER;
  
  Tcl_DeleteThreadExitHandler(Tkgecko_ThreadFinalize, clientData);

  for (ribbonPtr = tsdPtr->firstWidget; ribbonPtr;
                    ribbonPtr = ribbonPtr->nextWidgetPtr) {
    Ribbon* mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
    if (mRibbon) {
      // mRibbon->QueueEvent("stop");
      // mRibbon->QueueEvent("destroy");
      // mRibbon->QueueEvent("delete_ptr");
    }
    ribbonPtr->ribbonPtr = NULL;
  }
  REPORT_FUNCTION_CALL_LEAVE;
}; /* Tkgecko_ThreadFinalize */

/*
 *----------------------------------------------------------------------
 *
 * ConfigureTkRibbon --
 *
 *    This function is called to process an objv/objc list, plus the Tk
 *    option database, in order to configure (or reconfigure) a ribbon
 *    widget.
 *
 * Results:
 *    The return value is a standard Tcl result. If TCL_ERROR is returned,
 *    then the interp's result contains an error message.
 *
 * Side effects:
 *    Configuration information, such as text string, colors, font, etc. get
 *    set for ribbonPtr; old resources get freed, if there were any.
 *
 *----------------------------------------------------------------------
 */

static int
ConfigureTkRibbon(
  Tcl_Interp *interp,               /* Used for error reporting. */
  register TkRibbon *ribbonPtr,  /* Information about widget; may or may not
                                     * already have values for some fields. */
  int objc,                         /* Number of valid entries in objv. */
  Tcl_Obj *const objv[])            /* Arguments. */
{
  Tk_SavedOptions savedOptions;
  Tk_Window oldWindow = NULL;
  REPORT_FUNCTION_CALL_ENTER;

  if (Tk_SetOptions(interp, (char *) ribbonPtr,
          ribbonPtr->optionTable, objc, objv,
          ribbonPtr->tkwin, &savedOptions, NULL) != TCL_OK) {
      return TCL_ERROR;
  }
  Tk_FreeSavedOptions(&savedOptions);

  /*
   * A few of the options require additional processing.
   */

  if (ribbonPtr->border != NULL) {
    Tk_SetBackgroundFromBorder(ribbonPtr->tkwin, ribbonPtr->border);
  } else {
    Tk_SetWindowBackgroundPixmap(ribbonPtr->tkwin, None);
  }

  if (ribbonPtr->highlightWidth < 0) {
    ribbonPtr->highlightWidth = 0;
  }
  if (ribbonPtr->padX < 0) {
    ribbonPtr->padX = 0;
  }
  if (ribbonPtr->padY < 0) {
    ribbonPtr->padY = 0;
  }

  TkRibbonWidgetWorldChanged(ribbonPtr);
  REPORT_FUNCTION_CALL_LEAVE;
  return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * TkRibbonWidgetWorldChanged --
 *
 *    This function is called when the world has changed in some way and the
 *    widget needs to recompute all its graphics contexts and determine its
 *    new geometry.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    TkRibbon will be relayed out and redisplayed.
 *
 *---------------------------------------------------------------------------
 */

static void
TkRibbonWidgetWorldChanged(
    ClientData instanceData)        /* Information about widget. */
{
  TkRibbon *ribbonPtr = (TkRibbon *) instanceData;
  Tk_Window tkwin = ribbonPtr->tkwin;
  int bWidthLeft, bWidthRight, bWidthTop, bWidthBottom;
  REPORT_FUNCTION_CALL_ENTER;

  /*
   * Calculate individual border widths.
   */

  bWidthBottom = bWidthTop = bWidthRight = bWidthLeft =
          ribbonPtr->borderWidth + ribbonPtr->highlightWidth;

  bWidthLeft   += ribbonPtr->padX;
  bWidthRight  += ribbonPtr->padX;
  bWidthTop    += ribbonPtr->padY;
  bWidthBottom += ribbonPtr->padY;

  Tk_SetInternalBorderEx(tkwin, bWidthLeft, bWidthRight, bWidthTop,
          bWidthBottom);

  ComputeTkRibbonGeometry(ribbonPtr);
#if 0

  if ((ribbonPtr->width > 0) || (ribbonPtr->height > 0)) {
    Tk_GeometryRequest(tkwin, ribbonPtr->width, ribbonPtr->height);
    if (ribbonPtr->ribbonPtr) {
      Ribbon *mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
      Params p;
      memset(&p, 0, sizeof(Params));
      p.w = Tk_Width(tkwin);
      p.h = Tk_Height(tkwin);
      ((Ribbon *) ribbonPtr->ribbonPtr)->QueueEvent(&p, "resize");
      if (mRibbon->curr_width  != p.w || mRibbon->curr_height != p.h) {
      }
    }
  }
#endif

  if (Tk_IsMapped(tkwin)) {
    if (!(ribbonPtr->flags & REDRAW_PENDING)) {
        Tcl_DoWhenIdle(DisplayTkRibbon, ribbonPtr);
    }
    ribbonPtr->flags |= REDRAW_PENDING;
  }
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeTkRibbonGeometry --
 *
 *    This function is called to compute various geometrical information for
 *    a ribbon, such as where various things get displayed. It's called when
 *    the window is reconfigured.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Display-related numbers get changed in *ribbonPtr.
 *
 *----------------------------------------------------------------------
 */

static void
ComputeTkRibbonGeometry(
    register TkRibbon *ribbonPtr)        /* Information about widget. */
{
  REPORT_FUNCTION_CALL_ENTER;
  // ribbonPtr->width  = Tk_Width(ribbonPtr->tkwin);
  // ribbonPtr->height = Tk_Height(ribbonPtr->tkwin);
  // printf("W: %d, H: %d\n", Tk_Width(ribbonPtr->tkwin), Tk_Height(ribbonPtr->tkwin));
  // printf("w: %d, h: %d\n", ribbonPtr->width, ribbonPtr->height); fflush(0);
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * DisplayTkRibbon --
 *
 *    This function is invoked to display a ribbon widget.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Commands are output to X to display the ribbon in its current mode.
 *
 *----------------------------------------------------------------------
 */

#ifndef TkDrawInsetFocusHighlight
static void
TkDrawInsetFocusHighlight(
    Tk_Window tkwin,            /* Window whose focus highlight ring is to be
                                 * drawn. */
    GC gc,                      /* Graphics context to use for drawing the
                                 * highlight ring. */
    int width,                  /* Width of the highlight ring, in pixels. */
    Drawable drawable,          /* Where to draw the ring (typically a pixmap
                                 * for double buffering). */
    int padding)                /* Width of padding outside of widget. */
{
    XRectangle rects[4];

    rects[0].x = padding;
    rects[0].y = padding;
    rects[0].width = Tk_Width(tkwin) - (2 * padding);
    rects[0].height = width;
    rects[1].x = padding;
    rects[1].y = Tk_Height(tkwin) - width - padding;
    rects[1].width = Tk_Width(tkwin) - (2 * padding);
    rects[1].height = width;
    rects[2].x = padding;
    rects[2].y = width + padding;
    rects[2].width = width;
    rects[2].height = Tk_Height(tkwin) - 2*width - 2*padding;
    rects[3].x = Tk_Width(tkwin) - width - padding;
    rects[3].y = rects[2].y;
    rects[3].width = width;
    rects[3].height = rects[2].height;
    XFillRectangles(Tk_Display(tkwin), drawable, gc, rects, 4);
}
#endif /* TkDrawInsetFocusHighlight */

#ifndef TkpDrawHighlightBorder
static void
TkpDrawHighlightBorder(
    Tk_Window tkwin,
    GC fgGC,
    GC bgGC,
    int highlightWidth,
    Drawable drawable)
{
    TkDrawInsetFocusHighlight(tkwin, fgGC, highlightWidth, drawable, 0);
}
#endif /* TkpDrawHighlightBorder */

static void
DisplayTkRibbon(
    ClientData clientData)        /* Information about widget. */
{
  register TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  register Tk_Window tkwin = ribbonPtr->tkwin;
  int hlWidth;
  REPORT_FUNCTION_CALL_ENTER;

  ribbonPtr->flags &= ~REDRAW_PENDING;
  if ((ribbonPtr->tkwin == NULL) || !Tk_IsMapped(tkwin)) {
    return;
  }

#if 0
  if (ribbonPtr->ribbonPtr) {
    Ribbon *mRibbon = (Ribbon *) ribbonPtr->ribbonPtr;
    Params p;
    memset(&p, 0, sizeof(Params));
    p.w = Tk_Width(tkwin);
    p.h = Tk_Height(tkwin);
    if (mRibbon->curr_width  != p.w || mRibbon->curr_height != p.h) {
      mRibbon->QueueEvent(&p, "resize");
    }
    mRibbon->QueueEvent("expose");
  }
#endif

  /*
   * Highlight shall always be drawn if it exists, so do that first.
   */

  hlWidth = ribbonPtr->highlightWidth;
  // XLockDisplay(Tk_Display(tkwin));

  if (hlWidth != 0) {
    GC fgGC, bgGC;

    bgGC = Tk_GCForColor(ribbonPtr->highlightBgColorPtr,
                         Tk_WindowId(tkwin));
    if (ribbonPtr->flags & GOT_FOCUS) {
      fgGC = Tk_GCForColor(ribbonPtr->highlightColorPtr,
                           Tk_WindowId(tkwin));
      TkpDrawHighlightBorder(tkwin, fgGC, bgGC, hlWidth,
                             Tk_WindowId(tkwin));
    } else {
      TkpDrawHighlightBorder(tkwin, bgGC, bgGC, hlWidth,
                             Tk_WindowId(tkwin));
    }
  }


  /*
   * If -background is set to "", no interior is drawn.
   */

  if (ribbonPtr->border == NULL) {
    REPORT_FUNCTION_CALL_LEAVE;
    // XUnlockDisplay(Tk_Display(tkwin));
    return;
  }

  Tk_Fill3DRectangle(tkwin, Tk_WindowId(tkwin), ribbonPtr->border,
                     hlWidth, hlWidth, Tk_Width(tkwin) - 2 * hlWidth,
                     Tk_Height(tkwin) - 2 * hlWidth,
                     ribbonPtr->borderWidth, ribbonPtr->relief);
  // XUnlockDisplay(Tk_Display(tkwin));
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *--------------------------------------------------------------
 *
 * TkRibbonEventProc --
 *
 *    This function is invoked by the Tk dispatcher on structure changes to
 *    a ribbon. For ribbons with 3D borders, this function is also invoked for
 *    exposures.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    When the window gets deleted, internal structures get cleaned up.
 *    When it gets exposed, it is redisplayed.
 *
 *--------------------------------------------------------------
 */

static void
TkRibbonEventProc(
    ClientData clientData,        /* Information about window. */
    register XEvent *eventPtr)        /* Information about event. */
{
  register TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  REPORT_FUNCTION_CALL_ENTER;

  if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
    goto redraw;
  } else if (eventPtr->type == ConfigureNotify) {
    ComputeTkRibbonGeometry(ribbonPtr);
    goto redraw;
  } else if (eventPtr->type == DestroyNotify) {
    if (ribbonPtr->tkwin != NULL) {
      /*
       * If this window is a container, then this event could be coming
       * from the embedded application, in which case Tk_DestroyWindow
       * hasn't been called yet. When Tk_DestroyWindow is called later,
       * then another destroy event will be generated. We need to be
       * sure we ignore the second event, since the ribbon could be gone
       * by then. To do so, delete the event handler explicitly
       * (normally it's done implicitly by Tk_DestroyWindow).
       */

      /*
       * Since the tkwin pointer will be gone when we reach
       * DestroyTkRibbon, we must free all options now.
       */

      DestroyTkRibbonPartly(ribbonPtr);

      Tk_DeleteEventHandler(ribbonPtr->tkwin,
              ExposureMask|StructureNotifyMask|FocusChangeMask,
              TkRibbonEventProc, ribbonPtr);
      ribbonPtr->tkwin = NULL;
      Tcl_DeleteCommandFromToken(ribbonPtr->interp, ribbonPtr->widgetCmd);
    }
    if (ribbonPtr->flags & REDRAW_PENDING) {
      Tcl_CancelIdleCall(DisplayTkRibbon, ribbonPtr);
    }
    Tcl_CancelIdleCall(MapTkRibbon, ribbonPtr);
    Tcl_EventuallyFree(ribbonPtr, DestroyTkRibbon);
  } else if (eventPtr->type == FocusIn) {
    if (eventPtr->xfocus.detail != NotifyInferior) {
      ribbonPtr->flags |= GOT_FOCUS;
      if (ribbonPtr->highlightWidth > 0) {
        goto redraw;
      }
    }
  } else if (eventPtr->type == FocusOut) {
    if (eventPtr->xfocus.detail != NotifyInferior) {
      ribbonPtr->flags &= ~GOT_FOCUS;
      if (ribbonPtr->highlightWidth > 0) {
        goto redraw;
      }
    }
  } else if (eventPtr->type == ActivateNotify) {
  }
  REPORT_FUNCTION_CALL_LEAVE;
  return;

redraw:
  if ((ribbonPtr->tkwin != NULL) && !(ribbonPtr->flags & REDRAW_PENDING)) {
    Tcl_DoWhenIdle(DisplayTkRibbon, ribbonPtr);
    ribbonPtr->flags |= REDRAW_PENDING;
  }
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * TkRibbonCmdDeletedProc --
 *
 *    This function is invoked when a widget command is deleted. If the
 *    widget isn't already in the process of being destroyed, this command
 *    destroys it.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    The widget is destroyed.
 *
 *----------------------------------------------------------------------
 */

static void
TkRibbonCmdDeletedProc(
    ClientData clientData)        /* Pointer to widget record for widget. */
{
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  Tk_Window tkwin = ribbonPtr->tkwin;
  REPORT_FUNCTION_CALL_ENTER;

  /*
   * This function could be invoked either because the window was destroyed
   * and the command was then deleted (in which case tkwin is NULL) or
   * because the command was deleted, and then this function destroys the
   * widget.
   */

  if (tkwin != NULL) {
    /*
     * Some options need tkwin to be freed, so we free them here, before
     * setting tkwin to NULL.
     */

    DestroyTkRibbonPartly(ribbonPtr);

    ribbonPtr->tkwin = NULL;
    Tk_DestroyWindow(tkwin);
  }
  REPORT_FUNCTION_CALL_LEAVE;
}

/*
 *----------------------------------------------------------------------
 *
 * MapTkRibbon --
 *
 *    This function is invoked as a when-idle handler to map a newly-created
 *    top-level ribbon.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    The ribbon given by the clientData argument is mapped.
 *
 *----------------------------------------------------------------------
 */
static void
MapTkRibbon(
    ClientData clientData)                /* Pointer to ribbon structure. */
{
  TkRibbon *ribbonPtr = (TkRibbon *) clientData;
  REPORT_FUNCTION_CALL_ENTER;

  /*
   * Wait for all other background events to be processed before mapping
   * window. This ensures that the window's correct geometry will have been
   * determined before it is first mapped, so that the window manager
   * doesn't get a false idea of its desired geometry.
   */

  Tcl_Preserve(ribbonPtr);
  while (1) {
    if (Tcl_DoOneEvent(TCL_IDLE_EVENTS) == 0) {
      break;
    }

    /*
     * After each event, make sure that the window still exists and quit
     * if the window has been destroyed.
     */

    if (ribbonPtr->tkwin == NULL) {
      Tcl_Release(ribbonPtr);
      return;
    }
  }
  Tk_MapWindow(ribbonPtr->tkwin);
  Tcl_Release(ribbonPtr);
  REPORT_FUNCTION_CALL_LEAVE;
}
