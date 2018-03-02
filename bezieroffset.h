#ifndef BEZIEROFFSET_H
#define BEZIEROFFSET_H

#include"globalvariable.h"
#include<QDebug>

class BezierOffset
{
public:
    BezierOffset();

private:
    int splitNum;  //Bezier曲线离散个数
    float delta; //t增加的大小
    float num_points_judge; //决定Bezier曲线离散成线段的阈值，小于阈值时为一固定值，超过阈值时数目和曲线弧长成正比
    int  num_split_point;  // 小于阈值时离散的线段数目

private:
    bezier offsetCurves(bezier input,bool direction);
    tangent_vector computeTangent(float t,bezier curve);
    float_Point  computeStartPoint(float t,bezier curve);
    float_Point  computeTangenLine(float t,bezier curve);
    float_Point  computeOffsetPoint(tangent_vector input,bool direction);
    bezier  curveFit(QVector<float_Point> input,bool outer);
    float BernsteinValue(int index,float i);

    Bezier2Line  convertBezier2Lines(bezier curve);
    float  bezierCurveLength(bezier curve);
    void findIntersection(Bezier2Line &line1,Bezier2Line &line2);
    float_Point twoLineCross(Line_type line1,Line_type line2);
};

#endif // BEZIEROFFSET_H
