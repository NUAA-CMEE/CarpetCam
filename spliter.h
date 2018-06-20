#ifndef SPLITER_H
#define SPLITER_H

#include<QDebug>
#include<QString>
#include<QObject>
#include"globalvariable.h"

class Spliter
{
public:
    Spliter();

private:
    int count; //the ID of the extracting image
    int sideWidth;//the added width of the image boundary

private:
    void extractColorImage(RGB_Lab *rgb);
    void edgeDetection();

    float  caculateDistance(RGB_Lab *color, int R, int G,int B);
    float  caculateDistance(Cluster_point *color, int R, int G,int B);

};

#endif // SPLITER_H
