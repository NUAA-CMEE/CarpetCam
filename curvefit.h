#ifndef CURVEFIT_H
#define CURVEFIT_H

//曲线拟合类
#include <QObject>
#include<QDebug>
#include"datatype.h"
#include"globalvariable.h"
#include<showpic.h>

class curveFit : public QObject
{
    Q_OBJECT
public:
    explicit curveFit(QObject *parent = 0);

private:
    int skip_num;  //每隔多少点采样一次
    int groupSize;  //分组求最大曲率时的分组大小
    float precision;  //精度要求
    int spline_size; //每一条B样条被离散成的点数
    float split_precision;//型值点和分割点的精度差
    Pixel split_color;//分割点颜色
    Mat show_it; //要显示的图片

    QVector<split_point>  feature_point; //临时存储一个封闭环中的所有特征点
    QVector<split_point>  temp_feature_point;  //用来存储精度对比后新增加的型值点
    Spline_Node  Single_curve;  //一个封闭环上所有的控制点
    QVector<float>  coef_A;
    QVector<float>  coef_B;
    QVector<float>  coef_C;
    QVector<Image_IJ>  coef_D;  //点容器

    QVector<QVector<split_point>>  feature_point2; //第二种解决方案中用来存放分段特征点的容器
    QVector<QVector<split_point>>  temp_feature_point2; //第二种.......
    QVector<Spline_Node>  Single_curve2;  //一个封闭环上多段曲线的B样条控制点
    QVector<split_point> section_split; //存放将现有分段继续分割的新的分割点
    QVector<QVector<float>>  coef_A2;
    QVector<QVector<float>>  coef_B2;
    QVector<QVector<float>>  coef_C2;
    QVector<QVector<Image_IJ>>  coef_D2;  //点容器

    //Bezier拟合解决方案
    QVector<bezier>  Single_curve3;  //一个封闭环的所有Bezier组成

    QVector<bezier>  check_segment_precision; //存储单段曲线计算出来的Bezier结果，检测精度用
    QVector<Code>   check_segment_pixels;//存储每段Bezier曲线对应的连续像素，检测精度用



private:
    void clearAndCreate();
    void solution1();
    void solution2();
    void solution3();
    void solution4();
    void uglyAddFeaturePoint();
    void findLimitValuePoint(Code input);
    void findLimitValuePoint2(Code input);
    float caculateCurvature(Image_IJ input1,Image_IJ input2,Image_IJ input3);
    float caculateDistance(Image_IJ input1,Image_IJ input2);
    float caculateArea(float L1,float L2,float L3);
    void findMaxValuePoint(Code input);
    void findMaxValuePoint2(Code input);
    void addFeaturePoints();
    void addFeaturePoints2();
    void sortFeaturePoints();
    void sortFeaturePoints2();
    void splitProcess();
    void splitProcess3();
    void clearWeirdSplitPoint();
    void fillSplitPoints();
    static bool sortByIndex(const split_point &v1,const split_point &v2);
    bool checkVectorExistence(Code input,int index);
    bool checkVectorExistence2(Code input,int index);
    bool checkExistence(int X,int Y);
    bool checkExistence2(int X,int Y);
    Spline_Node caculateSpline();
    Spline_Node caculateSpline_zhu();
    QVector<Spline_Node> caculateSpline2();
    void fillParameter();
    void fillParameter_zhu();
    void fillParameter2();
    void solveEquationsFirst();
    void solveEquationsFirst2();
    void solveEquationsSecond();
    void solveEquationsSecond2();
    float findMinDistance();
    bool checkPrecision(Code input1,Spline_Node input2);
    bool checkPrecision2(Code input1,QVector<Spline_Node> input2);
    bool checkPrecision3(Code input1,QVector<Spline_Node> input2);
    bool istooClose(split_point &input1,QVector<split_point> input2);
    float distance_errror(Image_IJ input1,Spline_Node input2,int index);
    float distance_error_entirety(Image_IJ input1,Spline_Node input2);
    float calculatePointDistance(Image_IJ point1,point_f point2);
    float computeDistance(split_point point1,split_point point2);
    point_f splinePoint(float u,point_f input1,point_f input2,point_f input3,point_f input4);
    void splitContinueCurve();

    QVector<bezier> caculateBezierCurve(Code chain_points);
    bezier  segmentBezierCurve(Code chain_points);
    QVector<bezier> segmentBezierCurves(QVector<Code> input);
    void findLimitValuePoint_Bezier(Code input);
    void findMaxValuePoint_Bezier(Code input);
    void addFeaturePoints_Bezier();
    void sortFeaturePoints_Bezier();
    QVector<Code> splitChainList(Code input);
    QVector<Code> splitChainList2(Code input);
    bool checkBezierPrecesion();
    point_f getBezierPoint(bezier curve, int i, int size);
    float caculateDistance(Image_IJ input1, point_f input2);
    float BernsteinValue(int index,float i);

};

#endif // CURVEFIT_H
