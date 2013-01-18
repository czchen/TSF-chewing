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
