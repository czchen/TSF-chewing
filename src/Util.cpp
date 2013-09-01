#include "Util.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <iterator>
#include <Windows.h>

#if _WIN32_WINNT >= 0x0602
#include <Stringapiset.h> // Windows 8
#else
#include <Winnls.h> // Before Windows 8
#endif

#include "chewing.h"

ChewingString::ChewingString(ChewingContext *context, ChewingStringType type)
:mString()
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

    mString.resize(len);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], str.size(), &mString[0], mString.size());
}

ChewingString::~ChewingString()
{
}

const wchar_t *ChewingString::GetString()
{
    return &mString.at(0);
}

int ChewingString::GetLength()
{
    return mString.size();
}

int ChewingString::IsEmpty()
{
    return mString.empty();
}

ChewingCandidates::ChewingCandidates(ChewingContext *context)
{
    assert(context);

    if (chewing_cand_TotalPage(context)) {
        mCandidatePerPage = chewing_cand_ChoicePerPage(context);
        mCurrentPage = chewing_cand_CurrentPage(context);

        int count = mCandidatePerPage;

        // FIXME: chewing_cand_Enumerate does not set cand_no to 0.
        chewing_cand_Enumerate(context);

        // FIXME: chewing_cand_hasNext will not stop when reaching page end.
        while (chewing_cand_hasNext(context) && count--) {
            std::shared_ptr<char> candidateUtf8(chewing_cand_String(context), chewing_free);
            int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, candidateUtf8.get(), strlen(candidateUtf8.get()), NULL, 0);
            std::vector<wchar_t> candidateUtf16(len);
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, candidateUtf8.get(), strlen(candidateUtf8.get()), &candidateUtf16[0], candidateUtf16.size());
            mCandidate.push_back(candidateUtf16);
        }
    }
}

ChewingCandidates::~ChewingCandidates()
{
}

const wchar_t *ChewingCandidates::GetCandidate(int index) const
{
    return &mCandidate.at(index)[0];
}

int ChewingCandidates::GetCandidateLength(int index) const
{
    return mCandidate.at(index).size();
}

int ChewingCandidates::IsEmpty() const
{
    return mCandidate.empty();
}

int ChewingCandidates::GetCandidateCount() const
{
    return mCandidate.size();
}
