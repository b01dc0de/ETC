// NOTE: This file is included in haversine.cpp

enum struct MainExecType : u32
{
    Gen,
    Calc,
    Full,
    Error
};

struct MainExecParams
{
    MainExecType Type;
    u64 Seed;
    u64 Count;
    const char* InputFileName;
    bool bClustered;
};

MainExecType ParseExecType(const char* ArgV)
{
    MainExecType Result = MainExecType::Error;
    if (strcmp(ArgV, "gen") == 0)
    {
        Result = MainExecType::Gen;
    }
    else if (strcmp(ArgV, "calc") == 0)
    {
        Result = MainExecType::Calc;
    }
    else if (strcmp(ArgV, "all") == 0 || strcmp(ArgV, "full") == 0)
    {
        Result = MainExecType::Full;
    }
    return Result;
}

MainExecParams ParseCmdLine(int ArgCount, const char** ArgValues)
{
    MainExecParams Result = { MainExecType::Error, 0, 0, nullptr, true };

    // Try default format: haversine.exe default [gen/calc/all]
    if ((ArgCount == 2 || ArgCount == 3) && strcmp(ArgValues[1], "default") == 0)
    {
        if (ArgCount == 3) { Result.Type = ParseExecType(ArgValues[2]); }
        else { Result.Type = MainExecType::Full; }
        Result.Seed = DefaultSeed;
        Result.Count = DefaultCount;
        Result.bClustered = true;
    }
    // Try argument format: haversine.exe [gen/calc/all] [Seed] [Count]
    else if (ArgCount == 4)
    {
        Result.Type = ParseExecType(ArgValues[1]);
        if (Result.Type != MainExecType::Error)
        {
            Result.Seed = strtoull(ArgValues[2], nullptr, 10);
            Result.Count = strtoull(ArgValues[3], nullptr, 10);
            constexpr u64 MaxCount = UINT32_MAX;
            if (Result.Count > MaxCount) { Result.Count = MaxCount; }
            Result.bClustered = true;
        }
    }
    else if (ArgCount == 2)
    {
        Result.Type = MainExecType::Calc;
        Result.InputFileName = ArgValues[1];
    }

    return Result;
}

void Main_Exec(MainExecParams* ExecParams)
{
    if (ExecParams)
    {
        u64 Seed = ExecParams->Seed;
        u64 Count = ExecParams->Count;
        const char* InputFileName = ExecParams->InputFileName;
        bool bClustered = ExecParams->bClustered;
        if (InputFileName || (Seed && Count))
        {
            switch (ExecParams->Type)
            {
                case MainExecType::Gen:
                {
                    Haversine_Ref0::Gen(Seed, Count, bClustered);
                } break;
                case MainExecType::Calc:
                {
                    if (InputFileName) { Haversine_Ref0::Calc(InputFileName); }
                    else { Haversine_Ref0::Calc(Seed, Count, bClustered); }
                } break;
                case MainExecType::Full:
                {
                    Haversine_Ref0::Full(Seed, Count, bClustered);
                } break;
            }
        }
    }
}

void PrintProgramUsage(const char* ProgramName)
{
    fprintf(stdout, "\tUsage: %s [gen/calc/all] [Seed] [PairCount]\n",
            ProgramName);
    fprintf(stdout, "\tExample: %s all %d %d \n",
            ProgramName, DefaultSeed, DefaultCount);
    fprintf(stdout, "\tOr: %s default [gen/calc/all]\n", ProgramName);
    fprintf(stdout, "\t To use the above specified default values\n");
}

