#pragma once

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
