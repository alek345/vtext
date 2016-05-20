#include "vtext.h"

char* my_strdup(const char *str) {
    if(str == 0) return 0;
    
    int len = 0;
    char *ptr = (char*) str;
    while(*ptr) {
        len++;
        ptr++;
    }
    
    char *ret = (char*) malloc(sizeof(char)*(len+1));
    
    int i;
    for(i = 0; i < len; i++) {
        ret[i] = str[i];
    }
    ret[len] = 0;
    
    return ret;
}

int my_strlen(const char *str) {
    if(str == 0) return 0;
    
    int len = 0;
    while(*str) {
        str++;
        len++;
    }
    return len;
}