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

#include "TkRibbon.h"

class CCommandHandler : public IUICommandHandler {
  public:
    TkRibbon *ribbonPtr;
    /* Static method to create an instance of the object... */
    __checkReturn static HRESULT CreateInstance(TkRibbon *p_ribbon,
                       __deref_out IUICommandHandler **ppCommandHandler);

    /* IUnknown methods... */
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);

    /* IUICommandHandler methods... */
    STDMETHOD(UpdateProperty)(UINT nCmdID,
        __in REFPROPERTYKEY key,
        __in_opt const PROPVARIANT* ppropvarCurrentValue,
        __out PROPVARIANT* ppropvarNewValue);

    STDMETHOD(Execute)(UINT nCmdID,
        UI_EXECUTIONVERB verb, 
        __in_opt const PROPERTYKEY* key,
        __in_opt const PROPVARIANT* ppropvarValue,
        __in_opt IUISimplePropertySet* pCommandExecutionProperties);

  private:
    CCommandHandler() : m_cRef(1), ribbonPtr(NULL) {}

    LONG m_cRef;                        // Reference count.

}; /* class CCommandHandler */
