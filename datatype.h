#ifndef DATATYPE_H
#define DATATYPE_H

#include<QVector>
#include<QPointF>
#include<entitytype.h>

typedef struct pixel_type
{
    int R;
    int G;
    int B;
}Pixel;

typedef struct RGB_Lab_Node
{
    float Red;  //L
    float Green; //a
    float Blue;//b
    int  count;
    RGB_Lab_Node *pre;  //指向前一个
    RGB_Lab_Node *Next;  //指向下一个
}RGB_Lab;

typedef struct NBS_Node
{
    RGB_Lab *palette1;
    RGB_Lab *palette2;
    float NBS_distance;//上面两种颜色的NBS距离
    float weight;//权重
    NBS_Node *Next;
}NBS;

typedef struct Density_type
{
    float R;
    float G;
    float B;
    int den; //密度
    float distance;  //比当前节点密度大的最邻近节点距离
    float gama;  //密度和距离的乘积
    Density_type *pre;
    Density_type *pNext;
}Density;

typedef struct Hvc_Type
{
    float H;
    float V;
    float C;
}HVC;

typedef struct Hsi_Type
{
    float H; //色调
    float S;//饱和度
    float I; //强度
}HSI;

typedef struct Image_coordinate  //图像中像素点的坐标
{
    int i;  //row  行号  在图像显示时，对应的是Y轴坐标
    int j;  //column    列号   在图像显示时，对应的是X轴坐标
}Image_IJ;



typedef struct cluster_center  //聚类中心链表
{
    float Red;
    float Green;
    float Blue;
    QVector<Image_IJ> points;
    cluster_center *Next;
}Cluster_point;

typedef struct FreemanCode  //data struct for single linked pixel
{
    bool  outer;  //true: is the out loop       false: is the inner loop
    int  depth;//the depth of the linked list
    int  minX; //top left X axis value
    int  minY; //top left Y axis value
    int  maxX;//bottom right X axis value
    int  maxY;//bottom right Y axis value
    int X,Y;  //coordinate of start point
    QVector<Image_IJ> point;  //coordinate value
}Code;

typedef struct Picture_code  //data struct for all freemancode in a picture
{
    int R;
    int G;
    int B;
    int number;  //the number of chain lists
    QVector<Code> chain_lists;
}Pic_codes;

typedef struct all_picture
{
    int number; //the number of pictures
    QVector<Pic_codes>  codes;
}Pictures;

//typedef struct Bezier_Node  //三次bezier曲线数据结构
//{
//    point_f  p0;
//    point_f  p1;
//    point_f  p2;
//    point_f  p3;
//    bool isouter;  //true:外环   false:内环
//}Bezier;

typedef struct Spline_Node  //三次B样条曲线数据结构
{
    QVector<point_f>  points;
    bool isouter;  //true:外环  false:内环
}Spline;

typedef struct fill_struct
{
    QVector<point_f> points;
}Fills;

typedef struct color_Splines  //B样条 计算用
{
    int R;
    int G;
    int B;
    int number;  //封闭环个数
//    QVector<int> consist_number;  //每个封闭环被分割成B样条曲线的个数
    QVector<Spline_Node>  curves; //轮廓
    QVector<Fills> Points; //水平填充点集
}colors;

typedef struct color_Splines_2  //B样条数据结构   第二种B样条计算数据结构  注意和下面的结构体color_Splines2区别
{
    int R;
    int G;
    int B;
    int number;  //封闭环个数
    QVector<QVector<Spline_Node>>  curves; //轮廓
    QVector<Fills> Points; //水平填充点集
}colors_2;


typedef  struct split_point
{
    int x; //行号
    int y; //列号
    int index; //原本在边缘追踪点集中的序号
}features;

typedef struct bezier_type
{
    QPointF  point1;
    QPointF  point2;
    QPointF  point3;
    QPointF  point4;
    bool isouter;  //true:外环   false:内环
}bezier;

struct float_Point
{
    float x;
    float y;
};

struct Line_type
{
    float_Point start;
    float_Point end;
};

struct Bezier2Line   //Bezier曲线离散成直线
{
    bool isouter;
    QVector<Line_type>  lines;
};

typedef struct color_Splines2    //Bezier用
{
    int R;
    int G;
    int B;
    int number;  //封闭环个数
    QVector<QVector<bezier>>  curves; //轮廓
    QVector<QVector<Bezier2Line>>  curves2; //将Bezier曲线离散成若干段直线
    QVector<Fills> Points; //水平填充点集
}colors2;

typedef struct A_Color_curves //一个颜色中的外轮廓和内填充的数据结构
{
    int R;
    int G;
    int B;
    QVector<QVector<baseEntity *>>  curve; //同一种颜色中所有的封闭环，每个封闭环由若干段直线或曲线组成
    QVector<Fills> fillPoints;
}color3;

typedef struct all_Splines
{
    QVector<color_Splines> outlines;  //按颜色划分的外轮廓集和水平填充集
    QVector<color_Splines_2> outlines2; //按颜色划分的外轮廓集和水平填充集  给第二种计算用
    QVector<A_Color_curves> outlines3; //按颜色划分的外轮廓集和水平填充集  给直线和B样条混合计算用
    QVector<color_Splines2>  outlines4;//按颜色划分的外轮廓和水平填充集   给Bezier计算用
}Vecs;

struct tangent_vector
{
    float  start_x;  //切线起点x坐标
    float  start_y;  //切线起点y坐标
    float  tangent_x; //单位切向量的x坐标
    float  tangent_y; //单位切向量的y坐标
};


/* 图像坐标系原点在左上方，自上向下为X正方向，自左而右是Y正方向，
 * 所以扫描线是平行于Y轴，垂直于X轴。自上而下填充，一条线段靠近Y轴的端点定义为线段起点*/

//以下数据结构为生成填充轨迹用

struct Line_type2
{
    bool   isouter;
    float_Point start;
    float_Point end;
};

struct edgeTable_Node  //ET边表数据节点
{
    float  X;  //该节点存储的所有直线的起点的X坐标
    QVector<Line_type2>   lines;//所有起点的x坐标值为X的直线集合
    edgeTable_Node  *pBefore;
    edgeTable_Node  *pNext;
};

struct  point_Node   //点（节点）
{
    int  index; //一条扫描线上从左到右的编号  从0开始
    int  loopNum;//所在封闭环编号
    int  edgeNum; //所在单调链在其封闭环中的编号
    float x;//x坐标
    float y;//y坐标
    bool skip; //下针点
    bool process;//是否已经加工过
    point_Node *Next_Node;//同一行扫描线上的下一个节点
    point_Node *linked; //同一扫描行上可以相连的节点
    point_Node *disconnect;//同一扫描行上不可以相连的节点
    point_Node *chainBefore;//单调链上的前一个节点
    point_Node *chainNext;//单调链上的下一个节点
    point_Node *chainPcritical;//单调链上的转接节点
    point_Node *pNext;//实际走刀时的下一点
};

struct activeEdgeTable_Node  //扫描线节点（活性边表节点AET）
{
    float  X;  //扫描线的X坐标值
    activeEdgeTable_Node  *pBefore; //前一个扫描线
    activeEdgeTable_Node  *pNext;    //后一个扫描线
    point_Node* pHead; //该扫描线上与封闭环相交的交点链表的链表头
    int numOfPoints;  //扫描线交点个数
};

struct MonotonicChain  //单调链结构体
{
    float minX;
    float maxX;
    QVector<Line_type2>  lines;//该单调链上的所有线段
};

struct  loop  //一个封闭环
{
    bool  isouter;
    float  minX,maxX; //最小包围盒
    float  minY,maxY;
    float acreage;//包围盒面积
    QVector<Line_type2>  lines;
};

struct  color_loops  //一个颜色所有的封闭环
{
    int R;
    int G;
    int B;
    QVector<loop>  outers;  //所有的外环集合
    QVector<loop>  inners;   //所有的内环集合
};

struct processArea   //加工区域    一个加工区域通常由一个外环和若干个内环组成
{
    int R;
    int G;
    int B;
    loop  outer;//外环
    QVector<loop>  inners;//内环若干个
};

struct processAreaFill //一个封闭加工区域的水平填充线集
{
    QVector<QVector<float_Point>> points;  //QVector<float_Point>是一段连续的水平线填充点集  因为不连续，所以好几段
};

struct color_linesFillColor
{
    int R;
    int G;
    int B;
    QVector<processAreaFill>   areas; //所有加工区域
};

#endif // DATATYPE_H
