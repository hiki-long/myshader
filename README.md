# myshader
### 主要参考来源：

韦易笑大神的mini3d代码

https://github.com/skywind3000/mini3d

知乎代码解读

https://zhuanlan.zhihu.com/p/74510058

### 注意事项

在debug的时候要手动将dll文件放置于build目录，否则程序会找不到dll文件

主要的文件为myshader.cpp和myshader.h

其他的为opengl的api练习文件

myshader.h主要实现Vector_t(向量类),Matrix_t(矩阵类),Vertex_t(顶点类),myshader.cpp里还有Device_t(管理窗口)和一些窗口绘制函数

主要实现功能:

1. 基本的向量、矩阵计算方法
2. 线性插值法
3. 相机视角可改变
4. 用扫描线来快速获得三角形内的顶点
5. Bresenham绘制直线法
6. MVP变换,并能映射到屏幕上

图形化界面采用directx图形api,因此坐标系是左手坐标系,需注意实现MVP变换时候的z的方向。

由于实现光线追踪、光照渲染等功能用原生C++写感觉工作量过大，之后可以考虑转用opengl来实现以上效果。