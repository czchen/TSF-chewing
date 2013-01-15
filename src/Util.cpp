#include "Util.h"

#include <Windows.h>
#include <Stringapiset.h> // Windows 8
// #include <Winnls.h> // Before Windows 8

#include "chewing.h"

ChewingString::ChewingString(char *Utf8String)
:mUtf8String(Utf8String),mUtf16String(NULL),mUtf16StringLength(0)
{
    if (mUtf8String == NULL) // FIXME: log error here
        return;

    mUtf16StringLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mUtf8String, -1, NULL, 0);
    if (mUtf16StringLength == 0) // FIXME: log error here
        return;

    // FIXME: Error handling, NULL or throw?
    mUtf16String = new wchar_t [mUtf16StringLength];
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mUtf8String, -1, mUtf16String, mUtf16StringLength);
}

ChewingString::~ChewingString()
{
    delete [] mUtf16String;
    chewing_free(mUtf8String);
}

const wchar_t *ChewingString::GetUtf16String()
{
    return mUtf16String;
}

int ChewingString::GetUtf16StringLength()
{
    return mUtf16StringLength;
}