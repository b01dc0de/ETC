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

#define MAIN_ERRCHK(Exp, FuncName) if ((Exp)) { fprintf(stdout, "[error] %s failed!\n", #FuncName); return 1; }

int main(int ArgCount, const char* ArgValues[])
{
    HDC hDeviceContext = GetWindowDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hDeviceContext);

    HFONT FontHandle = DialogChooseFont();
    MAIN_ERRCHK(FontHandle == nullptr, DialogChooseFont);

    HGDIOBJ hOldFont = SelectObject(hMemoryDC, FontHandle);
    MAIN_ERRCHK(!hOldFont || hOldFont == HGDI_ERROR, SelectObject__Font);

    TEXTMETRICA FontMetrics = {};
    MAIN_ERRCHK(!GetTextMetricsA(hMemoryDC, &FontMetrics), GetTextMetrics);
    bool bFixedWidth = !(FontMetrics.tmPitchAndFamily & TMPF_FIXED_PITCH);

    int GlyphWidth = FontMetrics.tmAveCharWidth;//FontMetrics.tmMaxCharWidth;
    int GlyphHeight = FontMetrics.tmHeight;
    int GlyphsPerRow = 16;
    int NumRows = 16;
    int NumGlyphs = GlyphsPerRow * NumRows;

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
        char GlyphMap[256];
        for (int GlyphIdx = 0; GlyphIdx < 256; GlyphIdx++)
        {
            GlyphMap = (char)GlyphIdx;
        }
        GlyphMap[127] = ' '; // [127] == DEL char

        // Skip first two rows, [0, 32] == control chars
        for (int RowIdx = 2; RowIdx < NumRows; RowIdx++)
        {
            int RowY = GlyphHeight * RowIdx;
            char* StartGlyph = CharValues + (GlyphsPerRow * RowIdx);
            TextOutA(hMemoryDC, 0, RowY, StartGlyph, GlyphsPerRow);
        }
        WriteBMP(hMemoryDC, BitmapHandle);
    }

}

