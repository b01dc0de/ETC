#include "Common.h"

using std::string;

struct Solution0006
{
    string convert(string s, int numRows)
    {
        if (numRows == 1 || s.length() < numRows) { return string{ s }; }

        string Result{};

        int ZigLength = (numRows - 1) * 2;
        int NumFullZigs = s.length() / ZigLength;
        int ZigRemainder = s.length() % ZigLength;

        for (int Row = 0; Row < numRows; Row++)
        {
            bool bMiddleRow = (0 < Row) && (Row < (numRows - 1));

            int FirstCharIdx = Row;
            int NextOffset = ((numRows - 1) - Row) * 2;
            for (int Zig = 0; Zig < NumFullZigs; Zig++)
            {
                int IndexA = FirstCharIdx + (Zig * ZigLength);
                Result += s[IndexA];

                int IndexB = IndexA + NextOffset;
                if (bMiddleRow && IndexB < s.length())
                {
                    Result += s[IndexB];
                }
            }
            if (ZigRemainder > Row)
            {
                int IndexA = FirstCharIdx + (NumFullZigs * ZigLength);
                if (IndexA < s.length())
                {
                    Result += s[IndexA];
                }

                int IndexB = IndexA + NextOffset;
                if (bMiddleRow && IndexB < s.length())
                {
                    Result += s[IndexB];
                }
            }
        }

        return Result;
    }
};
