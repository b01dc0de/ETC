#include "Common.h"

using std::string;

struct Solution0058
{
    int lengthOfLastWord(string s)
    {
        int LastLetterIdx = s.length() - 1;
        while (s[LastLetterIdx] == ' ') { LastLetterIdx--; }
        int FirstLetterIdx = LastLetterIdx ? LastLetterIdx - 1 : LastLetterIdx;
        while (FirstLetterIdx && s[FirstLetterIdx] != ' ') { FirstLetterIdx--; }
        return LastLetterIdx - FirstLetterIdx + (s[FirstLetterIdx] == ' ' ? 0 : 1);
    }
};
