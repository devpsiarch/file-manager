#include "include/Win.h"
#include "include/b_item.h"
#include "include/blek.h"
// petition to make the main screen only a miniWin that displays shit 
// in that manner we display the main screen that only listens to the 
// keyboard, and when the triger got switch to some "feature windows" is 
// clicked we switch to that and hanlde acrodingly , but at the same time, 
// the other feature windows will repond to the results of the input given
// to the target feature window.


int main(void){
    blek b = blek(); 
    b.interactive();
    return 0;
}
