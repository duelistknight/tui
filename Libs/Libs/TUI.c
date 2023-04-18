// dashboard.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include"TUI.h"

//========Widgets===========================


TUI_Widget* getTUI_byId(TUI_Window* pWin, int id)
{
    for (int i = 0; i < pWin->pAttrib->WidgetCounts; ++i)
    {
        if (pWin->pAttrib->pWidget[i].id == id)
        {
            return pWin->pAttrib->pWidget + i;
        }
        if (pWin->pAttrib->pWidget[i].flags & TUI_WINDOW_WIDGET)
        {
            TUI_Widget* ptemp;
            ptemp = getTUI_byId((TUI_Window*)(pWin->pAttrib->pWidget + i), id);
            if (ptemp)
                return ptemp;
        }
    }
    return NULL;
}

TUI_Widget* getTUI_byId(int id)
{
    return getTUI_byId(&MainWindow, id);
}

void TUI_clr_widget(TUI_Widget* pWidget)
{
    int x = pWidget->scrPos.X;
    int y = pWidget->scrPos.Y;

    printf("\033[m\033[%d;%dm", pWidget->bg + 10, pWidget->fg);
    char* buffer = (char*)calloc(sizeof(char), pWidget->RestricedSize.X + 1);
    memset(buffer, ' ', pWidget->RestricedSize.X);
    buffer[pWidget->RestricedSize.X] = '\0';
    for (int i = 0; i < pWidget->RestricedSize.Y; i++)
    {
        _gotoxy(x, y + i);
        printf("%s", buffer);
        
    }
    free(buffer);
    printf("\033[m");
}

void Display_Widget(void* arg)
{
    TUI_Widget* pWidget = (TUI_Widget*)arg;

    if (!(pWidget->flags & TUI_DISPLAY_WIDGET))
        return;
    short len = strlen(pWidget->name);
    int x = pWidget->scrPos.X;
    int y = pWidget->scrPos.Y;
    bool flag = true;

    TUI_clr_widget(pWidget);
    gotoxy(pWidget->scrPos);
    for (int i = 0; i < pWidget->RestricedSize.Y; i++)
    {
        _gotoxy(x, y + i);
        printf("\033[%d;%dm", pWidget->bg + 10, pWidget->fg);
        for (int j = 0; j < pWidget->RestricedSize.X; j++)
        {
            if (flag && pWidget->name[i * pWidget->RestricedSize.Y + j] != '\0')
                printf("%c", pWidget->name[i * pWidget->RestricedSize.Y + j]);
            else
            {
                flag = false;
                printf(" ");
            }
        }
        printf("\033[m");
    }
}

bool insertTUI_Widget(TUI_Window* pWin, TUI_Widget* pWidget)
{
    if (pWin->pAttrib->MaxWidgetCounts == pWin->pAttrib->WidgetCounts)
    {
        TUI_Widget* ptemp = (TUI_Widget*)realloc(pWin->pAttrib->pWidget, sizeof(TUI_Widget) * (pWin->pAttrib->MaxWidgetCounts + 10));
        if (ptemp)
        {
            pWin->pAttrib->pWidget = ptemp;
            memset(ptemp + pWin->pAttrib->MaxWidgetCounts, 0, sizeof(TUI_Widget) * 10);
            pWin->pAttrib->MaxWidgetCounts += 10;
        }
        else
            return false;
    }
    if (!pWidget->display)
        pWidget->display = Display_Widget;

    pWin->pAttrib->pWidget[pWin->pAttrib->WidgetCounts] = *pWidget;
    pWin->pAttrib->pWidget[pWin->pAttrib->WidgetCounts].pParentWindow = pWin;
    pWin->pAttrib->WidgetCounts++;
}

void deleteWidget(TUI_Window* pWin, TUI_Widget* pWidget)
{
    for (int i = 0; i < pWin->pAttrib->WidgetCounts; ++i)
    {
        TUI_Widget* pWidget2 = pWin->pAttrib->pWidget + i;
        if (pWidget2 == pWidget)
        {
            //delete
            if (pWidget2->pArg && pWidget2->flags & TUI_DYNAMIC_ARG_WIDGET)
                free(pWidget2->pArg);
            memmove(pWin->pAttrib->pWidget + i, pWin->pAttrib->pWidget + i + 1, sizeof(TUI_Widget) * (pWin->pAttrib->WidgetCounts - i));
            pWin->pAttrib->WidgetCounts--;
            memset(pWin->pAttrib->pWidget + pWin->pAttrib->WidgetCounts, 0, sizeof(TUI_Widget));
            break;
        }
    }
}

//========Windows==========================

void Display_Window(void* arg)
{

    TUI_clr_widget(((TUI_Widget*)arg));
    TUI_Window* pWin = (TUI_Window*)arg;
    COORD StartPoint = { 1,1 };
    COORD end;
    end.X = pWin->scrPos.X + pWin->RestricedSize.X - 1;
    end.Y = pWin->scrPos.Y + pWin->RestricedSize.Y - 1;

    //todo:set on screen buffer
    for (int j = pWin->scrPos.Y; j <= end.Y; ++j)
    {
        for (int k = pWin->scrPos.X; k <= end.X; ++k)
        {
            consoleMap[(j - 1) * consoleSize.X + k] = (TUI_Widget*)pWin;
        }
    }

    if (pWin->flags & TUI_SCROLL_WINDOW)
    {
        pWin->RestricedSize.X--;
        pWin->RestricedSize.Y--;
        //display scroll
        int newScrolly = pWin->RestricedSize.Y / 2 + 1;
        int newScrollx = (pWin->RestricedSize.X) / 2 + 1;
        _gotoxy(pWin->scrPos.X , pWin->scrPos.Y);
        /*printf("r");
        for (int i = 0; i < pWin->RestricedSize.X - 1; ++i)
        {
            printf("-");
        }*/
        //for y
        for (int i = 0; i <= pWin->RestricedSize.Y ; ++i)
        {
            _gotoxy(pWin->scrPos.X, pWin->scrPos.Y + i + 1);
            //printf("|");
            _gotoxy(pWin->scrPos.X + pWin->RestricedSize.X + 1, pWin->scrPos.Y + i);
            if (i <= newScrolly)
            {
                printf("\033[43m \033[0m");
            }
            else
                printf("\033[45m \033[0m");
        }

        _gotoxy(pWin->scrPos.X , pWin->scrPos.Y + pWin->RestricedSize.Y + 1);
        for (int i = 0; i <= pWin->RestricedSize.X; ++i)
        {
            if (i <= newScrollx)
            {
                printf("\033[43m \033[0m");
            }
            else
                printf("\033[45m \033[0m");
        }

        StartPoint.X = pWin->pAttrib->Scroll.X;
        StartPoint.Y = pWin->pAttrib->Scroll.Y;

    }
    for (int i = 0; i < pWin->pAttrib->WidgetCounts; ++i)
    {
        TUI_Widget* pWidget = pWin->pAttrib->pWidget + i;
        if (pWin->pAttrib->ScrollRange.Y < pWidget->pos.Y)
            pWin->pAttrib->ScrollRange.Y = pWidget->pos.Y;
        if (pWin->pAttrib->ScrollRange.X < pWidget->pos.X)
            pWin->pAttrib->ScrollRange.X = pWidget->pos.X;

        int posx = pWin->scrPos.X + pWidget->pos.X - StartPoint.X;
        int posy = pWin->scrPos.Y + pWidget->pos.Y - StartPoint.Y;
        if (posx < pWin->scrPos.X || posy < pWin->scrPos.Y || posx>end.X || posy>end.Y)
        {
            pWidget->scrPos = { 0,0 };
            continue;
        }

        pWidget->scrPos.X = posx;
        pWidget->scrPos.Y = posy;

        //todo:set on screen buffer
        for (int j = pWidget->scrPos.Y; j < end.Y && j < pWidget->size.Y + pWidget->scrPos.Y; ++j)
        {
            if (pWidget->scrPos.Y + pWidget->size.Y > end.Y)
                pWidget->RestricedSize.Y = end.Y - pWidget->scrPos.Y;
            else
                pWidget->RestricedSize.Y = pWidget->size.Y;

            
            //pWidget->RestricedSize.Y = j - pWin->scrPos.Y + 1;
            for (int k = pWidget->scrPos.X; k < end.X && k < pWidget->size.X + pWidget->scrPos.X; ++k)
            {
                if (pWidget->scrPos.X + pWidget->size.X > end.X)
                    pWidget->RestricedSize.X = end.X - pWidget->scrPos.X;
                else
                    pWidget->RestricedSize.X = pWidget->size.X;
                //pWidget->RestricedSize.X = k - pWin->scrPos.X + 1;
                consoleMap[(j - 1) * consoleSize.X + k - 1] = pWidget;
            }
        }

        //=========================
        if (pWidget->display)
            pWidget->display(pWidget);

        //display scroll
    }

    if (pWin->flags & TUI_SCROLL_WINDOW)
    {
        
        pWin->RestricedSize.X++;
        pWin->RestricedSize.Y++;
    }
}

bool insertTUI_Window(TUI_Window* pWin, TUI_Window* pWidget)
{
    if (pWin->pAttrib->MaxWidgetCounts == pWin->pAttrib->WidgetCounts)
    {
        TUI_Widget* ptemp = (TUI_Widget*)realloc(pWin->pAttrib->pWidget, sizeof(TUI_Widget) * (pWin->pAttrib->MaxWidgetCounts + 10));
        if (ptemp)
        {
            pWin->pAttrib->pWidget = ptemp;
            memset(ptemp + pWin->pAttrib->MaxWidgetCounts, 0, sizeof(TUI_Widget) * 10);
            pWin->pAttrib->MaxWidgetCounts += 10;
        }
        else
            return false;
    }
    if (!pWidget->display)
        pWidget->display = Display_Window;
    
    pWidget->flags |= TUI_WINDOW_WIDGET;
    if (!pWidget->pAttrib)
    {
        pWidget->pAttrib = (TUI_Window_Attrib*)calloc(sizeof(TUI_Window_Attrib), 1);
        pWidget->flags |= TUI_DYNAMIC_ARG_WIDGET;
    }
    pWin->pAttrib->pWidget[pWin->pAttrib->WidgetCounts] = *((TUI_Widget*)pWidget);
    pWin->pAttrib->pWidget[pWin->pAttrib->WidgetCounts].pParentWindow = pWin;
    pWin->pAttrib->WidgetCounts++;
}

//void deleteWidget(TUI_Window* pWin, TUI_Widget* pWidget)
//{
//    for (int i = 0; i < pWin->pAttrib->WidgetCounts; ++i)
//    {
//        TUI_Widget* pWidget2 = pWin->pAttrib->pWidget + i;
//        if (pWidget2 == pWidget)
//        {
//            //delete
//            if (pWidget2->pArg && pWidget2->flags & TUI_DYNAMIC_ARG_WIDGET)
//                free(pWidget2->pArg);
//            memmove(pWin->pAttrib->pWidget + i, pWin->pAttrib->pWidget + i + 1, sizeof(TUI_Widget) * (pWin->pAttrib->WidgetCounts - i));
//            pWin->pAttrib->WidgetCounts--;
//            memset(pWin->pAttrib->pWidget + pWin->pAttrib->WidgetCounts, 0, sizeof(TUI_Widget));
//            break;
//        }
//    }
//}

void clearWidgetFromWindow(TUI_Window* pWin)
{
    for (int i = 0; i < pWin->pAttrib->WidgetCounts; i++)
    {
        TUI_Widget* pWidget = pWin->pAttrib->pWidget + i;
        if (pWidget->flags & TUI_DYNAMIC_ARG_WIDGET)
        {
            free(pWidget->pArg);
        }
    }
    free(pWin->pAttrib->pWidget);
    void* ptemp = pWin->pAttrib->pWinMeta;
    memset(pWin->pAttrib, 0, sizeof(TUI_Window_Attrib));
    pWin->pAttrib->pWinMeta = ptemp;
    
}

void update_TUI()
{
    memset(&consoleSize, 0, sizeof(COORD));

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    MainWindow.size = consoleSize;
    MainWindow.scrPos = { 1,1 };
    MainWindow.RestricedSize = consoleSize;

    Display_Window(&MainWindow);


}

void wheelScroll(int direction) //1-right,2-left,3-up,4-down
{
    //cursorPosition;
    for (int i = cursorPosition.Y; i < consoleSize.Y; ++i)
    {
        if (consoleMap[i * consoleSize.X + cursorPosition.X])
        {
            TUI_Window* pWin = (TUI_Window*)consoleMap[i * consoleSize.X + cursorPosition.X];
            if (pWin->flags & TUI_WINDOW_WIDGET && pWin->flags & TUI_SCROLL_WINDOW)
            {
                switch (direction)
                {
                case 1:
                    pWin->pAttrib->Scroll.X = (pWin->pAttrib->Scroll.X + 2 > pWin->pAttrib->ScrollRange.X - pWin->RestricedSize.X) ? pWin->pAttrib->ScrollRange.X - pWin->RestricedSize.X : pWin->pAttrib->Scroll.Y + 2;
                    pWin->display(pWin);
                    break;
                case 2:
                    pWin->pAttrib->Scroll.X = (pWin->pAttrib->Scroll.X - 2 < 0) ? 0 : pWin->pAttrib->Scroll.X - 2;
                    pWin->display(pWin);
                    break;
                case 3:
                    pWin->pAttrib->Scroll.Y = (pWin->pAttrib->Scroll.Y - 2 < 0) ? 0 : pWin->pAttrib->Scroll.Y - 2;
                    pWin->display(pWin);
                    break;
                case 4:
                    pWin->pAttrib->Scroll.Y = (pWin->pAttrib->Scroll.Y + 2 > pWin->pAttrib->ScrollRange.Y - pWin->RestricedSize.Y) ? pWin->pAttrib->ScrollRange.Y - pWin->RestricedSize.Y : pWin->pAttrib->Scroll.Y + 2;
                    pWin->display(pWin);

                }
                return;
            }

        }
    }
}


//=================================== [Input Event Handle:Start] ==========================================================================

bool mouseClick(int x, int y, short click)
{
    
    TUI_Widget* pWidget = consoleMap[(y - 1) * consoleSize.X + x - 1];


    if (pWidget->pos.X || pWidget->pos.Y)
    {
        if (pWidget->flags & TUI_WINDOW_WIDGET && pWidget->flags & TUI_SCROLL_WINDOW)
        {
            TUI_Window* pWin = (TUI_Window*)pWidget;
            if (x >= pWin->scrPos.X + pWin->RestricedSize.X - 1) //vertical scroll
            {
                if (y <= pWin->scrPos.Y + (pWin->RestricedSize.Y - 1)/2) //go up
                { 
                    pWin->pAttrib->Scroll.Y = (pWin->pAttrib->Scroll.Y >= 2) ? pWin->pAttrib->Scroll.Y - 2 : 0;

                }
                else //go down
                {
                    
                    pWin->pAttrib->Scroll.Y += 2;
                    if (pWin->pAttrib->Scroll.Y > (pWin->pAttrib->ScrollRange.Y - pWin->RestricedSize.Y))
                    {
                        pWin->pAttrib->Scroll.Y = (pWin->pAttrib->ScrollRange.Y - pWin->RestricedSize.Y);
                        pWin->pAttrib->Scroll.Y = (pWin->pAttrib->Scroll.Y < 0) ? 0 : pWin->pAttrib->Scroll.Y;
                    }
                    //(pWin->pAttrib->Scroll.Y <= ) ? pWin->pAttrib->Scroll.Y - 2 : 0;

                }
            }
            if (y == pWin->scrPos.Y + pWin->RestricedSize.Y + 1) //horizontal scroll
            {
                if (y <= pWin->scrPos.X + (pWin->RestricedSize.X - 1) / 2) //go left
                {
                    pWin->pAttrib->Scroll.X = (pWin->pAttrib->Scroll.X >= 2) ? pWin->pAttrib->Scroll.X - 2 : 0;

                }
                else //go right
                {

                    pWin->pAttrib->Scroll.X += 2;
                    if (pWin->pAttrib->Scroll.X > (pWin->pAttrib->ScrollRange.X - pWin->RestricedSize.X))
                    {
                        pWin->pAttrib->Scroll.X = (pWin->pAttrib->ScrollRange.X - pWin->RestricedSize.X);
                        pWin->pAttrib->Scroll.X = (pWin->pAttrib->Scroll.X < 0) ? 0 : pWin->pAttrib->Scroll.X;
                    }
                    //(pWin->pAttrib->Scroll.Y <= ) ? pWin->pAttrib->Scroll.Y - 2 : 0;

                }
            }
            //int ylim = pWin->pAttrib->ScrollRange.Y - pWin->RestricedSize.Y;
            /*pWin->RestricedSize.Y++;
            pWin->RestricedSize.X++;
            */
            pWin->display(pWin);
            return false;
        }

        if (pWidget->func)
        {
            if (pWidget->func(pWidget,click))
            {
                if (pWidget->display)
                    pWidget->display(pWidget);
            }
            else
            {
                update_TUI();
            }
            return true;
        }
        
    }
    return false;
}

VOID ErrorExit(const char* lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);
    // Restore input mode on exit.
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), oldInMode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), oldOutMode);

    ExitProcess(0);
}

VOID KeyEventProc(KEY_EVENT_RECORD ker)
{
    INPUT_RECORD irInBuf;
    DWORD cNumRead;
   
    switch (ker.wVirtualKeyCode)
    {
    case VK_BACK:
        if (back_space)
            back_space(NULL, 0);
        break;
    case VK_LEFT:
        {
        TUI_Widget* pWidget;
        pWidget = consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X];
        cursorPosition.X = pWidget->scrPos.X;
        
            for (; cursorPosition.Y >=1; cursorPosition.Y--)
            {
                for (; cursorPosition.X >= 1; cursorPosition.X--)
                {
                    if (consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X] && pWidget != consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X])
                    {
                        gotoxy(cursorPosition);
                        return;
                    }
                }
                cursorPosition.X = consoleSize.X;
            }
        }
        break;
    case VK_RIGHT:
        {
        TUI_Widget* pWidget;
        pWidget = consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X];
        cursorPosition.X = pWidget->scrPos.X;

            for (; cursorPosition.Y <= consoleSize.Y; cursorPosition.Y++)
            {
                for (; cursorPosition.X < consoleSize.X; cursorPosition.X++)
                {
                    if (consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X] && pWidget != consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X])
                    {
                        gotoxy(cursorPosition);
                        return;
                    }
                }
                cursorPosition.X = 1;
            }
        }
        break;
    case VK_UP:
        {
        gotoxy(cursorPosition);
            cursorPosition.Y = (cursorPosition.Y <= 1) ? 1 : cursorPosition.Y - 1;
            printf("\033[A");
        }
        break;
    case VK_DOWN:
        {
            TUI_Widget* pWidget;
            pWidget = consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X];
            cursorPosition.Y = pWidget->scrPos.Y;

            for (; cursorPosition.X < consoleSize.X; cursorPosition.X++)
            {
                for (; cursorPosition.Y <= consoleSize.Y; cursorPosition.Y++)
                {
                    if (consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X] && pWidget != consoleMap[(consoleSize.Y - 1) * cursorPosition.Y + consoleSize.X])
                    {
                        gotoxy(cursorPosition);
                        return;
                    }
                }
                cursorPosition.Y = pWidget->scrPos.Y;
            }
        }
        break;
    case VK_RETURN:
        {
            mouseClick(cursorPosition.X, cursorPosition.Y, 1);
        
        enterLoop:
            GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &cNumRead);
            if (cNumRead > 0)
            {
                PeekConsoleInputA(GetStdHandle(STD_INPUT_HANDLE), &irInBuf, 1, &cNumRead);
                if (irInBuf.EventType == KEY_EVENT && irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
                {
                    ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &irInBuf, 1, &cNumRead);
                    goto enterLoop;
                }
            }
        }
        break;
    }
    if (ker.dwControlKeyState == LEFT_CTRL_PRESSED && ker.uChar.AsciiChar=='A')
        printf("Key event: ");

    /*if (ker.bKeyDown)
        printf("key pressed\n");
    else printf("key released\n");*/
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
    
    static time_t prevTime;
    cursorPosition.X = mer.dwMousePosition.X + 1;
    cursorPosition.Y = mer.dwMousePosition.Y + 1;
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
    gotoxy(cursorPosition);// .X, mer.dwMousePosition.Y);
    switch (mer.dwEventFlags)
    {
    case 0:

        if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
        {
            time_t t = time(NULL);
            if (t - prevTime > 1)
                mouseClick(mer.dwMousePosition.X + 1, mer.dwMousePosition.Y + 1, 1);
            prevTime = time(NULL);
            //printf("left button press \n");
        }
        else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
        {
            mouseClick(mer.dwMousePosition.X + 1, mer.dwMousePosition.Y + 1, 3);

            //printf("right button press \n");
        }
        else
        {
            //mouseClick(mer.dwMousePosition.X + 1, mer.dwMousePosition.Y + 1, 0);

            //printf("button press\n");
        }
        break;
    case DOUBLE_CLICK:
        mouseClick(mer.dwMousePosition.X + 1, mer.dwMousePosition.Y + 1, 2);
        //printf("double click\n");
        break;
    case MOUSE_HWHEELED:
        printf("horizontal mouse wheel\n");
        break;
    case MOUSE_MOVED:
        //printf("mouse moved\n");
        break;
    case MOUSE_WHEELED:
        //printf("vertical mouse wheel\n");
        if (mer.dwButtonState > 800000)
            wheelScroll(3);
        else
            wheelScroll(4);
        break;

    }
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
    printf("Resize event\n");
    printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}

//===================================  [Input Event Handle:End]  ==========================================================================

void editValue(char* str)
{
    int size = strlen(str);
    int pos = size;
    char ch;
loop:
    ch = _getch();
    if (ch == -32)
    {
        ch = _getch();
        switch (ch)
        {
        case 'H': //up arraow
            break;
        case 'M': //right arrow
            if ((pos < size))
            {
                pos++;
                printf("\033[C");
            }
            else
                pos = size;
            break;
        case 'P': //down arrow
            break;
        case 'K': //left arrow
            pos = (pos <= 0) ? 0 : pos - 1;
            printf("\033[D");
            break;
        case 'S': //delete
            break;
        case 'G': //home
            break;
        case 'O': //end
            break;
        }
    }
    else
    {
        switch (ch)
        {
        case '\r':
            {
                INPUT_RECORD irInBuf;
                DWORD cNumRead;

            enterLoop:
                GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &cNumRead);
                if (cNumRead > 0)
                {
                    PeekConsoleInputA(GetStdHandle(STD_INPUT_HANDLE), &irInBuf, 1, &cNumRead);
                    if (irInBuf.EventType == KEY_EVENT && irInBuf.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
                    {
                        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &irInBuf, 1, &cNumRead);
                        goto enterLoop;
                    }
                }
                return;
            }
                    break;
        case '\b':
            memmove(str + pos - 1, str + pos, size - pos + 1);
            printf("\b\033[s%s \033[u", str + pos - 1);
            pos--;
            size--;
            break;
        default:
            if (pos < size)  // in middle of string
            {
                memmove(str + pos + 1, str + pos, size - pos + 1);
            }
            else
            {
                str[pos + 1] = '\0';
            }
            str[pos] = ch;
            printf("%c\033[s%s\033[u", ch, str + pos + 1);
            pos++;
            size++;
        }
    }
    goto loop;

}

void init_TUI()
{
    memset(&consoleSize, 0, sizeof(COORD));

    //Console size Calculation

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    //Enable virtual terminal feature
    GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &oldOutMode))
        ErrorExit("GetConsoleMode");
    DWORD newOutMode = oldOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), newOutMode))
        ErrorExit("SetConsoleMode");



    consoleMap = (TUI_Widget**)calloc(sizeof(TUI_Widget*), consoleSize.X * consoleSize.Y);

}

void TUI_resize()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    TUI_Widget** ptemp = (TUI_Widget**)realloc(consoleMap, sizeof(TUI_Widget*) * consoleSize.X * consoleSize.Y);
    if (ptemp)
    {
        consoleMap = ptemp;
    }
    update_TUI();
}

void deinit_TUI()
{
    /*_gotoxy(1, 1);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    char* spc = (char*)calloc(sizeof(char), consoleSize.X * consoleSize.Y);
    if (spc)
    {
        memset(spc, ' ', consoleSize.X * consoleSize.Y);
        spc[consoleSize.X * consoleSize.Y - 1] = '\0';
        printf("\033[m%s", spc);
    }*/
    fflush(stdout);
    printf("\033[2J");
    _gotoxy(1, 1);
    oldInMode = 0;
    oldOutMode = 0;
    consoleSize = { 0,0 };
    //MainWindow;
    for (int i = 0; i < MainWindow.pAttrib->WidgetCounts; i++)
    {
        TUI_Widget* pWidget = MainWindow.pAttrib->pWidget + i;
        if (pWidget->flags & TUI_WINDOW_WIDGET)
        {
            clearWidgetFromWindow((TUI_Window*)pWidget);
        }
        if (pWidget->flags & TUI_DYNAMIC_ARG_WIDGET)
            free(pWidget->pArg);
    }
    free(MainWindow.pAttrib);
    memset(&MainWindow, 0, sizeof(TUI_Window));
    isDone = false;
    if (consoleMap)
        consoleMap = NULL;
    cursorPosition = { 0,0 };
    buffer = NULL;
    back_space=NULL;

    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), oldOutMode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), oldInMode);
}

int EventLoop(VOID)
{
    //Enable capture of input events
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &oldInMode))
        ErrorExit("GetConsoleMode");
    DWORD fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (!SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), fdwMode))
        ErrorExit("SetConsoleMode");

    // Loop to read and handle the next 100 input events.
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];
    isDone = false;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    update_TUI();

    while (!isDone)
    {
        // Wait for the events.

        if (!ReadConsoleInput(
            hStdin,      // input buffer handle
            irInBuf,     // buffer to read into
            128,         // size of read buffer
            &cNumRead)) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch (irInBuf[i].EventType)
            {
            case KEY_EVENT: // keyboard input
                KeyEventProc(irInBuf[i].Event.KeyEvent);
                break;

            case MOUSE_EVENT: // mouse input
                MouseEventProc(irInBuf[i].Event.MouseEvent);
                break;

            case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                //ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent);
                break;

            case FOCUS_EVENT:  // disregard focus events

            case MENU_EVENT:   // disregard menu events
                break;

            default:
                ErrorExit("Unknown event type");
                break;
            }
        }
    }

    //deinit_TUI();

    return 0;
}

typedef struct
{
    char asciiKey;
    void(*func)(void*);
    void* pArgs;
}TUI_Ctrl_Event;

typedef struct
{
    TUI_Ctrl_Event pCtrl[26];
    int filledEvent;
}TUI_Ctrl_Header;

void addCtrlEvent(char ch, void(*func)(void*), void* pArgs)
{
    
}