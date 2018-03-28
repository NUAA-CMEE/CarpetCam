#ifndef FILLCOLORS_H
#define FILLCOLORS_H

#include"datatype.h"
#include<QDebug>


class fillColors
{
public:
    fillColors();

private:
    QVector<color_loops> all_color; //存储所有颜色的封闭环
    float num_points_judge; //决定Bezier曲线离散成线段的阈值，小于阈值时为一固定值，超过阈值时数目和曲线弧长成正比
    int  num_split_point;  // 小于阈值时离散的线段数目
    QVector<QVector<processArea>>  all_colorWorkArea; //所有颜色的加工区域
    QVector<color_linesFillColor>  final_fill; //所有颜色的填充结果

private:
    //预处理函数
    void splitBezierCurve();
    void findBoxArea(loop &input);
    void findWorkArea();
    void addLines(loop &input1,QVector<Line_type2> input2);
     QVector<Line_type2> convertBezier2Lines(bezier curve);
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
    QVector<MonotonicChain> findMontonicChain(loop input);
    QVector<QVector<MonotonicChain>> findMontonicChains(QVector<loop> input);
    bool  isHorizontalLine(Line_type2  input);
    activeEdgeTable_Node *createAetChain(processArea input);
    activeEdgeTable_Node *computeScanAet(float scan_x,processArea input);
    float  computeCrossPoint(float x,Line_type2 input);
    void sortScanCrossPoint(QVector<float_Point> &points);
    static bool sortCrossPoints(const float_Point &v1,const float_Point &v2);
    bool existLimitValue(Line_type2  pre,Line_type2  next);
    point_Node *createPointNode();
    point_Node* FirstChainNode(MonotonicChain chain,activeEdgeTable_Node *AET_Head);
    point_Node* buildYRelation(MonotonicChain chain,activeEdgeTable_Node *AET_Head);
    void buildAScanYRelation(MonotonicChain chain,activeEdgeTable_Node *AET_Head);
    bool isAllWorked(activeEdgeTable_Node *AET_Head);
    point_Node *findGoodPoint(activeEdgeTable_Node *AET_Head);
    void buildAET();
};

#endif // FILLCOLORS_H
