// Std lib headers
#include <stdio.h>
// Platform headers
#include <windows.h>

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

struct FontDesc
{
    char Name[LF_FACESIZE];
    int Pt;
    int Weight;
    bool bItalic;
};

void GetOutputFontFileName(char* OutputFileName, FontDesc* Desc)
{
    constexpr int MaxIdx = 255;

    int ReadIdx = 0;
    int WriteIdx = 0;
    while (ReadIdx < MaxIdx && Desc->Name[ReadIdx])
    {
        if (Desc->Name[ReadIdx] != ' ')
        {
            OutputFileName[WriteIdx++] = Desc->Name[ReadIdx];
        }
        ReadIdx++;
    }

    WriteIdx += sprintf(OutputFileName + WriteIdx, "_%dpt", Desc->Pt);

    const char* WeightName = nullptr;
    switch (Desc->Weight)
    {
        case FW_THIN: WeightName = "_Thin"; break;
        case FW_EXTRALIGHT: WeightName = "_ExtraLight"; break;
        case FW_LIGHT: WeightName = "_Light"; break;
        case FW_REGULAR: break;
        case FW_MEDIUM: WeightName = "_Medium"; break;
        case FW_SEMIBOLD: WeightName = "_SemiBold"; break;
        case FW_BOLD: WeightName = "_Bold"; break;
        case FW_EXTRABOLD: WeightName = "_ExtraBold"; break;
        case FW_HEAVY: WeightName = "_Heavy"; break;
    }
    if (WeightName)
    {
        WriteIdx += sprintf(OutputFileName + WriteIdx, WeightName);
    }
    if (Desc->bItalic)
    {
        WriteIdx += sprintf(OutputFileName + WriteIdx, "_italic");
    }
    WriteIdx += sprintf(OutputFileName + WriteIdx, ".bmp");
}

void WriteBMP(HDC hDeviceContext, HBITMAP hBitmap, char* FileName)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmp);

    int Width = bmp.bmWidth;
    int Height = bmp.bmHeight;
    int BitCount = bmp.bmBitsPixel;

    if (bmp.bmPlanes != 1 || (BitCount != 24 && BitCount != 32))
    {
        fprintf(stdout, "[error] Unexpected bitmap format!\n"
                "BitCount: %d\n", BitCount);
        return;
    }

    int TotalPixels = bmp.bmWidth * bmp.bmHeight;
    int PixelByteCount = 
        bmp.bmBitsPixel == 24 ? TotalPixels * 3 :
        bmp.bmBitsPixel == 32 ? TotalPixels * 4 :
        0;
    if (PixelByteCount % 4 != 0) { PixelByteCount += 4 - (PixelByteCount % 4); }

    BYTE* pBits = new BYTE[PixelByteCount];

    BITMAPINFO Info = {};
    Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Info.bmiHeader.biWidth = Width;
    Info.bmiHeader.biHeight = Height;
    Info.bmiHeader.biPlanes = bmp.bmPlanes;
    Info.bmiHeader.biBitCount = BitCount;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biSizeImage = PixelByteCount;

    GetDIBits(
        hDeviceContext,
        hBitmap,
        0,
        bmp.bmHeight,
        pBits,
        &Info,
        DIB_RGB_COLORS
    );

    BITMAPFILEHEADER FileHeader = {};
    BITMAPINFOHEADER InfoHeader = Info.bmiHeader;

    int TotalHeadersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    FileHeader.bfType = 0x4D42; // ASCII "BM"
    FileHeader.bfSize = TotalHeadersSize + PixelByteCount;
    FileHeader.bfOffBits = TotalHeadersSize;

    HANDLE FileHandle = CreateFileA(
        FileName,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD TotalBytesWritten = 0;
        DWORD BytesWritten = 0;
        bool bSuccess = WriteFile(
            FileHandle,
            &FileHeader,
            sizeof(BITMAPFILEHEADER),
            &BytesWritten,
            nullptr
        );
        TotalBytesWritten += BytesWritten;
        if (bSuccess && BytesWritten == sizeof(BITMAPFILEHEADER))
        {
            bSuccess = WriteFile(
                FileHandle,
                &InfoHeader,
                sizeof(BITMAPINFOHEADER),
                &BytesWritten,
                nullptr
            );
            TotalBytesWritten += BytesWritten;
        }
        if (bSuccess && BytesWritten == sizeof(BITMAPINFOHEADER))
        {
            bSuccess = WriteFile(
                FileHandle,
                pBits,
                PixelByteCount,
                &BytesWritten,
                nullptr
            );
            TotalBytesWritten += BytesWritten;
        }
        if (!bSuccess)
        {
            fprintf(stdout, "[error] Write BMP FAILED! :(\n");
        }
        CloseHandle(FileHandle);
    }
    else
    {
        fprintf(stdout, "[error] CreateFileA FAILED! :(\n");
    }
}

void PostProcessFontBMP(char* FileName)
{
    HANDLE FileHandle = CreateFileA(
        FileName,
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (!FileHandle) { return; }

    int FileSize = GetFileSize(FileHandle, nullptr);

    BYTE* FileContents = new BYTE[FileSize];
    DWORD BytesRead = 0;

    if (!ReadFile(FileHandle, FileContents,
        FileSize, &BytesRead, nullptr) ||
        BytesRead != FileSize)
    {
        delete[] FileContents;
        return;
    }
    CloseHandle(FileHandle);

    BITMAPFILEHEADER FileHeader = *(BITMAPFILEHEADER*)FileContents;
    BITMAPINFOHEADER InfoHeader = *(BITMAPINFOHEADER*)(FileContents + sizeof(BITMAPFILEHEADER));

    if (InfoHeader.biBitCount != 32) { return; }

    int PixelDataBytes = FileSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
    UINT *PixelData = (UINT*)(FileContents + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
    
    BYTE* ProcessedPixels = new BYTE[PixelDataBytes];

    int NumPixels = InfoHeader.biWidth * InfoHeader.biHeight;
    for (int PxIdx = 0; PxIdx < NumPixels; PxIdx++)
    {
        UINT* CurrPx = PixelData + PxIdx;
        BYTE Red = (*CurrPx & 0x00FF0000) >> 16;
        BYTE Green = (*CurrPx & 0x0000FF00) >> 8;
        BYTE Blue = (*CurrPx & 0x000000FF) >> 0;
        BYTE Alpha = (*CurrPx & 0xFF000000) >> 24;

        BYTE MaxRGB = (Red > Green) ? (Red > Blue ? Red : Blue)
            : (Green > Blue ? Green : Blue);
        constexpr BYTE CutoffValue = 0xFF / 2;
        if (MaxRGB > CutoffValue)
        {
            Red = 0xFF;
            Green = 0xFF;
            Blue = 0xFF;
            Alpha = MaxRGB;
        }
        else
        {
            Red = 0;
            Green = 0;
            Blue = 0;
            Alpha = 0;
        }
        
        UINT* OutPx = (UINT*)(ProcessedPixels + (PxIdx * 4));
        UINT OutRGB = Alpha << 24 | Red << 16 | Green << 8 | Blue << 0;
        *OutPx = OutRGB;
    }

    char OutFileName[256];
    sprintf(OutFileName, "postprocess_%s", FileName);
    HANDLE OutBitmapFileHandle = CreateFileA(
        OutFileName,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (!OutBitmapFileHandle) { return; }

    WriteFile(
        OutBitmapFileHandle,
        &FileHeader,
        sizeof(BITMAPFILEHEADER),
        nullptr,
        nullptr
    );
    WriteFile(
        OutBitmapFileHandle,
        &InfoHeader,
        sizeof(BITMAPINFOHEADER),
        nullptr,
        nullptr
    );
    WriteFile(
        OutBitmapFileHandle,
        ProcessedPixels,
        PixelDataBytes,
        nullptr,
        nullptr
    );

    fprintf(stdout, "Wrote processed file to %s\n", OutFileName);

    CloseHandle(OutBitmapFileHandle);
    delete[] FileContents;
    delete[] ProcessedPixels;
}

HFONT DialogChooseFont(FontDesc* OutDesc)
{
    LOGFONT LogFontDesc = {};
    LogFontDesc.lfItalic = FALSE;
    LogFontDesc.lfUnderline = FALSE;
    LogFontDesc.lfStrikeOut = FALSE;
    LogFontDesc.lfCharSet = ANSI_CHARSET;
    LogFontDesc.lfOutPrecision = OUT_DEFAULT_PRECIS;
    LogFontDesc.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    LogFontDesc.lfQuality = DEFAULT_QUALITY;
    LogFontDesc.lfPitchAndFamily = FIXED_PITCH|FF_DONTCARE;

    CHOOSEFONT FontChoiceDesc = {};
    FontChoiceDesc.lStructSize = sizeof(CHOOSEFONT);
    FontChoiceDesc.lpLogFont = &LogFontDesc;
    FontChoiceDesc.Flags =
        CF_INITTOLOGFONTSTRUCT|CF_NOVERTFONTS|CF_SELECTSCRIPT|
        CF_FIXEDPITCHONLY|CF_FORCEFONTEXIST|CF_NOSTYLESEL;

    if (!ChooseFontA(&FontChoiceDesc))
    {
        fprintf(stdout, "ChooseFont dialog was canceled... aborting.\n");
        return nullptr;
    }

    HFONT FontHandle = CreateFontIndirectA(FontChoiceDesc.lpLogFont);
    if (FontHandle)
    {
        strcpy_s(OutDesc->Name, LF_FACESIZE, LogFontDesc.lfFaceName);
        OutDesc->Pt = FontChoiceDesc.iPointSize / 10;
        OutDesc->Weight = LogFontDesc.lfWeight;
        OutDesc->bItalic = LogFontDesc.lfItalic;
    }
    return FontHandle;
}

#define MAIN_ERRCHK(Exp, FuncName) if ((Exp)) { fprintf(stdout, "[error] %s failed!\n", #FuncName); return 1; }

int main(int ArgCount, const char* ArgValues[])
{
    HDC hDeviceContext = GetWindowDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hDeviceContext);

    FontDesc ChosenFontDesc = {};

    HFONT FontHandle = DialogChooseFont(&ChosenFontDesc);
    MAIN_ERRCHK(FontHandle == nullptr, DialogChooseFont);

    HGDIOBJ hOldFont = SelectObject(hMemoryDC, FontHandle);
    MAIN_ERRCHK(!hOldFont || hOldFont == HGDI_ERROR, SelectObject__Font);

    TEXTMETRICA FontMetrics = {};
    MAIN_ERRCHK(!GetTextMetricsA(hMemoryDC, &FontMetrics), GetTextMetrics);
    bool bFixedWidth = !(FontMetrics.tmPitchAndFamily & TMPF_FIXED_PITCH);

    int GlyphWidth = FontMetrics.tmAveCharWidth;
    int GlyphHeight = FontMetrics.tmHeight;
    constexpr int GlyphsPerRow = 16;
    constexpr int NumRows = 16;
    constexpr int NumGlyphs = GlyphsPerRow * NumRows;
    constexpr int FirstPrintableGlyph = 32;

    char FontName[256];
    GetTextFaceA(hMemoryDC, 256, FontName);

    int BitmapWidth = GlyphWidth * GlyphsPerRow;
    int BitmapHeight = GlyphHeight * NumRows;
    HBITMAP BitmapHandle = CreateCompatibleBitmap(hDeviceContext, BitmapWidth, BitmapHeight);

    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, BitmapHandle);
    MAIN_ERRCHK(!hOldBitmap || hOldBitmap == HGDI_ERROR, SelectObject__Bitmap);

    COLORREF ColorWhite = RGB(0xFF, 0xFF, 0xFF);
    COLORREF ColorBlack = RGB(0, 0, 0);
    COLORREF ColorResult = SetBkColor(hMemoryDC, ColorBlack);
    MAIN_ERRCHK(ColorResult == CLR_INVALID, SetBkColor);
    ColorResult = SetTextColor(hMemoryDC, ColorWhite);
    MAIN_ERRCHK(ColorResult == CLR_INVALID, SetTextColor);

    // Write glyphs to bitmap
    {
        char GlyphMap[NumGlyphs];
        for (int GlyphIdx = FirstPrintableGlyph; GlyphIdx < NumGlyphs; GlyphIdx++)
        {
            GlyphMap[GlyphIdx] = (char)GlyphIdx;
        }
        GlyphMap[127] = ' '; // [127] == DEL char

        // Skip first two rows, [0, 32] == control chars
        for (int RowIdx = 2; RowIdx < NumRows; RowIdx++)
        {
            int RowY = GlyphHeight * RowIdx;
            char* StartGlyph = GlyphMap + (GlyphsPerRow * RowIdx);
            TextOutA(hMemoryDC, 0, RowY, StartGlyph, GlyphsPerRow);
        }
    }

    // Write bitmap to file
    char OutputFileName[256];
    GetOutputFontFileName(OutputFileName, &ChosenFontDesc);
    WriteBMP(hMemoryDC, BitmapHandle, OutputFileName);

    PostProcessFontBMP(OutputFileName);
}

