#include <iostream>
#include <vector>
#include <cstring>
#include <filesystem>
#include <ncurses.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

struct b_item {
    std::string name;
    std::filesystem::file_type type; 
    b_item(const char*name){
        std::filesystem::path p(name);
        std::error_code ec;
        std::filesystem::file_status st = std::filesystem::status(p,ec);
        if(ec) {
            std::cout << "Error getting status of " << p << '\n';
            exit(1);
        }
        this->type = st.type();
        this->name = name;
    };
};

class MiniWin {
private:
    int startx,starty;
    int height,width;
public:
    WINDOW* ptr;
    MiniWin(int sx,int sy,int h,int w){
        this->startx = sx;
        this->starty = sy;
        this->height = h;
        this->width = w;
        this->ptr = newwin(h,w,sy,sx);
        wtimeout(this->ptr,100);
    }
    MiniWin(const MiniWin&other){
        this->startx = other.startx;
        this->starty = other.starty;
        this->height = other.height;
        this->width  = other.width;
        this->ptr = newwin(this->height,this->width,this->starty,this->startx);          
    }
    MiniWin& operator=(const MiniWin&other){
        if(this == &other) return *this;
        delwin(ptr);
        this->startx = other.startx;
        this->starty = other.starty;
        this->height = other.height;
        this->width  = other.width;
        this->ptr = newwin(this->height,this->width,this->starty,this->startx);          
        return *this;
    }
    ~MiniWin() {delwin(this->ptr);}
    void render(){
        box(this->ptr,0,0);
        wrefresh(this->ptr);
    }
};

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
    blek(): cursor(0), top_line(0) , should_close(false) {
        items.push_back(b_item(".."));
        items.push_back(b_item("."));
        initscr();
        cbreak();
        nodelay(stdscr,TRUE);
        keypad(stdscr,TRUE);
        getmaxyx(stdscr,LINES,COLS);
        wtimeout(stdscr,100);
        start_color();

        init_pair(2,COLOR_BLUE,COLOR_BLACK);
        init_pair(1,COLOR_GREEN,COLOR_BLACK);

        signal(SIGWINCH,handle_resize);

    }
    void populate(){
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
    void render(){
        for(unsigned int i = this->top_line ; i < this->top_line+LINES && i < this->items.size() ;i++){
            switch(items[i].type){
                case std::filesystem::file_type::regular:
                    attron(COLOR_PAIR(1));
                    mvprintw(i-this->top_line,0,"%s",items[i].name.c_str());
                    attroff(COLOR_PAIR(1));
                    break;
                case std::filesystem::file_type::directory:
                    attron(COLOR_PAIR(2));
                    mvprintw(i-this->top_line,0,"%s/",items[i].name.c_str());
                    attroff(COLOR_PAIR(2));
                    break;
                default:
                    break;
            }
        }
        move(this->cursor,0);
    }
    void input(){
        int c = getch();
        switch(c){
            case KEY_DOWN:
                if(this->top_line >= this->items.size()-this->cursor) break;
                if(this->cursor < items.size()-1){
                    if(this->cursor >= LINES){
                        clear();
                        this->top_line++;
                    }else this->cursor++;
                }
                break;
            case KEY_UP:
                if(this->cursor == 0 && this->top_line > 0){
                    clear();
                    this->top_line--;
                    break;
                }
                if(this->cursor > 0) this->cursor--; 
                break;
            case '\n':{
                // we populate again
                // execute cd command , 
                // populate
                if(this->items[cursor].type != std::filesystem::file_type::directory) break;
                chdir(items[cursor].name.c_str());
                if(this->items.size() > 2){
                    this->items.erase(this->items.begin()+2,this->items.end());
                }
                this->populate();
                clear();
                break;
                }
            case KEY_F(1):
                this->should_close = true;
                break;
            default:
                break;
        }
    }
    void interactive(){
        this->populate();
        while(!this->should_close) {
            this->render();
            this->input();
        }
    }
    ~blek(){
        endwin();    
    }
};

int main(void){
    blek b = blek(); 
    b.interactive();
    return 0;
}
