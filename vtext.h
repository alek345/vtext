#ifndef VTEXT_H
#define VTEXT_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif /* EMSCRIPTEN */

///////////// Buffers and Lines ///////////////

typedef struct Line Line;
struct Line {
    Line *prev;
    Line *next;
    char *text;
};

typedef struct {
    Line *first_line;
    Line *top_line;
    Line *current_line;
    
    int current_x;
    int x_we_want;
    
    char *name;
    char *path;
    
    bool nixLines;
    bool new_file;
    bool changed;
    
    bool dirty;
} Buffer;

/// Duplicates a zero-terminated string
/// 
/// This has to exists because 
/// standards are dumb
char* my_strdup(const char *str);

int my_strlen(const char *str);

/// Creates a new buffer.
/// 
/// If name is NULL the buffer 
/// will get the name 'unnamed'
Buffer* buffer_new(char *name);

/// Reads a buffer from file
///
/// If path is NULL the function
/// returns NULL-
///
/// Returns NULL on failure
Buffer* buffer_read(char *path);

/// Saves a buffer to file
///
/// If the buffers path is NULL
/// the files is saved at './name_of_buffer'
///
/// Returns -1 on failure
int buffer_save(Buffer *buffer);

/// Set the buffer which is currently being edited
void set_buffer(Buffer *buffer);

void buffer_insert(Buffer *buffer, const char *text);

int buffer_get_current_line_number(Buffer *buffer);

int buffer_get_lines_from_top(Buffer *buffer);

void buffer_return(Buffer *buffer);

void buffer_backspace(Buffer *buffer);

///////////////// Window and UI //////////////////

typedef struct {
    SDL_Window *window;
    SDL_Surface *surface;
    
    int width;
    int height;
    bool open;
    bool dirty;
} WindowState;

typedef struct {
    int mouse_x;
    int mouse_y;
    bool mouse_down;
    
    int hot_item;
    int active_item;
} UIState;

void setup_window(char *title, int width, int height);
void setup_ui();

void update_window();
void clear_window();
void draw_window();

void draw_status();

void ui_begin();
void ui_end();

bool ui_button(int id, int x, int y, int w, int h, char *text);

void draw_buffer();

/////////////// Global variables /////////////7///

typedef enum {
    MODE_INSERT,
    MODE_OPEN,
} Mode;

Mode current_mode;

void set_mode(Mode mode);

#define OPEN_BUFFER_LEN 1024
char open_buffer[OPEN_BUFFER_LEN];
int open_buffer_ptr;
bool open_now;

TTF_Font *font;
SDL_Color fgcolor;
SDL_Color bgcolor;
SDL_Surface *green;

WindowState window_state;
UIState ui_state;

Buffer *active_buffer;

#endif /* VTEXT_H */