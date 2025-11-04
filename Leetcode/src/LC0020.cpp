#include "Common.h"

using std::string;

struct Solution0020
{
    bool isValid(string s)
    {
        std::vector<char> Stack;
        Stack.reserve(s.length());

        for (int Idx = 0; Idx < s.length(); Idx++)
        {
            switch (s[Idx])
            {
                case '(':
                case '{':
                case '[':
                {
                    Stack.push_back(s[Idx]);
                } break;

                case ')':
                {
                    if (Stack.size() == 0) { return false; }
                    if (Stack.back() == '(') { Stack.pop_back(); }
                    else { return false; }
                } break;
                case '}':
                {
                    if (Stack.size() == 0) { return false; }
                    if (Stack.back() == '{') { Stack.pop_back(); }
                    else { return false; }
                } break;
                case ']':
                {
                    if (Stack.size() == 0) { return false; }
                    if (Stack.back() == '[') { Stack.pop_back(); }
                    else { return false; }
                } break;

                default:
                {
                    return false;
                } break;
            }
        }

        return Stack.size() == 0;
    }
};
