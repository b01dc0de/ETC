#include <stdint.h>
#include <stdio.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

struct SudokuGrid
{
    static constexpr u32 Stride = 9;
    static constexpr u32 NumRows = Stride;
    static constexpr u32 Size = Stride * NumRows;

    u32 Grid[Size];

    void Init()
    {
        for (u32 Idx = 0; Idx < Size; Idx++)
        {
            Grid[Idx] = 0;
        }
    }

    void Print()
    {
        for (u32 RowIdx = 0; RowIdx < NumRows; RowIdx++)
        {
            for (u32 ColIdx = 0; ColIdx < Stride; ColIdx++)
            {
                printf("%u", Grid[RowIdx * Stride + ColIdx]);
                if (ColIdx == 2 || ColIdx == 5)
                {
                    printf(" | ");
                }
            }
            if (RowIdx == 2 || RowIdx == 5)
            {
                printf("\n---------------");
            }
            printf("\n");
        }
    }
};

int main()
{
    SudokuGrid TheGrid;
    TheGrid.Init();
    TheGrid.Print();

    return 0;
}
