// Std lib headers
#include <stdio.h>
// Platform headers
#include <windows.h>

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

struct BitmapFile
{
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER InfoHeader;
};

void WriteBMP()
{
    // TODO: Map this to the HBITMAP that we write the text out to
    int Width = 256;
    int Height = 256;
    int BytesPerPixel = 4;
    int NumPixels = Width * Height;
    int PixelByteCount = NumPixels * BytesPerPixel;
    BYTE* PixelData = new BYTE[PixelByteCount];

    int TotalHeadersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    int TotalFileSize = TotalHeadersSize + PixelByteCount;

    BitmapFile BitmapToWrite = {};
    {
        // Init FileHeader
        BitmapToWrite.FileHeader.bfType = 0x4D42; // ASCII "BM"
        BitmapToWrite.FileHeader.bfSize = TotalHeadersSize + PixelByteCount;
        BitmapToWrite.FileHeader.bfOffBits = TotalHeadersSize;
        // Init InfoHeader
        BitmapToWrite.InfoHeader.biSize = sizeof(BITMAPINFOHEADER);
        BitmapToWrite.InfoHeader.biWidth = Width;
        BitmapToWrite.InfoHeader.biHeight = -Height;
        BitmapToWrite.InfoHeader.biPlanes = 1;
        BitmapToWrite.InfoHeader.biBitCount = 32;
        BitmapToWrite.InfoHeader.biCompression = BI_RGB;
        // Init PixelData
        for (int PxIdx = 0; PxIdx < NumPixels; PxIdx ++)
        {
            // Pixel format: BB GG RR AA
            int CurrRow = PxIdx / Width;
            int CurrCol = PxIdx % Width;

            unsigned int* Pixel = ((unsigned int*)PixelData) + PxIdx;

            BYTE Red = 0;
            BYTE Green = 0;
            BYTE Blue = 0;
            BYTE Alpha = 0xFF;

            if (CurrRow == 0 && CurrCol == 0)
            {
                Red = 0xFF;
            }
            else if (CurrRow == 0 && CurrCol == Width - 1)
            {
                Green = 0xFF;
            }
            else if (CurrRow == Height - 1 && CurrCol == 0)
            {
                Blue = 0xFF;
            }
            else 
            {
                Red = (BYTE)CurrCol;
                Blue = (BYTE)CurrRow;
                Green = (BYTE)((float)PxIdx / NumPixels * 255.0f);
            }

            unsigned int OutValue = Alpha << 24 |
                Red << 16 | Green << 8 | Blue << 0;
            *Pixel = OutValue;
        }
    }

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
            &BitmapToWrite.FileHeader,
            sizeof(BITMAPFILEHEADER),
            &BytesWritten,
            nullptr
        );
        TotalBytesWritten += BytesWritten;
        if (bSuccess && BytesWritten == sizeof(BITMAPFILEHEADER))
        {
            bSuccess = WriteFile(
                FileHandle,
                &BitmapToWrite.InfoHeader,
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
                PixelData,
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

    BITMAP BitmapDesc = {};
    BitmapDesc.bmType = 0;
    BitmapDesc.bmWidth = GlyphWidth * GlyphCols;
    BitmapDesc.bmHeight = GlyphHeight * GlyphRows;
    BitmapDesc.bmWidthBytes = BitmapDesc.bmWidth * BytesPerPixel;
    BitmapDesc.bmPlanes = 1;
    BitmapDesc.bmBitsPixel = BytesPerPixel * 8;
    BitmapDesc.bmBits = nullptr; // TODO:
    HBITMAP BitmapHandle = CreateBitmapIndirect(&BitmapDesc);

    HGDIOBJ hOldFont = nullptr;
    HGDIOBJ hOldBitmap = nullptr;

    hOldFont = SelectObject(hMemoryDC, FontHandle);
    hOldBitmap = SelectObject(hMemoryDC, BitmapHandle);

    if (hOldFont && hOldFont != HGDI_ERROR && hOldBitmap && hOldBitmap != HGDI_ERROR)
    {
        if (TextOutA(hDeviceContext, 0, 0, "ABCD", 4))
        {
            fprintf(stdout, "TestOutA succeeded!\n");
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

    WriteBMP();

}

