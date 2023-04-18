#define _CRT_SECURE_NO_WARNINGS

#ifndef __WINDOW_TUI_ENV__
#define __WINDOW_TUI_ENV__

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>

#define _gotoxy(x,y) printf("\033[%d;%dH",y,x)
#define gotoxy(a) _gotoxy(a.X,a.Y)

//================== [Colours] ===========================

#define ANSI_BLACK			        30
#define ANSI_RED			        31
#define ANSI_GREEN			        32
#define ANSI_YELLOW			        33
#define ANSI_BLUE			        34
#define ANSI_MAGENTA		        35
#define ANSI_CYAN			        36
#define ANSI_WHITE			        37
#define ANSI_BRIGHT_BLACK	        90
#define ANSI_BRIGHT_RED		        91
#define ANSI_BRIGHT_GREEN	        92
#define ANSI_BRIGHT_YELLOW	        93
#define ANSI_BRIGHT_BLUE	        94
#define ANSI_BRIGHT_MAGENTA	        95
#define ANSI_BRIGHT_CYAN	        96
#define ANSI_BRIGHT_WHITE	        97

//================== [End Colour] ========================

//================== [Window Flag] =======================

#define TUI_WINDOW_WIDGET       0x01
#define TUI_DISPLAY_WIDGET      0x02
#define TUI_ACTIVE_WIDGET       0x04
#define TUI_BLINK_WIDGET        0x08
#define TUI_DYNAMIC_ARG_WIDGET  0x10
#define TUI_SCROLL_WINDOW       0x20
#define TUI_OUTREF_WIDGET       0x40
#define TUI_OUTREF_WINDOW       0x40

//================== [End Flag] =======================

typedef struct
{
    void (*display)(void*);
    bool (*func)(void*,int);  //true if TUI_Widget needs to redisplay and false for refresh complete display
    void* pArg;
    void* pParentWindow;
    COORD scrPos;
    COORD pos;
    COORD size;
    COORD RestricedSize;
    unsigned short bg;
    unsigned short fg;
    unsigned short id;
    char name[MAX_PATH];
    BYTE flags;

}TUI_Widget;

typedef struct
{
    TUI_Widget* pWidget;
    void* pWinMeta;  // this will not get free automatically
    unsigned short WidgetCounts;
    unsigned short MaxWidgetCounts;
    COORD Scroll;//current scroll positon
    COORD ScrollRange;//devided into 4 digit of x then y i.e. in format xxxxyyyy
}TUI_Window_Attrib;

typedef struct
{
    void (*display)(void*);
    bool (*func)(TUI_Widget*,int);
    TUI_Window_Attrib* pAttrib;
    void* pParentWindow;
    COORD scrPos;
    COORD pos;
    COORD size;
    COORD RestricedSize;
    unsigned short bg;
    unsigned short fg;
    unsigned short id;
    char name[MAX_PATH];
    BYTE flags;
}TUI_Window;

typedef struct
{
    char* buffer;
    int size;
    int position;
}SCANVAL;

static DWORD oldInMode;
static DWORD oldOutMode;
static COORD consoleSize;
static TUI_Window MainWindow;
bool isDone = false;
TUI_Widget** consoleMap = NULL;
void Display_Window(void*);
COORD cursorPosition;
char* buffer = NULL;
bool(*back_space)(void*, int);

TUI_Widget* getTUI_byId(TUI_Window* pWin, int id);

void clr_widget(TUI_Widget*);

void Display_Widget(void*);

bool insertTUI_Widget(TUI_Window*, TUI_Widget*);

void deleteTUI_Widget(TUI_Window*, TUI_Widget*);

//========Windows==========================

void Display_Window(void*);

bool insertTUI_Window(TUI_Window*, TUI_Window*);

void update_TUI();

void clearWidgetFromWindow(TUI_Window*);

//=================================== [Input Event Handle:Start] ==========================================================================

bool mouseClick(int , int , short);

VOID ErrorExit(const char*);

VOID KeyEventProc(KEY_EVENT_RECORD);

VOID MouseEventProc(MOUSE_EVENT_RECORD);

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

//===================================  [Input Event Handle:End]  ==========================================================================

void init_TUI();

void TUI_resize();

void deinit_TUI();

int EventLoop(VOID);

void editValue(char*);



#endif