#pragma once

#include <cassert>

class ChewingString {
public:
    ChewingString(char *Utf8String);
    ~ChewingString();

    const wchar_t *GetUtf16String();
    int GetUtf16StringLength();

private:
    ChewingString();
    ChewingString(const ChewingString&);
    ChewingString& operator=(const ChewingString&);

    char *mUtf8String;
    wchar_t *mUtf16String;
    int mUtf16StringLength;
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
