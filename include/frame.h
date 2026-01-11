#ifndef FRAME_H
#define FRAME_H

#include "include.h"

#define DEF_VEC2(T, n) \
typedef struct vec2##n { \
    union { \
        struct { \
            T x; \
            T y; \
        }; \
        T value[2]; \
    }; \
} vec2##n##_t; \
static inline vec2##n##_t vec2##n##_c(T x, T y) { return ((vec2##n##_t){ .value = {x, y}}); }

#define DEF_VEC3(T, n) \
typedef struct vec3##n { \
    union { \
        struct { \
            T x; \
            T y; \
            T z; \
        }; \
        T value[3]; \
    }; \
} vec3##n##_t; \
static inline vec3##n##_t vec3##n##_c(T x, T y, T z) { return ((vec3##n##_t){ .value = {x, y, z}}); }

DEF_VEC2(int,   i);
DEF_VEC2(float, f);
DEF_VEC3(int,   i);
DEF_VEC3(float, f);
DEF_VEC3(uint32_t, u32);

typedef struct {
    union {
        struct {
            int x;
            int y;
            int z;
            int w;
        };
        int value[4];
    };
} vec4i_t;
static inline vec4i_t vec4i_c(int x, int y, int z, int w) { return ((vec4i_t){ .value = {x, y, z, w}}); }

typedef struct buffer {void* i;}buffer_t;

typedef struct frame {void* i;} frame_t;

typedef struct window {void* i;} window_t;

#ifdef _WIN32
#include <windows.h>
typedef struct {
    void *pixels;
    uint width;
    uint height;
    uint stride;

    BITMAPINFO bmi;
} bufferi_t;

typedef struct {
    buffer_t image;
    buffer_t z_buffer;
} framei_t;

typedef struct {
    frame_t frame;
    int keys[348];
    
    HWND handle;
    MSG msg;
    int should_close;
} windowi_t;
#endif

buffer_t createBuffer(uint w, uint h, uint s);
void freeBuffer(buffer_t b);

frame_t createFrame(uint w, uint h);
void freeFrame(frame_t f);

window_t createWindow(window_t w, uint width, uint height, char *title);
void freeWindow(window_t w);
void displayWindow(window_t w);
void *makeWindow(window_t w, uint width, uint height, char *title);


#endif