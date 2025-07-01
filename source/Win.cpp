#include "../include/Win.h"
Win::Win(int sx,int sy,int h,int w){
    this->startx = sx;
    this->starty = sy;
    this->height = h;
    this->width = w;
    this->ptr = newwin(h,w,sy,sx);
    wtimeout(this->ptr,100);
}
Win::Win(const Win&other){
    this->startx = other.startx;
    this->starty = other.starty;
    this->height = other.height;
    this->width  = other.width;
    this->ptr = newwin(this->height,this->width,this->starty,this->startx);          
}
Win& Win::operator=(const Win&other){
    if(this == &other) return *this;
    delwin(ptr);
    this->startx = other.startx;
    this->starty = other.starty;
    this->height = other.height;
    this->width  = other.width;
    this->ptr = newwin(this->height,this->width,this->starty,this->startx);          
    return *this;
}
Win::~Win() {delwin(this->ptr);}
void Win::render(){
    box(this->ptr,0,0);
    wrefresh(this->ptr);
}
