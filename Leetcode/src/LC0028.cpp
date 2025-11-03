#include "Common.h"

using std::string;

struct Solution0028
{
    int strStr(string haystack, string needle)
    {
        int Result = -1;
        int Idx = 0;
        while (Idx < haystack.length())
        {
            if (haystack[Idx] == needle[0])
            {
                int FirstIdx = Idx;
                int NeedleIdx = 1;

                while ((FirstIdx + NeedleIdx) < haystack.length() &&
                       NeedleIdx < needle.length() &&
                       haystack[FirstIdx + NeedleIdx] == needle[NeedleIdx]) {
                    NeedleIdx++;
                }

                if (NeedleIdx == needle.length())
                {
                    Result = FirstIdx;
                    break;
                }
            }
            Idx++;
        }

        return Result;
    }
};

