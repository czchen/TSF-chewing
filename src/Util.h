#pragma once

#include <cassert>
#include <vector>

#include "chewing.h"

enum ChewingStringType {
    CHEWING_STRING_PREEDIT_ZUIN,
    CHEWING_STRING_COMMIT,
};

class ChewingString {
public:
    ChewingString(ChewingContext *context, ChewingStringType type);
    ~ChewingString();

    const wchar_t *GetUtf16String();
    int GetUtf16StringLength();
    int IsEmpty();

private:
    ChewingString();
    ChewingString(const ChewingString&);
    ChewingString& operator=(const ChewingString&);

    std::vector<wchar_t> mUtf16String;
};

template<typename T>
class ComObject {
public:
    ComObject(T* ptr = NULL):mPtr(ptr){}
    ~ComObject()
    {
        if (mPtr)
            mPtr->Release();
    }

    T*& GetPointer()
    {
        return mPtr;
    }

    T* operator->()
    {
        assert(mPtr);
        return mPtr;
    }

private:
    ComObject(const ComObject&);
    ComObject& operator=(const ComObject&);

    T *mPtr;
};
