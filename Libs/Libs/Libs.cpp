// Libs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include"TUI.c"

#include <windows.h>  
#include <tchar.h>   
#include <stdio.h>  
#include <strsafe.h>  
#include <shlwapi.h>
#pragma comment(lib, "User32.lib")    
#pragma comment(lib, "Shlwapi.lib")    



//File selector tool

typedef struct
{
    char type;
    char ffName[MAX_PATH];
}FFEntry;
#define FILESELECTOR_FILE       0x01
#define FILESELECTOR_FOLDER     0x02
typedef struct
{
    FFEntry* pFFentry;
    int count;
    int maxCount;
}FFHeader;

void load_new_Directory(const char*, char*, FFHeader*);
TUI_Window* pSubWindow = NULL;

void getAllFilesAndFolders(const char* path,FFHeader* pFFHeader)
{
    WIN32_FIND_DATAA ffd;
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    char szDir[MAX_PATH];

    sprintf(szDir, "%s\\*", path);
    hFind = FindFirstFileA(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        return;
    }

    // List all the files in the directory with some info about them.       

    do
    {
        if (ffd.cFileName[0] == '.')
        {
            continue;
        }

        if (pFFHeader->maxCount == pFFHeader->count)
        {
            FFEntry* ptemp = (FFEntry*)realloc(pFFHeader->pFFentry, sizeof(FFEntry) * (pFFHeader->maxCount + 50));
            if (ptemp)
            {
                pFFHeader->pFFentry = ptemp;
                memset(ptemp + pFFHeader->maxCount, 0, sizeof(FFEntry) * 50);
                pFFHeader->maxCount += 50;
            }
        }
        
        sprintf(pFFHeader->pFFentry[pFFHeader->count].ffName, "%s\\%s", path, ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            pFFHeader->pFFentry[pFFHeader->count].type = FILESELECTOR_FOLDER;
        }
        else
        {
            pFFHeader->pFFentry[pFFHeader->count].type = FILESELECTOR_FILE;
        }
        
        pFFHeader->count++;

        //if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        //    _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
        //}
        //else
        //{
        //    filesize.LowPart = ffd.nFileSizeLow;
        //    filesize.HighPart = ffd.nFileSizeHigh;
        //
        //    _tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
        //}
    } while (FindNextFileA(hFind, &ffd) != 0);

    dwError = GetLastError();

    if (dwError != ERROR_NO_MORE_FILES)
    {
    }

    FindClose(hFind);

}

typedef struct
{
    FFHeader* pFFSelected;
    char* teminatingStr;
    FFEntry sFFEntry;
}FF_Params;

void addFileAndFolder(FFEntry sFFEntry, FFHeader* pFFHeaderSelected)
{
    for (int i = 0; i < pFFHeaderSelected->count; i++)
    {
        if (_stricmp(pFFHeaderSelected->pFFentry[i].ffName, sFFEntry.ffName) == 0)
        {
            //delete entry
            pFFHeaderSelected->pFFentry[i] = pFFHeaderSelected->pFFentry[pFFHeaderSelected->count - 1];
            pFFHeaderSelected->count--;
            memset(pFFHeaderSelected->pFFentry + pFFHeaderSelected->count , 0, sizeof(FFEntry));
            return;
        }
    }
    if (pFFHeaderSelected->count >= pFFHeaderSelected->maxCount)
    {
        FFEntry* temp = (FFEntry*)realloc(pFFHeaderSelected->pFFentry, sizeof(FFEntry) * (pFFHeaderSelected->maxCount + 10));
        if (temp)
        {
            memset(temp + pFFHeaderSelected->maxCount, 0, sizeof(FFEntry) * 10);
            pFFHeaderSelected->pFFentry = temp;
            pFFHeaderSelected->maxCount += 10;
        }
        else
        {
            //allocation error
            return;
        }
    }
    pFFHeaderSelected->pFFentry[pFFHeaderSelected->count] = sFFEntry;
    pFFHeaderSelected->count++;
}

bool clickEvent(void* pArg, int click)
{
    TUI_Widget* pWidget = (TUI_Widget*)pArg;

    FF_Params& sParam = *((FF_Params*)pWidget->pArg);

    int count = sParam.pFFSelected->count;
    addFileAndFolder(sParam.sFFEntry, sParam.pFFSelected);
    if (count > sParam.pFFSelected->count)
    {
        //reduction
        pWidget->bg = ANSI_BRIGHT_BLACK;
    }
    else
        pWidget->bg = ANSI_YELLOW;
    if (sParam.sFFEntry.type == FILESELECTOR_FOLDER && click == 2)
    {
        load_new_Directory(sParam.sFFEntry.ffName, sParam.teminatingStr, sParam.pFFSelected);
        //update_TUI();
        return false;
    }
    return true;
}


void load_new_Directory(const char* pathInput,char*terminatingStr,FFHeader *pFFHeaderSelected)
{
    char path[MAX_PATH];
    memset(path, 0, MAX_PATH);
    strcpy(path, pathInput);


    int len = 0;
    if (terminatingStr)
        len = strlen(terminatingStr);

    int len2 = strlen(path) + 1;
    int len3 = 0;

    FFHeader sFFHeader;
    memset(&sFFHeader, 0, sizeof(FFHeader));
    getAllFilesAndFolders(path,&sFFHeader);

    clearWidgetFromWindow(pSubWindow);
    pSubWindow->flags |= TUI_SCROLL_WINDOW | TUI_DISPLAY_WIDGET;
    
    int dIdx = 0;

    for (int i = 0; i < sFFHeader.count; ++i)
    {
        int len4 = strlen(sFFHeader.pFFentry[i].ffName);
        if (len4 > len3)
            len3 = len4;
        if (sFFHeader.pFFentry[i].type == FILESELECTOR_FOLDER)
        {
            FFEntry temp = sFFHeader.pFFentry[i];
            memmove(sFFHeader.pFFentry + dIdx + 1, sFFHeader.pFFentry + dIdx, sizeof(FFEntry) * (i - dIdx));
            sFFHeader.pFFentry[dIdx] = temp;
            dIdx++;
        }
    }

    TUI_Widget widget;
    int x = 1;
    int y = 1;
    len3 -= len2;
    for (int i = 0; i < sFFHeader.count; ++i)
    {   
        memset(&widget, 0, sizeof(TUI_Widget));
        widget.pos.X = x;
        widget.pos.Y = y;
        widget.bg = ANSI_BRIGHT_BLACK;
        
        for (int j = 0; j < pFFHeaderSelected->count; ++j)
        {
            if (_stricmp(pFFHeaderSelected->pFFentry[i].ffName, sFFHeader.pFFentry[i].ffName) == 0)
            {
                widget.bg = ANSI_YELLOW;
                break;
            }
        }

        if (sFFHeader.pFFentry[i].type == FILESELECTOR_FOLDER)
            widget.fg = ANSI_BLUE;
        else
            widget.fg = ANSI_BRIGHT_WHITE;
        widget.id = 23;
        //widget.display = Display_Window;
        strcpy(widget.name, sFFHeader.pFFentry[i].ffName + len2);
        widget.pArg = calloc(sizeof(FF_Params),1);
        ((FF_Params*)widget.pArg)->sFFEntry = sFFHeader.pFFentry[i];
        ((FF_Params*)widget.pArg)->pFFSelected = pFFHeaderSelected;
        ((FF_Params*)widget.pArg)->teminatingStr = terminatingStr;
        widget.func = clickEvent;

        widget.size.X = len3;
        widget.size.Y = 1;

        widget.flags = TUI_DISPLAY_WIDGET | TUI_DYNAMIC_ARG_WIDGET;
        insertTUI_Widget(pSubWindow, &widget);

        x += len3 ;
        if (x > pSubWindow->size.X - 3)
        {
            x = 1;
            y++;
        }
    }
    free(sFFHeader.pFFentry);
    sFFHeader.pFFentry = NULL;

    TUI_Widget* pPath=getTUI_byId(2);
    if (pPath)
    {
        strcpy(pPath->name, path);
        if (pPath->display)
            pPath->display(pPath);
    }
    TUI_clr_widget((TUI_Widget*)pSubWindow);
    Display_Window(pSubWindow);
    
}

bool editPath(void* arg, int a)  //discard a
{
    TUI_Widget* pWidget = (TUI_Widget*)arg;

    char tempName[MAX_PATH];
    strcpy(tempName, pWidget->name);

    gotoxy(pWidget->scrPos);
    printf("\033[%d;%dm%s", pWidget->bg + 10, pWidget->fg, pWidget->name);
    editValue(pWidget->name);
    printf("\033[0m");

    //todo:verify validity if path
    if (PathFileExistsA(pWidget->name))
    {
        if (pWidget->name[strlen(pWidget->name) - 1] == '\\')
            pWidget->name[strlen(pWidget->name) - 1] = '\0';
        load_new_Directory(pWidget->name, ((FF_Params*)pSubWindow->pAttrib->pWinMeta)->teminatingStr, ((FF_Params*)pSubWindow->pAttrib->pWinMeta)->pFFSelected);
        return true;
    }
    else
    {
        strcpy(pWidget->name, tempName);
        gotoxy(pWidget->scrPos);
        printf("\033[%d;%dm%s\033[m", pWidget->bg + 10, pWidget->fg, pWidget->name);
    }

    return false;

}

bool backSpace(void* arg,int a)
{
    TUI_Widget* pWidget = getTUI_byId(2);
    if (pWidget)
    {
        int lastSlash = 0;
        for (int i = 0; pWidget->name[i] != '\0'; i++)
        {
            if (pWidget->name[i] == '\\')
            {
                lastSlash = i ;
            }
        }
        if (lastSlash)
        {
            pWidget->name[lastSlash] = '\0';
            load_new_Directory(pWidget->name, ((FF_Params*)pSubWindow->pAttrib->pWinMeta)->teminatingStr, ((FF_Params*)pSubWindow->pAttrib->pWinMeta)->pFFSelected);
        }

    }
    return false;
}

bool load(void* arg, int a)
{
    isDone = true;
    return true;
}

int main()
{
    init_TUI();
    memset(&MainWindow, 0, sizeof(TUI_Window));
    MainWindow.pAttrib = (TUI_Window_Attrib*)calloc(sizeof(TUI_Window_Attrib), 1);
    MainWindow.display = Display_Window;
    back_space = backSpace;

    FFHeader FFHeaderSelected;
    memset(&FFHeaderSelected, 0, sizeof(FFHeader));
    
    TUI_Widget widget;
    memset(&widget, 0, sizeof(TUI_Widget));
    widget.pos = { 1,1 };
    widget.bg = ANSI_BLACK;
    widget.fg = ANSI_BRIGHT_WHITE;
    widget.id = 1;
    strcpy(widget.name, " PATH ");
    widget.size = { 7,1 };
    widget.flags = TUI_DISPLAY_WIDGET;
    insertTUI_Widget(&MainWindow, &widget);

    memset(&widget, 0, sizeof(TUI_Widget));
    widget.pos = { 9,1 };
    widget.bg = ANSI_BRIGHT_CYAN;
    widget.fg = ANSI_BLACK;
    widget.id = 2;
    widget.func = editPath;
    strcpy(widget.name, " C:");
    buffer = widget.name;
    widget.size = { 100,1 };
    widget.flags = TUI_DISPLAY_WIDGET;
    insertTUI_Widget(&MainWindow, &widget);


    memset(&widget, 0, sizeof(TUI_Widget));
    widget.pos.X = (consoleSize.X) / 2 - 8;
    widget.pos.Y = 3;
    widget.fg = ANSI_BLACK;
    widget.bg = ANSI_BRIGHT_YELLOW;
    widget.id = 4;
    widget.func = backSpace;
    strcpy(widget.name, " BACK ");
    buffer = widget.name;
    widget.size = { 6,1 };
    widget.flags = TUI_DISPLAY_WIDGET;
    insertTUI_Widget(&MainWindow, &widget);

    memset(&widget, 0, sizeof(TUI_Widget));
    widget.pos.X = (consoleSize.X) / 2 + 1;
    widget.pos.Y = 3;
    widget.fg = ANSI_BLACK;
    widget.bg = ANSI_BRIGHT_YELLOW;
    widget.id = 4;
    widget.func = load;
    strcpy(widget.name, " LOAD ");
    buffer = widget.name;
    widget.size = { 6,1 };
    widget.flags = TUI_DISPLAY_WIDGET;
    insertTUI_Widget(&MainWindow, &widget);

    memset(&widget, 0, sizeof(TUI_Widget));
    widget.pos = { 1,5 };
    widget.bg = ANSI_BRIGHT_BLACK;
    widget.fg = ANSI_BRIGHT_WHITE;
    widget.id = 3;
    widget.pArg = (TUI_Window_Attrib*)calloc(sizeof(TUI_Window_Attrib), 1);
    FF_Params sParam;
    memset(&sParam, 0, sizeof(FF_Params));
    sParam.pFFSelected = &FFHeaderSelected;
    sParam.teminatingStr = NULL;
    ((TUI_Window_Attrib*)widget.pArg)->pWinMeta = (void*)&sParam ;
    widget.display = Display_Window;
    strcpy(widget.name, "Files & Folders");
    widget.size.X = consoleSize.X - 2;
    widget.size.Y = consoleSize.Y - 5;
    widget.flags = TUI_DISPLAY_WIDGET | TUI_WINDOW_WIDGET | TUI_DYNAMIC_ARG_WIDGET;
    insertTUI_Window(&MainWindow, (TUI_Window*) & widget);

    pSubWindow = (TUI_Window*)getTUI_byId(&MainWindow, 3);

    load_new_Directory("C:", NULL, &FFHeaderSelected);

    EventLoop();

    deinit_TUI();

    return 0;
}
