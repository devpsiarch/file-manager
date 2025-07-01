#pragma once
#include "./Win.h"
#include "./b_item.h"
#include <vector>

class blek {
private:
    inline static volatile sig_atomic_t resized = 0;
    size_t cursor;
    size_t top_line;
    std::vector<b_item> items;
    static void handle_resize(int i){
        (void)i;
        resized = 1;
    }
    bool should_close;
public:
    unsigned int LINES;
    unsigned int COLS;
    blek();
    void populate();
    void render();
    void input();
    void interactive();
    ~blek();
};
