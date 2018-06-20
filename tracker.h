#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include<QDebug>
#include<datatype.h>
#include"globalvariable.h"

class Tracker : public QObject
{
    Q_OBJECT
public:
    explicit Tracker(QObject *parent = 0);

public:
    void getAllPicCode();
    void getPropertyChainList();

private:
    void getOnePicCode(QString path);
    void getSingleChainCode();
    int traceChain(int row,int col);
    bool isEvenNumber(int input);
    int loopAdd(int src,int add);
    int loopSub(int src,int sub);
    Image_IJ nextPoint(Image_IJ input,int codeValue);
    Image_IJ surroundPoint(Image_IJ input,int codeValue);

    void findLimitValue();
    void caculateChainDepth();
    void judgeOuter();
    int radial(Pic_codes *aPic,int index);
    int radial_with_aCode(Code chain, int row, int col);
    bool  isVertex(Code chain,int index);
    bool  isBorder(Code chain,int index);
    bool isBorder2(Code chain,int index);

    float  caculateDistance(Cluster_point *color, int R, int G,int B);

private:
    Pic_codes  codes_in_aPic;
    Mat Pic;
    bool isEnd;
    int lastCode;
    int right_slope,left_slope;
    bool jump;
    pixel_type  backgroundColor;
};

#endif // TRACKER_H
