#include "Common.h"

using std::vector;

struct Solution0026
{
    int removeDuplicates(vector<int>& nums)
    {
        using const_iterator = vector<int>::const_iterator;

        int Idx = 0;
        while (Idx < nums.size())
        {
            int BaseVal = nums[Idx];
            int BaseIdx = Idx;
            int NextIdx = Idx + 1;
            if (NextIdx < nums.size() && nums[NextIdx] == BaseVal)
            {
                int FirstDupeIdx = NextIdx;
                do
                {
                    NextIdx++;
                } while (NextIdx < nums.size() && nums[NextIdx] == BaseVal);

                int LastDupeIdx = NextIdx - 1;
                if (FirstDupeIdx == LastDupeIdx)
                {
                    const_iterator Dupe = nums.cbegin() + FirstDupeIdx;
                    nums.erase(Dupe);
                }
                else
                {
                    const_iterator DupeBegin = nums.cbegin() + FirstDupeIdx;
                    const_iterator DupeEnd = nums.cbegin() + NextIdx;
                    // NOTE: Erases elements in the range [DupeBegin, DupeEnd)
                    nums.erase(DupeBegin, DupeEnd);
                }
            }
            Idx++;
        }

        // Return final, reduced size of nums
        int Result = nums.size();
        return Result;
    }
};
