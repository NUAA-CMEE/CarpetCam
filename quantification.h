#ifndef QUANTIFICATION_H
#define QUANTIFICATION_H

#include <QObject>
#include<QProgressDialog>
#include <QCoreApplication>
#include<QDebug>
#include"globalvariable.h"
#include"math.h"

class Quantification : public QObject
{
    Q_OBJECT
public:
    explicit Quantification(QObject *parent = 0);
    ~Quantification();

private:
    int target_colorNum;

public:
    RGB_Lab *Head_216; //第一种量化方法的颜色链表   均匀颜色空间
    RGB_Lab *Head;  //第二种量化方法的颜色链表

public:
    void RGB216_quantity(); //第一种：RGB均匀空间颜色量化方法
    void Lab_quantity();
    void filterProcess();

private:
    Pixel getReplacePixel(Pixel input);
    Pixel getReplacePixel2(Pixel input);
    float caculateOdistance(Pixel input,int *array);
    float caculateOdistance(RGB_Lab *Node1,RGB_Lab *Node2);
    float caculateOdistance(Pixel input,RGB_Lab * Node);
    void fill_RGB_Node();
    int compress(int color);
    bool checkSameNode(Pixel input,bool index);
    int countSize();
    void  RGB2Lab();
    void  Lab2RGB();
    float caculate1(float input);
    void  mergeColor();
    int  countRGB();
};

#endif // QUANTIFICATION_H
