#include "../include/blek.h"

volatile sig_atomic_t resized = 0;
extern "C" void handle_resize(int){
    resized = 1;
}

blek::blek(): cursor(0), top_line(0) , should_close(false) {
    items.push_back(b_item(".."));
    items.push_back(b_item("."));
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr,TRUE);
    keypad(stdscr,TRUE);
    getmaxyx(stdscr,LINES,COLS);
    wtimeout(stdscr,100);
    start_color();

    struct sigaction sa;
    sa.sa_handler = handle_resize;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, nullptr);

    init_pair(2,COLOR_BLUE,COLOR_BLACK);
    init_pair(1,COLOR_GREEN,COLOR_BLACK);

    this->browser_win = new Win(0,0,LINES,COLS/2);
    this->command = new Win(COLS/2,0,3,COLS/2);
    this->content = new Win(COLS/2,3,LINES-3,COLS/2);

    current_command = "$ ";

    current_win = Browser; 
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
void blek::handle_command_input(int c){
    if(c == '\n'){
        // execute the command 
        // /* soon */
        // clear the command / maybe later we print the result
        if(current_command.length() > 2) current_command.resize(2);
        wclear(this->command->ptr);
        wrefresh(this->command->ptr);
    }else if(c == KEY_BACKSPACE){
        if(current_command.length() <= 2) return;
        current_command.pop_back();
        wclear(this->command->ptr);
        wrefresh(this->command->ptr);
    }else if(isascii(static_cast<unsigned char>(c))) {
        current_command += (char)c;
    }
}
void blek::handle_browser_input(int c){
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
void blek::input(){
    int c = getch();
    switch(current_win) {
        case Browser:
            this->handle_browser_input(c);
            break;
        case Command:
            this->handle_command_input(c);
            break;
        case Content:
            break;
    }
    this->switch_win(c);
}
void blek::interactive(){
    this->populate_browser();
    while(!this->should_close) {
        this->input();
        switch(current_win) {
            case Browser:
            this->render_browser_win();
                break;
            case Command:
            this->render_command();
                break;
            case Content:
            this->render_content();
                break;
        }       
        this->listen_for_winch();
    }
}
blek::~blek(){
    delwin(this->browser_win->ptr);
    delete browser_win;
    delwin(this->command->ptr);
    delete command;
    delwin(this->content->ptr);
    delete content;
    endwin();  
}

void blek::listen_for_winch(void){
    if(resized){
        resized = 0;
        endwin();
        clear();
        wclear(browser_win->ptr);
        wclear(command->ptr);
        wclear(content->ptr);
        refresh();
        wrefresh(browser_win->ptr);
        wrefresh(content->ptr);
        wrefresh(command->ptr);
        getmaxyx(stdscr,LINES,COLS);
        // dont judge please
        delete this->browser_win;
        delete this->content;
        delete this->command;
        this->browser_win = new Win(0,0,LINES,COLS/2);
        this->command = new Win(COLS/2,0,3,COLS/2);
        this->content = new Win(COLS/2,3,LINES-3,COLS/2);
        if(this->cursor > browser_win->height-3)
            this->cursor = browser_win->height-3;
    }
}

void blek::render_command(){
    mvwprintw(command->ptr,1,1,"%s",this->current_command.c_str()); 
    wrefresh(command->ptr);
    box(this->command->ptr,0,0);
    move(this->cursor+1,1);
}
void blek::render_content(){
    wrefresh(content->ptr);
    box(this->content->ptr,0,0);
    move(this->cursor+1,1);
}

void blek::switch_win(int c){
    switch(c){
        case ESC:
            if(current_win != Browser) this->current_win = Browser;
            break;
        case ':':
            if(current_win == Browser) this->current_win = Command;
            break;
        case 'c':
            if(current_win == Browser) this->current_win = Content;
            break;
        default:
            break;
    }
}
