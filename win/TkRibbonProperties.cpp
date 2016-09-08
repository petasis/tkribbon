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
#include <UIRibbonPropertyHelpers.h>
#include "shlwapi.h"

typedef struct TkRibbonValue {
  PROPERTYKEY        key;
  enum TkRibbonTypes type;
} TkRibbonValue;

static const char *RibbonStatePropertyNames[] = {
  "UI_PKEY_BooleanValue",
  "UI_PKEY_ContextAvailable",
  "UI_PKEY_DecimalPlaces",
  "UI_PKEY_DecimalValue",
  "UI_PKEY_Enabled",
  "UI_PKEY_FormatString",
  "UI_PKEY_Increment",
  "UI_PKEY_MaxValue",
  "UI_PKEY_MinValue",
  "UI_PKEY_Pinned",
  "UI_PKEY_RecentItems",
  "UI_PKEY_RepresentativeString",
  "UI_PKEY_StringValue",
  NULL
};

static const TkRibbonValue RibbonStatePropertyValues[] = {
  {UI_PKEY_BooleanValue,         TK_RIBBON_TYPE_BOOLEAN},
  {UI_PKEY_ContextAvailable,     TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_DecimalPlaces,        TK_RIBBON_TYPE_UINT},
  {UI_PKEY_DecimalValue,         TK_RIBBON_TYPE_DECIMAL},
  {UI_PKEY_Enabled,              TK_RIBBON_TYPE_BOOLEAN},
  {UI_PKEY_FormatString,         TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_Increment,            TK_RIBBON_TYPE_DECIMAL},
  {UI_PKEY_MaxValue,             TK_RIBBON_TYPE_DECIMAL},
  {UI_PKEY_MinValue,             TK_RIBBON_TYPE_DECIMAL},
  {UI_PKEY_Pinned,               TK_RIBBON_TYPE_BOOLEAN},
  {UI_PKEY_RecentItems,          TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_RepresentativeString, TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_StringValue,          TK_RIBBON_TYPE_WSTR}
};

static const char *RibbonResourcePropertyNames[] = {
  "UI_PKEY_Keytip",
  "UI_PKEY_Label",
  "UI_PKEY_LabelDescription",
  "UI_PKEY_LargeHighContrastImage",
  "UI_PKEY_LargeImage",
  "UI_PKEY_SmallHighContrastImage",
  "UI_PKEY_SmallImage",
  "UI_PKEY_TooltipDescription",
  "UI_PKEY_TooltipTitle",
  NULL
};

static const TkRibbonValue RibbonResourcePropertyValues[] = {
  {UI_PKEY_Keytip,                 TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_Label,                  TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_LabelDescription,       TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_LargeHighContrastImage, TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_LargeImage,             TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_SmallHighContrastImage, TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_SmallImage,             TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_TooltipDescription,     TK_RIBBON_TYPE_WSTR},
  {UI_PKEY_TooltipTitle,           TK_RIBBON_TYPE_WSTR}
};

static const char *RibbonRibbonPropertyNames[] = {
  "UI_PKEY_Minimized",
  "UI_PKEY_QuickAccessToolbarDock",
  "UI_PKEY_Viewable",
  NULL
};

static const TkRibbonValue RibbonRibbonPropertyValues[] = {
  {UI_PKEY_Minimized,                 TK_RIBBON_TYPE_BOOLEAN},
  {UI_PKEY_QuickAccessToolbarDock,    TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_Viewable,                  TK_RIBBON_TYPE_BOOLEAN},
};

static const char *RibbonCollectionPropertyNames[] = {
  "UI_PKEY_Categories",
  "UI_PKEY_CategoryId",
  "UI_PKEY_CommandId",
  "UI_PKEY_CommandType",
  "UI_PKEY_ItemImage",
  "UI_PKEY_ItemsSource",
  "UI_PKEY_SelectedItem",
  NULL
};

static const TkRibbonValue RibbonCollectionPropertyValues[] = {
  {UI_PKEY_Categories,      TK_RIBBON_TYPE_UICOLLECTION},
  {UI_PKEY_CategoryId,      TK_RIBBON_TYPE_UINT},
  {UI_PKEY_CommandId,       TK_RIBBON_TYPE_UINT},
  {UI_PKEY_CommandType,     TK_RIBBON_TYPE_UINT},
  {UI_PKEY_ItemImage,       TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_ItemsSource,     TK_RIBBON_TYPE_UICOLLECTION},
  {UI_PKEY_SelectedItem,    TK_RIBBON_TYPE_UINT}
};

static const char *RibbonFrameworkPropertyNames[] = {
  "UI_PKEY_GlobalBackgroundColor",
  "UI_PKEY_GlobalHighlightColor",
  "UI_PKEY_GlobalTextColor",
  NULL
};

static const TkRibbonValue RibbonFrameworkPropertyValues[] = {
  {UI_PKEY_GlobalBackgroundColor, TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_GlobalHighlightColor,  TK_RIBBON_TYPE_UNSUPPORTED},
  {UI_PKEY_GlobalTextColor,       TK_RIBBON_TYPE_UNSUPPORTED}
};

const char* Tk_Ribbon_FindProperty(const PROPERTYKEY key,
                                   enum TkRibbonTypes *type) {
  unsigned int i;
  for (i = 0; RibbonStatePropertyNames[i] != NULL; ++i) {
    if (RibbonStatePropertyValues[i].key == key) {
      if (type) *type = RibbonStatePropertyValues[i].type;
      return RibbonStatePropertyNames[i];
    }
  }
  for (i = 0; RibbonResourcePropertyNames[i] != NULL; ++i) {
    if (RibbonResourcePropertyValues[i].key == key) {
      if (type) *type = RibbonResourcePropertyValues[i].type;
      return RibbonResourcePropertyNames[i];
    }
  }
  for (i = 0; RibbonCollectionPropertyNames[i] != NULL; ++i) {
    if (RibbonCollectionPropertyValues[i].key == key) {
      if (type) *type = RibbonCollectionPropertyValues[i].type;
      return RibbonCollectionPropertyNames[i];
    }
  }
  for (i = 0; RibbonRibbonPropertyNames[i] != NULL; ++i) {
    if (RibbonRibbonPropertyValues[i].key == key) {
      if (type) *type = RibbonRibbonPropertyValues[i].type;
      return RibbonRibbonPropertyNames[i];
    }
  }
  for (i = 0; RibbonFrameworkPropertyNames[i] != NULL; ++i) {
    if (RibbonFrameworkPropertyValues[i].key == key) {
      if (type) *type = RibbonFrameworkPropertyValues[i].type;
      return RibbonFrameworkPropertyNames[i];
    }
  }
  return NULL;
}; /* Tk_Ribbon_FindProperty */

int Tk_Ribbon_String2Property(Tcl_Interp *interp, Tcl_Obj *property,
                              enum TkRibbonTypes *type,
                              PROPERTYKEY *key) {
  int index;
  /* State property? */
  if (Tcl_GetIndexFromObj(interp, property, RibbonStatePropertyNames,
                          "property", 0, &index) == TCL_OK) {
    if (type) *type = RibbonStatePropertyValues[index].type;
    if (key)  *key  = RibbonStatePropertyValues[index].key;
  } else 
  /* Resource property? */
  if (Tcl_GetIndexFromObj(interp, property, RibbonResourcePropertyNames,
                          "property", 0, &index) == TCL_OK) {
    if (type) *type = RibbonResourcePropertyValues[index].type;
    if (key)  *key  = RibbonResourcePropertyValues[index].key;
  } else
  /* Ribbon property? */
  if (Tcl_GetIndexFromObj(interp, property, RibbonRibbonPropertyNames,
                          "property", 0, &index) == TCL_OK) {
    if (type) *type = RibbonRibbonPropertyValues[index].type;
    if (key)  *key  = RibbonRibbonPropertyValues[index].key;
  } else
  /* Collection property? */
  if (Tcl_GetIndexFromObj(interp, property, RibbonCollectionPropertyNames,
                          "property", 0, &index) == TCL_OK) {
    if (type) *type = RibbonCollectionPropertyValues[index].type;
    if (key)  *key  = RibbonCollectionPropertyValues[index].key;
  } else
  /* Framework property? */
  if (Tcl_GetIndexFromObj(interp, property, RibbonFrameworkPropertyNames,
                          "property", 0, &index) == TCL_OK) {
    if (type) *type = RibbonFrameworkPropertyValues[index].type;
    if (key)  *key  = RibbonFrameworkPropertyValues[index].key;
  } else {
    Tcl_SetResult(interp, "unknown property", TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}; /* Tk_Ribbon_String2Property */

int Tk_Ribbon_Tcl2Variant(TkRibbon *ribbonPtr, Tcl_Interp *interp,
                          enum TkRibbonTypes type,
                          Tcl_Obj *value_obj,
                          Tcl_Obj *images_obj,
                          Tcl_Obj *categories_obj,
                          REFPROPERTYKEY key, PROPVARIANT *var) {
  HRESULT hr;
  switch (type) {
    case TK_RIBBON_TYPE_BOOLEAN: {
      BOOL value;
      if (Tcl_GetBooleanFromObj(interp, value_obj, &value) != TCL_OK) {
        return TCL_ERROR;
      }
      hr = UIInitPropertyFromBoolean(key, value, var);
      break;
    }
    case TK_RIBBON_TYPE_UINT:
    case TK_RIBBON_TYPE_INT: {
      int value;
      if (Tcl_GetIntFromObj(interp, value_obj, &value) != TCL_OK) {
        return TCL_ERROR;
      }
      hr = UIInitPropertyFromUInt32(key, value, var);
      break;
    }
    case TK_RIBBON_TYPE_WSTR: {
      int len;
      WCHAR *value = (WCHAR *) Tcl_GetUnicodeFromObj(value_obj, &len);
      if (value == NULL) return TCL_ERROR;
      hr = UIInitPropertyFromString(key, value, var);
      break;
    }
    case TK_RIBBON_TYPE_UICOLLECTION_SIMPLE:
    case TK_RIBBON_TYPE_UICOLLECTION: {
      HBITMAP bitmap = NULL;
      Tcl_Obj **objv, **iobjv, **cobjv;
      int i, objc, iobjc = 0, cobjc = 0, len, cat_auto = 0,
          category = UI_COLLECTION_INVALIDINDEX;
      IUICollection *value;
      IUIImage      *image = NULL;
      if (Tcl_ListObjGetElements(interp, value_obj, &objc, &objv) != TCL_OK) {
        return TCL_ERROR;
      }
      hr = var->punkVal->QueryInterface(IID_PPV_ARGS(&value));
      if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);
      if (images_obj && ribbonPtr->imageFactory) {
        /* This object holds either an image, or a list of images! */
        if (Tcl_ListObjGetElements(interp, images_obj, &iobjc, &iobjv)
                                                                    != TCL_OK) {
          return TCL_ERROR;
        }
      }
      if (categories_obj) {
        /* This object holds either a category, or a list of categories! */
        if (Tcl_ListObjGetElements(interp, categories_obj, &cobjc, &cobjv)
                                                                    != TCL_OK) {
          return TCL_ERROR;
        }
        if (cobjc == 0) {
          category = 0;
          cat_auto = 1;
        }
      }

      /* Add all items... */
      value->Clear();
      for (i = 0; i < objc; ++i) {
        /* Create a new property set for each item... */
        CPropertySet *item;
        hr = CPropertySet::CreateInstance(&item);
        if (FAILED(hr)) {
          value->Release();
          return TkRibbon_ErrorMsg(interp, hr);
        }
        /*
         * Handle images...
         */
        if (i < iobjc) {
          HBITMAP   hbm = NULL;
          WORD      resource_id;
          LPCTSTR   name = NULL;
          HINSTANCE hinst;
          /* Can we get an integer from the image element? */
          if (Tcl_GetIntFromObj(NULL, iobjv[i],
                                      (int *) &resource_id) == TCL_OK) {
            hinst = ribbonPtr->dll_handle;
            name  = MAKEINTRESOURCE(resource_id);
          } else {
            hinst = NULL;
            name  = Tcl_GetStringFromObj(iobjv[i], &len);
            if (len == 0) name = NULL;
          }
          if (name) {
            hbm = (HBITMAP) LoadImage(hinst, name, IMAGE_BITMAP,
                                      0, 0, LR_CREATEDIBSECTION);
            if (hbm) {
              hr = ribbonPtr->imageFactory->CreateImage(hbm,
                              UI_OWNERSHIP_TRANSFER, &image);
              if (FAILED(hr)) DeleteObject(hbm);
            }
          } else {
            image = NULL;
          }
        } else if (iobjc != 1) {
          image = NULL;
        }
        /*
         * Handle categories...
         */
        if (cat_auto) {
          category = i;
        } else {
          if (i < cobjc) {
            if (Tcl_GetIntFromObj(NULL, cobjv[i], &category) != TCL_OK) {
              category = UI_COLLECTION_INVALIDINDEX;
            }
          } else if (cobjc != 1) {
            category = UI_COLLECTION_INVALIDINDEX;
          }
        }
        /*printf(" + %p %d %s\n", image, category, Tcl_GetString(objv[i]));*/
        item->InitializeItemProperties(image, /* Image */
                        (WCHAR *) Tcl_GetUnicodeFromObj(objv[i], &len),
                        category);
        value->Add(item);
        item->Release();
      }
      value->Release();
      break;
    }
    case TK_RIBBON_TYPE_UNSUPPORTED:
    default:
      goto unsupported;
  }
  if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);
  return TCL_OK;
unsupported:
  if (interp) {
    Tcl_SetResult(interp, "unsupported property value type", TCL_STATIC);
  }
  return TCL_ERROR;
}; /* Tk_Ribbon_Tcl2Variant */

Tcl_Obj *Tk_Ribbon_Variant2Tcl(Tcl_Interp *interp, enum TkRibbonTypes type, 
                               const PROPVARIANT *var) {
  Tcl_Obj *obj;
  HRESULT hr;
  if (var == NULL) return NULL;
  switch (type) {
    case TK_RIBBON_TYPE_BOOLEAN: {
      BOOL value;
      hr = PropVariantToBoolean(*var, &value);
      if (FAILED(hr)) goto error;
      obj = Tcl_NewBooleanObj(value);
      break;
    }
    case TK_RIBBON_TYPE_UINT: {
      unsigned int value;
      hr = PropVariantToUInt32(*var, &value);
      if (FAILED(hr)) goto error;
      obj = Tcl_NewIntObj(value);
      break;
    }
    case TK_RIBBON_TYPE_INT: {
      int value;
      hr = PropVariantToInt32(*var, &value);
      if (FAILED(hr)) goto error;
      obj = Tcl_NewIntObj(value);
      break;
    }
    case TK_RIBBON_TYPE_WSTR: {
      WCHAR value[2048];
      hr = PropVariantToString(*var, value, 2048);
      if (FAILED(hr)) goto error;
      obj = Tcl_NewUnicodeObj((Tcl_UniChar *)value, -1);
      break;
    }
    case TK_RIBBON_TYPE_UNSUPPORTED:
    default:
      if (interp) {
        Tcl_SetResult(interp, "unsupported property value type", TCL_STATIC);
      }
      return NULL;
  }
  return obj;
error:
  TkRibbon_ErrorMsg(interp, hr);
  return NULL;
}; /* Tk_Ribbon_Variant2Tcl */

int TkRibbonGetProperty(Tcl_Interp *interp, TkRibbon *ribbonPtr,
                        Tcl_Obj *property, Tcl_Obj *obj) {
  int status;
  PROPVARIANT var;
  enum TkRibbonTypes type;
  PROPERTYKEY key;
  HRESULT hr;
  long id;

  if (ribbonPtr->framework == NULL) {
    Tcl_SetResult(interp, "uninitialised framework", TCL_STATIC);
    return TCL_ERROR;
  }

  if (Tcl_GetLongFromObj(interp, obj, &id) != TCL_OK) return TCL_ERROR;

  status = Tk_Ribbon_String2Property(interp, property, &type, &key);
  if (status != TCL_OK) return status;

  hr = ribbonPtr->framework->GetUICommandProperty(id, key, &var);
  if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);

  obj = Tk_Ribbon_Variant2Tcl(interp, type, &var);
  if (obj == NULL) return TCL_ERROR;
  Tcl_SetObjResult(interp, obj);
  return TCL_OK;
}; /* TkRibbonGetProperty */

int TkRibbonSetProperty(Tcl_Interp *interp, TkRibbon *ribbonPtr,
                        Tcl_Obj *property, Tcl_Obj *obj,
                        Tcl_Obj *value_obj,
                        Tcl_Obj *images_obj = NULL,
                        Tcl_Obj *categories_obj = NULL) {
  int status;
  PROPVARIANT var;
  enum TkRibbonTypes type;
  PROPERTYKEY key;
  HRESULT hr;
  long id;

  if (ribbonPtr->framework == NULL) {
    Tcl_SetResult(interp, "uninitialised framework", TCL_STATIC);
    return TCL_ERROR;
  }

  if (Tcl_GetLongFromObj(interp, obj, &id) != TCL_OK) return TCL_ERROR;

  status = Tk_Ribbon_String2Property(interp, property, &type, &key);
  if (status != TCL_OK) return status;

  /* If type is a collection, try to get the current collection object... */
  if (type == TK_RIBBON_TYPE_UICOLLECTION ||
      type == TK_RIBBON_TYPE_UICOLLECTION_SIMPLE) {
    hr = ribbonPtr->framework->GetUICommandProperty(id, key, &var);
    if (FAILED(hr)) return TkRibbon_ErrorMsg(interp, hr);
  }
  
  if (Tk_Ribbon_Tcl2Variant(ribbonPtr, interp, type,
                            value_obj,
                            images_obj,
                            categories_obj, key, &var) != TCL_OK) {
    return TCL_ERROR;
  }

  if (type == TK_RIBBON_TYPE_UICOLLECTION ||
      type == TK_RIBBON_TYPE_UICOLLECTION_SIMPLE) {
    return TCL_OK;
  }

  hr = ribbonPtr->framework->SetUICommandProperty(id, key, var);
  if (FAILED(hr)) {
    if (HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) == hr) {
      Tcl_SetResult(interp, "use invalidate_property for this property",
                             TCL_STATIC);
      return TCL_ERROR;
    } else {
      return TkRibbon_ErrorMsg(interp, hr);
    }
  }
  return TCL_OK;
}; /* TkRibbonSetProperty */
