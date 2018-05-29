#ifndef CLUSTER_H
#define CLUSTER_H

#include <QObject>
#include<QDebug>
#include<QFile>
#include"datatype.h"
#include"math.h"

class Cluster : public QObject
{
    Q_OBJECT
public:
    explicit Cluster(QObject *parent = 0);
    ~Cluster();

private:
    float convergence_judge;//K均值聚类收敛判据

public:
    RGB_Lab *RGB_Head;  //量化后的标准调色板链表头
    NBS_Node *NBS_Head; //存储NBS距离大于3的所有颜色色差链表头
    float NBS_threshold; //加权阈值

    RGB_Lab *last_cluster_head; //上一次聚类中心的链表头，用来和新的聚类中心链表进行比较

    Density *density_Head; //密度峰链表头
    Density *sort_density_Head; //密度峰链表头
    Density *cluster_density_Head; //density peak list header

    int kernel_size;
    int normal_K;  //密度归一化因子
    int cluster_num;//聚类中心数目


public:
    void K_Means_Cluster();  //K均值自适应聚类
    void reGenerateRGB_LabPointer();
    void compareColorNBS(RGB_Lab *Node1, RGB_Lab *Node2);
    void  expandImage();
    void filterProcess();
    void filterProcess2();
    void reduceNosiePoint();
    void reduceNosiePoint2();
    void reduceNosiePoint3();

    void ADPC_cluster();  //密度峰聚类
    void direct_cluster();

private:
    void  create_NBS();
    void  caculate_weight();
    void  create_clusterCenter();
    void  create_clusterCenter2();
    void  create_clusterCenter3();
    void  distributePoint2Cluster();
    void  distributePoint2Cluster2();
    void  updateClusters();
    void  copyCluster_first();
    void  copyCluster_other();
    void fill_cluster_Image();
    bool compareCluster();
    bool checkSameNode(Pixel input);
    bool checkSameNode(Pixel input,RGB_Lab *Head);
    float caculate_NBS(RGB_Lab *Node1,RGB_Lab *Node2);
    float caculate_NBS(Cluster_point *input1,RGB_Lab *input2);
    float caculate_NBS(Cluster_point *input1,Pixel input2);
    float caculate_NBS(Pixel input1,Pixel input2);
    HVC RGB2HVC(RGB_Lab *Node);
    HSI  RGB2HSI(RGB_Lab *Node);
    HSI  RGB2HSI(Cluster_point *Node);
    HSI  RGB2HSI(Pixel Node);
    HSI RGB2HSI(Density *Node);
    float minRGB(float R,float G,float B);
    float function(float input);
    float NBS_function(HVC input1,HVC input2);
    float caculate_Odistance(Cluster_point *input1,Pixel input2);
    float caculate_Odistance(Cluster_point *input1,RGB_Lab * input2);
    float adjust_I(float I);
    RGB_Lab *findMaxNumColor();
    bool  isAcluster(RGB_Lab* input);
    bool RGBlist_isEmpty();
    Pixel findReplacePixel(int row, int col);
    Pixel findReplacePixel2(int row,int col);
    Pixel findReplacePixel3(int row,int col);

    void computerDensity();
    void create_density();
    void create_densityDistance();
    float findMaxNBSditance();
    Density *findMaxGanma(Density *Head);
    void sortDensityList();
    void releaseDensityNode(Density* Node);
    void findDensityCluster();
    void findDensityCluster2();
    void findDensityCluster3();
    bool canBeACluser(Density*input,Density*Head);
    float NBScaculate(Density *input1,Density *input2);
    float NBScaculate(RGB_Lab input1,Density *input2);
    void fill_Image();
    Pixel findReplacePixel(RGB_Lab input);
    float caculate_Odistance(RGB_Lab *input1,Density *input2);

};

#endif // CLUSTER_H
