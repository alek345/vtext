#include <stdio.h>
#include "vtext.h"

void mode_open() {
    clear_window();
            
    draw_status();
    draw_buffer();
            
    ui_begin();
            
    SDL_Color c = {0xff, 0xff, 0xff};
    SDL_Surface *text = TTF_RenderUTF8_Blended(
        font, open_buffer, c
    );
    SDL_Rect dst = {
        50, 30,
        200, 200
    };
    SDL_BlitSurface(text, NULL, window_state.surface, &dst);
    SDL_FreeSurface(text);
            
    if(ui_button(123, 50, 50, 50, 25, "Open")) {
        Buffer *b = buffer_read(open_buffer);
        if(b == NULL) {
            b = buffer_new(open_buffer);
        }
        
        if(b) {
            active_buffer = b;
            set_mode(MODE_INSERT);
        } else {
            set_mode(MODE_OPEN);
        }
        
        
    }
            
    ui_end();
    
    draw_window();
}

void mode_insert() {
    if(active_buffer->dirty) {
        clear_window();
        
        draw_status();
        draw_buffer();
        
        draw_window();
    } else {
        draw_status();
        draw_window();
    }
}

void set_mode(Mode mode) {
    current_mode = mode;
    
    switch(mode) {
        case MODE_INSERT: {
            active_buffer->dirty = true;
        } break;
        
        case MODE_OPEN: {
            open_buffer_ptr = 0;
            int i;
            for(i = 0; i < OPEN_BUFFER_LEN; i++) {
                open_buffer[i] = 0;
            }
            
            ui_state.mouse_x = 0;
            ui_state.mouse_y = 0;
        } break;
    }
}

#ifdef EMSCRIPTEN
void main_loop() {
    switch(current_mode) {
        case MODE_INSERT: {
            mode_insert();
        } break;
        
        case MODE_OPEN: {
            mode_open();
        } break;
    }
    
    update_window();
}
#endif /* EMSCRIPTEN */

int main(int argc, char **argv) {
    
    Buffer *buf = buffer_new("test_file.txt");
//    buffer_insert(buf, "Hello, world!");
//    buf->current_x = 6;
//    buffer_insert(buf, "(am i here?)");
//    buf->current_x = 1;
//    buffer_save(buf);
//    return 0;
    
//    Buffer *buffer = buffer_read("main.c");
//    buffer->path = "output.txt";
/*    
    buffer->nixLines = true;
    buffer->dirty = true;
    buffer_save(buffer);
 */   
    setup_window("vText - 0.0.1", 800, 600);
    setup_ui();
    
    Buffer *hamlet_buffer = buffer_read("hamlet.txt");
  
//    Buffer *big_file = buffer_read("big_file.txt");  
//    set_buffer(big_file);

    set_buffer(hamlet_buffer);
//    set_buffer(buffer);
//    set_buffer(buf);
    
    // Use this and SDL_TEXTINPUT event for actual text input
    SDL_StartTextInput();
    
    current_mode = MODE_INSERT;
    
#ifdef EMSCRIPTEN
#include <emscripten.h>
    emscripten_set_main_loop(main_loop, 60, true);
    return 0;
#endif /* EMSCRIPTEN */
    
    while(window_state.open) {
        //clear_window();
        
        
        switch(current_mode) {
            case MODE_INSERT: {
                mode_insert();
            } break;
            
            case MODE_OPEN: {
                mode_open();
            } break;
        }
        
        update_window();
        
        //draw_window();
        //SDL_WaitEvent(NULL);
        SDL_Delay(16);
        
    }
    
    return 0;
}