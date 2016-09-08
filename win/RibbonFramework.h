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

#include <UIRibbon.h>

HRESULT InitializeFramework(TkRibbon *ribbonPtr);
HRESULT GetRibbonHeight(TkRibbon *ribbonPtr, UINT* ribbonHeight);
void    TkRibbonGeometryRequest(TkRibbon *ribbonPtr);
void    DestroyFramework   (TkRibbon *ribbonPtr);
