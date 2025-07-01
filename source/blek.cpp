#include "../include/blek.h"
blek::blek(): cursor(0), top_line(0) , should_close(false) {
    items.push_back(b_item(".."));
    items.push_back(b_item("."));
    initscr();
    cbreak();
    nodelay(stdscr,TRUE);
    keypad(stdscr,TRUE);
    getmaxyx(stdscr,LINES,COLS);
    wtimeout(stdscr,100);
    start_color();

    signal(SIGWINCH,handle_resize);

    init_pair(2,COLOR_BLUE,COLOR_BLACK);
    init_pair(1,COLOR_GREEN,COLOR_BLACK);

    this->browser_win = new Win(0,0,LINES,COLS/2);
}
void blek::populate_browser(){
    this->cursor = 0;
    this->top_line = 0;
    FILE* pipe = popen("/bin/ls","r");
    if(pipe == NULL){
        std::cout << "Error creating pipe.\n";
        exit(1);
    }
    char buffer[1024];
    while(fgets(buffer,sizeof(buffer),pipe) != NULL){
        buffer[strlen(buffer)-1] = '\0';
        items.push_back(b_item(buffer));
    }
    pclose(pipe);
}
void blek::render_browser_win(){
    for(unsigned int i = this->top_line ; i < this->top_line+this->browser_win->height-2 && i < this->items.size() ;i++){
        switch(items[i].type){
            case std::filesystem::file_type::regular:
                wattron(browser_win->ptr,COLOR_PAIR(1));
                mvwprintw(browser_win->ptr,i-this->top_line+1,1,"%s",items[i].name.c_str());
                wattroff(browser_win->ptr,COLOR_PAIR(1));
                break;
            case std::filesystem::file_type::directory:
                wattron(browser_win->ptr,COLOR_PAIR(2));
                mvwprintw(browser_win->ptr,i-this->top_line+1,1,"%s/",items[i].name.c_str());
                wattroff(browser_win->ptr,COLOR_PAIR(2));
                break;
            default:
                break;
        }
    }
    wrefresh(browser_win->ptr);
    box(browser_win->ptr,0,0);
    move(this->cursor+1,1);
}
void blek::input(){
    int c = getch();
    switch(c){
        case KEY_DOWN:
            if(this->top_line >= this->items.size()-(1)-this->cursor) break;
            if(this->cursor < items.size()-(1)){
                if(this->cursor >= (size_t)this->browser_win->height-(3)){
                    wclear(this->browser_win->ptr);
                    this->top_line++;
                }else this->cursor++;
            }
            break;
        case KEY_UP:
            if(this->cursor == (0) && this->top_line > (0)){
                wclear(this->browser_win->ptr);
                this->top_line--;
                break;
            }
            if(this->cursor > (0)) this->cursor--; 
            break;
        case '\n':{
            // we populate again
            // execute cd command , 
            // populate
            size_t target = this->top_line + this->cursor;
            if(this->items[target].type != std::filesystem::file_type::directory) break;
            chdir(items[target].name.c_str());
            if(this->items.size() > 2){
                this->items.erase(this->items.begin()+2,this->items.end());
            }
            this->populate_browser();
            wclear(this->browser_win->ptr);
            break;
            }
        case KEY_F(1):
            this->should_close = true;
            break;
        default:
            break;
    }
}
void blek::interactive(){
    this->populate_browser();
    while(!this->should_close) {
        this->render_browser_win();
        this->input();
    }
}
blek::~blek(){
    endwin();    
}
