#include "include.h"
#include "frame.h"
#include <time.h>
#include <math.h>
// void point(buffer_t *b, vec2f_t p, uint32_t color)
// {
//     uint32_t *pixel = (uint32_t *)b->pixels;
//     vec2i_t p_i = vec2i_c((int)p.x, (int)p.y);
//     pixel[p_i.x + p_i.y * b->width] = color;
//     return;
// }

vec2i_t screen(buffer_t b, vec2f_t p)
{
    bufferi_t *bi = b.i;
    return vec2i_c(
        (p.x + 1) / 2 * bi->width,
        (p.y + 1) / 2 * bi->height);
}

vec2f_t project(vec3f_t p)
{
    if (p.z == 0)
        return vec2f_c(-1, -1);
    return vec2f_c(p.x / p.z, p.y / p.z);
}

vec3f_t translate(vec3f_t p1, vec3f_t p2)
{
    return vec3f_c(p1.x+p2.x, p1.y+p2.y, p1.z+p2.z);
}


vec3f_t rotate(vec3f_t p, vec3f_t v)
{
    // v = (yaw, pitch, roll)
    vec3f_t c = vec3f_c(cos(v.x), cos(v.y), cos(v.z)); 
    vec3f_t s = vec3f_c(sin(v.x), sin(v.y), sin(v.z));
    return vec3f_c(
        p.x * c.x * c.y
      + p.y * (c.x * s.y * s.z - s.x * c.z)
      + p.z * (c.x * s.y * c.z + s.x * s.z),

        p.x * s.x * c.y
      + p.y * (s.x * s.y * s.z + c.x * c.z)
      + p.z * (s.x * s.y * c.z - c.x * s.z),

       -p.x * s.y
      + p.y * c.y * s.z
      + p.z * c.y * c.z
    );
}




/*

TODO:
BENS MATH ORGY:
for y LOW to y HIGH:
        calculate x ENTER and x EXIT
        for X ENTER TO X EXIT:
            pixel(x,y) = color





*/






// typedef struct {
//     union {
//         struct {
//             struct {
//                 int x;
//                 int y;
//             } v1;
//             struct {
//                 int x;
//                 int y;
//             } v2;
//             struct {
//                 int x;
//                 int y;
//             } v3;
//         };
//         int v[6];
//     };
// }triangle_t;

typedef struct {
    union {
        struct {
            vec2i_t v1;
            vec2i_t v2;
            vec2i_t v3;
        };
        int v[6];
    };
}triangle_t;


void pixel(buffer_t b, int x, int y, uint32_t c)
{
    bufferi_t *bi = b.i;
    ((uint32_t *)bi->pixels)[x + y * bi->width] = c;
}



// void drawTriangle(buffer_t b, triangle_t t, uint32_t c)
// {
//     vec2i_t triangle[3];
//     {
//     vec2i_t ap = {t.v1.x, t.v1.y}, bp = {t.v2.x, t.v2.y}, cp = {t.v3.x, t.v3.y};
//     triangle[0] = ap; triangle[1] = bp; triangle[2] = cp;
//     }
//     for (int j = 0; j < 2; ++j)
//     {
//         for (int i = 0; i < 2; ++i)
//         {
//             if ( triangle[i].y < triangle[i+1].y )
//             {
//                 vec2i_t temp = triangle[i]; triangle[i] = triangle[i+1]; triangle[i+1] = temp;
//             }
//         }
//     }
//     vec2i_t ap = {triangle[0].x, triangle[0].y}, bp = {triangle[1].x, triangle[1].y}, cp = {triangle[2].x, triangle[2].y};
//     //printf("a(%d,%d), b(%d,%d), c(%d,%d)\n", ap.x, ap.y, bp.x, bp.y, cp.x, cp.y);



//     /* ME WHEN I LIE! :SIGMA SKIBIDI TOILET RIZZ: */
//     int det = edge(ap, bp, cp);
//     if (det == 0 || det < 0) return;

//     int ly, hy;
//     ly = cp.y; hy = ap.y;
//     for (int y = ly+1; y < hy; ++y)
//     {
//         int lx, hx;
//         if ( y > bp.y)
//         {
//             lx = ap.x + (y-ap.y) * (bp.x - ap.x)    / (bp.y - ap.y);
//             hx = lx+ap.x + (y-bp.y) * (cp.x - ap.x) / (cp.y - ap.y);
//         } else {
//             lx = bp.x + (y-bp.y) * (cp.x - bp.x)    / (cp.y - bp.y);
//             hx = lx+ap.x + (y-cp.y) * (cp.x - ap.x) / (cp.y - ap.y);
//         }
//         //enter = a.x + (y-a.y) * (b.x - a.x) // (b.y - a.y) 
//         //exit  = a.x + (b.y-y) * (c.x - a.x) // (c.y - a.y)
//         //printf("l%d,h%d\n", lx, hx);
//         for (int x = lx; x < hx; ++x)
//         {
//             pixel(b, x, y, c);
//         }
//     }

// }




void clearScreen(buffer_t b)
{
    bufferi_t *bi = b.i;
    for (uint x = 0; x < bi->width; ++x)
    {
        for (uint y = 0; y < bi->height; ++y)
        {
            ((uint32_t *)bi->pixels)[x + y * bi->width] = 0x007A3FCD;
        }
    }
}


void point(buffer_t b, vec2i_t p, int s, uint32_t c)
{
    bufferi_t *bi = b.i;
    //int x = p.x, y = p.y;
    for (int x = p.x-s/2; x < p.x+s/2; ++x)
    {
        for (int y = p.y-s/2; y < p.y+s/2; ++y)
        {
            ((uint32_t *)bi->pixels)[x + y * bi->width] = c;
        }
    }
}

int normalizeTriangle(buffer_t b, triangle_t t, vec4i_t *c)
{
    bufferi_t *bi = b.i;

    c->x=t.v1.x;
    c->y=t.v1.x;
    if (c->x > t.v2.x) c->x = t.v2.x;
    if (c->x > t.v3.x) c->x = t.v3.x;
    if (c->y < t.v2.x) c->y = t.v2.x;
    if (c->y < t.v3.x) c->y = t.v3.x;
    if (c->x < 0) c->x = 0;
    if (c->x >= (int)bi->width) return 0;
    if (c->y < 0) return 0;
    if (c->y >= (int)bi->width) c->y = bi->width-1;

    c->z=t.v1.y;
    c->w=t.v1.y;
    if (c->z > t.v2.y) c->z = t.v2.y;
    if (c->z > t.v3.y) c->z = t.v3.y;
    if (c->w < t.v2.y) c->w = t.v2.y;
    if (c->w < t.v3.y) c->w = t.v3.y;
    if (c->z < 0) c->z = 0;
    if (c->z >= (int)bi->height) return 0;
    if (c->w < 0) return 0;
    if (c->w >= (int)bi->height) c->w = bi->height-1; 

    //printf("%d %d %d %d\n", c->x, c->y, c->z, c->w);
    return 1;
}

int edge(vec2i_t a, vec2i_t b, vec2i_t c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

#define sign(x) (x>0) - (x<0)

int barycentricTriangle(triangle_t t, int x, int y, vec4i_t *u)
{
    int det = edge(t.v3, t.v1, t.v2);
    u->x    = edge(t.v1, t.v2, vec2i_c(x,y));
    u->y    = edge(t.v2, t.v3, vec2i_c(x,y));
    u->z    = det - u->x - u->y;
    u->w    = det;
    //return 1;
    return ((sign(u->x) == sign(det) || u->x == 0) 
         && (sign(u->y) == sign(det) || u->y == 0) 
         && (sign(u->z) == sign(det) || u->z == 0)
        );
}

uint32_t mixColors3(vec3u32_t c, vec4i_t u)
{
    if(u.w != 0)
    {
        uint32_t a1 = (c.z&0xFF000000) >> 24;
        uint32_t r1 = (c.z&0x00FF0000) >> 16;
        uint32_t g1 = (c.z&0x0000FF00) >> 8;
        uint32_t b1 = (c.z&0x000000FF) >> 0;
        //printf("%02x%02x%02x%02x\n", a1, r1, g1,b1);

        uint32_t a2 = (c.x&0xFF000000) >> 24;
        uint32_t r2 = (c.x&0x00FF0000) >> 16;
        uint32_t g2 = (c.x&0x0000FF00) >> 8;
        uint32_t b2 = (c.x&0x000000FF) >> 0;
        //printf("%02x%02x%02x%02x\n", a2, r2, g2,b2);

        uint32_t a3 = (c.y&0xFF000000) >> 24;
        uint32_t r3 = (c.y&0x00FF0000) >> 16;
        uint32_t g3 = (c.y&0x0000FF00) >> 8;
        uint32_t b3 = (c.y&0x000000FF) >> 0;
        //printf("%02x%02x%02x%02x\n", a3, r3, g3,b3);

        uint32_t r = (r1*u.x + r2*u.y + r3*u.z)/u.w;
        uint32_t g = (g1*u.x + g2*u.y + g3*u.z)/u.w;
        uint32_t b = (b1*u.x + b2*u.y + b3*u.z)/u.w;
        uint32_t a = (a1*u.x + a2*u.y + a3*u.z)/u.w;
        //printf("%02x%02x%02x%02x\n", a, r, g,b);
        uint32_t color = (b&0xFF) | (g&0xFF) << 8 | (r&0xFF) << 16 | (a&0xFF) << 24;
        //printf("%08x\n\n", color);


        return color;
    }
    return 0xFFFFFFFF;
}

void drawTriangle3c(buffer_t b, triangle_t t, vec3u32_t colors)
{
    
    vec4i_t vechl = {0};
    if(normalizeTriangle(b, t, &vechl))
    {
        int lx = vechl.x, hx = vechl.y, ly = vechl.z, hy = vechl.w;
        //printf("%d %d %d %d", lx, hx, ly, hy);
        for (int y = ly; y < hy; ++y)
        {
            for (int x = lx; x < hx; ++x)
            {
                vec4i_t u = {0};
                if((barycentricTriangle(t, x, y, &u)) != 0)
                {
                    pixel(b, x, y, mixColors3(colors, u));
                }
            }
        }
    }
}




const float    cubevs[] = {
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,

     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,

     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,

     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
};
const uint32_t cubecs[] = {
    0x004A7BD9,
    0x004A7BD9,
    0x004A7BD9,
    0x004A7BD9,
    0x004A7BD9,
    0x004A7BD9,

    0x00FA6C4E,
    0x00FA6C4E,
    0x00FA6C4E,
    0x00FA6C4E,
    0x00FA6C4E,
    0x00FA6C4E,

    0x0091D15B,
    0x0091D15B,
    0x0091D15B,
    0x0091D15B,
    0x0091D15B,
    0x0091D15B,

    0x00EB5AC1,
    0x00EB5AC1,
    0x00EB5AC1,
    0x00EB5AC1,
    0x00EB5AC1,
    0x00EB5AC1,
    
    0x006F48C9,
    0x006F48C9,
    0x006F48C9,
    0x006F48C9,
    0x006F48C9,
    0x006F48C9,

    0x00F2D04B,
    0x00F2D04B,
    0x00F2D04B,
    0x00F2D04B,
    0x00F2D04B,
    0x00F2D04B
};

void drawObject(buffer_t b, int f)
{ 
    for (int i = 0; i < 108; i+=9)
    {
        vec3f_t p1  = vec3f_c(cubevs[i], cubevs[i+1], cubevs[i+2]);
        vec2i_t tp1 = screen(b, project(translate(rotate(p1, vec3f_c((float)f/500.0f,0,0)), vec3f_c(-1.7f,  3.0f,  7.5f))));
        vec3f_t p2  = vec3f_c(cubevs[i+3], cubevs[i+4], cubevs[i+5]);
        vec2i_t tp2 = screen(b, project(translate(rotate(p2, vec3f_c((float)f/500.0f,0,0)), vec3f_c(-1.7f,  3.0f,  7.5f))));
        vec3f_t p3  = vec3f_c(cubevs[i+6], cubevs[i+7], cubevs[i+8]);
        vec2i_t tp3 = screen(b, project(translate(rotate(p3, vec3f_c((float)f/500.0f,0,0)), vec3f_c(-1.7f,  3.0f,  7.5f))));
        triangle_t t = {.v1 = tp1, .v2 = tp2, .v3 = tp3};
        drawTriangle3c(b, t, vec3u32_c(cubecs[i/3],cubecs[i/3+1],cubecs[i/3+2]));
    }
}

int main( int argc, char** argv )
{
    printargs(argc, argv, "Log: Args");
    window_t w = createWindow(w, 640, 580, "Hello, World!");
    windowi_t *wi = w.i; framei_t *fi = wi->frame.i;
    float f = 0;
    while(!wi->should_close)
    {
        f++;
        clearScreen(fi->image);
        drawObject(fi->image, f);

        // triangle_t t = { .v={0,0, 640,0, 320,580} };
        // drawTriangle3c(fi->image, t, vec3u32_c(0xFFFF0000,0xFF00FF00,0xFF0000FF));
        // point(fi->image, t.v1, 4, 0xFFFF0000);
        // point(fi->image, t.v2, 4, 0xFF00FF00);
        // point(fi->image, t.v3, 4, 0xFF0000FF);
        //break;
        displayWindow(w);   
    }
    
    freeWindow(w);
    return 0;
}