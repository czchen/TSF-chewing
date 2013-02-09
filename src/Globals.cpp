//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  Globals.cpp
//
//          Global variables
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1; // -1 /w no refs, for win95 InterlockedIncrement/Decrement compat

CRITICAL_SECTION g_cs;

/* 280B1AF2-59AD-46AD-811D-FEEA77DBEC1D */
const CLSID c_clsidTextService = {
    0x280B1AF2,
    0x59AD,
    0x46AD,
    {0x81, 0x1D, 0xFE, 0xEA, 0x77, 0xDB, 0xEC, 0x1D}
  };
/* 280B1AF2-59AD-46AD-811D-FEEA77DBEC1D */
const GUID c_guidProfile = {
	0x280B1AF2,
    0x59AD,
    0x46AD,
    {0x81, 0x1D, 0x1D, 0xFE, 0xEA, 0x77, 0xDB, 0x1D}
  };

//
//  define two guids for display attribute info. This textservice has
//  two display attribute. One is for input text and the other is for the
//  converted text.
//
//      c_guidDisplayAttributeInput 
//      c_guidDisplayAttributeConverted
//
/* 4e1aa3fe-6c7f-11d7-a6ec-00065b84435c */
const GUID c_guidDisplayAttributeInput = { 
    0x4e1aa3fe,
    0x6c7f,
    0x11d7,
    {0xa6, 0xec, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* 4e1aa3ff-6c7f-11d7-a6ec-00065b84435c */
const GUID c_guidDisplayAttributeConverted = { 
    0x4e1aa3ff,
    0x6c7f,
    0x11d7,
    {0xa6, 0xec, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

/* 4e1aa3ff-6c7f-11d7-a6ec-00065b84435c */
const GUID c_guidLangBarItemButton = { 
    0x4e1aa3ff,
    0x6c7f,
    0x11d7,
    {0xa6, 0xec, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

