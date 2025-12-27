#include "include.h"
#include <windows.h>
#include <math.h>

typedef struct BUFFER_T
{
    void *pixels;
    u_int width;
    u_int height;
    int pitch;

    BITMAPINFO bmi;
} buffer_t;

typedef struct renderer {
    buffer_t **bufs;
    size_t buf_count;

    int close;
} renderer_t;


void CreateBuffer(buffer_t *buf, u_int width, u_int height)
{
    buf->width = width;
    buf->height = height;

    buf->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    buf->bmi.bmiHeader.biWidth = width;
    buf->bmi.bmiHeader.biHeight = height;
    buf->bmi.bmiHeader.biPlanes = 1;
    buf->bmi.bmiHeader.biBitCount = 32; // 32-bit RGBA
    buf->bmi.bmiHeader.biCompression = BI_RGB;

    buf->pixels = calloc(width * height, sizeof(uint32_t));
    buf->pitch = width * 4;
}

void DisplayBuffer(HDC hdc, buffer_t *buf)
{
    StretchDIBits(
        hdc,
        0, 0, buf->width, buf->height,
        0, 0, buf->width, buf->height,
        buf->pixels,
        &buf->bmi,
        DIB_RGB_COLORS,
        SRCCOPY);
    return;
}

void clearBuffer(buffer_t *buf, uint32_t color)
{
    uint32_t *pixel = (uint32_t *)buf->pixels;
    for (u_int x = 0; x < buf->width; ++x)
    {
        for (u_int y = 0; y < buf->height; ++y)
        {
            pixel[x + y * buf->width] = color; // AARRGGBB
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    renderer_t *r = (renderer_t *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!r)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_SIZE:
    {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;
        free(r->bufs[0]->pixels);
        free(r->bufs[1]->pixels);
        CreateBuffer(r->bufs[0], width, height);
        CreateBuffer(r->bufs[1], width, height);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DisplayBuffer(hdc, r->bufs[0]);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_CLOSE:
        r->close = 1;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

HWND createWindow(u_int width, u_int height, char *title)
{
    HINSTANCE hInst = GetModuleHandle(NULL);

    // Register window class

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }

    struct windowEX_t
    {
        int width;
        int height;
        int x;
        int y;
        char *title;
    };

    struct windowEX_t myWindow;
    myWindow.width = width;
    myWindow.height = height;
    myWindow.x = CW_USEDEFAULT;
    myWindow.y = CW_USEDEFAULT;
    myWindow.title = title;

    HWND hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        wc.lpszClassName,
        myWindow.title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        myWindow.x, myWindow.y, myWindow.width, myWindow.height,
        NULL, NULL, hInst, NULL);

    if (hWnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }

    return hWnd;
}

void drawLine(buffer_t *buf, u_int x, u_int y1, u_int y2, uint32_t color)
{
    uint32_t *pixel = (uint32_t *)buf->pixels;
    if (y1 > y2)
    {
        y1 ^= y2;
        y2 ^= y1;
        y1 ^= y2;
    }

    if (y1 > buf->height)
        y1 = buf->height;

    if (y2 > buf->height)
        y2 = buf->height;

    if (x > buf->width)
        x = buf->width;

    for (u_int i = y1; i < y2; ++i)
    {
        pixel[x + i * buf->width] = color; // AARRGGBB
    }
}

typedef struct vec3f {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float value[3];
    };
} vec3f_t;
#define vec3f_c(x, y, z) (vec3f_t){{{x, y, z}}}

typedef struct vec2{ 
    union {
        struct {
            int x;
            int y;
        };
        int value[2];
    };
} vec2_t;
#define vec2_c(x, y) (vec2_t){{{x, y}}}

typedef struct vec2f {
    union {
        struct {
            float x;
            float y;
        };
        float value[2];
    };
} vec2f_t;
#define vec2f_c(x, y) (vec2f_t){{{x, y}}}

void point(buffer_t *b, vec2f_t p, uint32 color)
{
    uint32_t *pixel = (uint32_t *)b->pixels;
    vec2_t p_i = vec2_c((int)p.x, (int)p.y);
    pixel[p_i.x + p_i.y * b->width] = color;
    return;
}

vec2f_t screen(buffer_t b, vec2f_t p)
{
    return vec2f_c(
        (p.x + 1) / 2 * b.width,
        (p.y + 1) / 2 * b.height);
}

vec2f_t project(vec3f_t p)
{
    if (p.z == 0)
        return vec2f_c(-1, -1);
    return vec2f_c(p.x / p.z, p.y / p.z);
}

typedef struct object {
    /* Verticies */
    vec3f_t *vs;
    size_t count;

    /* Translate */
    vec3f_t tr;
    /* Rotate */
    vec3f_t ro;
} object_t;

const vec3f_t cube_vs[] = {
    // BACK  (-Z)
    vec3f_c( 0.5f, -0.5f, -0.5f),
    vec3f_c(-0.5f, -0.5f, -0.5f),
    vec3f_c(-0.5f,  0.5f, -0.5f),

    vec3f_c(-0.5f,  0.5f, -0.5f),
    vec3f_c( 0.5f,  0.5f, -0.5f),
    vec3f_c( 0.5f, -0.5f, -0.5f),

    // FRONT (+Z)
    vec3f_c(-0.5f, -0.5f,  0.5f),
    vec3f_c( 0.5f, -0.5f,  0.5f),
    vec3f_c( 0.5f,  0.5f,  0.5f),

    vec3f_c( 0.5f,  0.5f,  0.5f),
    vec3f_c(-0.5f,  0.5f,  0.5f),
    vec3f_c(-0.5f, -0.5f,  0.5f),

    // LEFT (-X)
    vec3f_c(-0.5f, -0.5f, -0.5f),
    vec3f_c(-0.5f, -0.5f,  0.5f),
    vec3f_c(-0.5f,  0.5f,  0.5f),

    vec3f_c(-0.5f,  0.5f,  0.5f),
    vec3f_c(-0.5f,  0.5f, -0.5f),
    vec3f_c(-0.5f, -0.5f, -0.5f),

    // RIGHT (+X)
    vec3f_c( 0.5f, -0.5f,  0.5f),
    vec3f_c( 0.5f, -0.5f, -0.5f),
    vec3f_c( 0.5f,  0.5f, -0.5f),

    vec3f_c( 0.5f,  0.5f, -0.5f),
    vec3f_c( 0.5f,  0.5f,  0.5f),
    vec3f_c( 0.5f, -0.5f,  0.5f),

    // BOTTOM (-Y)
    vec3f_c(-0.5f, -0.5f, -0.5f),
    vec3f_c( 0.5f, -0.5f, -0.5f),
    vec3f_c( 0.5f, -0.5f,  0.5f),

    vec3f_c( 0.5f, -0.5f,  0.5f),
    vec3f_c(-0.5f, -0.5f,  0.5f),
    vec3f_c(-0.5f, -0.5f, -0.5f),

    // TOP (+Y)
    vec3f_c(-0.5f,  0.5f,  0.5f),
    vec3f_c( 0.5f,  0.5f,  0.5f),
    vec3f_c( 0.5f,  0.5f, -0.5f),

    vec3f_c( 0.5f,  0.5f, -0.5f),
    vec3f_c(-0.5f,  0.5f, -0.5f),
    vec3f_c(-0.5f,  0.5f,  0.5f),
};

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

int edge(vec2f_t a, vec2f_t b, vec2f_t c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

typedef struct tri {
    vec2f_t points[3];
} triangle_t;

int intriangle(triangle_t t, vec2f_t p, int dir)
{
    int a = edge(t.points[0], t.points[1], p);
    int b = edge(t.points[1], t.points[2], p);
    int c = edge(t.points[2], t.points[0], p);
    if (dir)
    {
        if(a<0 && b<0 && c<0)
            return 1;
        return 0;
    }
    if(a>0 && b>0 && c>0)
        return 1;
    return 0;
}

void RenderObject(renderer_t *r, object_t o)
{
    int s = o.count;
    vec2f_t points[s];
    vec3f_t points3d[s];
    for (int i = 0; i < s; ++i)
    {
        vec3f_t p = translate(rotate(o.vs[i], o.ro), o.tr);
        points3d[i] = p;
        points[i] = screen(*r->bufs[0], project(p));
    }
    // for each triangle ...
    const uint32_t COLORS[6] = {
        0x004A7BD9,
        0x00FA6C4E,
        0x0091D15B,
        0x00EB5AC1,
        0x006F48C9,
        0x00F2D04B
    };
    for (int i = 0; i < s; i+=3)
    {
        uint32_t color = COLORS[i/6];
        triangle_t t = {{
            points[i],
            points[i+1],
            points[i+2],
        }};
        /*
            TODO: interperlate for per pixel z value for buffer.
        */

        vec2_t mins = vec2_c(
            max(min(min(t.points[0].x, t.points[1].x), t.points[2].x), 0),
            max(min(min(t.points[0].y, t.points[1].y), t.points[2].y), 0)
        );
        vec2_t maxs = vec2_c(
            min(max(max(t.points[0].x, t.points[1].x), t.points[2].x), r->bufs[0]->width),
            min(max(max(t.points[0].y, t.points[1].y), t.points[2].y), r->bufs[0]->height)
        );

        //printf("x:%d->%d\ny:%d->%d\n[tri:%d]\n", mins.x, maxs.x, mins.y, maxs.y, i);

        for (int x = mins.x; x < maxs.x; x++)
        {
            for (int y = mins.y; y < maxs.y; y++)
            {
                vec2f_t p = vec2f_c(x, y);
                if (intriangle(t, p, 1))
                {
                    
                    if ( ((uint32_t *)r->bufs[1]->pixels)[(int)p.x + (int)p.y * r->bufs[1]->width] < points3d[i].z*0xFF00 )
                    {
                        point(r->bufs[1], p, points3d[i].z*0xFF00);
                        point(r->bufs[0], p, color); //0x00ffa500  
                    }
                }
            }
        }
        
    }
}


void GameLoop(renderer_t *r, uint32_t frame)
{
    object_t cube;
    cube.vs = malloc(sizeof(cube_vs));
    memcpy(cube.vs, cube_vs, sizeof(cube_vs));
    cube.count = sizeof(cube_vs)/sizeof(cube_vs[0]);
    float t = (float)frame/1000;
    cube.ro = vec3f_c(t*3.0f, t, 0.0f);

    const vec3f_t cube_positions[] = {
        //vec3f_c( 0.0f,  0.0f,  0.0f),
        vec3f_c( 2.0f,  5.0f,  15.0f),
        vec3f_c(-1.5f, -2.2f,  2.5f),
        vec3f_c(-3.8f, -2.0f,  12.3f),
        vec3f_c( 2.4f, -0.4f,  3.5f),
        vec3f_c(-1.7f,  3.0f,  7.5f),
        vec3f_c( 1.3f, -2.0f,  2.5f),
        vec3f_c( 1.5f,  2.0f,  2.5f),
        vec3f_c( 1.5f,  0.2f,  1.5f),
        vec3f_c(-1.3f,  1.0f,  1.5f)
    };

    for (size_t i = 0; i < sizeof(cube_positions)/sizeof(cube_positions[0]); i++)
    {
        cube.tr = cube_positions[i];
        RenderObject(r, cube);
    }
    
    // cube.tr = vec3f_c(0.0f, 0.0f, 2.0f);

    // RenderObject(r, cube);

    // cube.tr = vec3f_c(0.0f, 5.0f, 5.0f);
    // RenderObject(r, cube);
    free(cube.vs);
}

int main(int argc UNUSED, char **argv UNUSED)
{

    HWND hWnd = createWindow(500 + 20, 500 + (500 - 457), "hallo :)");

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;


    renderer_t r = {0};
    r.close = 0;
    r.buf_count = 2;
    r.bufs = malloc(r.buf_count*sizeof(buffer_t*));
    r.bufs[0] = malloc(sizeof(buffer_t));
    CreateBuffer(r.bufs[0], width, height);
    r.bufs[1] = malloc(sizeof(buffer_t));
    CreateBuffer(r.bufs[1], width, height);

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&r);

    /**/
    uint32_t frame = 0;
    /**/
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    MSG Msg;
    while (!r.close)
    {
        clearBuffer(r.bufs[0], 0x00065535);
        clearBuffer(r.bufs[1], 0x00000000);
        GameLoop(&r, frame++);

        InvalidateRect(hWnd, NULL, FALSE);
        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    DestroyWindow(hWnd);
    free(r.bufs[0]->pixels);
    free(r.bufs[0]);
    free(r.bufs[1]->pixels);
    free(r.bufs[1]);
    free(r.bufs);
    return Msg.wParam;
}
