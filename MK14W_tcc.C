// **************************************************************************
//
//			Windows port of the Portable MK14 Emulator in 'C'
//
//						Written for Borland Turbo C++ v4.5
//						Written for Tiny CC
//
// Changes:-
// * The files needed the old DOS end of file character removed
// * MoveTo() needed to be converted to MoveToEx
// * The drag and drop needs more work and has been commented out.
//

/*
rem
rem build MK14 for Windows using Tiny C
rem

..\tcc MK14W_tcc.c cpu.c memory.c

pause

MK14W.exe
*/
//		Additional features :
//					Drag & Drop of Hex Files
//					Q or ESCAPE stops emulator running
//
// **************************************************************************

#include <windows.h>
//#include <shellapi.h>
#include <stdlib.h>
#include "scmp.h"


LRESULT CALLBACK MK14WindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
static void NEAR PASCAL RefreshLED(HDC hDC,int Led,int New,int Old);
static void NEAR PASCAL RefreshStatusLED(HDC hDC,int Led,int New,int Old);

HPEN hOnPen,hOffPen,hPausePen,hFlagsPen;						// Pens for painting LEDs
BOOL bRunning;
int  LEDStatus[DIGITS];									// LED Status
HDC  hDCWork;												// Repaint DC for running
char ObjectFile[128];

#define DELTA	(10)										// Mouse moves to halt

int PASCAL WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,
												LPSTR lpszCmdLine,int nCmdShow)
{
WNDCLASS wc;
HWND hWnd;
MSG Msg;

lstrcpy(ObjectFile,lpszCmdLine);

if (hPrevInst == NULL)									// Register class if required
	{
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MK14WindowProc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst,"APPICON");
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MK14WindowClass";
	RegisterClass(&wc);
	}

hOnPen    = CreatePen(PS_SOLID,4,RGB(220,0,0));    	// Create LED Draw Pens
hOffPen   = CreatePen(PS_SOLID,4,RGB(192,192,192));
hPausePen = CreatePen(PS_SOLID,4,RGB(128,128,128));

hFlagsPen = CreatePen(PS_SOLID,4,RGB(  0,200,0));    	// Create LED Draw Pens


hDCWork = NULL;

hWnd = CreateWindow("MK14WindowClass",				// Create and show window
						  "MK14 Emulator for Windows",
						  WS_OVERLAPPEDWINDOW,
						  CW_USEDEFAULT,CW_USEDEFAULT,350,150,
						  NULL,NULL,hInst,NULL);
ShowWindow(hWnd,nCmdShow);
// DragAcceptFiles(hWnd,TRUE);							// Can accept files...
hDCWork = GetDC(hWnd);
while (GetMessage(&Msg,(HWND)NULL,NULL,NULL))	// Main message loop
	{
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
	}

DeleteObject(hOnPen);									// Delete LED Draw Pens
DeleteObject(hOffPen);
DeleteObject(hPausePen);
ReleaseDC(hWnd,hDCWork);
return(0);
}

RECT rcFrame;												// Frame for display
RECT rcLED[DIGITS]; 										// LED Frames
RECT rcStatusLED[DIGITS];

LRESULT CALLBACK MK14WindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
LONG lRet = 0L;
int i,x,y,x1,xBorder,yBorder;
long xln;
PAINTSTRUCT ps;
HPEN hOldPen;
//HDROP hDrop;
MSG Msg;
POINT ptOld,ptNew;
BOOL bRange;

switch(iMessage)
	{
	case WM_CREATE:

/* these functions link in the MK 14 emulation code */
/*	*/	// Initialise window
		InitialiseDisplay();
		bRunning = FALSE;
		LoadROM();
		ResetCPU();
		LoadObject(ObjectFile);
		
/* */
		break;
/*
	case WM_DROPFILES:									// Drag and Drop for Application
		hDrop = (HDROP)wParam;
		i = DragQueryFile(hDrop,-1,NULL,0);			// Get count of files
		if (i == 1)       								// if only 1 file,load it
			{
			DragQueryFile(hDrop,0,(LPSTR)ObjectFile,sizeof(ObjectFile));
			LoadObject(ObjectFile);
			}
		break;
*/
	case WM_SIZE:											// Window resized
		x = LOWORD(lParam);y = HIWORD(lParam);
		xBorder = x / 40;
		yBorder = y / 40;
		
		rcFrame.left   = xBorder;
		rcFrame.right  = x - xBorder;
		
		rcFrame.top    = yBorder;
		rcFrame.bottom = y - yBorder;
		
		xln = (long)(rcFrame.right - rcFrame.left);
		x1 = x / 60;
		
		for (i = 0;i < DIGITS;i++)
			{
			rcLED[i].left = rcFrame.left + ((int)(xln * i / (LONG)DIGITS)) + x1;
			//rcLED[i].right = rcLED[i].left + (int)(xln/8) - x1*2;
			rcLED[i].right = rcLED[i].left + (int)(xln/DIGITS) - x1*2;
			rcLED[i].top = rcFrame.top + x1;
			rcLED[i].bottom = rcFrame.bottom-25 - x1;
			}
		break;

		for (i = 0;i < DIGITS;i++)
			{
			rcStatusLED[i].left = rcFrame.left + ((int)(xln * i / (LONG)DIGITS)) + x1;
			//rcLED[i].right = rcLED[i].left + (int)(xln/8) - x1*2;
			rcStatusLED[i].right = rcStatusLED[i].left + (int)(xln/DIGITS) - x1*2;
			
			rcStatusLED[i].top    = rcFrame.bottom -25 - x1;
			rcStatusLED[i].bottom = rcFrame.bottom - 5 - x1;
			}
		break;

		
		
		
		
	case WM_KEYUP:
/*
		//
		{
		  for (i = 0;i < DIGITS;i++){
		    LEDStatus[i] = wParam;
		  }
		  InvalidateRect(hWnd,NULL,TRUE); // triggeres a PAINT
		}
*/		
		if (wParam == 'G')
 		  SendMessage(hWnd,WM_LBUTTONDOWN,0,0L);

	    //InvalidateRect(hWnd,NULL,TRUE);
	  break;

	case WM_LBUTTONDOWN:									// Run program
		GetCursorPos(&ptOld);
		bRange = TRUE;
		bRunning = TRUE;
		for (i = 0;i < DIGITS;i++)
				RefreshLED(hDCWork,i,LEDStatus[i],-1);

		RefreshStatusLED(hDCWork,1,( Stat & 4  ),-1);
	    RefreshStatusLED(hDCWork,2,( Stat & 2  ),-1);
		RefreshStatusLED(hDCWork,3,( Stat & 1  ),-1);
	
			
			
		while (!CONKeyPressed(KEY_BREAK) && bRange)
			{
			BlockExecute();
			PeekMessage(&Msg,NULL,WM_KEYUP,WM_KEYDOWN,PM_REMOVE|PM_NOYIELD);
			GetCursorPos(&ptNew);
			if (abs(ptOld.x-ptNew.x) > DELTA ||
						abs(ptOld.y-ptNew.y) > DELTA) bRange = FALSE;
			}
		bRunning = FALSE;
		InvalidateRect(hWnd,NULL,TRUE);
		break;

	case WM_RBUTTONDOWN:
		MessageBox(hWnd,
			 "MK14 Emulator\n\n(C) Paul Robson 1998\n\nG to run emulator,Q to stop\nT:Term Z:Abort G:Go M:Mem\n",
			 "About MK14 for Windows",MB_OK | MB_ICONINFORMATION);
		break;

	case WM_PAINT:											// Paint window
		BeginPaint(hWnd,&ps);
		hOldPen = SelectObject(ps.hdc,GetStockObject(WHITE_PEN));
		// https://docs.microsoft.com/en-gb/windows/desktop/gdi/drawing-markers
		MoveToEx(ps.hdc,rcFrame.right,rcFrame.top, (LPPOINT) NULL );
		LineTo(ps.hdc,rcFrame.right,rcFrame.bottom);
		LineTo(ps.hdc,rcFrame.left,rcFrame.bottom);

		SelectObject(ps.hdc,GetStockObject(BLACK_PEN));
		LineTo(ps.hdc,rcFrame.left,rcFrame.top);
		LineTo(ps.hdc,rcFrame.right,rcFrame.top);

		//LEDStatus[0]=0xFF ^ Stat;
		//LEDStatus[1]=0x00 ^ Stat;
		
		for (i = 0;i < DIGITS;i++){
		  RefreshLED(ps.hdc,i,LEDStatus[i],-1);
		}
				
		RefreshStatusLED(ps.hdc,1,( Stat & 4  ),-1);
	    RefreshStatusLED(ps.hdc,2,( Stat & 2  ),-1);
		RefreshStatusLED(ps.hdc,3,( Stat & 1  ),-1);

		SelectObject(ps.hdc,hOldPen);
		EndPaint(hWnd,&ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
		
		
	default:
		lRet = DefWindowProc(hWnd,iMessage,wParam,lParam);
		break;
	}
return(lRet);
}




#define CHANGED(b) ((b & New) != (b & Old))
#define LINE(b,x1,y1,x2,y2) { SelectObject(hDC,b & New ? hUsePen : hOffPen);MoveToEx(hDC,x1,y1,(LPPOINT) NULL );LineTo(hDC,x2,y2); }

static void NEAR PASCAL RefreshLED(HDC hDC,int Led,int New,int Old)
{
RECT rc = rcLED[Led];
HPEN hPen = SelectObject(hDC,hOnPen);
HPEN hUsePen = bRunning ? hOnPen : hPausePen;
int ym = (rc.top+rc.bottom)/2;

if (Old < 0) Old = New ^ 0xFF;

/* only write segment if changed */
/* */
/*
Old = 0x0;
New = 0xff;
*/

if CHANGED(0x01) LINE(0x01,rc.left+4,rc.top,rc.right-4,rc.top);
if CHANGED(0x02) LINE(0x02,rc.right,rc.top+4,rc.right,ym-4);
if CHANGED(0x04) LINE(0x04,rc.right,ym+4,rc.right,rc.bottom-4);
if CHANGED(0x08) LINE(0x08,rc.left+4,rc.bottom,rc.right-4,rc.bottom);
if CHANGED(0x10) LINE(0x10,rc.left,rc.bottom-4,rc.left,ym+4);
if CHANGED(0x20) LINE(0x20,rc.left,rc.top+4,rc.left,ym-4);
if CHANGED(0x40) LINE(0x40,rc.left+4,ym,rc.right-4,ym);
if CHANGED(0x80) LINE(0x80,rc.right+4,rc.bottom,rc.right+4,rc.bottom);

SelectObject(hDC,hPen);
}

static void NEAR PASCAL RefreshStatusLED(HDC hDC,int Led,int New,int Old)
{
RECT rc      = rcLED[Led];
RECT rc2     = rcStatusLED[Led];
HPEN hPen    = SelectObject(hDC,hOnPen);

HPEN hUsePen = bRunning ? hFlagsPen : hPausePen;

if ( Old < 0  ) Old = New ^ 0xFF;

if ( New != 0 ){
  hUsePen = hFlagsPen; 
} else {
  hUsePen = hPausePen;
}  

SelectObject(hDC, hUsePen );
MoveToEx(hDC,rc.left,   rc.bottom+20,(LPPOINT) NULL );
LineTo(  hDC,rc.left+20,rc.bottom+20 );
SelectObject(hDC,hPen);
}




int CONKeyPressed(int Key)
{
int WKey;
if (Key == KEY_BREAK && (GetAsyncKeyState(VK_ESCAPE) & 0x8000)) return(TRUE);
switch(Key)
	{
	case KEY_TERM:	WKey = 'T';break;
	case KEY_GO:	WKey = 'G';break;
	case KEY_MEM:	WKey = 'M';break;
	case KEY_ABORT:WKey = 'Z';break;
	case KEY_BREAK:WKey = 'Q';break;
	case KEY_RESET:WKey = 'R';break;
	default:
			if (Key < 10) WKey = '0'+Key;
			else			  WKey = (Key-10)+'A';
			break;
	}
return((GetAsyncKeyState(WKey) & 0x8000) != 0);
}

void CONDrawLED(int LED,int Pattern)
{
if (hDCWork != NULL)
	{
	Pattern = Pattern & 0xFF;
	RefreshLED(hDCWork,LED,Pattern,LEDStatus[LED]);
	LEDStatus[LED] = Pattern;
	
	
	RefreshStatusLED(hDCWork,1,( Stat & 4  ),-1);
	RefreshStatusLED(hDCWork,2,( Stat & 2  ),-1);
	RefreshStatusLED(hDCWork,3,( Stat & 1  ),-1);

	
	}
}

void CONInitialise(void) { }
void CONTerminate(void)  { }

static long CycCount = 0L;
static DWORD ClockTick = 0L;

void CONSynchronise(long AddCycles)
{
DWORD c;

CycCount = CycCount + AddCycles;

while (CycCount > 60439L)
	{
	CycCount = CycCount - 60439L;
	while (c = GetTickCount(),c < ClockTick) {}
	ClockTick = c + 60;
	}
}
