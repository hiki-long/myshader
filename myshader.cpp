#include<iostream>
#include<cmath>
#include<cstdlib>
#include<tchar.h>
#include<windows.h>
#include<assert.h>
#include"myshader.h"

#define PI 3.1415926f
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#define RENDER_MODE_WIREFRAME 1
#define RENDER_MODE_TEXTURE 2
#define RENDER+MODE_COLOR 4


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
    yaxis = zaxis.Cross(xaxis);
    Matrix_t temp;
    temp.matrix_t[0][0] = xaxis.x;
    temp.matrix_t[1][0] = xaxis.y;
    temp.matrix_t[2][0] = xaxis.z;
    temp.matrix_t[3][0] = -xaxis.Product(eye);

    temp.matrix_t[0][1] = yaxis.x;
    temp.matrix_t[1][1] = yaxis.y;
    temp.matrix_t[2][1] = zaxis.z;
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
    res.matrix_t[2][3] = - zn*zf /(zf-zn);
    res.matrix_t[2][3] = 1.0f;
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
    float aspect = width / height;
    trans.world.SetIndentity();
    trans.view.SetIndentity();
    trans.projection = SetPerspective(2*PI * 0.5f, (float)width/(float)height, 1.0f, 500.0f);
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
        return false;
    return true;
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
        int need = sizeof(void*) * (height * 2 + 1024) * width * height * 8;
        char *ptr = new char[need + 64];
        // char *ptr = (char*)malloc(need + 64);
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
        this->framebuffer = NULL;
        this->zbuffer = NULL;
        this->texture = NULL;
    }

    
};


int main(void)
{

    return 0;
}