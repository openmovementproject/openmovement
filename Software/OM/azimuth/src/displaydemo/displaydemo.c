// TODO: Call poll in a loop (from a threaded reader? callbacks?)

// Display Demo
// Dan Jackson, 2013

#ifndef _WIN32
#error "This demo is WIN32 only."
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "azimuth/azimuth.h"
#include "azimuth/display.h"


// Includes
#define _WIN32_WINNT 0x501
//#pragma warning(disable: 4996)  // Disable warning C4996 - deprecated functions
//#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES       // For M_PI
#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

static const TCHAR *szAppName = TEXT("DisplayDemo");
static TCHAR szAppTitle[128] = TEXT("Display Demo");
static HINSTANCE hInstance = 0;
static HWND hWndMain = NULL;
static char isBorderless = 0;
static DWORD dwTimerId = 1;
#define REFRESH_INTERVAL (1000/20)

static HDC hWindowDC = NULL;
static int mode = 0;

static int ditherIndex = 4;
static char edgeEnhance = 1;

static int captureLeft = 0, captureTop = 0, captureWidth = 0, captureHeight = 0;

static HBITMAP hBufferBitmap = NULL;
static unsigned char *bufferBits = NULL;
static int captureTexture = -1;
static int actualBufferWidth = -1, actualBufferHeight = -1;
static int bufferWidth = -1, bufferHeight = -1;
static int bufferBitCount = 24;

static HWND hWndResizer = NULL;


image_t image = {0};
display_t display = {0};
azimuth_t azimuth = {0};


// Redirect standard I/O to a console (in debug builds)
static void RedirectIOToConsole()
{
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    HANDLE hStdOut, hStdIn, hStdErr;
    int handleOut, handleIn, handleErr;
    FILE *fpOut, *fpIn, *fpErr;

    // Create a console (if one doesn't already exist)
    if (GetConsoleWindow() == NULL) { AllocConsole(); }
    // Adjust the console parameters
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = 1000;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
    // STDOUT
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    handleOut = _open_osfhandle((intptr_t)hStdOut, _O_TEXT);
    fpOut = _fdopen(handleOut, "w");
    *stdout = *fpOut;
    setvbuf(stdout, NULL, _IONBF, 0);
    // STDIN
    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    handleIn = _open_osfhandle((intptr_t)hStdIn, _O_TEXT);
    fpIn = _fdopen(handleIn, "r");
    *stdin = *fpIn;
    setvbuf(stdin, NULL, _IONBF, 0);
    // STDERR
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    handleErr = _open_osfhandle((intptr_t)hStdErr, _O_TEXT);
    fpErr = _fdopen(handleErr, "w");
    *stderr = *fpErr;
    setvbuf(stderr, NULL, _IONBF, 0);  
    // Sync other streams: cout, wcout, cin, wcin, wcerr, cerr, wclog, clog
    //ios::sync_with_stdio();
}

static void PrintLastError()
{ 
    LPVOID lpMsgBuf; 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, (LPTSTR)&lpMsgBuf, 0, NULL); 
#ifdef _DEBUG
    OutputDebugString((TCHAR *)lpMsgBuf); 
#endif
    _tprintf((TCHAR *)lpMsgBuf);
    LocalFree(lpMsgBuf); 
}

BOOL WINAPI ConsoleHandler(DWORD event)
{
    if (event == CTRL_CLOSE_EVENT)  // CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
    {
        printf("Console close signal received...\n");
        PostMessage(hWndMain, WM_CLOSE, 0, 0);
        return TRUE;
    }
    return FALSE;
}


static LRESULT CALLBACK ResizerWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {

    case WM_CREATE:
        break;

    case WM_CLOSE: 
        {
            //if (hWnd != NULL) { DestroyWindow(hWnd); }
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
        break; 

    case WM_DESTROY:
        //PostQuitMessage(0);
        //return 0;
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);

            // Border
            RECT rc;
            GetClientRect(hWnd, &rc);
            if (ps.fErase)
            {
                HBRUSH hBrush = CreateSolidBrush(RGB(0xff,0x00,0xff)); 
                HGDIOBJ hOldBrush = SelectObject(hDC, (HGDIOBJ)hBrush);
                //FillRect(hDC, &rc, hBrush);
                Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom); 
                DeleteObject((HGDIOBJ)hBrush);
                SelectObject(hDC, hOldBrush);
            }

            EndPaint(hWnd, &ps);
            return 0;
        }
        break;

        //case WM_CHAR:
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE || wParam == VK_F1)
        {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;

    case WM_NCHITTEST:
        {
            LRESULT ht = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
            if (ht == HTCLIENT)
            {
                ht = HTCAPTION;
            }
            return ht;
        }
        break;

    case WM_MOUSEMOVE:
        //xPos = (int)LOWORD(lParam); 
        //yPos = (int)HIWORD(lParam);
        //InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_SIZE:
        break;

    case WM_MOVE:
        break;

    case WM_WINDOWPOSCHANGED:
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}


static HWND ResizerInit(HINSTANCE hInst, int left, int top, int width, int height)
{
    HWND hWnd;
    TCHAR *szMainWndClass = TEXT("Resizer");
    WNDCLASSEX wc;

    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.lpszClassName = szMainWndClass;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = ResizerWndProc;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; //(HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) 
    {
        MessageBox(NULL, TEXT("Error registering resizer window class."), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    hWnd = CreateWindowEx(
        WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_APPWINDOW | WS_EX_LAYERED,
        szMainWndClass,             // Class name
        TEXT("Capture Area"),       // Caption
        WS_OVERLAPPEDWINDOW,        // Style
        left, top, width, height,   // Position and size
        NULL, NULL, hInst, NULL);   // Parent, menu, instance and parameters

    SetLayeredWindowAttributes(hWnd, RGB(0xff,0x00,0xff), 0xff, LWA_COLORKEY | LWA_ALPHA);

    //AdjustWindowRect(&rect, 0, FALSE);
    //ShowWindow(hWnd, SW_SHOWNORMAL);
    //UpdateWindow(hWnd);

    return hWnd;
}


static char SetBorderless(HWND hwnd, char borderless)
{
    DWORD dwStyle;
    RECT r;

    dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    GetWindowRect(hwnd, &r);

    if (borderless)
    {
        SetWindowPos(hwnd, HWND_TOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, 0);
        dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP);
        dwStyle |= WS_SYSMENU;
        SetWindowLong(hwnd, GWL_STYLE, dwStyle);
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) /*| WS_EX_LAYERED*/);
        //SetLayeredWindowAttributes(hwnd, RGB(0xff,0x00,0xff), 0xff, LWA_COLORKEY | LWA_ALPHA);
    }
    else
    {
        dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP);
        dwStyle |= WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP;
        SetWindowLong(hwnd, GWL_STYLE, dwStyle);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(hwnd, HWND_TOPMOST, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER);
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) /*| WS_EX_LAYERED*/);
        //SetLayeredWindowAttributes(hwnd, RGB(0xff,0x00,0xff), 0x99, LWA_COLORKEY | LWA_ALPHA);
    }
    return borderless;
}

    //if (hWindowDC == NULL || hGLRC == NULL) { return false; }

    // Get screen size
    //int left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    //int top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    //int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    //int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // Resize window
    //RECT clientRect;
    //GetClientRect(hWndMain, &clientRect);
    //RECT windowRect; GetWindowRect(hWndMain, &windowRect);
    //MoveWindow(hWndMain, windowRect.left, windowRect.top, width + (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left), height + (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top), TRUE);

static char Render(HWND hWnd, HDC hDC, RECT *rect)
{
    /*
    HDC hScreenDC = GetDC(GetDesktopWindow());
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    bufferWidth = captureWidth; bufferHeight = captureHeight;
    if (hBufferBitmap == NULL || bufferWidth != actualBufferWidth || bufferHeight != actualBufferHeight)
    {
        actualBufferWidth = bufferWidth;
        actualBufferHeight = bufferHeight;
        //bufferBitCount = 24;

        if (hBufferBitmap != NULL)
        {
            DeleteObject(hBufferBitmap);
        }
        hBufferBitmap = CreateCompatibleBitmap(hScreenDC, actualBufferWidth, actualBufferHeight);

        BITMAPINFO bitmapInfo;
        ZeroMemory(&bitmapInfo, sizeof(BITMAPINFO));
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = actualBufferWidth;
        bitmapInfo.bmiHeader.biHeight = -actualBufferHeight;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = bufferBitCount;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        // Fill in remaining elements in bitmapInfo.bmiHeader 
        GetDIBits(hMemoryDC, hBufferBitmap, 0, bufferHeight, NULL, &bitmapInfo, DIB_RGB_COLORS);
        // bitmapInfo.bmiHeader.biSizeImage

        bufferBits = NULL;
        hBufferBitmap = CreateDIBSection(hMemoryDC, (LPBITMAPINFO)&bitmapInfo, DIB_RGB_COLORS, (void**)&bufferBits, NULL, 0);

        if (hBufferBitmap == NULL) { bufferBits = NULL; return false; }
    }

    HGDIOBJ hOldObject = SelectObject(hMemoryDC, hBufferBitmap);
    BitBlt(hMemoryDC, 0, 0, captureWidth, captureHeight, hScreenDC, captureLeft, captureTop, SRCCOPY);
    SelectObject(hMemoryDC, hOldObject);
    DeleteDC(hMemoryDC);
    ReleaseDC(GetDesktopWindow(), hScreenDC);



    HDC hdcMem = CreateCompatibleDC(hDC);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, (HGDIOBJ)hBufferBitmap);
    BITMAP bm;
    GetObject(hBufferBitmap, sizeof(bm), &bm);

    // rect
    if (isFlipped)
    {
        StretchBlt(hDC, bm.bmWidth - 1, 0, -bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    else
    {
        BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
    }

    HBRUSH hBrushBack = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
    RECT r;
    r.left = bm.bmWidth; r.top = 0; r.right = rect->right; r.bottom = rect->bottom;
    FillRect(hDC, &r, hBrushBack);
    r.left = 0; r.top = bm.bmHeight; r.right = bm.bmWidth; r.bottom = rect->bottom;
    FillRect(hDC, &r, hBrushBack);
    DeleteObject((HGDIOBJ)hBrushBack);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

    */

#if 0
    HDC hScreenDC = GetDC(GetDesktopWindow());
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    if (mode == 1)
    {
        StretchBlt(hDC, captureWidth - 1, 0, -captureWidth, captureHeight, hScreenDC, captureLeft, captureTop, captureWidth, captureHeight, SRCCOPY);
    }
    else if (mode == 2)
    {
        StretchBlt(hDC, 0, captureHeight - 1, captureWidth, -captureHeight, hScreenDC, captureLeft, captureTop, captureWidth, captureHeight, SRCCOPY);
    }
    else    // mode == 0
    {
        BitBlt(hDC, 0, 0, captureWidth, captureHeight, hScreenDC, captureLeft, captureTop, SRCCOPY);
    }

    DeleteDC(hMemoryDC);
    ReleaseDC(GetDesktopWindow(), hScreenDC);
#endif

#if 1
    {
        HDC hMemoryDC = CreateCompatibleDC(hDC);
        HBITMAP hbmOld = SelectObject(hMemoryDC, hBufferBitmap);

        BitBlt(hDC, 0, 0, bufferWidth, bufferHeight, hMemoryDC, 0, 0, SRCCOPY);

        SelectObject(hMemoryDC, hbmOld);
        DeleteDC(hMemoryDC);
    }
#endif

    // Background
    {
        HBRUSH hBrushBack = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
        RECT r;
        r.left = captureWidth; r.top = 0; r.right = rect->right; r.bottom = rect->bottom;
        FillRect(hDC, &r, hBrushBack);
        r.left = 0; r.top = captureHeight; r.right = captureWidth; r.bottom = rect->bottom;
        FillRect(hDC, &r, hBrushBack);
        DeleteObject((HGDIOBJ)hBrushBack);
    }

    return 1;
}




static char Screenshot()
{
    HGDIOBJ hOldObject;
    HDC hScreenDC = GetDC(GetDesktopWindow());
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    bufferWidth = captureWidth; bufferHeight = captureHeight;
    if (hBufferBitmap == NULL || bufferWidth != actualBufferWidth || bufferHeight != actualBufferHeight)
    {
        BITMAPINFO bitmapInfo;

        actualBufferWidth = bufferWidth;
        actualBufferHeight = bufferHeight;
        //bufferBitCount = 24;

        if (hBufferBitmap != NULL)
        {
            DeleteObject(hBufferBitmap);
        }
        hBufferBitmap = CreateCompatibleBitmap(hScreenDC, actualBufferWidth, actualBufferHeight);

        ZeroMemory(&bitmapInfo, sizeof(BITMAPINFO));
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = actualBufferWidth;
        bitmapInfo.bmiHeader.biHeight = -actualBufferHeight;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = bufferBitCount;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        // Fill in remaining elements in bitmapInfo.bmiHeader 
        GetDIBits(hMemoryDC, hBufferBitmap, 0, bufferHeight, NULL, &bitmapInfo, DIB_RGB_COLORS);
        // bitmapInfo.bmiHeader.biSizeImage

        bufferBits = NULL;
        hBufferBitmap = CreateDIBSection(hMemoryDC, (LPBITMAPINFO)&bitmapInfo, DIB_RGB_COLORS, (void**)&bufferBits, NULL, 0);

        if (hBufferBitmap == NULL) { bufferBits = NULL; return 0; }
    }

    hOldObject = SelectObject(hMemoryDC, hBufferBitmap);
    BitBlt(hMemoryDC, 0, 0, captureWidth, captureHeight, hScreenDC, captureLeft, captureTop, SRCCOPY);
    SelectObject(hMemoryDC, hOldObject);
    DeleteDC(hMemoryDC);
    ReleaseDC(GetDesktopWindow(), hScreenDC);

    return 1;
}




static char UpdateDisplay()
{
    int rowlength, span;
    int y;

    //BITMAP bufferBitmap = {0};
    //GetObject(hBufferBitmap, sizeof(bufferBitmap), &bufferBitmap);
    // bufferBitmap.bmWidth, bufferBitmap.bmHeight

    if (bufferBits == NULL) { return 0; }

    rowlength = bufferWidth;
    // Calculate word-aligned boundaries
    span = (bufferWidth * bufferBitCount / 8) + ((bufferWidth * (32 - bufferBitCount)) & 0x03);

    if (image.buffer == NULL || bufferWidth != image.width || bufferHeight != image.height)
    {
        ImageInit(&image, bufferWidth, bufferHeight);
    }

    // Copy bitmap to image
    for (y = 0; y < image.height; y++)
    {
        unsigned char *s = bufferBits + (span * y);
        unsigned char *d = image.buffer + (y * (image.width * 3));
        memcpy(d, s, (image.width * 3));
    }


    if (edgeEnhance) { ImageEdgeEnhance(&image, 1); }
    ImageDither(&image, (ditherIndex == 0) ? 0 : (1 << (ditherIndex - 1)));

#if 1
    // Copy image back to bitmap
    for (y = 0; y < image.height; y++)
    {
        unsigned char *d = bufferBits + (span * y);
        unsigned char *s = image.buffer + (y * (image.width * 3));
        memcpy(d, s, (image.width * 3));
    }
#endif

    ImageToDisplay(&image, &display, 0);

    DisplayToDevice(&azimuth, &display);

if (0)    for (;;)
    {
        unsigned long long start = TicksNow();
        unsigned long long elapsed;
        if (!AzimuthPoll(&azimuth)) { break; }
printf("%f\n", azimuth.linearAccel[0]);
        elapsed = TicksNow() - start;
        if (elapsed > 8) 
        { break; }
    }

    return 1;
} 


// Instance WndProc function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    LRESULT lRet = 0;

    switch (message)
    {

    case WM_CREATE: 
        {
            hWndMain = hWnd;
            SetTimer(hWnd, dwTimerId, REFRESH_INTERVAL, NULL);
            //SetCapture(hWnd);
            //ShowCursor(FALSE);
            isBorderless = SetBorderless(hWnd, isBorderless);
        } 
        break; 

    case WM_CLOSE: 
        {
            if (hWnd != NULL)
            {
                DestroyWindow(hWnd);
                hWnd = NULL;
            }
        }
        break; 

    case WM_DESTROY: 
        {
            //ShowCursor(TRUE);
            PostQuitMessage(0); 
        }
        break; 

    case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
	        if (wParam == VK_F2)
	        {
                ditherIndex = (ditherIndex + 1) % 5;     // 0,1,2,4,8
	        }
	        if (wParam == VK_F3)
	        {
                edgeEnhance = !edgeEnhance;
	        }
            if (wParam == VK_F6)
            {
                mode++;
                if (mode > 2) { mode = 0; }
            }
            if (wParam == VK_F1)
            {
                if (hWndResizer != NULL)
                {
                    if (!IsWindowVisible(hWndResizer))
                    {
                        ShowWindow(hWndResizer, SW_SHOW);
                    }
                    else 
                    {
                        ShowWindow(hWndResizer, SW_HIDE);
                    }
                }
            }
            if (wParam == VK_F11 || wParam == VK_F5)
            {
                isBorderless = SetBorderless(hWnd, !isBorderless);
            }
        }
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            //int wmEvent = HIWORD(wParam); 
            switch (wmId)
            {
            case IDOK:
                SendMessage(hWnd, WM_CLOSE, 0, 0);
                return 1;
                break;
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps = {0};
            HDC hDC = BeginPaint(hWnd, &ps);
            RECT rect = {0};
            GetClientRect(hWnd, &rect);
            if (!Render(hWnd, hDC, &rect))
            {
                HBRUSH hBrushBack = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
                FillRect(hDC, &rect, hBrushBack);
                DeleteObject((HGDIOBJ)hBrushBack);
            }
            EndPaint(hWnd, &ps);
            return 1;
        }
        break;

    case WM_MOVE:
    case WM_SIZE:
        {
            RECT rect = {0};
            POINT pointTL, pointBR;

            GetClientRect(hWnd, &rect);

            pointTL.x = rect.left; pointTL.y = rect.top; ClientToScreen(hWnd, &pointTL);
            pointBR.x = rect.right; pointBR.y = rect.bottom; ClientToScreen(hWnd, &pointBR);
            rect.left = pointTL.x; rect.top = pointTL.y; rect.right = pointBR.x; rect.bottom = pointBR.y;

            //MoveWindow(hWndAx, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
            //SetWindowPos(hWndAx, 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
        {
            //SendMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;

    case WM_MOUSEMOVE:
        {
            //int xPos = (int)((lParam >>  0) & 0xffff); 
            //int yPos = (int)((lParam >> 16) & 0xffff); 
        }
        break;

    case WM_TIMER:
        {
            if (wParam == dwTimerId)
            {

                if (hWndResizer != NULL)
                {
#if 0
                    RECT rect = {0};
                    GetClientRect(hWndResizer, &rect);

                    POINT pointTL;
                    pointTL.x = rect.left;
                    pointTL.y = rect.top;
                    ClientToScreen(hWndResizer, &pointTL);

                    POINT pointBR;
                    pointBR.x = rect.right - rect.left;
                    pointBR.y = rect.bottom - rect.top;
                    ClientToScreen(hWndResizer, &pointBR);

                    captureLeft = pointTL.x;
                    captureTop = pointTL.y;
                    captureWidth = pointBR.x - pointTL.x;
                    captureHeight = pointBR.y - pointTL.y;
#else
                    RECT rect = {0};
                    GetWindowRect(hWndResizer, &rect);

                    captureLeft = rect.left;
                    captureTop = rect.top;
                    captureWidth = rect.right - rect.left;
                    captureHeight = rect.bottom - rect.top;
#endif
                }

                //DWORD dwNow = GetTickCount();
Screenshot();
UpdateDisplay();

                //CreateTexture
                //BITMAP bufferBitmap = {0};
                //GetObject(hBufferBitmap, sizeof(bufferBitmap), &bufferBitmap);
                // bufferBitmap.bmWidth, bufferBitmap.bmHeight

                //if (bufferBits == NULL) { return false; }

                //int w = bufferWidth;
                //int h = bufferHeight;
                //int rowlength = bufferWidth;
                // Calculate word-aligned boundaries
                //int span = (bufferWidth * bufferBitCount / 8) + ((bufferWidth * (32 - bufferBitCount)) & 0x03);
                // BGR_EXT, UNSIGNED_BYTE, bufferBits

                //Display();
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;

        case WM_LBUTTONDBLCLK:
        case WM_NCLBUTTONDBLCLK:
            isBorderless = SetBorderless(hWnd, !isBorderless);
            return 0;
            break;

        case WM_NCHITTEST:
            {
                LRESULT ht = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
                POINT p;
                p.x = ((int)lParam >>  0) & 0xffff; 
                p.y = ((int)lParam >> 16) & 0xffff; 
                if (isBorderless)
                {
                    return HTCAPTION;
                }
                else
                {
                    if (ht == HTCLIENT)
                    {
                        ht = HTCAPTION;
                    }
                    return ht;
                }
            }
            break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam); 
}

// WinMain
int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    #define INITIAL_COMMAND ""
    const char *inputFile = "!", *initialCommand = INITIAL_COMMAND;

    int wtop = CW_USEDEFAULT, wleft = CW_USEDEFAULT, wwidth = CW_USEDEFAULT, wheight = CW_USEDEFAULT;
    HWND hWnd;
    DWORD dwStyle;
    MSG msg;

    // Create a text console window here (this makes sure it's behind our window) -- AllocConsole();
//#ifdef _DEBUG
    RedirectIOToConsole();
//#endif

    printf("Display Demo - Daniel Jackson, 2013\n");

    hInstance = hInst;

// Get the (single) screen size
captureLeft = 0, captureTop = 0;
captureWidth = GetSystemMetrics(SM_CXSCREEN);
captureHeight = GetSystemMetrics(SM_CYSCREEN);

// Get the virtual desktop screen size
captureLeft   = GetSystemMetrics(SM_XVIRTUALSCREEN);
captureTop    = GetSystemMetrics(SM_YVIRTUALSCREEN);
captureWidth  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
captureHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

// Fixed size
captureWidth = 128;
captureHeight = 64;

    // Parse command-line
    {
        char *c = lpCmdLine;  // GetCommandLine();
        char *token = NULL;
        char end = 0;
        while (!end) 
        {
            if (*c == '\0') { end = 1; }
            if (*c == '/' || *c == '-' || end) {
                *c = '\0';
                if (token != NULL) {
                    char *option;
                    for (option = token; *option == ' ' || *option == '/' || *option == '-'; option++);
                    if (*option != '\0') {
                        char *value = option;
                        for (;;) {
                            if (*value == '\0') { break; }
                            if (*value == ' ' || *value == ':' || *value == '\0') { *value = '\0'; value++; break; }
                            value++;
                        }

                        if (_stricmp(option, "?") == 0) {           // Help
                            printf("Options: -in:<input> -cleft:0 -ctop:0 -cwidth:128 -cheight:64 -wleft -wtop -wwidth -wheight -borderless -invert");
                        }
                        else if (_stricmp(option, "in") == 0 || _stricmp(option, "input") == 0) 
                        { 
                            inputFile = value; 
                        }
                        else if (_stricmp(option, "init") == 0) { initialCommand = value; }
                        else if (_stricmp(option, "cleft") == 0) { captureLeft = atoi(value); }
                        else if (_stricmp(option, "ctop") == 0) { captureTop = atoi(value); }
                        else if (_stricmp(option, "cwidth") == 0) { captureWidth = atoi(value); }
                        else if (_stricmp(option, "cheight") == 0) { captureHeight = atoi(value); }
                        else if (_stricmp(option, "wleft") == 0) { wleft = atoi(value); }
                        else if (_stricmp(option, "wtop") == 0) { wtop = atoi(value); }
                        else if (_stricmp(option, "wwidth") == 0) { wwidth = atoi(value); }
                        else if (_stricmp(option, "wheight") == 0) { wheight = atoi(value); }
                        else if (_stricmp(option, "borderless") == 0) { isBorderless = 1; }
                        else if (_stricmp(option, "flipnone") == 0) { mode = 0; }
                        else if (_stricmp(option, "flipmirror") == 0) { mode = 1; }
                        else if (_stricmp(option, "flipinvert") == 0) { mode = 2; }
                    }

                    token = NULL;
                }
            } else {
                if (token == NULL) {
                    token = c;
                }
            }
            c++;
        }
    }

    // Init
    ImageInit(&image, 128, 64);
    DisplayInit(&display);

    AzimuthInit(&azimuth, inputFile, initialCommand, 100, -1.0f);

    // Register window class
    {
        WNDCLASSEX wc = {0};
        memset(&wc, 0, sizeof(wc));
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.lpfnWndProc   = WndProc;       // Window procedure
        wc.hInstance     = hInstance;     // hInstance 
        wc.lpszClassName = szAppName;     // Window class name
        wc.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW);   // Cursor
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);  //(HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_WEBSCR_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
        wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);   //(HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_WEBSCR_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.cbClsExtra    = 0; 
        wc.cbWndExtra    = 0; //sizeof (LONG_PTR);
        wc.hbrBackground = NULL; // (HBRUSH)CreateSolidBrush(RGB(dwShade, dwShade, dwShade));  //(HBRUSH)(COLOR_WINDOW + 1);  //(HBRUSH)GetStockObject(GRAY_BRUSH);
        wc.lpszMenuName  = NULL; 
        if (!RegisterClassEx(&wc)) 
        {
            MessageBox(NULL, TEXT("Error registering window class."), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
            return FALSE;
        }
    }

    // Create the main window.  
    hWnd = CreateWindowEx(
        WS_EX_WINDOWEDGE | WS_EX_APPWINDOW,  //  | WS_EX_LAYERED | WS_EX_TOOLWINDOW
        szAppName,                     // window class name
        szAppTitle,                    // window title
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        wtop, wleft, wwidth, wheight,  // CW_USEDEFAULT
        NULL,                          // parent window 
        NULL,                          // no menu
        hInstance,                     // instance
        NULL);                         // window creation data

    hWndMain = hWnd;

    if (!hWnd)
    {
        MessageBox(NULL, TEXT("Error creating window."), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
    {
        printf("WARNING: Unable to install console control handler.\n");
    }

    hWndResizer = ResizerInit(hInstance, captureLeft, captureTop, captureWidth, captureHeight);
    if (!hWndResizer)
    {
        MessageBox(NULL, TEXT("Error creating resizer window."), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }


    dwStyle = GetWindowLong(hWnd, GWL_STYLE);

    /*
#ifndef _DEBUG
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN), 0);
    dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_POPUP);
    dwStyle |= WS_SYSMENU;
    SetWindowLong(hWnd, GWL_STYLE, dwStyle);
    SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
    */

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    // Process pending messages then do update stuff below,
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }

    if (hWindowDC != NULL) { DeleteDC(hWindowDC); hWindowDC = NULL; }

    AzimuthClose(&azimuth);

    // Return the exit code to the system
    return (int)(msg.wParam); 
}

