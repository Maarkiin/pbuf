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
    int close;
} buffer_t;

void CreateBuffer(buffer_t *buf, u_int width, u_int height)
{
    buf->close = 0;
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
    buffer_t *buf = (buffer_t *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!buf)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_SIZE:
    {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;
        free(buf->pixels);
        CreateBuffer(buf, width, height);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DisplayBuffer(hdc, buf);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_CLOSE:
        buf->close = 1;
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

/*

const vs = [
    {x:  0.25, y:  0.25, z:  0.25},
    {x: -0.25, y:  0.25, z:  0.25},
    {x: -0.25, y: -0.25, z:  0.25},
    {x:  0.25, y: -0.25, z:  0.25},

    {x:  0.25, y:  0.25, z: -0.25},
    {x: -0.25, y:  0.25, z: -0.25},
    {x: -0.25, y: -0.25, z: -0.25},
    {x:  0.25, y: -0.25, z: -0.25},
]

const fs = [
    [0, 1, 2, 3],
    [4, 5, 6, 7],
    [0, 4],
    [1, 5],
    [2, 6],
    [3, 7],
]

*/
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


// const vec3f_t cube_vs[] =
//     {
//         vec3f_c(-0.25f,  0.25f,  0.25f),
//         vec3f_c( 0.25f,  0.25f,  0.25f),
//         vec3f_c(-0.25f, -0.25f,  0.25f),
//         vec3f_c( 0.25f, -0.25f,  0.25f),

//         vec3f_c( 0.25f,  0.25f, -0.25f),
//         vec3f_c(-0.25f,  0.25f, -0.25f),
//         vec3f_c(-0.25f, -0.25f, -0.25f),
//         vec3f_c( 0.25f, -0.25f, -0.25f)
// };

// const int cube_is[] =
//     {
//         0,1,2,

// };

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
// return vec3f_c(
//     p.x*c.y-p.z*s.y,
//     p.y,
//     p.x*s.y+p.z*c.y
// );

/*

for x in range(screen_width):
    ys = []
    for (p0, p1) in segments:
        if segment_crosses_x(p0, p1, x):
            y = interpolate_y(p0, p1, x)
            ys.append(y)

    ys.sort()
    for i in range(0, len(ys), 2):
        draw_vertical_line(x, ys[i], ys[i+1])



*/

int edge(vec2f_t a, vec2f_t b, vec2f_t c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

typedef struct tri {
    vec2f_t points[3];
} triangle_t;

int intriangle(triangle_t t, vec2f_t p)
{
    int a = edge(t.points[0], t.points[1], p);
    int b = edge(t.points[1], t.points[2], p);
    int c = edge(t.points[2], t.points[0], p);
    if(a>0 && b>0 && c>0)
        return 1;
    return 0;
}

void GameLoop(buffer_t *buf, uint32_t frame)
{
    // vec2f_t p = screen(*buf, project(vec3f_c(0.5f, -1.0f, 1.0f)));
    // point(buf, p, 0x00ffa500);
    // printf("(%f/%d, %f/%d)\n", p.x, buf->width, p.y, buf->height);
    //printf("%d\n", frame);
    
    int s = (int)(sizeof(cube_vs)/sizeof(cube_vs[0]));
    vec2f_t points[s];
    for (int i = 0; i < s; ++i)
    {
        float t = (float)frame/1000;
        vec3f_t p = translate(rotate(cube_vs[i], vec3f_c(t*3.0f, t, 0.0f)), vec3f_c(0.0f, 0.0f, 2.0f));
        float brightness = 1 - (p.z/ 20.0f);
        uint32_t color = 0x00ffa500;
        uint8_t a = (color >> 24) & 0xFF;
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        r = (uint8_t)(r * brightness > 255 ? 255 : r * brightness);
        g = (uint8_t)(g * brightness > 255 ? 255 : g * brightness);
        b = (uint8_t)(b * brightness > 255 ? 255 : b * brightness);
        
        color  = (a << 24) | (r << 16) | (g << 8) | b;
        
        
        point(buf, screen(*buf, project(p)), color);
        points[i] = screen(*buf, project(p));
    }
    // for each triangle ...
    for (int i = 0; i < s; i+=3)
    {
        triangle_t t = {{
            points[i],
            points[i+1],
            points[i+2],
        }};

        for (u_int x = 0; x < buf->width; x++)
        {
            for (u_int y = 0; y < buf->height; y++)
            {
                vec2f_t p = vec2f_c(x, y);
                if (intriangle(t, p))
                {
                    point(buf, p, 0x00ffa500);
                }
            }
        }
        
    }
}

/*

// Let's assume we have a Point type with an x and y property
interface Point {
    x: number;
    y: number;
    }
    
    // Returns double the signed area but that's fine
    const edgeFunction = (a: Point, b: Point, c: Point) => {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
};

const ABC = edgeFunction(A, B, C)
*/

int main(int argc UNUSED, char **argv UNUSED)
{

    HWND hWnd = createWindow(500 + 20, 500 + (500 - 457), "hallo :)");

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    buffer_t buf = {0};
    CreateBuffer(&buf, width, height);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&buf);
    /**/
    uint32_t frame = 0;
    /**/
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    MSG Msg;
    // while (GetMessage(&Msg, hWnd, 0, 0) > 0)
    // {
    //     clearBuffer(&buf, 0x00065535);
    //     GameLoop(&buf, frame++);

    //     InvalidateRect(hWnd, NULL, FALSE);
    //     TranslateMessage(&Msg);
    //     DispatchMessage(&Msg);
    // }
    while (!buf.close)
    {
        clearBuffer(&buf, 0x00065535);
        GameLoop(&buf, frame++);

        InvalidateRect(hWnd, NULL, FALSE);
        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    DestroyWindow(hWnd);
    free(buf.pixels);
    return Msg.wParam;
}
