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

/*
 *  InitializeFramework:
 * ----------------------------------
 *
 *  Initialises the Ribbon framework and binds a Ribbon to the provided window.
 *    To get a Ribbon to display, the Ribbon framework must be initialized. 
 *    This involves three important steps:
 *      1) Instantiating the Ribbon framework object (CLSID_UIRibbonFramework).
 *      2) Passing the host HWND and IUIApplication object to the framework.
 *      3) Loading the binary markup compiled by UICC.exe.
 */
HRESULT InitializeFramework(TkRibbon *ribbonPtr) {
  HRESULT hr;
  
  if (!ribbonPtr) return E_POINTER;

  /*
   * Instantiate the Ribbon framework object...
   */
  hr = CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&(ribbonPtr->framework)));
  if (FAILED(hr) || !ribbonPtr->framework) return hr;

  hr = CoCreateInstance(CLSID_UIRibbonImageFromBitmapFactory, NULL, CLSCTX_ALL,
                        IID_PPV_ARGS(&(ribbonPtr->imageFactory)));

  /*
   * Create an application object (IUIApplication) and call
   * the framework Initialize method, passing the application object and
   * the host HWND that the Ribbon will attach itself to.
   */
  hr = CApplication::CreateInstance(ribbonPtr, &(ribbonPtr->application));
  if (FAILED(hr) || !ribbonPtr->application) return hr;

  hr = ribbonPtr->framework->Initialize(ribbonPtr->wrapper,
                                        ribbonPtr->application);
  return hr;

#if 0
  /* Finally, we load the binary markup.
   * This will initiate callbacks to the IUIApplication object 
   * that was provided to the framework earlier, allowing command handlers
   * to be bound to individual commands. */
  hr = ribbonPtr->framework->LoadUI(GetModuleHandle("libtkribbon1.0.dll"),
                                    L"APPLICATION_RIBBON");
#endif
} /* InitializeFramework */

void DestroyFramework(TkRibbon *ribbonPtr) {
  if (ribbonPtr->framework) {
    ribbonPtr->framework->Destroy();
    ribbonPtr->framework->Release();
    ribbonPtr->framework = NULL;
  }
  if (ribbonPtr->application) {
    ribbonPtr->application->Release();
    ribbonPtr->application = NULL;
  }
}; /* DestroyFramework */

HRESULT GetRibbonHeight(TkRibbon *ribbonPtr, UINT* ribbonHeight) {
  HRESULT hr = E_POINTER;
  *ribbonHeight = TK_RIBBON_MIN_HEIGHT;
  if (ribbonPtr && ribbonPtr->framework) {
    IUIRibbon* pRibbon = NULL;
    if (SUCCEEDED(ribbonPtr->framework->GetView(0, IID_PPV_ARGS(&pRibbon)))) {
      if (pRibbon) {
        hr = pRibbon->GetHeight(ribbonHeight);
        pRibbon->Release();
        if (*ribbonHeight < TK_RIBBON_MIN_HEIGHT) {
          *ribbonHeight = TK_RIBBON_MIN_HEIGHT;
        }
      }
    }
  }
  return hr;
}; /* GetRibbonHeight */

void TkRibbonGeometryRequest(TkRibbon *ribbonPtr) {
  UINT ribbonHeight;
  if (SUCCEEDED(GetRibbonHeight(ribbonPtr, &ribbonHeight))) {
    Tk_GeometryRequest(ribbonPtr->tkwin, ribbonPtr->width, ribbonHeight);
    printf("TkRibbonGeometryRequest: w: %d, h: %d\n",
                  ribbonPtr->width, ribbonHeight);
  }
}; /* TkRibbonGeometryRequest */
