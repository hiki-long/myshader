// typedef Vector_t Point_t;
float Interpolate(float x1, float x2, int t)
{//进行0~1范围线性插值的计算
    return x1 + (x2 - x1) * t;
}
class Matrix_t{
public:
    float matrix_t[4][4];

    Matrix_t()
    {
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                if(i == j)
                    matrix_t[i][j] = 1.0f;
                else
                    matrix_t[i][j] = 0.0f;
            }
        }
    }

    Matrix_t operator+(const Matrix_t & m) const
    {
        Matrix_t res;
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                res.matrix_t[i][j] = this->matrix_t[i][j] + m.matrix_t[i][j];
            }
        }
        return res;
    }

    Matrix_t operator-(const Matrix_t & m) const
    {
        Matrix_t res;
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                res.matrix_t[i][j] = this->matrix_t[i][j] - m.matrix_t[i][j];
            }
        }
        return res;
    }

    void operator=(const Matrix_t & m)
    {
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                this->matrix_t[i][j] = m.matrix_t[i][j];
            }
        }
    }

    //矩阵点乘,每个位置元素相乘
    Matrix_t dot(const Matrix_t & m) const
    {
        Matrix_t res;
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                res.matrix_t[i][j] = this->matrix_t[i][j] - m.matrix_t[i][j];
            }
        }
        return res;
    }

    Matrix_t Muti(const Matrix_t & m) const
    {
        Matrix_t res;
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                res.matrix_t[j][i] = (this->matrix_t[j][0] * m.matrix_t[0][i]) +
                                     (this->matrix_t[j][1] * m.matrix_t[1][i]) +
                                     (this->matrix_t[j][2] * m.matrix_t[2][i]) +
                                     (this->matrix_t[j][3] * m.matrix_t[3][i]);
            }
        }
        return res;
    }

    Matrix_t scale(float t) const
    {
        Matrix_t res;
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                res.matrix_t[i][j] *= t;
            }
        }
        return res;
    }

    void SetIndentity()
    {
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                if(i == j)
                    this->matrix_t[i][j] = 1.0f;
                else
                    this->matrix_t[i][j] = 0;
            }
        }
    }

    void SetZero()
    {
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                this->matrix_t[i][j] = 0;
            }
        }
    }

    //矩阵位移变换
    void Translate(float x, float y, float z)
    {
        this->matrix_t[3][0] = x;
        this->matrix_t[3][1] = y;
        this->matrix_t[3][2] = z;
    }

    //矩阵缩放x,y,z
    void Scale(float x, float y, float z)
    {
        this->matrix_t[0][0] *= x;
        this->matrix_t[1][1] *= y;
        this->matrix_t[2][2] *= z;
    }

    void Rotate(float x, float y, float z, float theta);

};
class Vector_t{
    //向量类
public:
    float x;
    float y;
    float z;
    float w;

    Vector_t()
    {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
        this->w = 1.0f;
    }

    Vector_t(int x, int y, int z, int w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    
    

    static float VectorLength(const Vector_t &vet)
    {//计算向量|v|的长度
        return (float)sqrt(vet.x * vet.x + vet.y * vet.y + vet.z * vet.z);
    }

    //运算符重载记得加上const
    Vector_t operator+(const Vector_t &v) const
    {
        Vector_t res(this->x+v.x, this->y+v.y , this->z+v.z, 1.0f);
        return res;
    }

    Vector_t operator-(const Vector_t &v) const
    {
        Vector_t res(this->x-v.x, this->y-v.y, this->z-v.z, 1.0f);
        return res;
    }

    void operator=(const Vector_t & v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;
    }

    Vector_t Cross(const Vector_t &v) const
    {//叉乘
        Vector_t res;
        res.x = this->y * v.z - this->z * v.y;
        res.y = this->z * v.x - this->x * v.z;
        res.z = this->x * v.y - this->y * v.x;
        res.w = 1.0f;
        return res;
    }

    float Product(const Vector_t &v) const
    {//向量点乘
        Vector_t res;
        res.x = this->x * v.x;
        res.y = this->y * v.y;
        res.z = this->z * v.z;
        return res.x + res.y + res.z;
    }

    Vector_t normalized() const
    {//返回自己规范化之后的值
        Vector_t res;
        float len = VectorLength(*this);
        if(len != 0.0f)
        {
            res.x = this->x / len;
            res.y = this->y / len;
            res.z = this->z / len;
            res.w = 1.0f;
        }
        return res;
    }

    void NormalizeSelf()
    {
        //将自己变为单位向量
        float len = VectorLength(*this);
        if(len != 0.0f)
        {
            this->x = this->x / len;
            this->y = this->y / len;
            this->z = this->z / len;
            this->w = 1.0f;
        }
    }

    static Vector_t VectorInterpolate(Vector_t & v1, Vector_t & v2, float t)
    {//向量线性插值
        Vector_t res;
        res.x = Interpolate(v1.x, v2.x, t);
        res.y = Interpolate(v1.y, v2.y, t);
        res.z = Interpolate(v1.z, v2.z, t);
        res.w = 1.0f;
        return res;
    }

    Vector_t MatrixMuti(const Matrix_t & m)
    {
        Vector_t res;
        res.x = this->x * m.matrix_t[0][0] + this->y * m.matrix_t[1][0] + this->z * m.matrix_t[2][0] + this->w * m.matrix_t[3][0];
        res.y = this->x * m.matrix_t[0][1] + this->y * m.matrix_t[1][1] + this->z * m.matrix_t[2][1] + this->w * m.matrix_t[3][1];
        res.z = this->x * m.matrix_t[0][2] + this->y * m.matrix_t[1][2] + this->z * m.matrix_t[2][2] + this->w * m.matrix_t[3][2];
        res.w = this->x * m.matrix_t[0][3] + this->y * m.matrix_t[1][3] + this->z * m.matrix_t[2][3] + this->w * m.matrix_t[3][3];
        return res;
    }


};

//超前引用,Matrix_t之前没有Vector_t,的定义,因此要在Vector_t定义完成之后才完善该函数
inline void Matrix_t::Rotate(float x, float y, float z, float theta)
{//旋转矩阵，网上搜公式
    //x,y,z是围绕旋转的向量
    float qsin = (float)sin(theta * 0.5f);
    float qcos = (float)cos(theta * 0.5f);
    Vector_t vec(x,y,z,1.0f);
    float w = qcos;
    vec.NormalizeSelf();
    x = vec.x * qsin;
    y = vec.y * qsin;
    z = vec.z * qsin;
    this->matrix_t[0][0] = 1 - 2 * y * y - 2 * z * z;
    this->matrix_t[1][0] = 2 * x * y - 2 * w * z;
    this->matrix_t[2][0] = 2 * x * z + 2 * w * y;
    this->matrix_t[0][1] = 2 * x * y + 2 * w * z;
    this->matrix_t[1][1] = 1 - 2 * x * x - 2 * z * z;
    this->matrix_t[2][1] = 2 * y * z - 2 * w * x;
    this->matrix_t[0][2] = 2 * x * z - 2 * w * y;
    this->matrix_t[1][2] = 2 * y * z + 2 * w * x;
    this->matrix_t[2][2] = 1 - 2 * x * x - 2 * y * y;
    this->matrix_t[0][3] = this->matrix_t[1][3] = this->matrix_t[2][3] = 0.0f;
    this->matrix_t[3][0] = this->matrix_t[3][1] = this->matrix_t[3][2] = 0.0f;
    this->matrix_t[3][3] = 1.0f;
}


class Vertex_t{
    //顶点类
public:
    float x,y,z,w;
    float r,g,b;
    float u,v;
    float rhw;//变化后的w
    Vertex_t()
    {
        x = y = z = 0.0f;
        w = 1.0f;
        r = g = b = 0.0f;
        u = v = 0.0f;
        rhw = 1.0;
    }

    void Init()
    {//初始化w,进行透视矫正，x和y与u/z,v/z呈线性相关,而变换后坐标的vertex的w正是原来坐标的z的值
        this->rhw = 1 / this->w;
        this->u *= rhw;
        this->v *= rhw;
        this->r *= rhw;
        this->g *= rhw;
        this->b *= rhw;
    }

    

    

    Vertex_t operator+(const Vertex_t & v) const
    {
        Vertex_t res;
        res.x = this->x + v.x;
        res.y = this->y + v.y;
        res.z = this->z + v.z;
        res.w = this->w + v.w;
        res.rhw = this->rhw + v.rhw;
        res.u = this->u + v.u;
        res.v = this->v + v.v;
        res.r = this->r + v.r;
        res.g = this->g + v.g;
        res.b = this->b + v.b;
        return res;
    }

    Vertex_t operator=(const Vertex_t & v)
    {
        Vertex_t res;
        res.x = v.x;
        res.y = v.y;
        res.z = v.z;
        res.w = v.w;
        res.r = v.r;
        res.g = v.g;
        res.b = v.b;
        res.u = v.u;
        res.v = v.v;
        res.rhw = v.rhw;
        return res;
    }
};

//进行顶点相减
Vertex_t CountStep(Vertex_t &begin, Vertex_t &end, float width)
{
    Vertex_t res;
    float inv = 1.0f / width;
    res.x = (end.x - begin.x) * inv;
    res.y = (end.y - begin.y) * inv;
    res.z = (end.z - begin.z) * inv;
    res.w = (end.w - begin.w) * inv;
    res.u = (end.u - begin.u) * inv;
    res.v = (end.v - begin.v) * inv;
    res.r = (end.r - begin.r) * inv;
    res.g = (end.g - begin.g) * inv;
    res.b = (end.b - begin.b) * inv;
    res.rhw = (end.rhw - begin.rhw) * inv;
}

class Edge_t{
public:
    //v1,v2是边的端点,v是边上一点
    Vertex_t v, v1, v2;
    Edge_t() = default;
};

class Scanline_t{
public:
    //v是扫描线的起点,step是步长(x移动一个单位),x,y是扫描的点的坐标
    Vertex_t v, step; int x, y, width;
    Scanline_t() = default;
};

//顶点交换
void Swap(Vertex_t & p1, Vertex_t & p2)
{
    Vertex_t p = p1;
    p1 = p2;
    p2 = p;
}

//顶点插值
Vertex_t VInterpolate(const Vertex_t & v1, const Vertex_t & v2, float t)
{
    Vertex_t res;
    res.x = Interpolate(v1.x, v2.x, t);
    res.y = Interpolate(v1.y, v2.y, t);
    res.z = Interpolate(v1.z, v2.z, t);
    res.w = 1.0f;
    res.r = Interpolate(v1.r, v2.r, t);
    res.g = Interpolate(v1.g, v2.g, t);
    res.b = Interpolate(v1.b, v2.b, t);
    res.rhw = Interpolate(v1.rhw, v2.rhw, t);
}


class Trapezoid_t{
public:
    //所在范围的左右两边
    Edge_t left;
    Edge_t right;
    //扫描的y轴大小
    float top;
    float bottom;
    Trapezoid_t() = default;
    int CountTriangle(Trapezoid_t *trap, const Vertex_t & p1, const Vertex_t & p2, const Vertex_t & p3)
    {
        //计算按y轴扫描可能碰到的三角形数量,范围在[0-2]
        Vertex_t v1,v2,v3;
        v1 = p1, v2 = p2, v3 = p3;
        float k,x;
        if(v1.y > v2.y)
            Swap(v1,v2);
        if(v1.y > v3.y)
            Swap(v1,v3);
        if(v2.y > v3.y)
            Swap(v2,v3);
        //下面处理在同一条线上的情况
        if(v1.y == v2.y && v2.y == v3.y)
            return 0;
        if(v1.x == v2.x && v2.x == v3.x)
            return 0;
        //v1应该是最左边最上一点
        if(v1.y == v2.y)
        {
            //三角形朝下
            if(v1.x > v2.x)
                Swap(v1, v2);
            trap[0].top = v1.y;
            trap[0].bottom = v3.y;
            trap[0].left.v1 = v1;
            trap[0].left.v2 = v3;
            trap[0].right.v1 = v2;
            trap[0].right.v2 = v3;
            return trap[0].top < trap[0].bottom ? 1 : 0;
        }
        if(v2.y == v3.y)
        {
            //三角形朝上
            if(v2.x > v3.x)
                Swap(v2,v3);
            trap[0].top = v1.y;
            trap[0].bottom = v3.y;
            trap[0].left.v1 = v1;
            trap[0].left.v2 = v2;
            trap[0].right.v1 = v1;
            trap[0].right.v2 = v3;
            return trap[0].top < trap[0].bottom ? 1 : 0;
        }

        //v1,v2形成的边
        trap[0].top = v1.y;
        trap[0].bottom = v2.y;
        //v2,v3形成的边
        trap[1].top = v2.y;
        trap[1].bottom = v3.y;
        
        k = (v3.y - v1.y) / (v2.y - v1.y);
        x = v1.x + (v2.x - v1.x) * k;

        if( x <= v3.x)
        {
            trap[0].left.v1 = v1;
            trap[0].left.v2 = v2;
            trap[0].right.v1 = v1;
            trap[0].right.v2 = v3;
            trap[1].left.v1 = v2;
            trap[1].left.v2 = v3;
            trap[1].right.v1 = v1;
            trap[1].right.v2 = v2;
        }
        else
        {
            trap[0].left.v1 = v1;
            trap[0].left.v2 = v3;
            trap[0].right.v1 = v1;
            trap[0].right.v2 = v2;
            trap[1].left.v1 = v1;
            trap[1].left.v2 = v3;
            trap[1].right.v1 = v2;
            trap[1].right.v2 = v3;
        }

        return 2;      
    }

    void YinEdge(float y)
    {
        //计算在左右两侧的边上高度为y的两个点,通过插值计算可以得出来
        float s1 = this->left.v2.y - this->left.v1.y;
        float s2 = this->right.v2.y - this->right.v1.y;
        float t1 = (y - this->left.v1.y) / s1;
        float t2 = (y - this->right.v1.y) / s2;
        this->left.v = VInterpolate(this->left.v1, this->left.v2, t1);
        this->right.v = VInterpolate(this->right.v2, this->right.v2, t2);
    }

    void InitScanLine(Scanline_t & scan, int y)
    {
        float width = this->right.v.x - this->left.v.x;
        scan.x = (int)(this->left.v.x + 0.5f);
        scan.width = (int)(this->right.v.x + 0.5f) - scan.x;
        scan.y = y;
        scan.v = this->left.v;
        //不可以出现左边超过右边范围的情况
        if(this->left.v.x >= this->right.v.x) scan.width = 0;
        scan.step = CountStep(this->left.v, this->right.v, width);
    }

};