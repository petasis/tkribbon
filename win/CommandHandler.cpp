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
CCommandHandler::CreateInstance(TkRibbon *p_ribbon,
                        __deref_out IUICommandHandler **ppCommandHandler) {
  HRESULT hr = S_OK;
  if (!ppCommandHandler) return E_POINTER;
  *ppCommandHandler = NULL;
  
  CCommandHandler* pCommandHandler = new CCommandHandler();
  if (pCommandHandler != NULL) {
     pCommandHandler->ribbonPtr = p_ribbon;
    *ppCommandHandler = static_cast<IUICommandHandler *>(pCommandHandler);
  } else {
    hr = E_OUTOFMEMORY;
  }
  return hr;
}; /* CCommandHandler::CreateInstance */

STDMETHODIMP_(ULONG) CCommandHandler::AddRef() {
  return InterlockedIncrement(&m_cRef);
}; /* CCommandHandler::AddRef */

STDMETHODIMP_(ULONG) CCommandHandler::Release() {
  LONG cRef = InterlockedDecrement(&m_cRef);
  if (cRef == 0) delete this;
  return cRef;
}; /* CCommandHandler::Release */

STDMETHODIMP CCommandHandler::QueryInterface(REFIID iid, void** ppv) {
  if (iid == __uuidof(IUnknown)) {
    *ppv = static_cast<IUnknown*>(this);
  } else if (iid == __uuidof(IUICommandHandler)) {
    *ppv = static_cast<IUICommandHandler*>(this);
  } else {
    *ppv = NULL;
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}; /* CCommandHandler::QueryInterface */

/*
 *  UpdateProperty:
 * ------------------------------
 *  Called by the Ribbon framework when a command property (PKEY)
 *  needs to be updated.
 *  This function is used to provide new command property values,
 *  such as labels, icons, or tooltip information, when requested
 *  by the Ribbon framework.
 */
STDMETHODIMP CCommandHandler::UpdateProperty(
  UINT nCmdID,
  __in REFPROPERTYKEY key,
  __in_opt const PROPVARIANT* ppropvarCurrentValue,
  __out PROPVARIANT* ppropvarNewValue) {
  Tcl_Obj **objv, *obj;
  int i, status, objc;
  enum TkRibbonTypes type;
  const char *property_str = Tk_Ribbon_FindProperty(key, &type);
  UNREFERENCED_PARAMETER(nCmdID);
  UNREFERENCED_PARAMETER(key);
  UNREFERENCED_PARAMETER(ppropvarCurrentValue);
  UNREFERENCED_PARAMETER(ppropvarNewValue);
  if (!ribbonPtr || type == TK_RIBBON_TYPE_UNSUPPORTED) return E_NOTIMPL;
  if (ribbonPtr->commandPtr == NULL) return E_NOTIMPL;
  /* Ensure that the command is not the empty string! */
  status = Tcl_ListObjLength(NULL, ribbonPtr->commandPtr, &objc);
  if (status != TCL_OK || objc < 1) return E_NOTIMPL;
  /*
   * Generate the script to execute...
   */
  Tcl_Obj *script_objs = Tcl_DuplicateObj(ribbonPtr->commandPtr);
  /*
   * Append the callback arguments...
   */
  Tcl_ListObjAppendElement(NULL, script_objs, Tcl_NewLongObj(nCmdID));
  Tcl_ListObjAppendElement(NULL, script_objs,
                                 Tcl_NewStringObj(property_str, -1));
  /* Append the current value (if successful)... */
  if (ppropvarCurrentValue) {
    obj = Tk_Ribbon_Variant2Tcl(NULL, type, ppropvarCurrentValue);
    if (obj) {
      Tcl_ListObjAppendElement(NULL, script_objs, obj);
    }
  }
  Tcl_ListObjGetElements(NULL, script_objs, &objc, &objv);
  for (i = 0; i < objc; ++i) Tcl_IncrRefCount(objv[i]);
  status = Tcl_EvalObjv(ribbonPtr->interp, objc, objv, TCL_EVAL_GLOBAL);
  for (i = 0; i < objc; ++i) Tcl_DecrRefCount(objv[i]);
  if (status != TCL_OK) return E_NOTIMPL;
  /*
   * When we modify collections (i.e. UI_PKEY_ItemsSource, UI_PKEY_Categories)
   * we modify the existing object instead of creating a new one...
   */
  if (key == UI_PKEY_ItemsSource) {
    /*
     * In item sources, we expect a list of three items:
     *   - The list of items,
     *   - The list of images or a single image element for all items,
     *   - The list of categories, or a single category for all items.
     */
    status = Tcl_ListObjGetElements(ribbonPtr->interp,
                                    Tcl_GetObjResult(ribbonPtr->interp),
                                    &objc, &objv);
    if (status != TCL_OK || objc != 3) return E_NOTIMPL;
    status = Tk_Ribbon_Tcl2Variant(ribbonPtr, ribbonPtr->interp, type, 
                                   objv[0], objv[1], objv[2],
                                   key, (PROPVARIANT *) ppropvarCurrentValue);
  } else if (key == UI_PKEY_Categories) {
    /*
     * In category sources, we expect a list of two items:
     *   - The list of items,
     *   - The list of categories, or a single category for all items
     *     If the element is the empty string, autoincrement (starting from 0)
     *     is used.
     */
    status = Tcl_ListObjGetElements(ribbonPtr->interp,
                                    Tcl_GetObjResult(ribbonPtr->interp),
                                    &objc, &objv);
    if (status != TCL_OK || objc != 2) return E_NOTIMPL;
    status = Tk_Ribbon_Tcl2Variant(ribbonPtr, ribbonPtr->interp, type, 
                                   objv[0], NULL, objv[1],
                                   key, (PROPVARIANT *) ppropvarCurrentValue);
  } else {
    status = Tk_Ribbon_Tcl2Variant(ribbonPtr, ribbonPtr->interp, type, 
                                   Tcl_GetObjResult(ribbonPtr->interp),
                                   NULL, NULL, key, ppropvarNewValue);
  }
  if (status == TCL_OK) return S_OK;
  return E_NOTIMPL;
}; /* CCommandHandler::UpdateProperty */

/*
 *  Execute:
 * ------------------------------
 *  Called by the Ribbon framework when a command is executed by the user.
 *  For example, when a button is pressed.
 */
STDMETHODIMP CCommandHandler::Execute(
    UINT nCmdID,
    UI_EXECUTIONVERB verb,
    __in_opt const PROPERTYKEY* key,
    __in_opt const PROPVARIANT* ppropvarValue,
    __in_opt IUISimplePropertySet* pCommandExecutionProperties) {
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);
    UNREFERENCED_PARAMETER(ppropvarValue);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(nCmdID);
    UINT event = 0, i;
    if (!ribbonPtr) return S_OK;
    switch (verb) {
      case UI_EXECUTIONVERB_EXECUTE:
        event = TK_RIBBON_STR_onExecute; /* <<onExecute>> */
        break;
      case UI_EXECUTIONVERB_PREVIEW:
        event = TK_RIBBON_STR_onPreview; /* <<onPreview>> */
        break;
      case UI_EXECUTIONVERB_CANCELPREVIEW:
        event = TK_RIBBON_STR_onCancelPreview; /* <<onCancelPreview>> */
    }

    ribbonPtr->cmd_objects[3] = ribbonPtr->cmd_types[event];
    ribbonPtr->cmd_objects[5] = Tcl_NewLongObj(nCmdID);
    for (i=0; i<6; ++i) Tcl_IncrRefCount(ribbonPtr->cmd_objects[i]);
    Tcl_EvalObjv(ribbonPtr->interp, 6, ribbonPtr->cmd_objects, TCL_EVAL_GLOBAL);
    for (i=0; i<6; ++i) Tcl_DecrRefCount(ribbonPtr->cmd_objects[i]);
    ribbonPtr->cmd_objects[3] = NULL;
    ribbonPtr->cmd_objects[5] = NULL;
    return S_OK;
}; /* CCommandHandler::Execute */
