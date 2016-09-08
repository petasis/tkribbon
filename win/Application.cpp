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
 * Static method to create an instance of the object...
 */
__checkReturn HRESULT
CApplication::CreateInstance(TkRibbon *p_ribbon,
                             __deref_out IUIApplication **ppApplication) {
  HRESULT hr = S_OK;
  if (!ppApplication) return E_POINTER;
  *ppApplication = NULL;
  
  CApplication* pApplication = new CApplication();
  if (pApplication != NULL) {
     pApplication->ribbonPtr = p_ribbon;
    *ppApplication = static_cast<IUIApplication *>(pApplication);
  } else {
    hr = E_OUTOFMEMORY;
  }
  return hr;
}; /* CApplication::CreateInstance */

STDMETHODIMP_(ULONG) CApplication::AddRef() {
  return InterlockedIncrement(&m_cRef);
}; /* CApplication::AddRef */

STDMETHODIMP_(ULONG) CApplication::Release() {
  LONG cRef = InterlockedDecrement(&m_cRef);
  if (cRef == 0) delete this;
  return cRef;
}; /* CApplication::Release */

STDMETHODIMP CApplication::QueryInterface(REFIID iid, void** ppv) {
  if (iid == __uuidof(IUnknown)) {
    *ppv = static_cast<IUnknown*>(this);
  } else if (iid == __uuidof(IUIApplication)) {
    *ppv = static_cast<IUIApplication*>(this);
  } else {
    *ppv = NULL;
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}; /* CApplication::QueryInterface */

/*  OnCreateUICommand:
 * ------------------------------
 *  Called by the Ribbon framework for each command specified in markup,
 *  to allow the host application to bind a command handler to that command.
 */
STDMETHODIMP CApplication::OnCreateUICommand(
    UINT nCmdID,
    __in UI_COMMANDTYPE typeID,
    __deref_out IUICommandHandler** ppCommandHandler) {
  UNREFERENCED_PARAMETER(typeID);
  UNREFERENCED_PARAMETER(nCmdID);

  if (m_pCommandHandler == NULL) {
    HRESULT hr = CCommandHandler::CreateInstance(ribbonPtr, &m_pCommandHandler);
    if (FAILED(hr)) return hr;
  }
  return m_pCommandHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
}; /* CApplication::OnCreateUICommand */

/*
 *  OnViewChanged:
 * ------------------------------
 *  Called when the state of a View (Ribbon is a view) changes, for example,
 *  created, destroyed, or resized.
 */
STDMETHODIMP CApplication::OnViewChanged(
    UINT viewId,
    __in UI_VIEWTYPE typeId,
    __in IUnknown* pView,
    UI_VIEWVERB verb,
    INT uReasonCode) {
  UNREFERENCED_PARAMETER(uReasonCode);
  UNREFERENCED_PARAMETER(viewId);

  HRESULT hr = E_NOTIMPL;
  // if (ribbonPtr) InvalidateRect(ribbonPtr->wrapper, NULL, TRUE);

  /* Checks to see if the view that was changed was a Ribbon view... */
  if (UI_VIEWTYPE_RIBBON == typeId) {
    switch (verb) {           
      case UI_VIEWVERB_CREATE: /* The view was newly created */
        if (NULL == g_pRibbon) {
          // Retrieve and store the IUIRibbon
          hr = pView->QueryInterface(&g_pRibbon);
        }
        hr = S_OK;
        if (ribbonPtr) {
          unsigned int i;
          ribbonPtr->cmd_objects[3] = 
                     TK_RIBBON_STR(TK_RIBBON_STR_onCreateUICommand);
          ribbonPtr->cmd_objects[5] = TK_RIBBON_STR(TK_RIBBON_STR_EMPTY);
          for (i=0; i<6; ++i) Tcl_IncrRefCount(ribbonPtr->cmd_objects[i]);
          Tcl_EvalObjv(ribbonPtr->interp, 6, ribbonPtr->cmd_objects,
                                             TCL_EVAL_GLOBAL);
          for (i=0; i<6; ++i) Tcl_DecrRefCount(ribbonPtr->cmd_objects[i]);
          ribbonPtr->cmd_objects[3] = NULL;
          ribbonPtr->cmd_objects[5] = NULL;
        }
        break;
      case UI_VIEWVERB_SIZE:
        /* The view has been resized. For the Ribbon view, the application
           should call GetHeight to determine the height of the ribbon. */
        if (NULL == g_pRibbon) {
          // Retrieve and store the IUIRibbon
          hr = pView->QueryInterface(&g_pRibbon);
        }
        /*
         * Notify Tk that the height has changed!
         */
        if (ribbonPtr) {
          unsigned int i;
          Tk_GeometryRequest(ribbonPtr->tkwin, ribbonPtr->width,
                                               GetHeight());
          ribbonPtr->cmd_objects[3] =
                     TK_RIBBON_STR(TK_RIBBON_STR_onViewChanged);
          ribbonPtr->cmd_objects[5] = TK_RIBBON_STR(TK_RIBBON_STR_EMPTY);
          for (i=0; i<6; ++i) Tcl_IncrRefCount(ribbonPtr->cmd_objects[i]);
          Tcl_EvalObjv(ribbonPtr->interp, 6, ribbonPtr->cmd_objects,
                                             TCL_EVAL_GLOBAL);
          for (i=0; i<6; ++i) Tcl_DecrRefCount(ribbonPtr->cmd_objects[i]);
          ribbonPtr->cmd_objects[3] = NULL;
          ribbonPtr->cmd_objects[5] = NULL;
        }
        break;
      case UI_VIEWVERB_DESTROY: /* The view was destroyed */
        if (ribbonPtr) {
          unsigned int i;
          ribbonPtr->cmd_objects[3] =
                     TK_RIBBON_STR(TK_RIBBON_STR_onDestroyUICommand);
          ribbonPtr->cmd_objects[5] = TK_RIBBON_STR(TK_RIBBON_STR_EMPTY);
          for (i=0; i<6; ++i) Tcl_IncrRefCount(ribbonPtr->cmd_objects[i]);
          Tcl_EvalObjv(ribbonPtr->interp, 6, ribbonPtr->cmd_objects,
                                             TCL_EVAL_GLOBAL);
          for (i=0; i<6; ++i) Tcl_DecrRefCount(ribbonPtr->cmd_objects[i]);
          ribbonPtr->cmd_objects[3] = NULL;
          ribbonPtr->cmd_objects[5] = NULL;
        }
        g_pRibbon = NULL;
        hr = S_OK;
        break;
    }
  }
  return hr;
}; /* CApplication::OnViewChanged */

/*
 *  OnDestroyUICommand:
 * ------------------------------
 *  Called by the Ribbon framework for each command at the time of ribbon
 *  destruction.
 */
STDMETHODIMP CApplication::OnDestroyUICommand(
    UINT32 nCmdID,
    __in UI_COMMANDTYPE typeID,
    __in_opt IUICommandHandler* commandHandler) {
    UNREFERENCED_PARAMETER(commandHandler);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(nCmdID);

    return E_NOTIMPL;
}; /* CApplication::OnDestroyUICommand */

int CApplication::GetHeight(void) {
  if (g_pRibbon) {
    g_pRibbon->GetHeight(&uRibbonHeight);
  }
  if (uRibbonHeight < TK_RIBBON_MIN_HEIGHT) {
    uRibbonHeight = TK_RIBBON_MIN_HEIGHT;
  }
  return uRibbonHeight;
}; /* CApplication::GetHeight */
