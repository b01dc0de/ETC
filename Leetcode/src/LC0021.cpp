#include "Common.h"

// NOTE: Renamed ListNode -> ListNode0021 to avoid potential future
//  conflicts when compling multiple files in a unity build
struct ListNode0021
{
    int val;
    ListNode0021* next;

    ListNode0021() : val(0), next(nullptr) {}
    ListNode0021(int x) : val(x), next(nullptr) {}
    ListNode0021(int x, ListNode0021* next) : val(x), next(next) {}
};

using ListNode = ListNode0021;

struct Solution0021
{
    void printList(ListNode* list)
    {
        Outf("[ ");
        ListNode* Cond = list;
        while (Cond)
        {
            Outf("%d ", Cond->val);
            Cond = Cond->next;
        }
        Outf("]\n");
    }
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2)
    {
        // Case 0: list1 and list2 are nullptr
        if (!list1 && !list2)
        {
            return nullptr;
        }
        // Case 1: One of list1, list2 are nullptr
        if (!list1 || !list2)
        {
            return list1 ? list1 : list2;
        }
        // Case 2: Both lists are non-empty
        ListNode* Result = nullptr;
        ListNode* NewCond = nullptr;
        ListNode* Next1 = nullptr;
        ListNode* Next2 = nullptr;
        if (list1->val <= list2->val)
        {
            Result = list1;
            NewCond = list1;
            Next1 = list1->next;
            Next2 = list2;
        }
        else
        {
            Result = list2;
            NewCond = list2;
            Next1 = list1;
            Next2 = list2->next;
        }
        while (Next1 || Next2)
        {
            if (!Next1)
            {
                NewCond->next = Next2;
                Next2 = Next2->next;
                NewCond = NewCond->next;
            }
            else if (!Next2)
            {
                NewCond->next = Next1;
                Next1 = Next1->next;
                NewCond = NewCond->next;
            }
            else if (Next1->val <= Next2->val)
            {
                NewCond->next = Next1;
                Next1 = Next1->next;
                NewCond = NewCond->next;
            }
            else
            {
                NewCond->next = Next2;
                Next2 = Next2->next;
                NewCond = NewCond->next;
            }
        }

        return Result;
    }
};
