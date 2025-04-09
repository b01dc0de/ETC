// Std lib headers
#include <stdio.h>
// Platform headers
#include <windows.h>

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

void WriteBMP(HDC hDeviceContext, HBITMAP hBitmap)
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

    bool bSuccess = GetDIBits(
        hDeviceContext,
        hBitmap,
        0,
        bmp.bmHeight,
        pBits,
        &Info,
        DIB_RGB_COLORS
    );
    if (!bSuccess)
    {
        fprintf(stdout, "[error] GetDIBits error!\n");
    }
    else
    {
        fprintf(stdout, "GetDIBits SUCCESS!\n");
    }

    BITMAPFILEHEADER FileHeader = {};
    BITMAPINFOHEADER InfoHeader = Info.bmiHeader;

    int TotalHeadersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    FileHeader.bfType = 0x4D42; // ASCII "BM"
    FileHeader.bfSize = TotalHeadersSize + PixelByteCount;
    FileHeader.bfOffBits = TotalHeadersSize;

    const char* OutFileName = "test_out.bmp";
    HANDLE FileHandle = CreateFileA(
        OutFileName,
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
        if (bSuccess)
        {
            fprintf(stdout, "Wrote BMP SUCCEEDED! :)\n");
        }
        else
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

HFONT DialogChooseFont()
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
        CF_NOVERTFONTS|CF_SELECTSCRIPT|
        CF_FIXEDPITCHONLY|CF_FORCEFONTEXIST|
        CF_NOSTYLESEL;

    ChooseFontA(&FontChoiceDesc);

    HFONT FontHandle = CreateFontIndirectA(FontChoiceDesc.lpLogFont);
    return FontHandle;
}

int main(int ArgCount, const char* ArgValues[])
{
    HDC hDeviceContext = nullptr;
    hDeviceContext = GetWindowDC(nullptr);

    if (0)
    {
#define PRINTCAPS(Value, CmpVal) if ((Value) & (CmpVal)) { fprintf(stdout, "\t" #CmpVal "\n"); }
        int RasterCaps = GetDeviceCaps(hDeviceContext, RASTERCAPS);
        fprintf(stdout, "Device Context RASTERCAPS:\n");
        PRINTCAPS(RasterCaps, RC_BITBLT);
        PRINTCAPS(RasterCaps, RC_BANDING);
        PRINTCAPS(RasterCaps, RC_SCALING);
        PRINTCAPS(RasterCaps, RC_BITMAP64);
        PRINTCAPS(RasterCaps, RC_DI_BITMAP);
        PRINTCAPS(RasterCaps, RC_PALETTE);
        PRINTCAPS(RasterCaps, RC_DIBTODEV);
        PRINTCAPS(RasterCaps, RC_STRETCHBLT);
        PRINTCAPS(RasterCaps, RC_FLOODFILL);
        PRINTCAPS(RasterCaps, RC_STRETCHDIB);
    }

    HDC hMemoryDC = nullptr;
    hMemoryDC = CreateCompatibleDC(hDeviceContext);

    HFONT FontHandle = DialogChooseFont();
    int GlyphWidth = 12; // TODO:
    int GlyphHeight = 24; // TODO:
    int GlyphRows = 16;
    int GlyphCols = 16;
    int NumGlyphs = GlyphRows * GlyphCols;
    int BytesPerPixel = 4;

    int Width = GlyphWidth * GlyphCols;
    int Height = GlyphHeight * GlyphRows;
    HBITMAP BitmapHandle = CreateCompatibleBitmap(hDeviceContext, Width, Height);

    HGDIOBJ hOldFont = nullptr;
    HGDIOBJ hOldBitmap = nullptr;

    hOldFont = SelectObject(hMemoryDC, FontHandle);
    hOldBitmap = SelectObject(hMemoryDC, BitmapHandle);

    if (hOldFont && hOldFont != HGDI_ERROR && hOldBitmap && hOldBitmap != HGDI_ERROR)
    {
        if (TextOutA(hMemoryDC, 0, 0, "ABCD", 4))
        {
            fprintf(stdout, "TestOutA succeeded!\n");

            WriteBMP(hMemoryDC, BitmapHandle);
        }
        else
        {
            fprintf(stdout, "TestOutA FAILED! :(\n");
        }
    }
    else
    {
        fprintf(stdout, "SelectObject FAILED! :(\n");
    }
}

