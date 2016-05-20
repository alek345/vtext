#include "vtext.h"

Buffer* buffer_new(char *name) {
    Buffer *buffer = (Buffer*) malloc(sizeof(Buffer));
    
    Line *l = (Line*) malloc(sizeof(Line));
    l->text = NULL;
    l->prev = NULL;
    l->next = NULL;
    
    buffer->first_line = l;
    buffer->top_line = l;
    buffer->current_line = l;
    
    buffer->current_x = 1;
    buffer->x_we_want = 1;
    
    if(name == NULL) {
        buffer->name = "unnamed";
    }else {
        buffer->name = my_strdup(name);
    }
    buffer->path = NULL;
    
    buffer->nixLines = true;
    buffer->dirty = true;
    
    return buffer;
}

Buffer* buffer_read(char *path) {
    if(path == NULL) return NULL;
    
    FILE *f = fopen(path, "rb");
    if(f == NULL) {
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    rewind(f);
    char *data = (char*) malloc(sizeof(char)*(len+1));
    
    int read = fread(data, sizeof(char), len, f);
    if(read != len) {
        fclose(f);
        return NULL;
    }
    data[len] = 0;
    fclose(f);
    
    Buffer *buffer = (Buffer*) malloc(sizeof(Buffer));
    buffer->path = my_strdup(path);
    buffer->name = my_strdup(path);
    // /\ FIXME: Should be the last part of the path. ex: 
    // "/somewhere/cool/safe.txt" -> "safe.txt"
    buffer->current_x = 1;
    buffer->x_we_want = 1;
    buffer->dirty = false;
    
    
    Line *line = NULL;
    char *ptr = data;
    bool nixLines = true;
    bool endingFound = false;
    
    while(*ptr) {
        // On the first line decide wheter or not 
        // the line endings are dos or unix. 
        // Save this info in the Buffer so that
        // we know what endings to use when writing
        
        char *start = ptr;
        int len = 0;
        
        if(endingFound) {
            
            if(nixLines) {
                
                while(*ptr) {
                    if(*ptr == '\n') break;
                    
                    len++;
                    ptr++;
                }
                if(*ptr != 0) ptr++;
                
                Line *l = (Line*) malloc(sizeof(Line));
                l->prev = line;
                l->next = NULL;
                line->next = l;
            
                char *text = (char*) malloc(sizeof(char)*(len+1));
                int i;
                for(i = 0; i < len; i++) {
                    text[i] = start[i];
                }
                text[len] = 0;
                l->text = text;
            
                line = l;
                
            } else {
                
                while(*ptr) {
                    if(*ptr == '\r') break;
                    
                    len++;
                    ptr++;
                }
                if(ptr != 0) ptr += 2;
                
                Line *l = (Line*) malloc(sizeof(Line));
                l->prev = line;
                l->next = NULL;
                line->next = l;
            
                char *text = (char*) malloc(sizeof(char)*(len+1));
                int i;
                for(i = 0; i < len; i++) {
                    text[i] = start[i];
                }
                text[len] = 0;
                l->text = text;
            
                line = l;
            }
            
        } else {
            while(*ptr) {
                if(*ptr == '\n') break;
                if(*ptr == '\r') break;
                
                len++;
                ptr++;
            }
            
            if(*ptr == '\r') {
                nixLines = false;
                ptr += 2;
            } else if(*ptr == '\n'){
                nixLines = true;
                ptr++;
            } else {
                // Default to nix line endings if we
                // never find a newline
                nixLines = true;
            }
            
            endingFound = true;
            
            Line *l = (Line*) malloc(sizeof(Line));
            l->prev = NULL;
            l->next = NULL;
            
            char *text = (char*) malloc(sizeof(char)*(len+1));
            int i;
            for(i = 0; i < len; i++) {
                text[i] = start[i];
            }
            text[len] = 0;
            l->text = text;
            
            line = l;
            buffer->first_line = l;
            buffer->top_line = l;
            buffer->current_line = l;
        }
    }
    
    buffer->nixLines = nixLines;
    
    return buffer;
}

int buffer_save(Buffer *buffer) {    
    if(buffer->path == NULL) {
        buffer->path = buffer->name;
    }
    
    FILE *f = fopen(buffer->path, "wb");
    if(f == NULL) {
        return -1;
    }
    
    if(buffer->first_line == NULL) {
        fclose(f);
        return 0;
    }
    
    Line *line = buffer->first_line;
    while(line) {
        int len = my_strlen(line->text);
        
        if(len != 0) {
            int wrote = fwrite(line->text, sizeof(char), len, f);
            
            if(wrote != len) {
                fclose(f);
                return -1;
            }
            
        
            if(line->next != NULL) {
                if(buffer->nixLines) {
                    fputc('\n', f);
                } else {
                    fputc('\r', f);
                    fputc('\n', f);
                }
            }
        } else {
            if(line->next != NULL) {
                if(buffer->nixLines) {
                    fputc('\n', f);
                } else {
                    fputc('\r', f);
                    fputc('\n', f);
                }
            }
        }
        
        line = line->next;
    }
    
    fflush(f);
    fclose(f);
    
    return 0;
}

void set_buffer(Buffer *buffer) {
    active_buffer = buffer;
    active_buffer->dirty = true;
}

void buffer_insert(Buffer *buffer, const char *text) {
    
    if(buffer->current_line == NULL) {
        buffer->current_line = (Line*) malloc(sizeof(Line));
        buffer->current_line->prev = NULL;
        buffer->current_line->next = NULL;
        buffer->current_line->text = NULL;
        buffer->first_line = buffer->top_line = buffer->current_line;
    }
    
    if(buffer->current_line->text == NULL) {
        int len = my_strlen(text);
        char *n_text = (char*) malloc(sizeof(char)*(len+1));
        int i;
        for(i = 0; i < len; i++) {
            n_text[i] = text[i];
        }
        n_text[len] = 0;
        buffer->current_line->text = n_text;
        
        buffer->dirty = true;
        
        return;
    }
    
    
    int len_until_insert = buffer->current_x-1;
    int len_after_insert = my_strlen(buffer->current_line->text) - len_until_insert;
    int len_insert = my_strlen(text);
    
    int total_len = len_until_insert + len_after_insert + len_insert;
    
    char *new_text = (char*) malloc(sizeof(char)*(total_len+1));
    
    int i;
    for(i = 0; i < len_until_insert; i++) {
        new_text[i] = buffer->current_line->text[i];
    }
    int offset = i;
    
    for(i = 0; i < len_insert; i++) {
        new_text[offset+i] = text[i];
    }
    offset += i;
    
    for(i = 0; i < len_after_insert; i++) {
        new_text[offset+i] = buffer->current_line->text[len_until_insert+i];
    }
    
    new_text[total_len] = 0;
    
    free(buffer->current_line->text);
    buffer->current_line->text = new_text;
    
    buffer->dirty = true;
}

int buffer_get_current_line_number(Buffer *buffer) {
    Line *current = buffer->current_line;
    
    if(current == buffer->first_line) {
        return 1;
    }
    
    int lineno = 1;
    Line *line = buffer->first_line;
    while(line) {
        if(line == buffer->current_line) {
            return lineno;
        }
        lineno++;
        line = line->next;
    }
    
    return -1337;
}

int buffer_get_lines_from_top(Buffer *buffer) {
    Line *top = buffer->top_line;
    
    if(buffer->top_line->next == NULL) return 0;
    if(buffer->top_line == buffer->current_line) return 0;
    
    int lineno = 1;
    Line *l = top;
    while(l) {
        if(l == buffer->current_line) {
            return lineno;   
        }
        lineno++;
        l = l->next;
    }
    
    return -420;
}

void buffer_return(Buffer *buffer) {
    
    if(buffer->current_x == 1) {
     
        Line *l = (Line*) malloc(sizeof(Line));
        l->text = NULL;
        l->next = buffer->current_line;
        l->prev = buffer->current_line->prev;
        
        if(buffer->current_line == buffer->first_line) {
            buffer->first_line = l;
        }
        if(buffer->current_line == buffer->top_line) {
            buffer->top_line = l;
        }
        
        Line *next = buffer->current_line;
        Line *prev = buffer->current_line->prev;
        
        next->prev = l;
        if(prev) prev->next = l;
        
        buffer->dirty = true;
        
        return;
    }
    
    if(buffer->current_x == my_strlen(buffer->current_line->text)+1) {
        
        Line *l = (Line*) malloc(sizeof(Line));
        l->text = NULL;
        l->next = buffer->current_line->next;
        l->prev = buffer->current_line;
        
        Line *prev = buffer->current_line;
        Line *next = buffer->current_line->next;
        
        prev->next = l;
        if(next) next->prev = l;
        
        buffer->current_line = l;
        
        buffer->current_x = 1;
        buffer->dirty = true;
        
        return;
    }
    
    // Split and new line
    
    int offset = buffer->current_x - 1;
    Line *original = buffer->current_line;
    
    Line *new_line = (Line*) malloc(sizeof(Line));
    new_line->prev = original;
    new_line->next = original->next;
    
    if(original->next) original->next->prev = new_line;
    original->next = new_line;
    
    int original_len = my_strlen(original->text);
    int len = original_len - offset;
    new_line->text = (char*) malloc(sizeof(char)*(len+1));
    new_line->text[len] = 0;
    int i;
    for(i = 0; i < len; i++) {
        new_line->text[i] = original->text[offset+i];
    }
    
    original->text[offset] = 0;
    
    buffer->current_x = 1;
    buffer->x_we_want = 1;
    
    buffer->current_line = new_line;
    buffer->dirty = true;
}

void buffer_backspace(Buffer *buffer) {
    
    int len = my_strlen(buffer->current_line->text);
    
    buffer->dirty = true;
    
    if(len == 0) {
        
        if(buffer->current_line->prev == NULL &&
            buffer->current_line->next == NULL) 
        {
            // This is the only line in the file
            // Dont do shit
            
            return;
        }
        
        if(buffer->current_line->prev == NULL &&
            buffer->current_line->next != NULL)
        {
            // There is a line below so we can remove
            // this line
            // Set currentline and first, and set x to 1
            
            Line *next = buffer->current_line->next;
            next->prev = NULL;
            
            free(buffer->current_line->text);
            free(buffer->current_line);
            
            buffer->first_line = next;
            buffer->current_line = next;
            buffer->current_x = 1;
            buffer->x_we_want = 1;
            
            return;
        }
        
        if(buffer->current_line->prev != NULL &&
            buffer->current_line->next == NULL)
        {
            
            Line *prev = buffer->current_line->prev;
            Line *old = buffer->current_line;
            
            prev->next = NULL;
            if(buffer->current_line == buffer->first_line) {
                buffer->first_line = prev;
            }
            if(buffer->current_line == buffer->top_line) {
                buffer->top_line = prev;
            }
            buffer->current_line = prev;
            
            if(old->text) free(old->text);
            free(old);
            
            buffer->current_x = my_strlen(prev->text) + 1;
            buffer->x_we_want = buffer->current_x;
            
            return;
        }
        
        // There are plenty of lines
        // just remove this one
        // and update references
        
        Line *prev = buffer->current_line->prev;
        Line *next = buffer->current_line->next;
        Line *old = buffer->current_line;
        
        prev->next = next;
        next->prev = prev;
        free(old->text);
        free(old);
        
        buffer->current_line = prev;
        buffer->current_x = my_strlen(prev->text) + 1;
        buffer->x_we_want = buffer->current_x;
        
        return;
        
    } else {
        
        if(buffer->current_x == 1) {
            
            // if we have a line above us add this one
            // to the end of that one and remove this line
            
            if(buffer->current_line->prev != NULL) {
                Line *prev = buffer->current_line->prev;
                
                int prev_len = my_strlen(prev->text);
                
                if(prev->text == NULL || prev_len == 0) {
                    if(prev->text) free(prev->text);
                    prev->text = buffer->current_line->text;
                    buffer->current_x = 1;
                    buffer->x_we_want = 1;
                    
                    if(buffer->current_line->next) {
                        prev->next = buffer->current_line->next;
                    } else {
                        prev->next = NULL;
                    }
                    
                    if(buffer->current_line == buffer->first_line) {
                        buffer->first_line = prev;
                    }
                    if(buffer->current_line == buffer->top_line) {
                        buffer->top_line = prev;
                    }
                    
                    free(buffer->current_line);
                    buffer->current_line = prev;
                    
                    return;
                    
                }
                
                int old_len = my_strlen(buffer->current_line->text);
                int new_len = prev_len + old_len;
                
                char *new_text = (char*) malloc(sizeof(char)*(new_len+1));
                new_text[new_len] = 0;
                
                int i;
                for(i = 0; i < new_len; i++) {
                    if(i >= prev_len) {
                        new_text[i] = buffer->current_line->text[i-prev_len];
                    } else {
                        new_text[i] = prev->text[i];
                    }
                }
                
                free(prev->text);
                free(buffer->current_line->text);
                
                prev->text = new_text;
                
                buffer->current_x = prev_len + 1;
                buffer->x_we_want = buffer->current_x;
                
                Line *next = buffer->current_line->next;
                if(next) next->prev = prev;
                prev->next = next;
                
                if(buffer->current_line == buffer->first_line) {
                    buffer->first_line = prev;
                }
                if(buffer->current_line == buffer->top_line) {
                    buffer->top_line = prev;
                }
                
                free(buffer->current_line);
                buffer->current_line = prev;
                
                return;
                
            }
            
            return;
        }
        
        int char_to_remove = buffer->current_x - 1;
        int old_len = my_strlen(buffer->current_line->text);
        
        if(char_to_remove == old_len) {
            
            buffer->current_line->text[old_len-1] = 0;
            buffer->current_x--;
            buffer->x_we_want = buffer->current_x;
            
            return;
        }
        
        int new_len = old_len - 1;
        char *new_line = (char*) malloc(sizeof(char)*(new_len+1));
        new_line[new_len] = 0;
        
        int i;
        for(i = 0; i < char_to_remove; i++) {
            new_line[i] = buffer->current_line->text[i];
        }
        
        for(i = char_to_remove-1; i < new_len; i++) {
            new_line[i] = buffer->current_line->text[i+1];
        }
        
        free(buffer->current_line->text);
        buffer->current_line->text = new_line;
        buffer->current_x--;
        buffer->x_we_want = buffer->current_x;
        
        return;
    }
    
}