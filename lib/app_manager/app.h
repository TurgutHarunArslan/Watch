#ifndef APP_H
#define APP_H


/*
init -> create new screen and set it to a variable
deinit -> remove the screen and delete the data 
loop -> loop the app update screen
*/

typedef struct App {
    void (*init)(struct App* self);
    void (*deinit)(struct App* self);
    void (*loop)(struct App* self);
} App;

#endif
