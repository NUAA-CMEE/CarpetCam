#ifndef ANYANGLEFILL_H
#define ANYANGLEFILL_H

#include"datatype.h"
#include"math.h"
#include<QDebug>
#include <QObject>

#define PI 3.1415926

class AnyAngleFill : public QObject
{
    Q_OBJECT
public:
    explicit AnyAngleFill(QObject *parent = 0);

private:
    float num_points_judge; //决定Bezier曲线离散成线段的阈值，小于阈值时为一固定值，超过阈值时数目和曲线弧长成正比
    int  num_split_point;  // 小于阈值时离散的线段数目
    float chain_judge;//单调链划分阈值   用于判断两个非连续直线是否可以划分到一个单调链中
    float filter_distance; //生成填充轨迹时，将较短直线滤去
    QVector<QVector<processArea>>  all_colorWorkArea; //所有颜色的加工区域

    float fill_angle;  //填充线的角度
    float fill_K;//填充线的斜率
    bool isVertical;//true  垂直填充

private:
    void computeK(float angle);
    void splitBezierCurve();
    void sortLoopArea();
    void  findWorkArea();
    void addLines(loop &input1,QVector<Line_type2> input2);
    void findBoxArea(loop &input);

    //预处理函数
    QVector<Line_type2> convertBezier2Lines(bezier curve);
    bool bezierIsLine(bezier curve);
    float bezierCurveLength(bezier curve);
    static bool sortByArea(const loop &v1,const loop &v2);
    QVector<processArea>  groupProcessArea(color_loops input);
    QVector<loop>  findInnerLoopsForOuterLoop(loop outer, QVector<loop> &inners);
    bool outerIncludeInner(loop inner,loop outer);
    bool  isHorizontalLine(Line_type2  input);
    float computeDistance(float_Point end,float_Point start);
    
    //轨迹填充函数
    void planAreaFill();
    color_linesFillColor  computeAcolor(QVector<processArea> input);
    processAreaFill  computeAarea(processArea input);
    QVector<MonotonicChain> findMontonicChain(loop input);
    QVector<QVector<MonotonicChain>> findMontonicChains(QVector<loop> input);
    QVector<MonotonicChain> findMontonicChain2(loop input);

signals:

public slots:
};

#endif // ANYANGLEFILL_H
