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

//+---------------------------------------------------------------------------
//
// _HandleCharacterKey
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CTextService::_HandleCharacterKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    WCHAR ch;
    BOOL fCovered;

    // Start the new compositon if there is no composition.
    if (!_IsComposing())
        _StartComposition(pContext);

    //
    // Assign VK_ value to the char. So the inserted the character is always
    // uppercase.
    //
    ch = (WCHAR)wParam;

    // first, test where a keystroke would go in the document if an insert is done
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

    // is the insertion point covered by a composition?
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            goto Exit;
        }
    }

    // insert the text
    // Use SetText here instead of InsertTextAtSelection because a composition is already started
    // Don't allow the app to adjust the insertion point inside our composition
    if (tfSelection.range->SetText(ec, 0, &ch, 1) != S_OK)
        goto Exit;

    // update the selection, and make it an insertion point just past
    // the inserted text.
    tfSelection.range->Collapse(ec, TF_ANCHOR_END);
    pContext->SetSelection(ec, 1, &tfSelection);

    //
    // set the display attribute to the composition range.
    //
    _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeInput);

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleReturnKey
//
//----------------------------------------------------------------------------

HRESULT CTextService::_HandleReturnKey(TfEditCookie ec, ITfContext *pContext)
{
    // just terminate the composition
    _TerminateComposition(ec, pContext);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleSpaceKey
//
//----------------------------------------------------------------------------

HRESULT CTextService::_HandleSpaceKey(TfEditCookie ec, ITfContext *pContext)
{
    //
    // set the display attribute to the composition range.
    //
    // The real text service may have linguistic logic here and set 
    // the specific range to apply the display attribute rather than 
    // applying the display attribute to the entire composition range.
    //
    _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeConverted);

    // 
    // create an instance of the candidate list class.
    // 
    if (_pCandidateList == NULL)
        _pCandidateList = new CCandidateList(this);

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
            _pCandidateList->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange);
            pRange->Release();
        }
        pDocumentMgr->Release();
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CTextService::_HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    LONG cch;
    BOOL fEqual;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection?
        return S_OK; // eat the keystroke
    }

    // get the composition range
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        goto Exit;

    // adjust the selection
    if (wParam == VK_LEFT)
    {
        if (tfSelection.range->IsEqualStart(ec, pRangeComposition, TF_ANCHOR_START, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftStart(ec, -1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_START);
    }
    else
    {
        // VK_RIGHT
        if (tfSelection.range->IsEqualEnd(ec, pRangeComposition, TF_ANCHOR_END, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftEnd(ec, +1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_END);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK; // eat the keystroke
}

HRESULT CTextService::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    ULONG cFetched;
//    WCHAR ch;
    BOOL fCovered;

    if (_IsComposing()) {
        /*
         * FIXME: Cursor might be moved by mouse, so we need to adjust it.
         */

        /*
         * FIXME: Erase original Composition. We will create a new one.
         */
        _TerminateComposition(ec, pContext);
    }

    /*
     * FIXME: the following keys are not handled:
     * shift left
     * shift right
     * caps lock
     * page up
     * page down
     * ctrl num
     * shift space
     * numlock num
     * -=\[];',./
     */

    if (('A' <= wParam && wParam <= 'Z')) {
            chewing_handle_Default(mChewingContext, wParam - 'A' + 'a');
    } else if ('0' <= wParam && wParam <= '9') {
            chewing_handle_Default(mChewingContext, wParam);
    } else {
        switch(wParam) {
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

    int hasCommit = chewing_commit_Check(mChewingContext);
    if (hasCommit) {
        // FIXME: Implement this
        ChewingString commitString(chewing_commit_String(mChewingContext));
    }

    /*
     * Composition is mapped to preedit buffer + zuin buffer
     */
    int hasPreedit = chewing_buffer_Check(mChewingContext);
    int hasZuin = !chewing_zuin_Check(mChewingContext);
    if (hasPreedit || hasZuin) {
        _StartComposition(pContext);

        TF_SELECTION tfSelection;
        // FIXME: Why we need this here?
        // first, test where a keystroke would go in the document if an insert is done
        if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
            return S_FALSE;

        // FIXME: Why we need this here?
        // is the insertion point covered by a composition?
        if (_pComposition->GetRange(&pRangeComposition) == S_OK)
        {
            fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

            pRangeComposition->Release();

            if (!fCovered)
            {
                goto Exit;
            }
        }

        // FIXME: What is the correct selection?
        if (hasPreedit)
        {
            // insert text in preedit buffer
            ChewingString preeditString(chewing_buffer_String(mChewingContext));
            if (tfSelection.range->SetText(ec, 0, preeditString.GetUtf16String(), preeditString.GetUtf16StringLength()) != S_OK)
                goto Exit;

            tfSelection.range->Collapse(ec, TF_ANCHOR_END);
            pContext->SetSelection(ec, 1, &tfSelection);
        }

        if (hasZuin)
        {
            // insert text in zuin buffer
            int dummy;
            ChewingString zuinString(chewing_zuin_String(mChewingContext, &dummy));
            if (tfSelection.range->SetText(ec, 0, zuinString.GetUtf16String(), zuinString.GetUtf16StringLength()) != S_OK)
                goto Exit;

            //tfSelection.range->Collapse(ec, TF_ANCHOR_END);
            //pContext->SetSelection(ec, 1, &tfSelection);
        }
        //
        // set the display attribute to the composition range.
        //
        _SetCompositionDisplayAttributes(ec, pContext, _gaDisplayAttributeInput);
Exit:
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

