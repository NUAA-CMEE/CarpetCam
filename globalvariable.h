#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H
#include<QString>
#include<QImage>
#include"datatype.h"
#include"cv.h"
#include<opencv2/opencv.hpp>

using namespace cv;

extern QString  filePath;  //默认选择的图片路径
extern QString  fileName;  //图片名称
extern QImage qImage;//qt中的图像容器     颜色通道顺序RGB
extern int image_width;//图像宽度
extern int image_height;//图像高度
extern Mat Image;//opecv中的图像容器   颜色通道顺序BGR
extern Mat Image_quantity;//量化后的图片
extern Mat Image_cluster;//聚类后的图片
extern Mat Image_edge;//边缘提取后的图片
extern Mat Image_track;//轮廓追踪后的图片
extern Mat Image_fill;//轮廓填充后的图片
extern float offset_distance; //轮廓偏移距离

extern int standRGB[256][3]; //标准色板
extern bool quantity_first;//第一种量化方法
extern float NBS_yuzhi; //NBS阈值
extern int kernel_size;//卷积核大小

extern Cluster_point *cluster_Head;//聚类中心链表头
extern Pictures allPic;  //存储所有的图片的链表(像素坐标)
extern all_Splines total_content;  //存储外轮廓和水平填充点集
extern int total_pic_number; //聚类图像分解后的数量   颜色种类数

QImage Mat2QImage(cv::Mat cvImg);

#endif // GLOBALVARIABLE_H
