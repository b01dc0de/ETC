#include "Common.h"

using std::string;
using std::vector;

struct Solution0014
{
    string longestCommonPrefix(vector<string>& strs)
    {
        int ShortestLength = strs[0].length();
        for (int Idx = 1; Idx < strs.size(); Idx++)
        {
            if (strs[Idx].length() < ShortestLength)
            {
                ShortestLength = strs[Idx].length();
            }
        }

        if (ShortestLength == 0) { return string{}; }

        string Result{};
        Result.reserve(ShortestLength);

        int CharIdx = 0;
        while (CharIdx < ShortestLength)
        {
            char RefChar = strs[0][CharIdx];
            bool bCommon = true;
            for (int Idx = 1; Idx < strs.size(); Idx++)
            {
                if (strs[Idx][CharIdx] != RefChar)
                {
                    bCommon = false;
                    break;
                }
            }

            if (bCommon)
            {
                Result += RefChar;
                CharIdx++;
            }
            else { break; }
        }

        return Result;
    }
};
