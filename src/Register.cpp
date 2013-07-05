//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  Register.cpp
//
//          Server registration code.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <ole2.h>
#include "msctf.h"
#include "globals.h"

#define CLSID_STRLEN 38  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")

static const WCHAR c_szInfoKeyPrefix[] = L"CLSID\\";
static const WCHAR c_szInProcSvr32[] = L"InProcServer32";
static const WCHAR c_szModelName[] = L"ThreadingModel";

//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL RegisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    WCHAR achIconFile[MAX_PATH];
    WCHAR achFileName[MAX_PATH];
    DWORD cch;
    int cchIconFile=0;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return E_FAIL;

    hr = pInputProcessProfiles->Register(c_clsidTextService);

    if (hr != S_OK)
        goto Exit;

    cch = GetModuleFileNameW(g_hInst, achFileName, ARRAYSIZE(achFileName));


    achIconFile[cchIconFile] = '\0';

    hr = pInputProcessProfiles->AddLanguageProfile(c_clsidTextService,
                                  TEXTSERVICE_LANGID,
                                  c_guidProfile,
                                  TEXTSERVICE_DESC,
                                  (ULONG)wcslen(TEXTSERVICE_DESC),
                                  achIconFile,
                                  cchIconFile,
                                  TEXTSERVICE_ICON_INDEX);

Exit:
    pInputProcessProfiles->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void UnregisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return;

    pInputProcessProfiles->Unregister(c_clsidTextService);
    pInputProcessProfiles->Release();
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL RegisterCategories()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (hr != S_OK)
        return FALSE;

    //
    // register this text service to GUID_TFCAT_TIP_KEYBOARD category.
    //
    hr = pCategoryMgr->RegisterCategory(c_clsidTextService,
                                        GUID_TFCAT_TIP_KEYBOARD,
                                        c_clsidTextService);

    //
    // register this text service to GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER category.
    //
    hr = pCategoryMgr->RegisterCategory(c_clsidTextService,
                                        GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
                                        c_clsidTextService);


    pCategoryMgr->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterCategories
//
//----------------------------------------------------------------------------

void UnregisterCategories()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (hr != S_OK)
        return;

    //
    // unregister this text service from GUID_TFCAT_TIP_KEYBOARD category.
    //
    pCategoryMgr->UnregisterCategory(c_clsidTextService,
                                     GUID_TFCAT_TIP_KEYBOARD,
                                     c_clsidTextService);

    //
    // unregister this text service from GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER category.
    //
    pCategoryMgr->UnregisterCategory(c_clsidTextService,
                                     GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
                                     c_clsidTextService);

    pCategoryMgr->Release();
    return;
}
// CLSIDToStringA
//
//----------------------------------------------------------------------------

BOOL CLSIDToStringW(REFGUID refGUID, WCHAR *pchW)
{
    static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                   8, 9, '-', 10, 11, 12, 13, 14, 15};

    static const WCHAR szDigits[] = L"0123456789ABCDEF";

    int i;
    WCHAR *p = pchW;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = L'{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = '-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = L'}';
    *p   = '\0';

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------
LONG RecurseDeleteKey(HKEY hParentKey, LPCWSTR lpszKey)
{
    HKEY hKey;
    LONG lRes;
    FILETIME time;
    WCHAR szBuffer[256];
    DWORD dwSize = ARRAYSIZE(szBuffer);

    if (RegOpenKeyW(hParentKey, lpszKey, &hKey) != ERROR_SUCCESS)
        return ERROR_SUCCESS; // let's assume it couldn't be opened because it's not there

    lRes = ERROR_SUCCESS;
    while (RegEnumKeyExW(hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time)==ERROR_SUCCESS)
    {
        szBuffer[ARRAYSIZE(szBuffer)-1] = '\0';
        lRes = RecurseDeleteKey(hKey, szBuffer);
        if (lRes != ERROR_SUCCESS)
            break;
        dwSize = ARRAYSIZE(szBuffer);
    }
    RegCloseKey(hKey);

    return lRes == ERROR_SUCCESS ? RegDeleteKeyW(hParentKey, lpszKey) : lRes;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL RegisterServer()
{
    DWORD dw;
    HKEY hKey;
    HKEY hSubKey;
    BOOL fRet;
    WCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];
    WCHAR achFileName[MAX_PATH];

    if (!CLSIDToStringW(c_clsidTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return FALSE;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(WCHAR));


    if (fRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw)
            == ERROR_SUCCESS)
    {
        fRet &= RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)TEXTSERVICE_DESC, (wcslen(TEXTSERVICE_DESC)+1)*sizeof(wchar_t))
            == ERROR_SUCCESS;

        if (fRet &= RegCreateKeyExW(hKey, c_szInProcSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, &dw)
            == ERROR_SUCCESS)
        {
            dw = GetModuleFileNameW(g_hInst, achFileName, ARRAYSIZE(achFileName));
            fRet &= RegSetValueExW(hSubKey, NULL, 0, REG_SZ, (BYTE *)achFileName, (wcslen(achFileName)+1)*sizeof(achFileName[0])) == ERROR_SUCCESS;
            fRet &= RegSetValueExW(hSubKey, c_szModelName, 0, REG_SZ, (BYTE*)TEXTSERVICE_MODEL, (wcslen(TEXTSERVICE_MODEL)+1)*sizeof(wchar_t)) == ERROR_SUCCESS;
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    return fRet;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void UnregisterServer()
{
    WCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];

    if (!CLSIDToStringW(c_clsidTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(WCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
