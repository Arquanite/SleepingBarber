#include "ulepszenia_terminala.h"

void UstawKolor(int Kolor, int KolorTla){
    #ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Kolor + KolorTla*16);
    #else
    printf("\033[%d;%dm", Kolor, KolorTla+10);
    #endif

}

void UstawKursor(int x, int y){
    #ifdef _WIN32
    COORD c;
    c.X = x;
    c.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
    #else
    printf("\033[%d;%dH", y+1, x+1);
    #endif
}

XY RozmiarKonsoli(){
    XY r;
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    r.x = csbi.srWindow.Bottom+1;
    r.y = csbi.srWindow.Right+1;
    #else
    struct winsize ws;
    ioctl(0, TIOCGWINSZ , &ws);
    r.x = ws.ws_col + 1;
    r.y = ws.ws_row + 1;
    #endif
    return r;
}

void Poczekaj(int milisekundy){
    usleep(milisekundy*1000);
}

void Wyczysc(){
    #ifdef _WIN32
    system("cls");
    #else
    printf("\033[2J");
    #endif
}

void ZmienTryb(bool tryb){
    #ifndef _WIN32
    static struct termios oldt, newt;
    if(tryb){
        tcgetattr( STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    } else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    #endif
}

char Znak(){
    #ifdef _WIN32
    return getch();
    #else
    return getchar();
    #endif
}
