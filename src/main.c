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

vec3f_t scale(vec3f_t p1, vec3f_t p2)
{
    return vec3f_c(p1.x*p2.x, p1.y*p2.y, p1.z*p2.z);
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
    ((uint32_t *)bi->pixels)[x + y * bi->width] = c;
}








void clearScreen(buffer_t b, uint32_t c)
{
    bufferi_t *bi = b.i;
    for (uint x = 0; x < bi->width; ++x)
    {
        for (uint y = 0; y < bi->height; ++y)
        {
            ((uint32_t *)bi->pixels)[x + y * bi->width] = c;
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
void drawTriangle(buffer_t b, buffer_t depth, vec3f_t zs, triangle_t t, uint32_t c)
{
    // for (int i = 0; i<3; ++i)
    // {
    //     zs.value[i] = (zs.value[i] - 0.0f) / (100.0f - 0.0f) * UINT32_MAX;
    // }

    bufferi_t *depthi = depth.i;
    vec2i_t triangle[3];
    {
        vec2i_t ap = vec2i_c(t.v1.x, t.v1.y), bp = vec2i_c(t.v2.x, t.v2.y), cp = vec2i_c(t.v3.x, t.v3.y);
        triangle[0] = ap; triangle[1] = bp; triangle[2] = cp;
        int det = edge(ap, bp, cp);
        if (det == 0 || det > 0) return; // If our triangle is 0 area or backfacing, skip it.
    }

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
                vec4i_t u;
                barycentricTriangle(t, x, y, &u);
                
                uint32_t z = (zs.x*u.x + zs.y*u.y + zs.z*u.z)/u.w;
                if ( z < ((uint32_t*)depthi->pixels)[x + y*depthi->width] )
                {
                    pixel(depth, x, y, z);
                    pixel(b, x, y, c);
                }
            }
        }
        for (int y = my; y < hy + 1; ++y)
        {
            int lx, hx;

            lx = bp.x + (y-bp.y) * (ap.x - bp.x) / (ap.y - bp.y);
            hx = cp.x + (y-cp.y) * (ap.x - cp.x) / (ap.y - cp.y);

            for (int x = lx; x < hx; ++x)
            {
                vec4i_t u;
                barycentricTriangle(t, x, y, &u);
                
                uint32_t z = (zs.x*u.x + zs.y*u.y + zs.z*u.z)/u.w;
                if ( z < ((uint32_t*)depthi->pixels)[x + y*depthi->width] )
                {
                    pixel(depth, x, y, z);
                    pixel(b, x, y, c);
                }
            }
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
                vec4i_t u;
                barycentricTriangle(t, x, y, &u);
                
                uint32_t z = (zs.x*u.x + zs.y*u.y + zs.z*u.z)/u.w;
                if ( z < ((uint32_t*)depthi->pixels)[x + y*depthi->width] )
                {
                    pixel(depth, x, y, z);
                    pixel(b, x, y, c);
                }
            }
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
                vec4i_t u;
                barycentricTriangle(t, x, y, &u);

                uint32_t z = (zs.x*u.x + zs.y*u.y + zs.z*u.z)/u.w;
                if ( z < ((uint32_t*)depthi->pixels)[x + y*depthi->width] )
                {
                    pixel(depth, x, y, z);
                    pixel(b, x, y, c);
                }
            }
        }
    }
}

typedef struct {
    /*vertices*/
    size_t  size_vs;
    float  *vs;
    /*indices*/
    size_t  size_indices;
    int    *indices;
    /*uv coords*/
    size_t  size_uvs;
    float  *uvs;
    size_t  size_uvsi;
    uint   *uvsi;
    /*normals*/
    size_t  size_ns;
    float  *ns;
    size_t  size_nsi;
    uint   *nsi;

    /*translation/rotation/scale*/
    vec3f_t tr;
    vec3f_t ro;
    vec3f_t sc;
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

void createObject(object_t *o, char* FILEPATH)
{
    o->size_vs      = 0;
    o->size_indices = 0;
    o->size_ns      = 0;
    o->size_nsi     = 0;
    o->size_uvs     = 0;
    o->size_uvsi    = 0;
    //
    const uint SIZE = 4096;
    FILE *fp;
    fp = fopen(FILEPATH, "r");
    char buf[SIZE];
    while(fgets(buf, SIZE, fp))
    {
        if (strstar(buf, "v "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "v"))
                {
                    o->size_vs++; 
                }
                p = strtok(NULL, " ");
            }
        }
        if (strstar(buf, "vn "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "vn"))
                {
                    o->size_ns++;
                }
                p = strtok(NULL, " ");
            }
        }
        if (strstar(buf, "vt "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "vt"))
                {
                    o->size_uvs++;
                }
                p = strtok(NULL, " ");
            }
        }
        if (strstar(buf, "f "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "f"))
                {
                    char *tok = strtok_r(p, "/", &p);
                    int i = 0;
                    while (tok!=NULL)
                    {
                        switch (i)
                        {
                        case 0:
                            o->size_indices++;
                        break;
                        case 1:
                            o->size_uvsi++;
                        break;
                        case 2:
                            o->size_nsi++;
                        break;  
                        }
                        tok = strtok_r(p, "/", &p);
                        i++;
                    }
                }
                p = strtok(NULL, " ");
            }
        }
    }

    o->vs           = malloc(o->size_vs*sizeof(float));
    o->size_vs      = 0;

    o->indices      = malloc(o->size_indices*sizeof(float));
    o->size_indices = 0;
    
    o->ns           = malloc(o->size_ns*sizeof(uint));
    o->size_ns      = 0;

    o->nsi          = malloc(o->size_nsi*sizeof(float));
    o->size_nsi     = 0;

    o->uvs          = malloc(o->size_uvs*sizeof(uint));
    o->size_uvs     = 0;

    o->uvsi         = malloc(o->size_uvsi*sizeof(uint));
    o->size_uvsi    = 0;

    fseek(fp, 0, SEEK_SET);
    while(fgets(buf, SIZE, fp))
    {
        if (strstar(buf, "v "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "v"))
                {
                    //printf("%s ", p);
                    o->vs[o->size_vs++] = (float)atof(p);
                    
                }
                p = strtok(NULL, " ");
            }
        }
        if (strstar(buf, "vn "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "vn"))
                {
                    //printf("%s ", p);
                    o->ns[o->size_ns++] = (float)atof(p);
                    
                }
                p = strtok(NULL, " ");
            }
        }
        if (strstar(buf, "f "))
        {
            char *p = strtok(buf, " ");
            while (p!=NULL)
            {
                if (!strstar(p, "f"))
                {
                    char *tok = strtok_r(p, "/", &p);
                    int i = 0;
                    while (tok!=NULL)
                    {
                        switch (i)
                        {
                        case 0:
                            o->indices[o->size_indices++] = (uint)atoi(tok);
                        break;
                        case 1:
                            o->uvsi[o->size_uvsi++] = (uint)atoi(tok);
                        break;
                        case 2:
                            o->nsi[o->size_nsi++]   = (uint)atoi(tok);
                        break;  
                        }
                        tok = strtok_r(p, "/", &p);
                        i++;
                    }
                }
                p = strtok(NULL, " ");
            }
        }
    }
    fclose(fp);
}

void drawObject(buffer_t b, buffer_t d, object_t* o)
{ 
    for (uint i = 0; i < o->size_indices; i+=3)
    {
        vec3f_t ps[3]; vec2i_t psi[3];
        vec3f_t zs;
        ps[0] = vec3f_c(o->vs[(o->indices[i+0]-1)*3 +0], o->vs[(o->indices[i+0]-1)*3 +1], o->vs[(o->indices[i+0]-1)*3 +2]);
        ps[1] = vec3f_c(o->vs[(o->indices[i+1]-1)*3 +0], o->vs[(o->indices[i+1]-1)*3 +1], o->vs[(o->indices[i+1]-1)*3 +2]);
        ps[2] = vec3f_c(o->vs[(o->indices[i+2]-1)*3 +0], o->vs[(o->indices[i+2]-1)*3 +1], o->vs[(o->indices[i+2]-1)*3 +2]);
        for (int i = 0; i < 3; ++i)
        {
            //printf("%f %f %f \n", ps[i].x, ps[i].y, ps[i].z);
            vec3f_t p3d = translate(rotate(scale(ps[i], o->sc), o->ro), o->tr);
            zs.value[i] = p3d.z;
            psi[i] = screen(b, project(p3d));
        }
        
        triangle_t t1 = {.v1 = psi[0], .v2 = psi[1], .v3 = psi[2]};

        uint32_t color = 0;
        uint32_t A = 0xFF;
        uint32_t R = o->ns[(o->nsi[i]-1)*3 + 0] * 0xFF;
        uint32_t G = o->ns[(o->nsi[i]-1)*3 + 1] * 0xFF;
        uint32_t B = o->ns[(o->nsi[i]-1)*3 + 2] * 0xFF;
        color = (B&0xFF) | (G&0xFF) << 8 | (R&0xFF) << 16 | (A&0xFF) << 24;



        drawTriangle(b, d, zs, t1, color);
    }
}

int main( int argc, char** argv )
{
    printargs(argc, argv, "Log: Args");
    window_t w = createWindow(w, 640, 580, ":fent reactor online:");
    windowi_t *wi = w.i; framei_t *fi = wi->frame.i;
    if (argc < 2) wi->showZbuf = 0;
    else wi->showZbuf = atoi(argv[1]);
    float f = 0;
    object_t monkey;
    createObject(&monkey, "res/monkey.obj");
    monkey.tr = vec3f_c( 2.0f,  5.0f,  15.0f);
    monkey.sc = vec3f_c( 2.0f,  2.0f,  2.0f);
    object_t mill;
    createObject(&mill, "res/Low Poly Mill.obj");
    mill.tr = vec3f_c(0.0f, -2.0f, 15.0f);
    mill.sc = vec3f_c(10.0f, 10.0f, 10.0f);
    
    while(!wi->should_close)
    {
        f++;
        clearScreen(fi->image, 0x00303030);
        clearScreen(fi->z_buffer, 0xFFFFFFFF);
        monkey.ro = vec3f_c((f*3)/500.0f,(f*1)/500.0f,0);
        drawObject(fi->image, fi->z_buffer, &monkey);
        mill.ro = vec3f_c(0.0f, f/1500.0f, 0.0f);
        drawObject(fi->image, fi->z_buffer, &mill);
        //break;
        displayWindow(w);   
    }
    
    freeWindow(w);
    return 0;
}