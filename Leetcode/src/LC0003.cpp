#include "Common.h"

using std::string;

struct Solution0003
{
    // TODO(CKA): Speed this up, currently bottom 8 percentile
    int lengthOfLongestSubstring(string s)
    {
        if (s.empty()) { return 0; }

        int Result = 1;
        string SubStr{ "" };
        SubStr.reserve(s.length());
        for (int Idx = 0; Idx < s.length(); Idx++)
        {
            SubStr.clear();
            SubStr.push_back(s[Idx]);

            for (int SubIdx = 1; (SubIdx + Idx) < s.length(); SubIdx++)
            {
                char NextChar = s[Idx + SubIdx];
                bool bDupe = false;
                for (int CmpIdx = 0; CmpIdx < SubIdx; CmpIdx++)
                {
                    if (SubStr[CmpIdx] == NextChar)
                    {
                        bDupe = true;
                        break;
                    }
                }
                if (!bDupe) { SubStr += NextChar; }
                else { break; }
            }

            if (SubStr.length() > Result)
            {
                Result = SubStr.length();
            }
        }

        return Result;
    }
};
