#pragma once
#include <ncurses.h>
#include <signal.h>
#include <sys/stat.h>
class Win {
public:
    unsigned int startx,starty;
    unsigned int height,width;
    WINDOW* ptr;
    Win(int sx,int sy,int h,int w);
    Win(const Win&other);
    Win& operator=(const Win&other);
    ~Win();
    void render();
};
