#include "Common.h"

struct Solution0009
{
    bool isPalindrome(int x)
    {
        using std::string;

        if (x < 0) { return false; }

        string AsString = std::to_string(x);
        int HalfLength = AsString.length() / 2; // NOTE: We don't care about the middle digit
        int Length = AsString.length();

        for (int CharIdx = 0; CharIdx < HalfLength; CharIdx++)
        {
            char BegChar = AsString[CharIdx];
            char EndChar = AsString[AsString.length() - 1 - CharIdx];
            if (BegChar != EndChar)
            {
                return false;
            }
        }

        return true;
    }
};
