#include <stdint.h>
#include <stdio.h>

#include <random>

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
    static constexpr u32 NumInitVals = 17;
    static constexpr s32 MinVal = 1;
    static constexpr s32 MaxVal = 9;

    u32 Grid[Size];

    u32 GetRandomGridCellIdx()
    {
        static std::random_device RandomDevice;
        static std::mt19937 MersenneTwisterGenerator(RandomDevice());
        static std::uniform_int_distribution<> CellIdxDistrib(0, Size - 1);
        return CellIdxDistrib(MersenneTwisterGenerator);
    }

    s32 GetRandomSudokuValue()
    {
        static std::random_device RandomDevice;
        static std::mt19937 MersenneTwisterGenerator(RandomDevice());
        static std::uniform_int_distribution<> SudokuValDistrib(MinVal, MaxVal);
        return SudokuValDistrib(MersenneTwisterGenerator);
    }

    void Init()
    {
        for (u32 Idx = 0; Idx < Size; Idx++)
        {
            Grid[Idx] = 0;
        }

        for (u32 Idx = 0; Idx < NumInitVals; Idx++)
        {
            u32 GridCellIdx = GetRandomGridCellIdx();
            while (Grid[GridCellIdx] != 0)
            {
                GridCellIdx = GetRandomGridCellIdx();
            }

            Grid[GridCellIdx] = GetRandomSudokuValue();
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
