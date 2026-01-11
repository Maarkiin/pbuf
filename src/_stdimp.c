#define _NOOS
#ifdef _WIN32
#undef _NOOS
//#elifdef __linux__
//#undef _NOOS
#endif

#ifdef _NOOS

#include "include.h"
#include "frame.h"

typedef struct buffer {
    void *pixels;
    uint width;
    uint height;
    uint stride;
} buffer_t;

buffer_t *createBuffer(uint h, uint w, uint s)
{
    buffer_t *b = malloc(sizeof(buffer_t));
    b->height = h;
    b->width = w;
    b->stride = s;
    
    b->pixels = malloc(h*w*s);
    memset(b->pixels, 0, h*w*s);
    return b;
}

void freeBuffer(buffer_t *b)
{
    free(b->pixels);
    free(b);
}

void displayWindow(window_t *w UNUSED)
{
    printf("ERR: OS NOT IMPLEMENTED\n");
    exit(EXIT_FAILURE);
}

#endif