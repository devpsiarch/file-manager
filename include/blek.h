#pragma once
#include "./Win.h"
#include "./b_item.h"
#include <vector>

#define ESC 27

typedef enum {
    Browser,
    Command,
    Content
}active_window;

class blek {
private:
    size_t cursor;
    size_t top_line;
    std::vector<b_item> items;
    bool should_close;
    Win* browser_win;
    Win* command;
    Win* content;
    std::string current_command;
    active_window current_win;
public:
    unsigned int LINES;
    unsigned int COLS;
    blek();
    void populate_browser();
    
    void render_browser_win();
    void render_command();
    void render_content();

    void input();
    void handle_browser_input(int c);
    void handle_command_input(int c);
    void handle_content_input(int c);
    
    void interactive();
    void listen_for_winch(void);
    void switch_win(int c);
    ~blek();
};
