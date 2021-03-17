#include<iostream>
#include<cmath>
#include<cstdlib>
#include<tchar.h>
#include<windows.h>
#include<assert.h>
#include"myshader.h"
#include<vector>

#define PI 3.1415926f
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#define RENDER_MODE_WIREFRAME 1
#define RENDER_MODE_TEXTURE 2
#define RENDER_MODE_COLOR 4

int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0; //screen间距

static LRESULT ScreenEvents(HWND hWnd, UINT msg,WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE: screen_exit = 1; break;
        case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
        case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
        default: return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
Vertex_t mesh[8] = {
	{  -1, -1,  1, 1 ,  0, 0 ,  1.0f, 0.2f, 0.2f , 1 },
	{   1, -1,  1, 1 ,  0, 1 ,  0.2f, 1.0f, 0.2f , 1 },
	{   1,  1,  1, 1 ,  1, 1 ,  0.2f, 0.2f, 1.0f , 1 },
	{  -1,  1,  1, 1 ,  1, 0 ,  1.0f, 0.2f, 1.0f , 1 },
	{  -1, -1, -1, 1 ,  0, 0 ,  1.0f, 1.0f, 0.2f , 1 },
	{   1, -1, -1, 1 ,  0, 1 ,  0.2f, 1.0f, 1.0f , 1 },
	{   1,  1, -1, 1 ,  1, 1 ,  1.0f, 0.3f, 0.3f , 1 },
	{  -1,  1, -1, 1 ,  1, 0 ,  0.2f, 1.0f, 0.3f , 1 },
};

int InArea(int num, int min, int max)
{//处理颜色范围0~255区间
    return (num < min) ? min : ((num > max) ? max : num);
}

Matrix_t SetView(const Vector_t& eye, const Vector_t& at, const Vector_t& up)
{//左手系坐标,求出相机的坐标系
    Vector_t xaxis,yaxis,zaxis;
    zaxis = at - eye;
    zaxis.NormalizeSelf();
    xaxis = up.Cross(zaxis);
    xaxis.NormalizeSelf();
    yaxis = zaxis.Cross(xaxis);
    Matrix_t temp;
    temp.matrix_t[0][0] = xaxis.x;
    temp.matrix_t[1][0] = xaxis.y;
    temp.matrix_t[2][0] = xaxis.z;
    temp.matrix_t[3][0] = -xaxis.Product(eye);

    temp.matrix_t[0][1] = yaxis.x;
    temp.matrix_t[1][1] = yaxis.y;
    temp.matrix_t[2][1] = yaxis.z;
    temp.matrix_t[3][1] = -yaxis.Product(eye);

    temp.matrix_t[0][2] = zaxis.x;
    temp.matrix_t[1][2] = zaxis.y;
    temp.matrix_t[2][2] = zaxis.z;
    temp.matrix_t[3][2] = -zaxis.Product(eye);

    temp.matrix_t[0][3] = temp.matrix_t[1][3] = temp.matrix_t[2][3] = 0.0f;
    temp.matrix_t[3][3] = 1.0f;

    return temp;
}

Matrix_t SetPerspective(float fovy, float aspect, float zn, float zf)
{
    //下面是DirectX3D的左手坐标系转换
    //fovy是视角大小，aspect是屏幕长宽比,zn是靠近的z轴,zf是远离的z轴
    float fax = 1.0f / (float)tan(fovy*0.5f);
    Matrix_t res;
    res.SetZero();
    res.matrix_t[0][0] = fax/aspect;
    res.matrix_t[1][1] = fax;
    res.matrix_t[2][2] = zf/(zf-zn);
    res.matrix_t[2][3] = 1.0f;
    res.matrix_t[3][2] = - zn*zf /(zf-zn);
    return res;
}


typedef struct { 
	Matrix_t world;         // 世界坐标变换
	Matrix_t view;          // 摄影机坐标变换
	Matrix_t projection;    // 投影变换
	Matrix_t transform;     // transform = world * view * projection
	float width, height;             // 屏幕大小
}	Transform_t;


void MVPTrans(Transform_t &trans)
{
    //进行mvp变化投影到[-1,1]范围的立方体空间
    Matrix_t temp;
    temp = trans.world.Muti(trans.view);
    temp = temp.Muti(trans.projection);
    trans.transform = temp;
}

void MVPInit(Transform_t &trans, int width, int height)
{
    //初始化MVP变换需要用到的矩阵
    float aspect = (float)width / (float)height;
    trans.world.SetIndentity();
    trans.view.SetIndentity();
    //Π = 180°角 , 设置相机的视角为90°
    trans.projection = SetPerspective(PI * 0.5f, aspect, 1.0f, 500.0f);
    trans.width = width;
    trans.height = height;
    MVPTrans(trans);
}

Vector_t VectorTrans(Transform_t &trans, Vector_t v)
{//将向量投影到立方体
    Vector_t res = v.MatrixMuti(trans.transform);
    return res;
}

bool CheckCVV(const Vector_t& v)
{//DirectX3D的z的范围[0,1];
    //最后要除以w,范围要落在[-1,1],因此绝对值比w大的要进行空间上的裁剪,不给予显示
    float w = v.w;
    if(v.z < 0.0f || v.z > w || v.x < -w || v.x > w || v.y < -w || v.y > w)
        return true;
    return false;
}

Vector_t ToScreen(const Transform_t& ts, const Vector_t& v)
{
    //将立方体的图形映射到屏幕上的点,将向量变为单位向量
    Vector_t res;
    float ratio = 1.0f / v.w;
    //y = 1/2 x + 1/2 直线等比例放缩到屏幕，这是宽度x的函数[-1,1]对应[0,width]
    res.x = ts.width * (v.x * ratio + 1.0f) * 0.5f;
    //y = -1/2 x + 1/2 这是高度y的函数[-1,1] 对应 [height, 0],左下角为原点(0,0)
    res.y = ts.height * (1.0f - v.y * ratio) * 0.5f;
    res.z = v.z * ratio;
    res.w = 1.0f;
    return res;
}

class Device_t{
public:
    Transform_t transform; //坐标变换器
    int wwidth; //窗口高度
    int wheight; //窗口宽度
    unsigned int **framebuffer; //像素缓存
    float **zbuffer; //深度缓存
    unsigned int **texture; //纹理缓存
    int tex_width;//纹理宽度
    int tex_height; //纹理高度
    float max_u; //纹理最大宽度
    float max_v; //纹理最大高度
    int render_status; //当前渲染模式
    int background; //背景色
    int foreground; //前景色

    

    Device_t() = default;

    Device_t(int width, int height, void* fb)
    {
        //fb是外部帧缓存,非NULL可以使用
        int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
        char *ptr = new char[need + 64];
        char *framebuf, *zbuf;
        int j;
        assert(ptr);
        this->framebuffer = (unsigned int**)ptr;
        this->zbuffer = (float**)(ptr + sizeof(void*) * height);
        ptr += sizeof(void*) * height * 2;
        this->texture = (unsigned int **)ptr;
        ptr += sizeof(void*) * 1024;
        framebuf = (char*)ptr;
        zbuf = (char*)ptr + width * height * 4;
        ptr += width * height * 8;
        if(fb != NULL) framebuf = (char*)fb;
        for(j = 0; j < height; j++)
        {
            this->framebuffer[j] = (unsigned int*)(framebuf + width * 4 * j);
            this->zbuffer[j] = (float*)(zbuf + width * 4 * j);
        }
        this->texture[0] = (unsigned int*)ptr;
        this->texture[1] = (unsigned int*)(ptr + 16);
        memset(this->texture[0], 0, 64);
        this->tex_width = 2;
        this->tex_height = 2;
        this->max_u = 1.0f;
        this->max_v = 1.0f;
        this->wwidth = width;
        this->wheight = height;
        this->background = 0x0c0c0;
        this->foreground = 0;
        MVPInit(this->transform, this->wwidth, this->wheight);
        this->render_status = RENDER_MODE_WIREFRAME;
    }

    ~Device_t()
    {
        if(this->framebuffer)
            delete [] this->framebuffer;
            // free(this->framebuffer);
        this->framebuffer = NULL;
        this->zbuffer = NULL;
        this->texture = NULL;
    }

    void SetTexture(void *bits, long pitch, int w, int h)
    {
        char *ptr = (char*)bits;
        int j;
        assert(w <= 1024 && h <= 1024);
        for(j = 0; j < h; ptr += pitch, j++)
        {
            this->texture[j] = (unsigned int*)ptr;
        }
        this->tex_width = w;
        this->tex_height = h;
        this->max_u = (float)(w-1);
        this->max_v = (float)(h-1);
    }

    void Clear(int mode)
    {
        //重新改变渲染模式
        int y,x;
        int height = this->wheight;
        for(y = 0; y < this->wheight; y++)
        {
            unsigned int* dst = this->framebuffer[y];
            unsigned int cc = (height - 1 - y) * 230 / (height - 1);
            cc = (cc << 16) | (cc << 8) | cc;
            if(mode == 0) cc = this->background;
            for(x = this->wwidth; x > 0; dst++, x--) dst[0] = cc;
        }
        for(y = 0; y < this->wheight; y++)
        {
            float *dst = this->zbuffer[y];
            for(x = this->wwidth; x > 0; dst++, x--) dst[0] = 0.0f;
        }
    }

    void DrawPixel(int x, int y, unsigned int color)
    {
        if((unsigned int)x < this->wwidth && (unsigned int)y < this->wheight)
        {
            this->framebuffer[y][x] = color;
        }
    }

    void DrawLine(int x1, int y1, int x2, int y2, unsigned int c)
    {
        //Bresenham绘制方法,网上搜索实现方法
        int x, y, rem = 0;
        if (x1 == x2 && y1 == y2) {
            DrawPixel( x1, y1, c);
        }	else if (x1 == x2) {
            int inc = (y1 <= y2)? 1 : -1;
            for (y = y1; y != y2; y += inc) DrawPixel( x1, y, c);
            DrawPixel( x2, y2, c);
        }	else if (y1 == y2) {
            int inc = (x1 <= x2)? 1 : -1;
            for (x = x1; x != x2; x += inc) DrawPixel( x, y1, c);
            DrawPixel( x2, y2, c);
        }	else {
            int dx = (x1 < x2)? x2 - x1 : x1 - x2;
            int dy = (y1 < y2)? y2 - y1 : y1 - y2;
            if (dx >= dy) {
                //更靠近x轴
                if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
                for (x = x1, y = y1; x <= x2; x++) {
                    DrawPixel( x, y, c);
                    rem += dy;
                    if (rem >= dx) {
                        rem -= dx;
                        y += (y2 >= y1)? 1 : -1;
                        DrawPixel( x, y, c);
                    }
                }
                DrawPixel( x2, y2, c);
            }	else {
                //更靠近y轴
                if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
                for (x = x1, y = y1; y <= y2; y++) {
                    DrawPixel( x, y, c);
                    rem += dx;
                    if (rem >= dy) {
                        rem -= dy;
                        x += (x2 >= x1)? 1 : -1;
                        DrawPixel( x, y, c);
                    }
                }
                DrawPixel( x2, y2, c);
            }
        }
    }

    unsigned int TextureRead(float u, float v)
    {//读取纹理
        int x, y;
        u = u * this->max_u;
        v = v * this->max_v;
        x = (int)(u + 0.5f);
        y = (int)(v + 0.5f);
        x = InArea(x, 0, this->tex_width - 1);
        y = InArea(y, 0, this->tex_height - 1);
        return this->texture[y][x];
    }

    void DrawScanLine(Scanline_t & scan)
    {
        unsigned int *framebuffer = this->framebuffer[scan.y];
        float *zbuffer = this->zbuffer[scan.y];
        int x = scan.x;
        int w = scan.width;
        int width = this->wwidth;
        int render_state = this->render_status;
        for(; w > 0; x++, w--)
        {
            if(x >= 0 && x < width)
            {
                float rhw = scan.v.rhw;
                //z比较小的保存,因为rhw = 1/z,所以rhw大的保存
                if(rhw >= zbuffer[x])
                {
                    float w = 1.0f / rhw;
                    zbuffer[x] = rhw;//rhw代表空间坐标的z
                    if(render_state & RENDER_MODE_COLOR)
                    {
                        float r = scan.v.r * w;
                        float g = scan.v.g * w;
                        float b = scan.v.b * w;
                        int R = (int)(r * 255.0f);
                        int G = (int)(g * 255.0f);
                        int B = (int)(b * 255.0f);
                        R = InArea(R, 0, 255);
                        G = InArea(G, 0, 255);
                        B = InArea(B, 0, 255);
                        framebuffer[x] = (R << 16) | (G << 8) | (B);
                    }
                    if(render_state & RENDER_MODE_TEXTURE)
                    {
                        float u = scan.v.u * w;
                        float v = scan.v.v * w;
                        unsigned int cc = TextureRead(u, v);
                        framebuffer[x] = cc;
                        // std::cout << cc << std::endl;
                    }
                }
            }
            scan.v = scan.v + scan.step;
            if(x >= width) break;
        }
    }

    void MainRender(Trapezoid_t & trap)
    {
        Scanline_t scan;
        int j, top, bottom;
        top = (int)(trap.top + 0.5f);
        bottom = (int)(trap.bottom + 0.5f);
        for(j = top; j < bottom; j++)
        {
            if(j >= 0 && j < this->wheight)
            {
                trap.YinEdge((float)j + 0.5f);
                trap.InitScanLine(scan, j);
                DrawScanLine(scan);
            }
            if(j >= this->wheight) break;
        }
    }

    Vector_t VertoVec(const Vertex_t & v)
    {
        Vector_t res;
        res.x = v.x;
        res.y = v.y;
        res.z = v.z;
        res.w = v.w;
        return res;
    }

    void DrawTriangle(const Vertex_t & v1, const Vertex_t & v2, const Vertex_t & v3)
    {
        Vector_t p1,p2,p3,c1,c2,c3,d1,d2,d3;
        int render_state = this->render_status;
        d1 = VertoVec(v1);
        d2 = VertoVec(v2);
        d3 = VertoVec(v3);
        c1 = this->transform.transform.VectorMuti(d1);
        c2 = this->transform.transform.VectorMuti(d2);
        c3 = this->transform.transform.VectorMuti(d3);
        //超出视角范围不进行渲染
        if(CheckCVV(c1) || CheckCVV(c2) || CheckCVV(c3))
            return ;
        p1 = ToScreen(this->transform, c1);
        p2 = ToScreen(this->transform, c2);
        p3 = ToScreen(this->transform, c3);
        //绘制色彩或纹理
        if(render_state & (RENDER_MODE_COLOR | RENDER_MODE_TEXTURE))
        {
            Vertex_t t1 = v1, t2 = v2, t3 = v3;
            Trapezoid_t traps[2];
            int n;
            t1.x = p1.x; t1.y = p1.y; t1.z = p1.z; t1.w = c1.w;
            t2.x = p2.x; t2.y = p2.y; t2.z = p2.z; t2.w = c2.w;
            t3.x = p3.x; t3.y = p3.y; t3.z = p3.z; t3.w = c3.w;
            // std::cout << t1.x << " " << t1.y << " " << t1.z << " " << t1.w <<std::endl;
            //初始化顶点
            t1.Init();
            t2.Init();
            t3.Init();
            n = CountTriangle(traps, t1, t2, t3);
            if(n >= 1) MainRender(traps[0]);
            if(n >= 2) MainRender(traps[1]);
        }

        if(render_state & RENDER_MODE_WIREFRAME)
        {
            DrawLine((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, this->foreground);
            DrawLine((int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, this->foreground);
            DrawLine((int)p3.x, (int)p3.y, (int)p2.x, (int)p2.y, this->foreground);
        }
    }

    void DrawPlane(int a, int b, int c, int d) {
        //一个平面由两个三角形构成
        Vertex_t p1 = mesh[a], p2 = mesh[b], p3 = mesh[c], p4 = mesh[d];
        p1.u = 0, p1.v = 0, p2.u = 0, p2.v = 1;
        p3.u = 1, p3.v = 1, p4.u = 1, p4.v = 0;
        DrawTriangle(p1, p2, p3);
        DrawTriangle(p3, p4, p1);
    }

    void DrawBox(float theta) {
        //设置坐标系
        Matrix_t m;
        m.Rotate(-1, -0.5, 1, theta);
        this->transform.world = m;
        MVPTrans(this->transform);
        DrawPlane( 0, 1, 2, 3);
        DrawPlane( 7, 6, 5, 4);
        DrawPlane( 0, 4, 5, 1);
        DrawPlane( 1, 5, 6, 2);
        DrawPlane( 2, 6, 7, 3);
        DrawPlane( 3, 7, 4, 0);
    }

    void CameraInit(float x, float y, float z) {
        Vector_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
        this->transform.view = SetView(eye, at, up);
        MVPTrans(this->transform);
    }

    void InitTexture() {
        static unsigned int texture[256][256];
        int i, j;
        for (j = 0; j < 256; j++) {
            for (i = 0; i < 256; i++) {
                int x = i / 32, y = j / 32;
                texture[j][i] = ((x + y) & 1)? 0xffffff : 0xdc143c;
            }
        }
        SetTexture(texture, 256 * 4, 256, 256);
    }

};

int ScreenClose(void)
{
    if(screen_dc)
    {
        if(screen_ob)
        {
            SelectObject(screen_dc, screen_ob);
            screen_ob = NULL;
        }
        DeleteDC(screen_dc);
        screen_dc = NULL;
    }
    if(screen_hb)
    {
        DeleteObject(screen_hb);
        screen_hb = NULL;
    }
    if(screen_handle)
    {
        CloseWindow(screen_handle);
        screen_handle = NULL;
    }
    return 0;
}



void ScreenDispatch(void)
{
    MSG msg;
    while(1)
    {
        if(!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
        if(!GetMessage(&msg, NULL, 0, 0)) break;
        DispatchMessage(&msg);
    }
}

void ScreenUpdate(void)
{
    HDC hDC = GetDC(screen_handle);
    BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
    ReleaseDC(screen_handle, hDC);
    ScreenDispatch();
}

int ScreenInit(int w, int h, const TCHAR *title)
{
    WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)ScreenEvents, 0, 0, 0, 
    NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
    // BITMAPINFO bi = { {sizeof(BITMAPINFOHEADER), w , -h, 1, 32, BI_RGB, w * h * 4, 0, 0, 0, 0 } };
    BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,w * h * 4, 0, 0, 0, 0 } };
    RECT rect = {0, 0, w, h};
    int wx, wy, sx, sy;
    LPVOID ptr;
    HDC hDC;
    ScreenClose();
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if(!RegisterClass(&wc)) return -1;
    screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    if(screen_handle == NULL) return -2;
    screen_exit = 0;
    hDC = GetDC(screen_handle);
    screen_dc = CreateCompatibleDC(hDC);
    ReleaseDC(screen_handle, hDC);
    screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
    if(screen_hb ==  NULL) return -3;
    screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
    screen_fb = (unsigned char*)ptr;
    screen_w = w;
    screen_h = h;
    screen_pitch = w * 4;
    AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
    wx = rect.right - rect.left;
    wy = rect.bottom - rect.top;
    sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
    sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
    if(sy < 0) sy = 0;
    SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
    SetForegroundWindow(screen_handle);
    ShowWindow(screen_handle, SW_NORMAL);
    ScreenDispatch();
    memset(screen_keys, 0, sizeof(int)*512);
    memset(screen_fb, 0, w * h * 4);
    return 0;
}

class Light
{//光源
public:
    Vector_t position;
    float intensity;
    Light() = default;
    Light(Vector_t p, float intense)
    {
        this->position = p;
        this->intensity = intense;
    }
};

class Color
{
    float r;
    float g;
    float b;
    
    Color(){this->r = 0.0f, this->g = 0.0f, this->b = 0.0f;}
    Color(float x, float y, float z)
    {
        this->r = x;
        this->g = y;
        this->b = z;
    }
};

Vector_t phong_fragment_shader(Vector_t point, Vector_t color, Vector_t normal)
{//由于设计之处忘记处理平面垂直的向量的值,现在很难把光照加入,暂定不修改
    //ambient 环境光
    float ka = 0.005;
    //diffuse 漫反射
    Vector_t kd = color;
    //Specular 高光
    float ks = 0.7937;
    float intense = 500;
    Vector_t p1(20,20,20);
    Vector_t p2(-20,20,0);
    auto l1 = Light(p1,intense);
    auto l2 = Light(p2,intense);
    //光源位置和强度
    std::vector<Light> lights = {l1, l2};
    //环境光强度
    float amb_light_intensity = 10;
    //视觉角度
    Vector_t eye_pos(0, 0, 10);

    //高光的幂指数
    float p = 150;

    Vector_t result_color(0, 0, 0);
    for (auto& light : lights)
    {
        // 光的强度为 I/r² 跟距离有关
        float ambient = ka*amb_light_intensity;
        Vector_t res1;
        res1.x = res1.y = res1.z = ambient;
        Vector_t r = light.position - point;
        Vector_t temp = r;
        temp.NormalizeSelf();
        Vector_t res2 = kd;
        res2.Scale((light.intensity/(r.Product(r))) * std::max(0.0f, temp.Product(normal)));
        Vector_t v = eye_pos - point;
        Vector_t temp2 = r + v;
        temp2.NormalizeSelf();
        float sepcular = ks*(light.intensity/(r.Product(r))) * pow(std::max(0.0f, temp2.Product(normal)),p);
        Vector_t res3;
        res3.x = res3.y = res3.z = sepcular;
        result_color = result_color + res1 + res2 + res3;
    }
    //0~1范围区间的颜色
    return result_color;
}

int main(void)
{
    TCHAR *title = _T("MyShader") _T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");
    if(ScreenInit(800, 600, title))
        return -1;
    Device_t device(800, 600, screen_fb);
    int status[] = {RENDER_MODE_TEXTURE,RENDER_MODE_COLOR, RENDER_MODE_WIREFRAME};
    int indicator = 0;
    int kbhit = 0;
    float alpha = 1.0f;
    float pos = 3.5;
    device.CameraInit(3.0f, 0.0f, 0.0f);
    device.InitTexture();
    device.render_status = RENDER_MODE_TEXTURE;
    while(screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
    {
        // std::cout << pos << std::endl;
        ScreenDispatch();
        device.Clear(1);
        device.CameraInit(pos, 0, 0);
        if(screen_keys[VK_UP]) pos -= 0.01f;
        if(screen_keys[VK_DOWN]) pos += 0.01f;
        if(screen_keys[VK_LEFT]) alpha += 0.01f;
        if(screen_keys[VK_RIGHT]) alpha -= 0.01f;
        if(screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_status = status[indicator];
			}
		}	else {
			kbhit = 0;
		}
        device.DrawBox(alpha);
        ScreenUpdate();
        Sleep(1);
    }
    return 0;
}