#include "Util.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <iterator>
#include <Windows.h>
#include <Stringapiset.h> // Windows 8
// #include <Winnls.h> // Before Windows 8

#include "chewing.h"

ChewingString::ChewingString(ChewingContext *context, ChewingStringType type)
:mUtf16String()
{
    assert(context);

    std::vector<char> str;

    switch (type) {
    case CHEWING_STRING_PREEDIT_ZUIN:
        if (chewing_buffer_Check(context)) {
            std::shared_ptr<char> preedit(chewing_buffer_String(context), chewing_free);
            std::copy(preedit.get(), preedit.get() + strlen(preedit.get()), std::back_inserter(str));
        }

        if (!chewing_zuin_Check(context)) {
            int dummy;
            std::shared_ptr<char> zuin(chewing_zuin_String(context, &dummy), chewing_free);
            std::copy(zuin.get(), zuin.get() + strlen(zuin.get()), std::back_inserter(str));
        }
        break;

    case CHEWING_STRING_COMMIT:
        if (chewing_commit_Check(context)) {
            std::shared_ptr<char> commit(chewing_commit_String(context), chewing_free);
            std::copy(commit.get(), commit.get() + strlen(commit.get()), std::back_inserter(str));
        }
        break;

    default:
        assert(!"unknown type");
    }

    if (str.empty())
        return;

    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], str.size(), NULL, 0);
    if (len == 0) // FIXME: log error here
        return;

    mUtf16String.resize(len);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], str.size(), &mUtf16String[0], mUtf16String.size());
}

ChewingString::~ChewingString()
{
}

const wchar_t *ChewingString::GetUtf16String()
{
    return &mUtf16String[0];
}

int ChewingString::GetUtf16StringLength()
{
    return mUtf16String.size();
}

int ChewingString::IsEmpty()
{
    return mUtf16String.empty();
}
