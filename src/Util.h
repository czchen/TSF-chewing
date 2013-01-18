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

    const wchar_t *GetString();
    int GetLength();
    int IsEmpty();

private:
    ChewingString();
    ChewingString(const ChewingString&);
    ChewingString &operator=(const ChewingString&);

    std::vector<wchar_t> mString;
};

class ChewingCandidates {
public:
    ChewingCandidates(ChewingContext *context);
    ~ChewingCandidates();

    const wchar_t *GetCandidate(int index) const;
    int GetCandidateLength(int index) const;
    int GetCandidateCount() const;
    int IsEmpty() const;

private:
    ChewingCandidates();
    ChewingCandidates(const ChewingCandidates&);
    ChewingCandidates &operator=(const ChewingCandidates&);

    int toRealIndex(int index) const;

    std::vector<std::vector<wchar_t> > mCandidate;
    int mCandidatePerPage;
    int mCurrentPage;
};
