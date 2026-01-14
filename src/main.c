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
    if (x > (int)bi->width-1) return;
    if (x < 0) return;
    if (y > (int)bi->height-1) return;
    if (y < 0) return;
    printf("1");
    ((uint32_t *)bi->pixels)[x + y * bi->width] = c;
}








void clearScreen(buffer_t b)
{
    bufferi_t *bi = b.i;
    for (uint x = 0; x < bi->width; ++x)
    {
        for (uint y = 0; y < bi->height; ++y)
        {
            ((uint32_t *)bi->pixels)[x + y * bi->width] = 0x00303030;
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
    int det = -edge(t.v3, t.v1, t.v2);
    u->x    = -edge(t.v1, t.v2, vec2i_c(x,y));
    u->y    = -edge(t.v2, t.v3, vec2i_c(x,y));
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



// drawTriangle with ray intersection method via integer division
void drawTriangle(buffer_t b, triangle_t t, uint32_t c)
{
    vec2i_t triangle[3];
    {
        vec2i_t ap = vec2i_c(t.v1.x, t.v1.y), bp = vec2i_c(t.v2.x, t.v2.y), cp = vec2i_c(t.v3.x, t.v3.y);
        triangle[0] = ap; triangle[1] = bp; triangle[2] = cp;
        int det = edge(ap, bp, cp);
        if (det == 0 || det > 0) return; // If our triangle is 0 area or backfacing, skip it.
    }
    
    printf("2 ");
    for (int j = 0; j < 2; ++j) // Bubble sort to order vertices by y-coordinate
    {
        for (int i = 0; i < 2-j; ++i)
        {
            if ( triangle[i].y < triangle[i+1].y || (triangle[i].y == triangle[i+1].y && triangle[i].x > triangle[i+1].x))
            {
                vec2i_t temp = triangle[i]; triangle[i] = triangle[i+1]; triangle[i+1] = temp;
            }
        }
    }
    vec2i_t ap = vec2i_c(triangle[0].x, triangle[0].y), bp = vec2i_c(triangle[1].x, triangle[1].y), cp = vec2i_c(triangle[2].x, triangle[2].y);


    int det_2 = edge(ap, bp, cp); // Update the det to be the determinant of the re-ordered vertices
    if (det_2 > 0)
    {
        int ly, my, hy;
        ly = cp.y; hy = ap.y; my = bp.y;

        // main rasterization loops
        for (int y = ly; y < my; ++y)
        {
            int lx, hx;
            int denominator = (bp.y - cp.y);
            int safe_denominator = denominator + (denominator == 0);

            lx = cp.x + (y-cp.y) * (bp.x - cp.x) / (safe_denominator);
            hx = cp.x + (y-cp.y) * (ap.x - cp.x) / (ap.y - cp.y);

            for (int x = lx; x < hx; ++x)
            {
                // vec4i_t u;
                // barycentricTriangle(t, x, y, &u);
                // pixel(b, x, y, mixColors3(colors, u));
                pixel(b, x, y, c);
            }
            // pixel(b, lx, y, c);
            // pixel(b, hx, y, c);
        }
        for (int y = my; y < hy + 1; ++y)
        {
            int lx, hx;

            lx = bp.x + (y-bp.y) * (ap.x - bp.x) / (ap.y - bp.y);
            hx = cp.x + (y-cp.y) * (ap.x - cp.x) / (ap.y - cp.y);

            for (int x = lx; x < hx; ++x)
            {
                // vec4i_t u;
                // barycentricTriangle(t, x, y, &u);
                // pixel(b, x, y, mixColors3(colors, u));
                pixel(b, x, y, c);
            }
            // pixel(b, lx, y, c);
            // pixel(b, hx, y, c);
        }
    }
    
    if (det_2 < 0)
    {
        int ly, my, hy;
        ly = cp.y; hy = ap.y; my = bp.y;
        
        // main rasterization loops
        for (int y = ly; y < my; ++y)
        {
            int lx, hx;
            
            hx = cp.x + (y-cp.y) * (bp.x - cp.x) / (bp.y - cp.y);
            lx = cp.x + (y-cp.y) * (ap.x - cp.x) / (ap.y - cp.y);
            
            for (int x = lx; x < hx; ++x)
            {
                // vec4i_t u;
                // barycentricTriangle(t, x, y, &u);
                // pixel(b, x, y, mixColors3(colors, u));
                pixel(b, x, y, c);
            }
            // pixel(b, lx, y, c);
            // pixel(b, hx, y, c);
        }
        for (int y = my; y < hy; ++y)
        {
            int lx, hx;
            int denominator = (ap.y - bp.y);
            int safe_denominator = denominator + (denominator == 0);
            
            hx = bp.x + (y-bp.y) * (ap.x - bp.x) / (safe_denominator);
            lx = cp.x + (y-cp.y) * (ap.x - cp.x) / (ap.y - cp.y);
            
            for (int x = lx; x < hx; ++x)
            {
                // vec4i_t u;
                // barycentricTriangle(t, x, y, &u);
                // vec3f_t uv =
                // vec2f_c(
                //     tuv[0].x * u.x +
                //     tuv[1].x * u.y +
                //     tuv[2].x * u.z,
                //     tuv[0].y * u.x +
                //     tuv[1].y * u.y +
                //     tuv[2].y * u.z,
                // );
                // int x = (int)(uv.x * (float)tex.width);
                // int y = (int)(uv.y * (float)tex.height);

                // // x = clamp(x, 0, tex.width  - 1);
                // // y = clamp(y, 0, tex.height - 1);

                // tex.data[y * tex.width + x];
                // uint32_t texel = sampleTexture(texture, uv);
                pixel(b, x, y, c);
            }
            // pixel(b, lx, y, c);
            // pixel(b, hx, y, c);
        }
    }
}

// drawTriangle REWRITE with ray intersection method via derivative and bit shifting arithmetic

// functions to swap pointers of ints and vector2is
void swap_int(int *a, int *b) { int t = *a; *a = *b; *b = t; }
void swap_vec(vec2i_t *a, vec2i_t *b) { vec2i_t t = *a; *a = *b; *b = t; }

#define swap_var(type, a, b) type t = *a; *a = *b; *b = t;

void drawTriangle2(buffer_t b, triangle_t t, uint32_t c)
{
    //vec2i_t triangle[3];
    vec2i_t ap = vec2i_c(t.v1.x, t.v1.y), bp = vec2i_c(t.v2.x, t.v2.y), cp = vec2i_c(t.v3.x, t.v3.y);
    //triangle[0] = ap; triangle[1] = bp; triangle[2] = cp;
    int det = edge(ap, bp, cp);
    if (det == 0 || det < 0) return; // If our triangle is 0 area or backfacing, skip it. (remove this and det calculation if backface culling moved to drawObject)

    if (ap.y < bp.y) { swap_var(vec2i_t,&ap, &bp); }
    if (ap.y < cp.y) { swap_vec(&ap, &cp); }
    if (bp.y < cp.y) { swap_vec(&bp, &cp); }

    int total_height = ap.y - cp.y;
    int bottomTriangleHeight = bp.y - cp.y;
    int topTriangleHeight = ap.y - bp.y;
    if (total_height == 0) return; // skip degenerate triangles (already handled above but needed if we move backface culling to drawObject)

    // we calculate the long edge dx/dy and shift by 16 to store precision of the gradients but keep them as integers
    // we use dx/dy as dy is never zero now that we have skipped triangles with zero height
    int dx_dy_ac = ((ap.x - cp.x) << 16) / total_height;
    
    // Draw bottom triangle
    if (bottomTriangleHeight > 0)
    {
        int dx_dy_bc = ((bp.x - cp.x) << 16) / bottomTriangleHeight;
    
        // We define the increments for drawing out the bottom triangle
        int x_right_increment = dx_dy_ac;
        int x_left_increment = dx_dy_bc;
        // We move one height up from C and check the two intersections, if the AC intersection has higher x value, the left and right increments must be swapped 
        if ((cp.x << 16) + x_right_increment > (cp.x << 16) + x_left_increment) {
            swap_int(&x_right_increment, &x_left_increment);
        }

        //int x_short = cp.x << 16; // The bottom triangle's first point starts at C
        int x_left = cp.x << 16;
        int x_right = cp.x << 16;
        
        for (int y = cp.y; y < bp.y; y++) // colours the triangle's horizontal pixels from C to B
        {
            // We shift our left and right points back down as we calculated them using shifted up numbers for gradient precision
            int start_pixel = x_left >> 16;
            int end_pixel = x_right >> 16;
            for (int x = start_pixel; x < end_pixel; ++x)
            {
                pixel(b, x, y, c); // Would x and y ever enter this outside of the window's resolution? Or do we ensure that all triangles are in the window in other functions?
            }

            x_left += x_left_increment;
            x_right += x_right_increment;
        }
    }

    // Draw top triangle
    if (topTriangleHeight > 0)
    {
        int dx_dy_ab = ((ap.x - bp.x) << 16) / bottomTriangleHeight;
    
        // We define the increments for drawing out the bottom triangle
        int x_right_increment = dx_dy_ac;
        int x_left_increment = dx_dy_ab;
        // We move one height up from C and check the two intersections, if the AC intersection has higher x value, the left and right increments must be swapped 
        if ((cp.x << 16) + x_right_increment > (cp.x << 16) + x_left_increment) {
            swap_int(&x_right_increment, &x_left_increment);
        }

        //int x_short = cp.x << 16; // The bottom triangle's first point starts at C
        int x_left = cp.x << 16;
        int x_right = cp.x << 16;

        for (int y = cp.y; y < bp.y; y++) // colours the triangle's horizontal pixels from C to B
        {
            // We shift our left and right points back down as we calculated them using shifted up numbers for gradient precision
            int start_pixel = x_left >> 16;
            int end_pixel = x_right >> 16;
            for (int x = start_pixel; x < end_pixel; ++x)
            {
                pixel(b, x, y, c); // Would x and y ever enter this outside of the window's resolution? Or do we ensure that all triangles are in the window in other functions?
            }

            x_left += x_left_increment;
            x_right += x_right_increment;
        }
    }
}

/*
    1 2 3 / 3 4 1
*/


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
// const vec2f_t cubeuv[] = {
//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(0.0f, 1.0f),

//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(1.0f, 0.0f),

//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(1.0f, 1.0f),

//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(0.0f, 0.0f),

//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(0.0f, 1.0f),

//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(1.0f, 0.0f),

//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(1.0f, 1.0f),

//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(0.0f, 0.0f),

//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(1.0f, 0.0f),

//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(0.0f, 1.0f),

//     vec2f_c(0.0f, 0.0f),
//     vec2f_c(1.0f, 0.0f),
//     vec2f_c(1.0f, 1.0f),

//     vec2f_c(1.0f, 1.0f),
//     vec2f_c(0.0f, 1.0f),
//     vec2f_c(0.0f, 0.0f)
// };
const uint32_t cubecs[] = {
    0xFFFF0000,
    0xFFFF7F00,
    0xFFFFFF00,
    0xFF7FFF00,
    0xFF00FF00,
    0xFF00FF7F,
    0xFF00FFFF,
    0xFF007FFF,
    0xFF0000FF,
    0xFF7F00FF,
    0xFFFF00FF,
    0xFFFF007F,
    0xFF8B0000,
    0xFFB22222,
    0xFFDC143C,
    0xFFFF4500,
    0xFFFF8C00,
    0xFFFFD700,
    0xFFADFF2F,
    0xFF32CD32,
    0xFF228B22,
    0xFF20B2AA,
    0xFF00CED1,
    0xFF1E90FF,
    0xFF000080,
    0xFF4B0082,
    0xFF800080,
    0xFF8A2BE2,
    0xFFDA70D6,
    0xFFFF69B4
};

typedef struct {
    size_t  size_vs;
    float  *vs;
    size_t  size_indices;
    int    *indices;
} object_t;

int strstar(char *s, char *c)
{
    int r = 1, j = 0;
    uint n = strlen(c);
    for (uint i = 0; i < n; ++i)
    {
        r = r && s[j++] == c[i];
    }
    return r;
}

void createObject(object_t *o)
{
    //printf("TEST!\n");
    const uint SIZE = 2048;
    float vs[SIZE]; size_t size_vs = 0;
    uint  indices[SIZE]; size_t size_indices = 0;
    FILE *fp;
    fp = fopen("res/monkey.obj", "r");
    char buf[SIZE];
    int line = 1;
    while(fgets(buf, SIZE, fp))
    {
        if (strstar(buf, "v "))
        {
            //printf("%lld: %s", strlen(buf), buf);
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "v"))
                {
                    //printf("%s ", p);
                    vs[size_vs++] = (float)atof(p);
                    
                }
                p = strtok(NULL, " ");
            }
        }
        //printf("%d %f :", line++, vs[0]);
        if (strstar(buf, "f "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "f"))
                {
                    *(p+1) = '\0';
                    //printf("%s ", p);
                    indices[size_indices++] = (uint)atoi(p);
                }
                p = strtok(NULL, " ");
            }
        }
    }
    fclose(fp);
    printf("%llu", SIZE_MAX/sizeof(float));
    // for (size_t i = 0; i < size_vs; i++)
    // {
    //     printf("%f ", vs[i]);
    //     if ((i+1)%3 == 0) printf("\n");
    // }

    o->size_vs   = size_vs;
    o->vs        = malloc(o->size_vs*sizeof(float));
    for (size_t i = 0; i < size_vs; i++)
    {
        o->vs[i] = vs[i];
    }
    //memcpy(o->vs, vs, o->size_vs);

    o->size_indices  = size_indices;
    o->indices       = malloc(o->size_indices*sizeof(uint));
    for (size_t i = 0; i < size_indices; i++)
    {
        o->indices[i] = indices[i];
    }
    //memcpy(o->indices, indices, o->size_indices);
}

void drawObject(buffer_t b, object_t* o, int f)
{ 
    const vec3f_t cube_positions[] = {
        vec3f_c( 2.0f,  5.0f,  15.0f),
        // vec3f_c(-1.5f, -2.2f,  2.5f),
        // vec3f_c(-3.8f, -2.0f,  12.3f),
        // vec3f_c( 2.4f, -0.4f,  3.5f),
        // vec3f_c(-1.7f,  3.0f,  7.5f),
        // vec3f_c( 1.3f, -2.0f,  2.5f),
        // vec3f_c( 1.5f,  2.0f,  2.5f),
        // vec3f_c( 1.5f,  0.2f,  1.5f),
        // vec3f_c(-1.3f,  1.0f,  1.5f)
    };
    for (size_t j = 0; j < sizeof(cube_positions)/sizeof(cube_positions[0]); ++j)
    {
        vec3f_t tr = cube_positions[j];
        vec3f_t ro = vec3f_c((f*3)/500.0f,(f*1)/500.0f,0);
        for (uint i = 0; i < o->size_indices; i+=3)
        {
            vec3f_t ps[3]; vec2i_t psi[3];
            ps[0] = vec3f_c(o->vs[(o->indices[i+0]-1)*3 +0], o->vs[(o->indices[i+0]-1)*3 +1], o->vs[(o->indices[i+0]-1)*3 +2]);
            ps[1] = vec3f_c(o->vs[(o->indices[i+1]-1)*3 +0], o->vs[(o->indices[i+1]-1)*3 +1], o->vs[(o->indices[i+1]-1)*3 +2]);
            ps[2] = vec3f_c(o->vs[(o->indices[i+2]-1)*3 +0], o->vs[(o->indices[i+2]-1)*3 +1], o->vs[(o->indices[i+2]-1)*3 +2]);
            for (int i = 0; i < 3; ++i)
            {
                //printf("%f %f %f \n", ps[i].x, ps[i].y, ps[i].z);
                psi[i] = screen(b, project(translate(rotate(ps[i], ro), tr)));
            }
            
            triangle_t t1 = {.v1 = psi[0], .v2 = psi[1], .v3 = psi[2]};
            drawTriangle(b, t1, cubecs[i%6]);

           // drawTriangle(b, t2, cubecs[i%3]);
        }
        
        
    }
}



/*

for (int i = 0; i < 108; i+=9)
        {
            vec3f_t tr = cube_positions[j];
            //vec3f_t r = vec3f_c((float)f/500.0f,0,0);
            vec3f_t r = vec3f_c((f*3)/500.0f,(f*1)/500.0f,0);
            vec3f_t p1  = vec3f_c(cubevs[i], cubevs[i+1], cubevs[i+2]);
            vec2i_t tp1 = screen(b, project(translate(rotate(p1, r), tr)));
            vec3f_t p2  = vec3f_c(cubevs[i+3], cubevs[i+4], cubevs[i+5]);
            vec2i_t tp2 = screen(b, project(translate(rotate(p2, r), tr)));
            vec3f_t p3  = vec3f_c(cubevs[i+6], cubevs[i+7], cubevs[i+8]);
            vec2i_t tp3 = screen(b, project(translate(rotate(p3, r), tr)));
            triangle_t t = {.v1 = tp1, .v2 = tp2, .v3 = tp3};
            //drawTriangle3c(b, t, vec3u32_c(cubecs[i/3],cubecs[i/3+1],cubecs[i/3+2]));
            drawTriangle(b, t, vec3u32_c(cubecs[i/3],cubecs[i/3+1],cubecs[i/3+2]));
        }

*/

int main( int argc, char** argv )
{
    printargs(argc, argv, "Log: Args");
    window_t w = createWindow(w, 640, 580, ":fent reactor online:");
    windowi_t *wi = w.i; framei_t *fi = wi->frame.i;
    float f = 0;
    object_t cube;
    createObject(&cube);
    //printf("TEST!\n");
    //printf("%lld ", cube.size_vs);
    // printf("\n");
    // printf("\n");
    // for (size_t i = 0; i < cube.size_vs; i++)
    // {
    //     printf("%f ", cube.vs[i]);
    //     if ((i+1)%3 == 0) printf("\n");
    // }
    
    while(!wi->should_close)
    {
        f++;
        clearScreen(fi->image);
        drawObject(fi->image, &cube, f);
        break;
        displayWindow(w);   
    }
    
    freeWindow(w);
    return 0;
}