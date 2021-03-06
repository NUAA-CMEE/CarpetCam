#ifndef YFILLCOLORS_H
#define YFILLCOLORS_H

#include <QObject>
#include<QDebug>
#include"datatype.h"

class YFillColors
{
public:
    YFillColors();

private:
    float num_points_judge; //决定Bezier曲线离散成线段的阈值，小于阈值时为一固定值，超过阈值时数目和曲线弧长成正比
    int  num_split_point;  // 小于阈值时离散的线段数目
    float chain_judge;//单调链划分阈值   用于判断两个非连续直线是否可以划分到一个单调链中
    float filter_distance; //生成填充轨迹时，将较短直线滤去
    QVector<QVector<processArea>>  all_colorWorkArea; //所有颜色的加工区域

private:
    //预处理函数
    void splitBezierCurve();
    void findBoxArea(loop &input);
    void findWorkArea();
    void addLines(loop &input1,QVector<Line_type2> input2);
     QVector<Line_type2> convertBezier2Lines(bezier curve);
     bool bezierIsLine(bezier curve);
     float bezierCurveLength(bezier curve);
     void sortLoopArea();
     static bool sortByArea(const loop &v1,const loop &v2);
     QVector<processArea>  groupProcessArea(color_loops input);
     QVector<loop>  findInnerLoopsForOuterLoop(loop outer, QVector<loop> &inners);
     bool outerIncludeInner(loop inner,loop outer);

     //轨迹生成函数
     void planAreaFill();
     color_linesFillColor  computeAcolor(QVector<processArea> input);
     processAreaFill  computeAarea(processArea input);
     QVector<MonotonicChain> findMontonicChain2(loop input);
     QVector<QVector<MonotonicChain>> findMontonicChains(QVector<loop> input);
     bool  isVectalLine(Line_type2  input);
     activeEdgeTable_Node *createAetChain(processArea input);
     activeEdgeTable_Node *computeScanAet(float scan_y, processArea input);
     float  computeCrossPoint(float y, Line_type2 input);
     bool existLimitValue(Line_type2  pre,Line_type2  next);
     int  heavyNode(Line_type2 pre,Line_type2 next);
     void sortScanCrossPoint(QVector<float_Point> &points);
     static bool sortCrossPoints(const float_Point &v1,const float_Point &v2);
     point_Node *createPointNode();
     float computeDistance(float_Point end,float_Point start);
     point_Node* buildXRelation(MonotonicChain chain,activeEdgeTable_Node *AET_Head);
     point_Node* FirstChainNode(MonotonicChain chain,activeEdgeTable_Node *AET_Head);
     bool isAllWorked(activeEdgeTable_Node *AET_Head);
     point_Node *findGoodPoint(activeEdgeTable_Node *AET_Head);

};

#endif // YFILLCOLORS_H
