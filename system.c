/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

/*****************************************************************************/
#ifdef _WINDOWS
#include <windows.h>
#include <conio.h>
#endif
#include "gbagi.h"
#include "system.h"
#include "screen.h"
#include "system.h"
#include "input.h"
#include "wingui.h"
#include "commands.h"
//extern U8 *code;

#ifdef _WINDOWS
U16 prevKeys,kDown,kUp;
#else
U32 prevKeys;
#endif
int holdium;

extern int msgX, msgY, maxWidth;

const _RECT scrRect={0,0,SCREEN_MAXX,SCREEN_MAXY};  
const U16 Palette[256] = {
0x0C63,0x2C63,0x0D63,0x2D63,0x0C6F,0x3CEB,0x0CEB,0x2D6B,0x1CE7,0x3CE3,0x1DE3,0x3DE7,0x1D6F,0x3CEF,0x0DEF,0x3DEF,
	0x2C63,0x5C63,0x2D63,0x5D63,0x2C6F,0x6CEB,0x2CEB,0x5D6B,0x3CE7,0x6CE3,0x3DE3,0x6DE7,0x4D6F,0x6CEF,0x2DEF,0x6DEF,
	0x0D63,0x2D63,0x0EE3,0x2EE3,0x0D6F,0x3E6B,0x0DEB,0x2EEB,0x1DE7,0x3E63,0x1EE3,0x3F67,0x1E6F,0x3DEF,0x0F6F,0x3F6F,
	0x2D63,0x5D63,0x2EE3,0x5EE3,0x2D6F,0x6E6B,0x2DEB,0x5EEB,0x3DE7,0x6E63,0x3EE3,0x6F67,0x4E6F,0x6DEF,0x2F6F,0x6F6F,
	0x0C6F,0x2C6F,0x0D6F,0x2D6F,0x0C7B,0x3CFB,0x0CF7,0x2D7B,0x1CF3,0x3CEF,0x1DEF,0x3DF3,0x1D7F,0x3CFF,0x0DFB,0x3DFF,
	0x3CEB,0x6CEB,0x3E6B,0x6E6B,0x3CFB,0x6DF7,0x3DF7,0x6E77,0x4DF3,0x7DEB,0x4E6B,0x7EF3,0x5DFB,0x7DFB,0x3EFB,0x7EFB,
	0x0CEB,0x2CEB,0x0DEB,0x2DEB,0x0CF7,0x3DF7,0x0D73,0x2E77,0x1D6F,0x3D6B,0x1E6B,0x3EEF,0x1DFB,0x3D7B,0x0E77,0x3EFB,
	0x2D6B,0x5D6B,0x2EEB,0x5EEB,0x2D7B,0x6E77,0x2E77,0x5EF7,0x4E73,0x6E6B,0x4F6B,0x6F73,0x4EFB,0x6E7B,0x2F7B,0x6F7B,
	0x1CE7,0x3CE7,0x1DE7,0x3DE7,0x1CF3,0x4DF3,0x1D6F,0x4E73,0x2D6B,0x5DE7,0x2E67,0x5EEB,0x3DF7,0x5D77,0x1EF7,0x5EF7,
	0x3CE3,0x6CE3,0x3E63,0x6E63,0x3CEF,0x7DEB,0x3D6B,0x6E6B,0x5DE7,0x7DE3,0x5E63,0x7EE7,0x5DEF,0x7DEF,0x3EEF,0x7EEF,
	0x1DE3,0x3DE3,0x1EE3,0x3EE3,0x1DEF,0x4E6B,0x1E6B,0x4F6B,0x2E67,0x5E63,0x2F63,0x5FE7,0x3EEF,0x5E6F,0x1F6F,0x5FEF,
	0x3DE7,0x6DE7,0x3F67,0x6F67,0x3DF3,0x7EF3,0x3EEF,0x6F73,0x5EEB,0x7EE7,0x5FE7,0x7FEB,0x5F77,0x7EF7,0x3FF7,0x7FF7,
	0x1D6F,0x4D6F,0x1E6F,0x4E6F,0x1D7F,0x5DFB,0x1DFB,0x4EFB,0x3DF7,0x5DEF,0x3EEF,0x5F77,0x3E7F,0x5DFF,0x1EFF,0x5F7F,
	0x3CEF,0x6CEF,0x3DEF,0x6DEF,0x3CFF,0x7DFB,0x3D7B,0x6E7B,0x5D77,0x7DEF,0x5E6F,0x7EF7,0x5DFF,0x7D7F,0x3EFF,0x7EFF,
	0x0DEF,0x2DEF,0x0F6F,0x2F6F,0x0DFB,0x3EFB,0x0E77,0x2F7B,0x1EF7,0x3EEF,0x1F6F,0x3FF7,0x1EFF,0x3EFF,0x0FFF,0x3FFF,
	0x3DEF,0x6DEF,0x3F6F,0x6F6F,0x3DFF,0x7EFB,0x3EFB,/*0x739C*/0x77BD,0x5EF7,0x7EEF,0x5FEF,0x7FF7,0x5F7F,0x7EFF,0x3FFF,0x7FFF/*
0x0C63,0x3C63,0x0D63,0x2D63,0x0C6F,0x3CEB,0x0CEB,0x2D6B,0x1CE7,0x3D63,0x1DE3,0x3DE7,0x2D6F,0x3CEF,0x0DEF,0x3DEF,
	0x3C63,0x7C63,0x3D63,0x6D63,0x3C6F,0x7CEB,0x3CEB,0x6D6B,0x5CE7,0x7D63,0x5DE3,0x7DE7,0x6D6F,0x7CEF,0x3DEF,0x7DEF,
	0x0D63,0x3D63,0x0EE3,0x2EE3,0x0D6F,0x3E6B,0x0DEB,0x2EEB,0x1DE7,0x3EE3,0x1EE3,0x3F67,0x2EEF,0x3DEF,0x0F6F,0x3F6F,
	0x2D63,0x6D63,0x2EE3,0x5EE3,0x2D6F,0x6E6B,0x2DEB,0x5EEB,0x3DE7,0x6EE3,0x3EE3,0x6F67,0x4EEF,0x6DEF,0x2F6F,0x6F6F,
	0x0C6F,0x3C6F,0x0D6F,0x2D6F,0x0C7B,0x3CFB,0x0CF7,0x2D7B,0x1CF3,0x3D6F,0x1DEF,0x3DF3,0x2D7F,0x3CFF,0x0DFB,0x3DFF,
	0x3CEB,0x7CEB,0x3E6B,0x6E6B,0x3CFB,0x6DF7,0x3DF7,0x6E77,0x4DF3,0x7E6B,0x4E6B,0x7EF3,0x5E7B,0x7DFB,0x3EFB,0x7EFB,
	0x0CEB,0x3CEB,0x0DEB,0x2DEB,0x0CF7,0x3DF7,0x0D73,0x2E77,0x1D6F,0x3E6B,0x1E6B,0x3EEF,0x2DFB,0x3D7B,0x0E77,0x3EFB,
	0x2D6B,0x6D6B,0x2EEB,0x5EEB,0x2D7B,0x6E77,0x2E77,0x5EF7,0x4E73,0x6EEB,0x4F6B,0x6F73,0x5EFB,0x6E7B,0x2F7B,0x6F7B,
	0x1CE7,0x5CE7,0x1DE7,0x3DE7,0x1CF3,0x4DF3,0x1D6F,0x4E73,0x2D6B,0x5E67,0x2E67,0x5EEB,0x3DF7,0x5D77,0x1EF7,0x5EF7,
	0x3D63,0x7D63,0x3EE3,0x6EE3,0x3D6F,0x7E6B,0x3E6B,0x6EEB,0x5E67,0x7EE3,0x5F63,0x7F67,0x6EEF,0x7E6F,0x3F6F,0x7F6F,
	0x1DE3,0x5DE3,0x1EE3,0x3EE3,0x1DEF,0x4E6B,0x1E6B,0x4F6B,0x2E67,0x5F63,0x2F63,0x5FE7,0x3EEF,0x5E6F,0x1F6F,0x5FEF,
	0x3DE7,0x7DE7,0x3F67,0x6F67,0x3DF3,0x7EF3,0x3EEF,0x6F73,0x5EEB,0x7F67,0x5FE7,0x7FEB,0x6F77,0x7EF7,0x3FF7,0x7FF7,
	0x2D6F,0x6D6F,0x2EEF,0x4EEF,0x2D7F,0x5E7B,0x2DFB,0x5EFB,0x3DF7,0x6EEF,0x3EEF,0x6F77,0x4E7F,0x6DFF,0x2F7F,0x6F7F,
	0x3CEF,0x7CEF,0x3DEF,0x6DEF,0x3CFF,0x7DFB,0x3D7B,0x6E7B,0x5D77,0x7E6F,0x5E6F,0x7EF7,0x6DFF,0x7D7F,0x3EFF,0x7EFF,
	0x0DEF,0x3DEF,0x0F6F,0x2F6F,0x0DFB,0x3EFB,0x0E77,0x2F7B,0x1EF7,0x3F6F,0x1F6F,0x3FF7,0x2F7F,0x3EFF,0x0FFF,0x3FFF,
	0x3DEF,0x7DEF,0x3F6F,0x6F6F,0x3DFF,0x7EFB,0x3EFB,0x6F7B,0x5EF7,0x7F6F,0x5FEF,0x7FF7,0x6F7F,0x7EFF,0x3FFF,0x7FFF
	0x0C63,0x2C63,0x0D63,0x2D63,0x0C6B,0x2C6B,0x0CEB,0x2D6B,0x1CE7,0x3CE7,0x1DE3,0x3DE7,0x1CEF,0x3CEF,0x1DEF,0x3DEF,
	0x2C63,0x5C63,0x2D63,0x5D63,0x2C6B,0x5C6B,0x2CEB,0x5D6B,0x3CE7,0x6CE7,0x3DE3,0x6DE7,0x3CEF,0x6CEF,0x3DEF,0x6DEF,
	0x0D63,0x2D63,0x0EE3,0x2EE3,0x0D6B,0x2D6B,0x0DEB,0x2EEB,0x1DE7,0x3DE7,0x1F63,0x3F67,0x1DEF,0x3DEF,0x1F6F,0x3F6F,
	0x2D63,0x5D63,0x2EE3,0x5EE3,0x2D6B,0x5D6B,0x2DEB,0x5EEB,0x3DE7,0x6DE7,0x3F63,0x6F67,0x3DEF,0x6DEF,0x3F6F,0x6F6F,
	0x0C6B,0x2C6B,0x0D6B,0x2D6B,0x0C77,0x2C73,0x0CF7,0x2D77,0x1CEF,0x3CEF,0x1DEB,0x3DEF,0x1CFB,0x3CFB,0x1DFB,0x3DFB,
	0x2C6B,0x5C6B,0x2D6B,0x5D6B,0x2C73,0x5C73,0x2CF3,0x5D73,0x3CEF,0x6CEF,0x3DEB,0x6DEF,0x3CF7,0x6CF7,0x3DF7,0x6DF7,
	0x0CEB,0x2CEB,0x0DEB,0x2DEB,0x0CF7,0x2CF3,0x0D77,0x2DF7,0x1D6F,0x3D6F,0x1EEB,0x3EEF,0x1D7B,0x3D7B,0x1EFB,0x3EFB,
	0x2D6B,0x5D6B,0x2EEB,0x5EEB,0x2D77,0x5D73,0x2DF7,0x5EF7,0x3DEF,0x6DEF,0x3F6B,0x6F6F,0x3DFB,0x6DFB,0x3F7B,0x6F7B,
	0x1CE7,0x3CE7,0x1DE7,0x3DE7,0x1CEF,0x3CEF,0x1D6F,0x3DEF,0x2D6B,0x5D6B,0x2EE7,0x5EEB,0x2D77,0x5D77,0x2EF7,0x5EF7,
	0x3CE7,0x6CE7,0x3DE7,0x6DE7,0x3CEF,0x6CEF,0x3D6F,0x6DEF,0x5D6B,0x7D6B,0x5EE7,0x7EEB,0x5D77,0x7D77,0x5EF7,0x7EF7,
	0x1DE3,0x3DE3,0x1F63,0x3F63,0x1DEB,0x3DEB,0x1EEB,0x3F6B,0x2EE7,0x5EE7,0x2FE3,0x5FE7,0x2EEF,0x5EEF,0x2FEF,0x5FEF,
	0x3DE7,0x6DE7,0x3F67,0x6F67,0x3DEF,0x6DEF,0x3EEF,0x6F6F,0x5EEB,0x7EEB,0x5FE7,0x7FEB,0x5EF7,0x7EF7,0x5FF7,0x7FF7,
	0x1CEF,0x3CEF,0x1DEF,0x3DEF,0x1CFB,0x3CF7,0x1D7B,0x3DFB,0x2D77,0x5D77,0x2EEF,0x5EF7,0x2D7F,0x5D7F,0x2EFF,0x5EFF,
	0x3CEF,0x6CEF,0x3DEF,0x6DEF,0x3CFB,0x6CF7,0x3D7B,0x6DFB,0x5D77,0x7D77,0x5EEF,0x7EF7,0x5D7F,0x7D7F,0x5EFF,0x7EFF,
	0x1DEF,0x3DEF,0x1F6F,0x3F6F,0x1DFB,0x3DF7,0x1EFB,0x3F7B,0x2EF7,0x5EF7,0x2FEF,0x5FF7,0x2EFF,0x5EFF,0x2FFF,0x5FFF,
	0x3DEF,0x6DEF,0x3F6F,0x6F6F,0x3DFB,0x6DF7,0x3EFB,0x6F7B,0x5EF7,0x7EF7,0x5FEF,0x7FF7,0x5EFF,0x7EFF,0x5FFF,0x7FFF,   */
};
/*****************************************************************************/

#ifdef _WINDOWS
const int sysRemap[]={
	'X',		//BTN_A
	'Z',		//BTN_B
	VK_SPACE,		//BTN_SELECT
	VK_RETURN,		//BTN_START
	VK_RIGHT,		//BTN_RIGHT
	VK_LEFT,		//BTN_LEFT
	VK_UP,			//BTN_UP
	VK_DOWN,		//BTN_DOWN
	'S',			//BTN_R
	'A',			//BTN_L
	0,
};
#endif
const KEYMAP buttonKeymap[] = {
	{KEY_RESET,		B_START|B_SELECT|B_A|B_B},

	{KEY_NUMPAD7,	B_UP|B_LEFT},
	{KEY_NUMPAD9,	B_UP|B_RIGHT},
	{KEY_NUMPAD1,	B_DOWN|B_LEFT},
	{KEY_NUMPAD3,	B_DOWN|B_RIGHT},

	{KEY_START,		B_START},
	{KEY_SELECT,	B_SELECT},
	{KEY_TABFWD,	B_R},
	{KEY_TABREV,	B_L},

	{KEY_ENTER,		B_A},
	{KEY_ESC,		B_B},
	{KEY_RIGHT,		B_RIGHT},
	{KEY_LEFT,		B_LEFT},
	{KEY_UP,		B_UP},
	{KEY_DOWN,		B_DOWN},
	{0,0},
};

/*****************************************************************************/
U16 bGetW(U8 *p)
{
	return (*p)+(p[1]<<8);
}
/*****************************************************************************/
U16 beGetW(U8 *p)
{
	return (p[1])+(p[0]<<8);
}
/*****************************************************************************/
#ifdef _WINDOWS
HBITMAP hBitmap;
HDC ahDC;
BITMAPINFO *binfo;
BITMAPINFOHEADER bmih,binfoh;

    HWND hwnd;
    WNDCLASSEX WndClass;
    MSG Msg;
HINSTANCE ghInstance;

const UINT idTimer1 = 1;
UINT nTimerDelay = 75;

static char g_szClassName[]="GBAGI";
static HINSTANCE g_hInst = NULL;

U8 oscr[SCREEN_SIZE];

/*****************************************************************************/

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void DoMsg(void);
U8 tu8,*oc;
/*****************************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;
    U8 *p;
    int y;
   switch(Message)
   {
      case WM_PAINT:
                p = oscr;
                for(y=SCREEN_MAXY;y>=0;y--) {
                    memcpy(p,screenBuf+(y*SCREEN_WIDTH),SCREEN_WIDTH);
                    p+=SCREEN_WIDTH;
                }
              hDC = BeginPaint(hwnd, &ps);
            StretchDIBits(
                hDC,
                0,
		        0,
		        SCREEN_WIDTH<<1,
		        SCREEN_HEIGHT<<1,
		        0,
                0,
		        SCREEN_WIDTH,
		        SCREEN_HEIGHT,
		        oscr,
		        binfo,
		        DIB_RGB_COLORS,
		        SRCCOPY
            );

            EndPaint(hwnd, &ps);
      break;
      case WM_CLOSE:
         DestroyWindow(hwnd);
      break;
      case WM_DESTROY:
         PostQuitMessage(0);
      break;
      case WM_KEYUP:
         kUp=wParam;
      break;
      case WM_KEYDOWN:
         kDown=wParam;   /*
         if(kDown>='0'&&kDown<='9') {
         	oc = code;
            tu8 = kDown-'0';
            code = &tu8;
        	cObjStatusV();
            code = oc;
         }   */
      break;
      default:
         return DefWindowProc(hwnd, Message, wParam, lParam);
   }
   return 0;
}

/*****************************************************************************/
BOOL SystemInit()
{
	int i;
                   /*
	int j;
    FILE *f=fopen("pal.pal","w");
    fprintf(f,"JASC-PAL\r\n0100\r\n256\r\n");
    for(i=0;i<256;i++)
    	fprintf(f,"%d %d %d\r\n",(Palette[i]&0x1F)<<3,((Palette[i]>>5)&0x1F)<<3,((Palette[i]>>10)&0x1F)<<3);
	fclose(f);    */
                 /*
    FILE *f=fopen("e:\\gba\\bg10\\font.c","w");
    	U8 *p=fontData;
   		for(i=0;i<128*8;i++) {
        	for(j=7;j>=0;j--) {
             	fprintf(f,"0x%02X,",((*p>>j)&1)*0x99);
            }
            if(i&1)fprintf(f,"\r\n");
            p++;
        }
    fclose(f);      */

	prevKeys=0;
    kDown=0;
    kUp=0;
    btnstate.kbstate = 0;
    btnstate.kbkey = KEY_ESC;
    btnstate.kbrow = 0;
    btnstate.kbcol = 0;

    g_hInst = 0;

    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.style         = 0;
    WndClass.lpfnWndProc   = WndProc;
    WndClass.cbClsExtra    = 0;
    WndClass.cbWndExtra    = 0;
    WndClass.hInstance     = g_hInst;
    WndClass.hIcon         = LoadIcon(g_hInst, IDI_APPLICATION);
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(0);
    WndClass.lpszMenuName  = NULL;
    WndClass.lpszClassName = g_szClassName;
    WndClass.hIconSm       = LoadIcon(g_hInst, IDI_APPLICATION);

    if(!RegisterClassEx(&WndClass)) {
        MessageBox(0, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
        return 0;
    }

    hwnd = CreateWindowEx(
        0,
        g_szClassName,
        g_szClassName,
        WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN)-(SCREEN_WIDTH*2))>>1,
        (GetSystemMetrics(SM_CYSCREEN)-(SCREEN_HEIGHT*2))>>1,
        (SCREEN_WIDTH*2)  + (GetSystemMetrics(SM_CXEDGE)<<1)+2,
        (SCREEN_HEIGHT*2) + GetSystemMetrics(SM_CYCAPTION) + (GetSystemMetrics(SM_CYEDGE)<<1)+2,
        NULL, NULL, g_hInst, NULL
    );

    if(hwnd == NULL) {
        MessageBox(0, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
        return 0;
    }

    ShowWindow(hwnd, 1);
    UpdateWindow(hwnd);
    binfoh.biSize          = sizeof(BITMAPINFOHEADER);
    binfoh.biWidth         = SCREEN_WIDTH;
    binfoh.biHeight        = SCREEN_HEIGHT;
    binfoh.biPlanes        = 1;
    binfoh.biBitCount      = 8;
    binfoh.biCompression   = BI_RGB;
    binfoh.biSizeImage     = 0;
    binfoh.biXPelsPerMeter = 0;
    binfoh.biYPelsPerMeter = 0;
    binfoh.biClrUsed       = 0;
    binfoh.biClrImportant  = 0;

    binfo = (BITMAPINFO *)malloc( sizeof(*binfo) + 256*sizeof(RGBQUAD) );
    binfo->bmiHeader = binfoh;

//        binfo->bmiColors = (RGBQUAD *)malloc( 256*sizeof(RGBQUAD) );

        for(i = 0;i<256;i++) {
            binfo->bmiColors[i].rgbBlue      = ((Palette[i]>>10)&0x1F)<<3;
            binfo->bmiColors[i].rgbGreen    = ((Palette[i]>>5)&0x1F)<<3;
            binfo->bmiColors[i].rgbRed     = (Palette[i]&0x1F)<<3;
            binfo->bmiColors[i].rgbReserved = 0;
        }

    ahDC = GetDC( hwnd );
    hBitmap = CreateDIBitmap(
      ahDC,
      &binfo->bmiHeader,
      CBM_INIT,
      screenBuf,
      binfo,
      DIB_RGB_COLORS
    );
    ReleaseDC( hwnd, ahDC );

    memset(screenBuf,0x39,sizeof(screenBuf));
                                    
    msgX			= -1;
    msgY			= -1;
    maxWidth		= -1;

	return TRUE;
}
#else
/*****************************************************************************/
void GBA_Flip()
{
	U16 *p1,*p2,*pend; /*
	__asm //inline assembler
	{
        mov r0, #0x4000006 //0x4000006 is vertical trace counter it’s address is loaded into r0
        scanline_wait: //lable
        ldrh r1, [r0] //move the value stored in the scan line register into r1
        cmp r1, #160 //comp it with 160
        bne scanline_wait //if it is equal then we are done else it jumps to scanline wait 
	}       */
    if(REG_DISPCNT & BACKBUFFER) //back buffer is the current buffer so we need to switch it to the font buffer
    {
    	p1 = ((U16*)0x600A000);
    	p2 = ((U16*)0x6000000);
    	pend = p1+GBASCR_SIZEOF;
		while(p1<pend)
			*p1++=*p2++;
        REG_DISPCNT |= BACKBUFFER; //flip active buffer to front buffer by clearing back buffer bit
        vidPtr = ((U16*)0x6000000); //now we point our drawing buffer to the back buffer
    }

    else //front buffer is active so switch it to backbuffer
    { 
    	p1 = ((U16*)0x6000000);
    	p2 = ((U16*)0x600A000);
    	pend = p1+GBASCR_SIZEOF;
		while(p1<pend)
			*p2++=*p1++;
        REG_DISPCNT &= ~BACKBUFFER; //flip active buffer to back buffer by setting back buffer bit
        vidPtr = ((U16*)0x600A000); //now we point our drawing buffer to the front buffer
    }
}
/*****************************************************************************/
void GBA_Flip2()
{
	U16 *p1,*p2,*pend;
    if(REG_DISPCNT & BACKBUFFER) //back buffer is the current buffer so we need to switch it to the font buffer
    {
    	p1 = ((U16*)0x6000000);
    	p2 = ((U16*)0x600A000);
    	pend = p1+GBASCR_SIZEOF;
		while(p1<pend)
			*p1++=*p2++;
        REG_DISPCNT |= BACKBUFFER; //flip active buffer to front buffer by clearing back buffer bit
        vidPtr = ((U16*)0x600A000); //now we point our drawing buffer to the back buffer
    }

    else //front buffer is active so switch it to backbuffer
    { 
    	p1 = ((U16*)0x6000000);
    	p2 = ((U16*)0x600A000);
    	pend = p1+GBASCR_SIZEOF;
		while(p1<pend)
			*p2++=*p1++;
        REG_DISPCNT &= ~BACKBUFFER; //flip active buffer to back buffer by setting back buffer bit
        vidPtr = ((U16*)0x6000000); //now we point our drawing buffer to the front buffer
    }
}
/*****************************************************************************/
BOOL SystemInit()
{
	int i=0;
	SetMode(MODE_4|BG2_ENABLE);
	for(i=0;i<256;i++)
		BGPaletteMem[i]=Palette[i];


    REG_DISPCNT |= BACKBUFFER; //flip active buffer to front buffer by clearing back buffer bit
    vidPtr = ((U16*)0x600A000); //now we point our drawing buffer to the back buffer

    wSetPort((_RECT*)&scrRect);

   	prevKeys = *KEYS;
    btnstate.kbstate = 0;
    btnstate.kbkey = KEY_ESC;
    btnstate.kbrow = 0;
    btnstate.kbcol = 0;

	REG_SOUNDCNT_X = 0x0080;
	REG_SOUNDCNT_L = 0xFF77;
	REG_SOUNDCNT_H = 0x0002;
	
	EnableInterupts(INT_TIMER3|INT_TIMER2);
	REG_TM2CNT_H = TIME_FREQUENcy256 | TIME_ENABLE | TIME_IRQ_ENABLE;
	REG_TM2CNT_L = 65535-1090;
	REG_TM3CNT_H = TIME_FREQUENcy64 | TIME_ENABLE | TIME_IRQ_ENABLE;
	REG_TM3CNT_L = 0;
	             
    msgX			= -1;
    msgY			= -1;
    maxWidth		= -1;

	return TRUE;
}    
#endif
/*****************************************************************************/
void gfxGUIEnter()
{
	GUI_ACTIVE = TRUE;
#ifndef _WINDOWS
	GBA_Flip2();
	GBA_Flip();
#endif        /*
#ifndef _WINDOWS
    if(REG_DISPCNT & BACKBUFFER)
        vidPtr = ((U16*)0x600A000);
    else
        vidPtr = ((U16*)0x6000000);
#endif    */
}
/*****************************************************************************/
void gfxGUIExit()
{
	GUI_ACTIVE = FALSE;
#ifndef _WINDOWS
    if(REG_DISPCNT & BACKBUFFER)
        vidPtr = ((U16*)0x600A000);
    else
        vidPtr = ((U16*)0x6000000);
#endif
	RedrawScreen();
#ifdef _WINDOWS
	SystemUpdate();
#endif
}
/*****************************************************************************/
void SystemDoit()
{
	//WinGUIDoit();
#ifdef _WINDOWS
    SystemUpdate();
#endif
}
/*****************************************************************************/
void SystemUpdate()
{
#ifdef _WINDOWS
        GetMessage(&Msg, NULL, 0, 0);
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        InvalidateRect(hwnd,0,0);
#else
	if(GUI_ACTIVE)
		GBA_Flip();
#endif
}
/*****************************************************************************/

/*****************************************************************************/

/*****************************************************************************/
BTNSTATE *ParseButtons(U16 buttons)
{
    const KEYMAP *km;
    int i;

    if(btnstate.state==BTN_INJECTED) {
		btnstate.state	= BTN_PRESS;
    	prevKeys = 0x8000;        
        holdium = 2;
    	return &btnstate;
    }

	btnstate.state=BTN_IDLE;

	km = buttonKeymap;
    i = 0;
    if(prevKeys == buttons) {
    	while(km->key) {
    		if((buttons&km->buttons)==km->buttons) {
        		btnstate.btn	= km->key;
				btnstate.state	= holdium?BTN_HOLD:BTN_PRESS;
                holdium = TRUE;
            	break;
            }
            km++;
            i++;
        }
    } else {
        btnstate.state	= BTN_RELEASE;
        if(holdium) holdium--;
    }

    prevKeys = buttons;

    return &btnstate;
}
/*****************************************************************************/
#ifdef _WINDOWS
BTNSTATE *GBACheckButtons()
{
	const int *sr;
    U16 buttons = 0;
    int bmask = 0x0001;

    SystemUpdate();

	sr = sysRemap;
    while(*sr) {
    	if(GetKeyState(*sr++)<0) {
        	buttons |= bmask;
        }
        bmask <<= 1;
    }

	return ParseButtons(buttons);
}
#else
BTNSTATE *GBACheckButtons()
{
	U16 buttons = (~(*KEYS))&0x3FF;
	return ParseButtons(buttons);
}
#endif
/*****************************************************************************/
int SystemCheckKey()
{
	GBACheckButtons();
	if(btnstate.state==BTN_PRESS||btnstate.state==BTN_HOLD) {
    	switch(btnstate.btn) {
         	case KEY_RESET:
            	QUIT_FLAG = TRUE;
        }
		if(btnstate.state==BTN_PRESS)
        	return btnstate.btn;
    }
    return 0;
}
/*****************************************************************************/
void SystemExit()
{
#ifdef _WINDOWS
    exit(1);
#endif
}
/*****************************************************************************/






