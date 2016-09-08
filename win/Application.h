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

class CApplication : public IUIApplication {
  public:
    TkRibbon *ribbonPtr;
    /* Static method to create an instance of the object... */
    static HRESULT CreateInstance(TkRibbon *p_ribbon,
                                  __deref_out IUIApplication **ppApplication);

    /* IUnknown methods... */
    STDMETHOD_(ULONG, AddRef());
    STDMETHOD_(ULONG, Release());
    STDMETHOD(QueryInterface(REFIID iid, void** ppv));

    /* IUIApplication methods... */
    STDMETHOD(OnCreateUICommand)(UINT nCmdID,
        __in UI_COMMANDTYPE typeID,
        __deref_out IUICommandHandler** ppCommandHandler);

    STDMETHOD(OnViewChanged)(UINT viewId,
        __in UI_VIEWTYPE typeId,
        __in IUnknown* pView,
        UI_VIEWVERB verb,
        INT uReasonCode);

    STDMETHOD(OnDestroyUICommand)(UINT32 commandId, 
        __in UI_COMMANDTYPE typeID,
        __in_opt IUICommandHandler* commandHandler);

    int GetHeight(void);

  private:
    CApplication() : m_cRef(1) , m_pCommandHandler(NULL),
                     ribbonPtr(NULL), g_pRibbon(NULL), uRibbonHeight(30) {}

    ~CApplication() {
      if (m_pCommandHandler) {
        m_pCommandHandler->Release();
        m_pCommandHandler = NULL;
      }
      if (g_pRibbon) {
        g_pRibbon->Release();
        g_pRibbon = NULL;
      }
    }

    LONG                m_cRef;             // Reference count.
    IUICommandHandler  *m_pCommandHandler;  // Generic Command Handler
    UINT                uRibbonHeight;
    IUIRibbon*          g_pRibbon;
}; /* class CApplication */
