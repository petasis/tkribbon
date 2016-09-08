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

#ifndef _TKRIBBON_WIDGET_H
#define _TKRIBBON_WIDGET_H
#include "tcl.h"
#include "tk.h"

#if defined(__WIN32__) || defined(_WIN32) || defined(__MINGW32__)
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <tkPlatDecls.h>
#ifdef __cplusplus
}
#endif
//#include "tkWinInt.h"
#endif


/* ======================================================================
 * +++ Ribbon widget:
 */

typedef struct TkRibbon {
    Tk_Window tkwin;            /* Window that embodies the browser. NULL means
                                 * that the window has been destroyed but the
                                 * data structures haven't yet been cleaned
                                 * up. */
    Display *display;           /* Display containing widget. Used, among
                                 * other things, so that resources can be
                                 * freed even after tkwin has gone away. */
    Tcl_Interp *interp;         /* Interpreter associated with widget. */
    Tcl_Command widgetCmd;      /* Token for browser's widget command. */
    Tk_OptionTable optionTable; /* Table that defines configuration options
                                 * available for this widget. */
    char *className;            /* Class name for widget (from configuration
                                 * option). Malloc-ed. */
    char *visualName;           /* Textual description of visual for window,
                                 * from -visual option. Malloc-ed, may be
                                 * NULL. */
    char *colormapName;         /* Textual description of colormap for window,
                                 * from -colormap option. Malloc-ed, may be
                                 * NULL. */
    Colormap colormap;          /* If not None, identifies a colormap
                                 * allocated for this window, which must be
                                 * freed when the window is deleted. */
    Tk_3DBorder border;         /* Structure used to draw 3-D border and
                                 * background. NULL means no background or
                                 * border. */
    int borderWidth;            /* Width of 3-D border (if any). */
    int relief;                 /* 3-d effect: TK_RELIEF_RAISED etc. */
    int highlightWidth;         /* Width in pixels of highlight to draw around
                                 * widget when it has the focus. 0 means don't
                                 * draw a highlight. */
    XColor *highlightBgColorPtr;
                                /* Color for drawing traversal highlight area
                                 * when highlight is off. */
    XColor *highlightColorPtr;  /* Color for drawing traversal highlight. */
    int width;                  /* Width to request for window. <= 0 means
                                 * don't request any size. */
    int height;                 /* Height to request for window. <= 0 means
                                 * don't request any size. */
    Tk_Cursor cursor;           /* Current cursor for window, or None. */
    char *takeFocus;            /* Value of -takefocus option; not used in the
                                 * C code, but used by keyboard traversal
                                 * scripts. Malloc'ed, but may be NULL. */
    int flags;                  /* Various flags; see below for
                                 * definitions. */
    Tcl_Obj *padXPtr;           /* Value of -padx option: specifies how many
                                 * pixels of extra space to leave on left and
                                 * right of child area. */
    int padX;                   /* Integer value corresponding to padXPtr. */
    Tcl_Obj *padYPtr;           /* Value of -padx option: specifies how many
                                 * pixels of extra space to leave above and
                                 * below child area. */
    int padY;                   /* Integer value corresponding to padYPtr. */

    void *ribbonPtr;
    struct TkRibbon *nextWidgetPtr;
} TkRibbon;

#define REPORT_FUNCTION_CALL_ENTER
#define REPORT_FUNCTION_CALL_LEAVE
#define Ribbon void
#endif
