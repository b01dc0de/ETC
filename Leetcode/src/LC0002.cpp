#include "Common.h"

struct ListNode0002
{
    int val;
    ListNode0002* next;

    ListNode0002() : val(0), next(nullptr) {}
    ListNode0002(int x) : val(x), next(nullptr) {}
    ListNode0002(int x, ListNode0002* next) : val(x), next(next) {}
};

using ListNode = ListNode0002;

struct Solution0002
{
    void printNumber(ListNode* l)
    {
        std::vector<ListNode*> Stack;
        ListNode* Cond = l;
        while (Cond)
        {
            Stack.push_back(Cond);
            Cond = Cond->next;
        }

        while (!Stack.empty())
        {
            Outf("%d", Stack.back()->val);
            Stack.pop_back();
        }
        Outf("\n");
    }
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2)
    {
        ListNode* Digit1 = l1;
        ListNode* Digit2 = l2;

        ListNode* Head = new ListNode{ 0, nullptr };
        ListNode* Cond = Head;
        int Carry = 0;
        while (Digit1 || Digit2)
        {
            int DigitSum = (Digit1 ? Digit1->val : 0) + (Digit2 ? Digit2->val : 0) + Carry;
            int NewCarry = 0;
            if (DigitSum >= 10)
            {
                NewCarry = DigitSum / 10;
                DigitSum = DigitSum % 10;
            }
            Cond->val = DigitSum;
            Carry = NewCarry;

            if (Digit1) { Digit1 = Digit1->next; }
            if (Digit2) { Digit2 = Digit2->next; }
            if (Digit1 || Digit2)
            {
                Cond->next = new ListNode{ 0, nullptr };
                Cond = Cond->next;
            }
        }
        if (Carry > 0)
        {
            Cond->next = new ListNode{ Carry, nullptr };
        }

        return Head;
    }
};
