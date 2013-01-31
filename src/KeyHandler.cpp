//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  KeyHandler.cpp
//
//          the handler routines for key events
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "EditSession.h"
#include "TextService.h"
#include "CandidateList.h"

#include "Util.h"

//+---------------------------------------------------------------------------
//
// CKeyHandlerEditSession
//
//----------------------------------------------------------------------------

class CKeyHandlerEditSession : public CEditSessionBase
{
public:
    CKeyHandlerEditSession(CTextService *pTextService, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pTextService, pContext)
    {
        _wParam = wParam;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    WPARAM _wParam;
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CKeyHandlerEditSession::DoEditSession(TfEditCookie ec)
{
    return _pTextService->_HandleKey(ec, _pContext, _wParam);
}

//+---------------------------------------------------------------------------
//
// IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//
//----------------------------------------------------------------------------

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover)
{
    LONG lResult;

    if (pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult) != S_OK ||
        lResult > 0)
    {
        return FALSE;
    }

    if (pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult) != S_OK ||
        lResult < 0)
    {
        return FALSE;
    }

    return TRUE;
}

HRESULT CTextService::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{

    /*
     * FIXME: the following keys are not handled:
     * shift left		VK_LSHIFT
     * shift right		VK_RSHIFT
     * caps lock		VK_CAPITAL
     * page up			VK_PRIOR
     * page down		VK_NEXT
     * ctrl num
     * shift space
     * numlock num		VK_NUMLOCK
	 */

    if (('A' <= wParam && wParam <= 'Z')) {
            chewing_handle_Default(mChewingContext, wParam - 'A' + 'a');
    } else if ('0' <= wParam && wParam <= '9') {
            chewing_handle_Default(mChewingContext, wParam);
    } else {
        switch(wParam) {
            case VK_OEM_COMMA:
                chewing_handle_Default(mChewingContext, ',');
                break;

            case VK_OEM_MINUS:
                chewing_handle_Default(mChewingContext, '-');
                break;

            case VK_OEM_PERIOD:
                chewing_handle_Default(mChewingContext, '.');
                break;

            case VK_OEM_1:
                chewing_handle_Default(mChewingContext, ';');
                break;

            case VK_OEM_2:
                chewing_handle_Default(mChewingContext, '/');
                break;

            case VK_OEM_3:
                chewing_handle_Default(mChewingContext, '`');
                break;

            case VK_SPACE:
                chewing_handle_Space(mChewingContext);
                break;

            case VK_ESCAPE:
                chewing_handle_Esc(mChewingContext);
                break;

            case VK_RETURN:
                chewing_handle_Enter(mChewingContext);
                break;

            case VK_DELETE:
                chewing_handle_Del(mChewingContext);
                break;

            case VK_BACK:
                chewing_handle_Backspace(mChewingContext);
                break;

            case VK_UP:
                chewing_handle_Up(mChewingContext);
                break;

            case VK_DOWN:
                chewing_handle_Down(mChewingContext);

            case VK_LEFT:
                chewing_handle_Left(mChewingContext);
                break;

            case VK_RIGHT:
                chewing_handle_Right(mChewingContext);
                break;

            case VK_HOME:
                chewing_handle_Home(mChewingContext);
                break;

            case VK_END:
                chewing_handle_End(mChewingContext);
                break;

            default:
                return S_OK;
        }
    }

    // Remove old candidate list. We will create a new one if necessary.
    _pCandidateList->_EndCandidateList();

    ChewingCandidates candidate(mChewingContext);
    if (!candidate.IsEmpty()) {
        _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeConverted);

        //
        // The document manager object is not cached. Get it from pContext.
        //
        ITfDocumentMgr *pDocumentMgr;
        if (pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
        {
            //
            // get the composition range.
            //
            ITfRange *pRange;
            if (_pComposition->GetRange(&pRange) == S_OK)
            {
                _pCandidateList->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, candidate);
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
        return S_OK;
    }

    ChewingString commit(mChewingContext, CHEWING_STRING_COMMIT);
    if (!commit.IsEmpty()) {
        // FIXME: Need a better way to submit a string
        if (!_IsComposing())
            _StartComposition(pContext);

        ULONG cFetched;
        BOOL fCovered;
        TF_SELECTION tfSelection;

        // FIXME: Why we need this here?
        // first, test where a keystroke would go in the document if an insert is done
        if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
            return S_FALSE;

        // FIXME: Why we need this here?
        // is the insertion point covered by a composition?
        ITfRange *pRangeComposition;
        if (_pComposition->GetRange(&pRangeComposition) == S_OK)
        {
            fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

            pRangeComposition->Release();

            if (!fCovered)
            {
                goto End3;
            }
        }

        if (tfSelection.range->SetText(ec, 0, commit.GetString(), commit.GetLength()) != S_OK)
            goto End3;

        _TerminateComposition(ec, pContext);
End3: // FIXME: RAII?
        tfSelection.range->Release();
    }

    /*
     * Composition is mapped to preedit buffer + zuin buffer
     */
    ChewingString preedit_zuin(mChewingContext, CHEWING_STRING_PREEDIT_ZUIN);
    if (preedit_zuin.IsEmpty()) {
        // Remove composition
        if (_IsComposing())
        {
            ULONG cFetched;
            BOOL fCovered;
            TF_SELECTION tfSelection;

            // FIXME: Why we need this here?
            // first, test where a keystroke would go in the document if an insert is done
            if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
                return S_FALSE;

            // FIXME: Why we need this here?
            // is the insertion point covered by a composition?
            ITfRange *pRangeComposition;
            if (_pComposition->GetRange(&pRangeComposition) == S_OK)
            {
                fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

                pRangeComposition->Release();

                if (!fCovered)
                {
                    goto End;
                }
            }

            // Empties the composition
            tfSelection.range->SetText(ec, 0, L"", 0);

            _TerminateComposition(ec, pContext);
End: // FIXME: RAII?
            tfSelection.range->Release();
        }
    } else {
        if (!_IsComposing())
            _StartComposition(pContext);

        ULONG cFetched;
        BOOL fCovered;
        TF_SELECTION tfSelection;
        // FIXME: Why we need this here?
        // first, test where a keystroke would go in the document if an insert is done
        if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
            return S_FALSE;

        // FIXME: Why we need this here?
        // is the insertion point covered by a composition?
        ITfRange *pRangeComposition;
        if (_pComposition->GetRange(&pRangeComposition) == S_OK)
        {
            fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

            pRangeComposition->Release();

            if (!fCovered)
            {
                goto End2;
            }
        }
        if (tfSelection.range->SetText(ec, 0, preedit_zuin.GetString(), preedit_zuin.GetLength()) != S_OK)
            goto End2;

        _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeInput);
End2: // FIXME: RAII?
        tfSelection.range->Release();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InvokeKeyHandler
//
// This text service is interested in handling keystrokes to demonstrate the
// use the compositions. Some apps will cancel compositions if they receive
// keystrokes while a compositions is ongoing.
//
//----------------------------------------------------------------------------

HRESULT CTextService::_InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam)
{
    CKeyHandlerEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    // Insert a char in place of this keystroke
    if ((pEditSession = new CKeyHandlerEditSession(this, pContext, wParam)) == NULL)
        goto Exit;

    // a lock is required
    // nb: this method is one of the few places where it is legal to use
    // the TF_ES_SYNC flag
    hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);

    pEditSession->Release();

Exit:
    return hr;
}

