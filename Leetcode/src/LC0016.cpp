#include "Common.h"

using std::vector;

struct Solution0016
{
    int threeSumClosest(vector<int>& nums, int target)
    {
        std::sort(nums.begin(), nums.end());

        int Result = nums[0] + nums[1] + nums[2];
        int ResultDiff = Result - target;
        int ResultDiffAbs = ResultDiff < 0 ? -ResultDiff : ResultDiff;
        int Idx = 0;
        while (Idx < nums.size() - 2)
        {
            int JIdx = Idx + 1;
            int KIdx = nums.size() - 1;

            while (JIdx < KIdx)
            {
                int Sum = nums[Idx] + nums[JIdx] + nums[KIdx];
                int Diff = (Sum - target);
                int DiffAbs = Diff < 0 ? -Diff : Diff;

                if (DiffAbs < ResultDiffAbs)
                {
                    Result = Sum;
                    ResultDiff = Diff;
                    ResultDiffAbs = DiffAbs;
                }

                if (Diff < 0) { JIdx++; }
                else { KIdx--; } // Sum > target
            }

            Idx++;
        }

        return Result;
    }
};
