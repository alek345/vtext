#include "vtext.h"
#include "res/icon.c"

void setup_window(char *title, int width, int height) {
    SDL_Init(SDL_INIT_EVERYTHING);
    
    window_state.window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_RESIZABLE
    );
    
    window_state.surface = SDL_GetWindowSurface(window_state.window);
    
    window_state.width = width;
    window_state.height = height;
    window_state.open = true;
    
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceFrom(
        icon.pixel_data,
        icon.width, icon.height,
        32, icon.bytes_per_pixel*icon.width,
        0xFF, 0xFF00, 0xFF0000, 0xFF000000
    );
    SDL_SetWindowIcon(window_state.window, icon_surface);
    SDL_FreeSurface(icon_surface);
    
    TTF_Init();
    font = TTF_OpenFont("fonts/Hack-Regular.ttf", 16);
    fgcolor.r = 0x90; fgcolor.g = 0xB0; fgcolor.b = 0x80;
    bgcolor.r = 0x0C; bgcolor.g = 0x0C; bgcolor.b = 0x0C;
    
    green = SDL_CreateRGBSurface(
        0,
        1, 1,
        32,
        0xFF0000,
        0xFF00,
        0xFF,
        0xFF000000
    );
    ((Uint32*) green->pixels)[0] = 0x7F00FF00;
    SDL_SetSurfaceBlendMode(green, SDL_BLENDMODE_BLEND);
    
    SDL_SetSurfaceBlendMode(window_state.surface, SDL_BLENDMODE_BLEND);
}

void input_insert(SDL_Event e) {
    switch(e.type) {
        case SDL_KEYDOWN: {

            if(
                (e.key.keysym.sym == SDLK_v) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                if(SDL_HasClipboardText() == SDL_TRUE) {
                    char* text = SDL_GetClipboardText();
                    int x = active_buffer->current_x;
                    buffer_insert(active_buffer, text);
                    active_buffer->current_x = x + my_strlen(text);
                    active_buffer->x_we_want = active_buffer->current_x;
                    SDL_free(text);
                    active_buffer->changed = true;
                }
            }
            
            if(
                (e.key.keysym.sym == SDLK_c) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                SDL_SetClipboardText(active_buffer->current_line->text);
            }
            
            if(
                (e.key.keysym.sym == SDLK_a) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                active_buffer->top_line = active_buffer->current_line;
                active_buffer->dirty = true;
            }
            
            if(
                (e.key.keysym.sym == SDLK_d) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                active_buffer->dirty = true;
                
                Line *to_remove = active_buffer->current_line;
                /*if(to_remove->prev == NULL && to_remove->next == NULL) {
                    free(to_remove->text);
                    to_remove->text = NULL;
                    break;
                }*/
                
                Line *prev = to_remove->prev;
                Line *next = to_remove->next;
                
                if(prev) {
                    if(next) prev->next = next;
                    else prev->next = NULL;
                }
                
                if(next) {
                    if(prev) next->prev = prev;
                    else next->prev = NULL;
                }
                
                if(next) {
                    active_buffer->current_line = next;
                } else if (prev) {
                    active_buffer->current_line = prev;
                } else {
                    active_buffer->current_line = NULL;
                }
                active_buffer->current_x = 1;
                active_buffer->x_we_want = 1;
                
                active_buffer->changed = true;
                
                free(to_remove->text);
                free(to_remove);
            }
                
            if(
                (e.key.keysym.sym == SDLK_o) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                set_mode(MODE_OPEN);
            }
            
            if(
                (e.key.keysym.sym == SDLK_s) &&
                (e.key.keysym.mod & KMOD_CTRL)
            ) {
                if(buffer_save(active_buffer) < 0) {
                    printf("Save failed!\n");
                }
            }
                
            if(e.key.keysym.sym == SDLK_TAB) {
                buffer_insert(active_buffer, "    ");
                active_buffer->dirty = true;
                active_buffer->current_x += 4;
                active_buffer->x_we_want = active_buffer->current_x;
            }
            
            if(e.key.keysym.sym == SDLK_DOWN) {
                if (active_buffer->current_line->next) {
                    active_buffer->current_line = active_buffer->current_line->next;
                    active_buffer->dirty = true;
                    int len = my_strlen(active_buffer->current_line->text);
                    if(active_buffer->x_we_want == -1) {
                        active_buffer->current_x = len + 1;
                    } else if(active_buffer->x_we_want > len) {
                        active_buffer->current_x = len + 1;
                    } else {
                        active_buffer->current_x = active_buffer->x_we_want;
                    }
                    
                }
                
                int line = buffer_get_lines_from_top(active_buffer) + 1;
                // FIXME: Hardcoded value
                if((line * 18) >= window_state.height) {
                    if(active_buffer->top_line->next) {
                        active_buffer->top_line = active_buffer->top_line->next;
                    }
                    
                    active_buffer->dirty = true;
                }
            } else if(e.key.keysym.sym == SDLK_UP) {
                int line = buffer_get_lines_from_top(active_buffer);
                
                if(active_buffer->current_line->prev) {
                    active_buffer->current_line = active_buffer->current_line->prev;
                    active_buffer->dirty = true;
                    int len = my_strlen(active_buffer->current_line->text);
                    if(active_buffer->x_we_want == -1) {
                        active_buffer->current_x = len + 1;
                    } else if(active_buffer->x_we_want > len) {
                        active_buffer->current_x = len + 1;
                    } else {
                        active_buffer->current_x = active_buffer->x_we_want;
                    }
                }
                
                if(line == 0) {
                    if(active_buffer->top_line->prev) {
                        active_buffer->top_line = active_buffer->top_line->prev;
                    }
                    
                    active_buffer->dirty = true;
                }
            }
                
            if(e.key.keysym.sym == SDLK_LEFT) {
                if(active_buffer->current_x == 1) {
                    active_buffer->x_we_want = 1;
                } else {
                    active_buffer->x_we_want = active_buffer->current_x - 1;
                }
                active_buffer->current_x = active_buffer->x_we_want;
                active_buffer->dirty = true;
            } else if(e.key.keysym.sym == SDLK_RIGHT) {
                if(active_buffer->current_x > my_strlen(active_buffer->current_line->text)) {
                    active_buffer->x_we_want = 1+my_strlen(active_buffer->current_line->text);
                } else {
                    active_buffer->x_we_want = active_buffer->current_x + 1;
                }
                active_buffer->current_x = active_buffer->x_we_want;
                active_buffer->dirty = true;
            }
                
            if(e.key.keysym.sym == SDLK_HOME) {
                active_buffer->x_we_want = 1;
                active_buffer->current_x = 1;
                active_buffer->dirty = true;
            } else if(e.key.keysym.sym == SDLK_END) {
                int len = my_strlen(active_buffer->current_line->text);
                active_buffer->x_we_want = -1; // -1 means we want the END
                active_buffer->current_x = len+1;
                active_buffer->dirty = true;
            }
            
            if(e.key.keysym.sym == SDLK_RETURN) {
                buffer_return(active_buffer);
            }
            
            if(e.key.keysym.sym == SDLK_BACKSPACE) {
                buffer_backspace(active_buffer);
            }
                
        } break;
        
        case SDL_MOUSEWHEEL: {
            if(e.wheel.y > 0) {
                // Normally up
                if(e.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
                    if(active_buffer->top_line->prev) {
                        active_buffer->top_line = active_buffer->top_line->prev;
                        active_buffer->dirty = true;
                    }
                } else {
                    if(active_buffer->top_line->next) {
                        active_buffer->top_line = active_buffer->top_line->next;
                        active_buffer->dirty = true;
                    }
                }
            } else {
                // Normally down
                if(e.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
                    if(active_buffer->top_line->next) {
                        active_buffer->top_line = active_buffer->top_line->next;
                        active_buffer->dirty = true;
                    }
                } else {
                    if(active_buffer->top_line->prev) {
                        active_buffer->top_line = active_buffer->top_line->prev;
                        active_buffer->dirty = true;
                    }
                }
            }
        } break;
        
        case SDL_TEXTINPUT: {
            if(e.text.text[1] == 0) {
                buffer_insert(active_buffer, e.text.text);
                active_buffer->current_x += my_strlen(e.text.text);
                active_buffer->x_we_want = active_buffer->current_x;
            }
        } break;
    }
}

void input_open(SDL_Event e) {
    switch(e.type) {
        case SDL_TEXTINPUT: {
            if(e.text.text[1] == 0) {
                open_buffer_ptr++;
                open_buffer[open_buffer_ptr-1] = e.text.text[0];
            }
        } break;
        
        case SDL_KEYDOWN: {
            switch(e.key.keysym.sym) {
                case SDLK_BACKSPACE: {
                    if(open_buffer_ptr == 0) break;
                    
                    open_buffer[open_buffer_ptr-1] = 0;
                    open_buffer_ptr--;
                } break;
                
                case SDLK_RETURN: {
                    open_now = true;
                } break;
            }
        } break;
        
        case SDL_MOUSEMOTION: {
            ui_state.mouse_x = e.motion.x;
            ui_state.mouse_y = e.motion.y;
        } break;
        
        case SDL_MOUSEBUTTONDOWN: {
            if(e.button.button == 1) {
                ui_state.mouse_down = true;
            }
        } break;
        
        case SDL_MOUSEBUTTONUP: {
            if(e.button.button == 1) {
                ui_state.mouse_down = false;
            }
        } break;
    }
}

void update_window() {
    SDL_Event e;
    
    while(SDL_PollEvent(&e)) {
        
        switch(current_mode) {
            case MODE_INSERT: {
                input_insert(e);
            } break;
            
            case MODE_OPEN: {
                input_open(e);
            } break;
        }
        
        switch(e.type) {
            case SDL_QUIT: {
                window_state.open = false;
            } break;
            
            case SDL_WINDOWEVENT: {
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        window_state.width = e.window.data1;
                        window_state.height = e.window.data2;
                        active_buffer->dirty = true;
                    } break;
                    
                    case SDL_WINDOWEVENT_EXPOSED: {
                        window_state.surface = SDL_GetWindowSurface(window_state.window);
                        active_buffer->dirty = true;
                    } break;
                }
            } break;
        }
        
    }
}

void clear_window() {
    SDL_FillRect(
        window_state.surface,
        NULL,
//        SDL_MapRGB(window_state.surface->format, 0, 0, 0)
        SDL_MapRGB(window_state.surface->format, bgcolor.r, bgcolor.g, bgcolor.b)
    );
}

void draw_window() {
    SDL_UpdateWindowSurface(window_state.window);
}

void setup_ui() {
    ui_state.mouse_x = 0;
    ui_state.mouse_y = 0;
    ui_state.mouse_down = false;
    
    ui_state.hot_item = 0;
    ui_state.active_item = 0;
}

bool aabb(int x, int y, int w, int h) {
    if(
        ui_state.mouse_x < x ||
        ui_state.mouse_x >= x + w ||
        ui_state.mouse_y < y ||
        ui_state.mouse_y >= y + h
    ) {
        return false;
    }
    
    return true;
}

void draw_rect(int x, int y, int w, int h, int color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(
        window_state.surface,
        &rect,
        SDL_MapRGB(window_state.surface->format, r, g, b)
    );
}

void ui_begin() {
    ui_state.hot_item = 0;
}

void ui_end() {
    if(ui_state.mouse_down == false) {
        ui_state.active_item = 0;
    } else {
        if(ui_state.active_item == 0) {
            ui_state.active_item = -1;
        }
    }
}

bool ui_button(int id, int x, int y, int w, int h, char *text) {
    
    if(aabb(x, y, w, h)) {
        ui_state.hot_item = id;
        if(ui_state.active_item == 0 && ui_state.mouse_down) {
            ui_state.active_item = id;
        }
    }
    
    if(ui_state.hot_item == id) {
        if(ui_state.active_item == id) {
            draw_rect(x, y, w, h, 0xFF0000);
        } else {
            draw_rect(x, y, w, h, 0x00FF00);
        }
    } else {
        if(ui_state.active_item == id) {
            draw_rect(x, y, w, h, 0x7F0000);
        } else {
            draw_rect(x, y, w, h, 0x0000FF);
        }
    }
    
    int t_w = 0;
    int t_h = 0;
    TTF_SizeUTF8(font, text, &t_w, &t_h);
    SDL_Rect dst = {
        x + (w/2) - (t_w / 2),
        y + (h/2) - (t_h / 2),
        w, h
    };
    SDL_Color color = {0, 0, 0};
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(
        font, text, color
    );
    SDL_BlitSurface(text_surface, NULL, window_state.surface, &dst);
    SDL_FreeSurface(text_surface);
    
    if(ui_state.mouse_down == false && 
       ui_state.hot_item == id &&
       ui_state.active_item == id
    ) {
        return true;
    }
    
    return false;
}

void draw_status() {
    Buffer *buffer = active_buffer;
    
    char text[1024];
    sprintf(
        text, 
        "%s - L%d C%d - %s",
        buffer->name, 
        buffer_get_current_line_number(buffer),
        buffer->current_x,
        buffer->nixLines ? "unix" : "dos"
    );
    if(buffer->new_file) {
        int offset = my_strlen(text);
        sprintf(
            text+offset,
            " NEW"
        );
    }
    
    SDL_Color c = {0, 0, 0};
    if(buffer->changed) {
        c.r = 179;
        c.g = 58;
        c.b = 58;
    }
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, c);
    
    draw_rect(0, 0, window_state.width, 20, 0x888888);
    
    SDL_Rect dst = { 4, 0, window_state.width, 18};
    SDL_BlitSurface(text_surface, NULL, window_state.surface, &dst);
    SDL_FreeSurface(text_surface);
}

void draw_buffer() {
    
    Buffer *buffer = active_buffer;
    
    Line *l = buffer->top_line;
    
    SDL_Rect dst = { 0, 20, window_state.width, 16 };
    
    while(l != NULL) {
        if(l == buffer->current_line) {
            draw_rect(
                0, dst.y,
                window_state.width, 18,
                0x000044
            );
        }
        
        SDL_Surface *text = TTF_RenderUTF8_Blended(
            font, l->text, fgcolor
        );
        SDL_BlitSurface(text, NULL, window_state.surface, &dst);
        
        if(l == buffer->current_line) {
            int char_width = 0;
            TTF_GlyphMetrics(font, 'w', NULL, NULL, NULL, NULL, &char_width);
            SDL_Rect gr = { 
                char_width * (buffer->current_x-1), 
                dst.y,
                char_width,
                18
            };
            /*SDL_FillRect(
                window_state.surface,
                &gr,
                SDL_MapRGBA(
                    window_state.surface->format, 
                    0, 0xee, 0, 0x7f
                )
            );*/
            
            
            SDL_BlitScaled(green, NULL, window_state.surface, &gr);
        }
        
        dst.y += 18;
        SDL_FreeSurface(text);
        
        if(dst.y >= window_state.height) break;
        
        l = l->next;
    }
    
    active_buffer->dirty = false;
    
}