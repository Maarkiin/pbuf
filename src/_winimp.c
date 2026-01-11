#ifdef _WIN32

#include "include.h"
#include "frame.h"
#include <windows.h>




buffer_t createBuffer(uint w, uint h, uint s)
{
    buffer_t b; b.i = malloc(sizeof(bufferi_t));
    bufferi_t *bi = b.i;
    bi->height = h;
    bi->width = w;
    bi->stride = s;
    
    memset(&bi->bmi, 0, sizeof(BITMAPINFO));
    bi->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmi.bmiHeader.biWidth = w;
    bi->bmi.bmiHeader.biHeight = h;
    bi->bmi.bmiHeader.biPlanes = 1;
    bi->bmi.bmiHeader.biBitCount = s*8; // 32-bit RGBA
    bi->bmi.bmiHeader.biCompression = BI_RGB;

    bi->pixels = malloc(h*w*s);
    memset(bi->pixels, 0, h*w*s);
    return b;
}

void freeBuffer(buffer_t b)
{
    bufferi_t *bi = b.i;
    free(bi->pixels);
    free(bi);
}

frame_t createFrame(uint w, uint h)
{
    frame_t f; f.i = malloc(sizeof(framei_t));
    framei_t *fi = f.i;
    fi->image = createBuffer(w, h, sizeof(uint32_t));
    fi->z_buffer = createBuffer(w, h, sizeof(uint32_t));
    return f;
}

void freeFrame(frame_t f)
{
    framei_t *fi = f.i;
    freeBuffer(fi->image);
    freeBuffer(fi->z_buffer);
    free(fi);
}

window_t createWindow(window_t w, uint width, uint height, char *title)
{
    w.i = malloc(sizeof(windowi_t));
    windowi_t *wi = w.i;
    wi->frame = createFrame(width, height);
    memset(wi->keys, 0, sizeof(wi->keys));
    wi->should_close = 0;
    wi->handle = (HWND)makeWindow(w, width, height, title);
    //wi->msg = {0};
    return w;

}

void freeWindow(window_t w)
{
    windowi_t *wi = w.i;
    freeFrame(wi->frame);
    free(wi);
}

void displayBuffer(buffer_t b, void* hdc)
{
    bufferi_t *bi = b.i;
    StretchDIBits(
        hdc,
        0, 0, bi->width, bi->height,
        0, 0, bi->width, bi->height,
        bi->pixels,
        &bi->bmi,
        DIB_RGB_COLORS,
        SRCCOPY);
    return;
}

void resizeBuffer(buffer_t b, uint w, uint h)
{
    if(w == 0 || h == 0) return;
    bufferi_t *bi = b.i;
    bi->pixels = realloc(bi->pixels, w*h*bi->stride);
    bi->height = h;
    bi->width = w;
    bi->bmi.bmiHeader.biWidth = w;
    bi->bmi.bmiHeader.biHeight = h;
    for (uint i = 0; i < bi->width*bi->height; ++i)
    {
        ((uint32_t *)bi->pixels)[i] = 0x0091D15B;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    windowi_t *wi = (windowi_t *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!wi)
        return DefWindowProc(hWnd, msg, wParam, lParam);
    framei_t *fi = wi->frame.i;
    buffer_t img = fi->image;
    buffer_t zbuf = fi->z_buffer;
    switch (msg)
    {
    case WM_SIZE:
    {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;
        resizeBuffer(img, width, height);
        resizeBuffer(zbuf, width, height);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        displayBuffer(img, hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_CLOSE:
        wi->should_close = 1;
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

HWND makeWindowi(window_t w, uint width, uint height, char *title);
void *makeWindow(window_t w, uint width, uint height, char *title)
{
    return makeWindowi(w, width, height, title);
}

HWND makeWindowi(window_t w, uint width, uint height, char *title)
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

    struct windowEX_t {
        int width;
        int height;
        int x;
        int y;
        char *title;
    };


    struct windowEX_t myWindow;

    {
        int w = width+20;
        int h = height+43;
        myWindow.width = w;
        myWindow.height = h;
        myWindow.x = CW_USEDEFAULT;
        myWindow.y = CW_USEDEFAULT;
        myWindow.title = title;
    }

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

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)w.i);
    return hWnd;
}

void displayWindow(window_t w)
{
    windowi_t *wi = w.i;
    HWND hWnd = wi->handle;
    MSG Msg;
    InvalidateRect(hWnd, NULL, FALSE);
    while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}




#endif