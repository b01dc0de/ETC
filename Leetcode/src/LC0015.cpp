#include "Common.h"

using std::vector;

struct Solution0015
{
    vector<vector<int>> threeSum(vector<int>& nums)
    {
        vector<vector<int>> Result;
        std::sort(nums.begin(), nums.end());

        if (nums.size() < 3) { return Result; }

        for (int Idx = 0; Idx < nums.size() - 2; Idx++)
        {
            int A = nums[Idx];
            if (0 < Idx && A == nums[Idx - 1]) { continue; }

            int JIdx = Idx + 1;
            int KIdx = nums.size() - 1;
            while (JIdx < KIdx)
            {
                int Sum = A + nums[JIdx] + nums[KIdx];
                if (Sum == 0)
                {
                    Result.push_back(vector<int>{nums[Idx], nums[JIdx], nums[KIdx]});
                    // Skip duplicates
                    while (JIdx < KIdx && nums[JIdx] == nums[JIdx + 1]) { JIdx++; }
                    while (JIdx < KIdx && nums[KIdx] == nums[KIdx - 1]) { KIdx--; }
                    JIdx++;
                    KIdx--;
                }
                else if (Sum < 0) { JIdx++; }
                else { KIdx--; } // Sum > 0
            }
        }

        return Result;
    }
};
