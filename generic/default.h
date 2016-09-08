/*
 * This file is part of the TkRibbon package, a Tcl/Tk extension that embeds
 * Windows Ribbon as a Tk widget.
 *
 * Copyright (C) 2010 by:
 * Georgios P. Petasis, petasis@iit.demokritos.gr,
 * P.O. Box 8055, Aspropirgos, 19300, Attiki, Greece. All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _TKWINDEFAULT
#define _TKWINDEFAULT


#if defined(__WIN32__) || defined(_WIN32) || \
    defined(__MINGW32__)

#define BLACK                            "#000000"
#define WHITE                            "#ffffff"
#define NORMAL_BG                        "SystemButtonFace"
#define NORMAL_FG                        "SystemButtonText"
#define ACTIVE_BG                        NORMAL_BG
#define TEXT_FG                          "SystemWindowText"
#define SELECT_BG                        "SystemHighlight"
#define SELECT_FG                        "SystemHighlightText"
#define TROUGH                           "SystemScrollbar"
#define INDICATOR                        "SystemWindow"
#define DISABLED                         "SystemDisabledText"
#define MENU_BG                          "SystemMenu"
#define MENU_FG                          "SystemMenuText"
#define HIGHLIGHT                        "SystemWindowFrame"
#define DEF_RIBBON_HIGHLIGHT_WIDTH      "0"

#else
#   if defined(MAC_OSX_TK)

#define BLACK                            "Black"
#define WHITE                            "White"
#define NORMAL_BG                        "systemWindowBody"
#define ACTIVE_BG                        "systemButtonFacePressed"
#define ACTIVE_FG                        "systemPushButtonPressedText"
#define SELECT_BG                        "systemHighlight"
#define SELECT_FG                        None
#define INACTIVE_SELECT_BG               "systemHighlightSecondary"
#define TROUGH                           "#c3c3c3"
#define INDICATOR                        "#b03060"
#define DISABLED                         "#a3a3a3"
#define HIGHLIGHT                        BLACK
#define DEF_RIBBON_HIGHLIGHT_WIDTH      "0"

#   else

#define BLACK                            "#000000"
#define WHITE                            "#ffffff"
#define NORMAL_BG                        "#d9d9d9"
#define ACTIVE_BG                        "#ececec"
#define SELECT_BG                        "#c3c3c3"
#define TROUGH                           "#b3b3b3"
#define CHECK_INDICATOR                  WHITE
#define MENU_INDICATOR                   BLACK
#define DISABLED                         "#a3a3a3"
#define HIGHLIGHT                        BLACK
#define DEF_RIBBON_HIGHLIGHT_WIDTH      "1"

#   endif
#endif

/*
 * Defaults for frames:
 */

#define DEF_RIBBON_BG_COLOR             NORMAL_BG
#define DEF_RIBBON_BG_MONO              WHITE
#define DEF_RIBBON_BORDER_WIDTH         "0"
#define DEF_RIBBON_CLASS                "Ribbon"
#define DEF_RIBBON_COLORMAP             ""
#define DEF_RIBBON_CONTAINER            "0"
#define DEF_RIBBON_CURSOR               ""
#define DEF_RIBBON_HEIGHT               "200"
#define DEF_RIBBON_HIGHLIGHT_BG         NORMAL_BG
#define DEF_RIBBON_HIGHLIGHT            HIGHLIGHT
#define DEF_RIBBON_PADX                 "0"
#define DEF_RIBBON_PADY                 "0"
#define DEF_RIBBON_RELIEF               "flat"
#define DEF_RIBBON_TAKE_FOCUS           "1"
#define DEF_RIBBON_VISUAL               ""
#define DEF_RIBBON_WIDTH                "200"

/*
 * Flag bits for buttons:
 *
 * REDRAW_PENDING:          Non-zero means a DoWhenIdle handler has
 *                          already been queued to redraw this window.
 * SELECTED:                Non-zero means this button is selected, so
 *                          special highlight should be drawn.
 * GOT_FOCUS:               Non-zero means this button currently has the
 *                          input focus.
 * BUTTON_DELETED:          Non-zero needs that this button has been
 *                          deleted, or is in the process of being deleted
 */

#define REDRAW_PENDING          (1 << 0)
#define SELECTED                (1 << 1)
#define GOT_FOCUS               (1 << 2)
#define BUTTON_DELETED          (1 << 3)
#define TRISTATED               (1 << 4)

#endif /* _TKWINDEFAULT */
