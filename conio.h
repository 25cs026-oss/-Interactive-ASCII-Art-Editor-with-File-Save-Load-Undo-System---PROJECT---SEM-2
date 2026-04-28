// conio.h (custom)
#ifndef CONIO_H
#define CONIO_H

#include <stdio.h>
#include <stdlib.h>

void clrscr() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int getch() {
    int ch;
    system("stty raw");
    ch = getchar();
    system("stty cooked");
    return ch;
}

#endif
