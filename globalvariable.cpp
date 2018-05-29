#include"globalvariable.h"

QString  filePath = "./pictureFile/";
QString  fileName = "";
QImage qImage;//qt中的图像容器
int image_width = 1000;//图像宽度
int image_height = 1000;//图像高度
Mat Image ;//读取的原始图片数据
Mat Image_quantity;//量化后的图片
Mat Image_cluster;//聚类后的图片
Mat Image_edge;//边缘提取后的图片
Mat Image_track;//轮廓追踪后的图片
Mat Image_fill;//轮廓填充后的图片
bool quantity_first = false;//true   216颜色量化方法    false 颜色合并量化方法
float NBS_yuzhi = 3.2; //NBS阈值  2.7 k mean   density peak3
int kernel_size = 3;
int total_pic_number = 1; //聚类图像分解后的数量
float offset_distance = 1; //轮廓偏置距离

float  Stitch = 5;
float  offsetRange = 2;

Cluster_point *cluster_Head = NULL;//聚类中心链表头
Pictures allPic;
all_Splines total_content;

QVector<color_linesFillColor>  final_fill; //所有颜色的填充结果
QVector<color_loops> all_color;//存储所有颜色的封闭环


int standRGB[256][3]={ {255,255,255},{255,255,204},{255,255,153},{255,255,102},{255,255,51},{255,255,0},
                                   {255,204,255},{255,204,204},{255,204,153},{255,104,102},{255,204,51},{255,204,0},
                                   {255,153,255},{255,153,204},{255,153,153},{255,153,102},{255,153,51},{255,153,0},
                                   {255,102,255},{255,102,204},{255,102,153},{255,102,102},{255,102,51},{255,102,0},
                                   {255,51,255},{255,51,204},{255,51,153},{255,51,102},{255,51,51},{255,51,0},
//                                   {255,68,255},{255,68,204},{255,68,153},{255,68,102},{255,68,51},{255,68,0},//多插值的6个
                                   {255,0,255},{255,0,204},{255,0,153},{255,0,102},{255,0,51},{255,0,0},

                                   {204,255,255},{204,255,204},{204,255,103},{204,255,102},{204,255,51},{204,255,0},
                                   {204,204,255},{204,204,204},{204,204,153},{204,204,102},{204,204,51},{204,204,0},
                                   {204,153,255},{204,153,204},{204,153,153},{204,153,102},{204,153,51},{204,153,0},
//                                   {204,126,255},{204,126,204},{204,126,153},{204,126,102},{204,126,51},{204,153,0},//多插值的6个
                                   {204,102,255},{204,102,204},{204,102,153},{204,102,102},{204,102,51},{204,102,0},
                                   {204,51,255},{204,51,204},{204,51,153},{204,51,102},{204,51,51},{204,51,0},
                                   {204,0,255},{204,0,204},{204,0,153},{204,0,102},{204,0,51},{204,0,0},

                                   {153,255,255},{153,255,204},{153,255,153},{153,255,102},{153,255,51},{153,255,0},
                                   {153,204,255},{153,204,204},{153,204,153},{153,204,102},{153,204,51},{153,204,0},
                                   {153,153,255},{153,153,204},{153,153,153},{153,153,102},{153,153,51},{153,153,0},
//                                   {153,126,255},{153,126,204},{153,126,153},{153,126,102},{153,126,51},{153,126,0},//多插值的6个
                                   {153,102,255},{153,102,204},{153,102,153},{153,102,102},{153,102,51},{153,102,0},
                                   {153,51,255},{153,51,204},{153,51,153},{153,51,102},{153,51,51},{153,51,0},
                                   {153,0,255},{153,0,204},{153,0,153},{153,0,102},{153,0,51},{153,0,0},

                                   {102,255,255},{102,255,204},{102,255,153},{102,255,102},{102,205,51},{102,205,0},
                                   {102,204,255},{102,204,204},{102,204,153},{102,204,102},{102,204,51},{102,204,0},
                                   {102,153,255},{102,153,204},{102,153,153},{102,153,102},{102,153,51},{102,153,0},
//                                   {102,126,255},{102,126,204},{102,126,153},{102,126,102},{102,126,51},{102,126,0}, //多的6个
                                   {102,102,255},{102,102,204},{102,102,153},{102,102,102},{102,102,51},{102,102,0},
                                   {102,51,255},{102,51,204},{102,51,153},{102,51,102},{102,51,51},{102,51,0},
                                   {102,0,255},{102,0,204},{102,0,153},{102,0,102},{102,0,51},{102,0,0},

                                   {51,255,255},{51,255,204},{51,255,153},{51,255,102},{51,255,51},{51,255,0},
                                   {51,204,255},{51,204,204},{51,204,153},{51,204,102},{51,204,51},{51,204,0},
                                   {51,153,255},{51,153,204},{51,153,153},{51,153,102},{51,153,51},{51,153,0},
//                                   {51,126,255},{51,126,204},{51,126,153},{51,126,102},{51,126,51},{51,126,0}, //多的6个
                                   {51,102,255},{51,102,204},{51,102,153},{51,102,102},{51,102,51},{51,102,0},
                                   {51,51,255},{51,51,204},{51,51,153},{51,51,102},{51,51,51},{51,51,0},
                                   {51,0,255},{51,0,204},{51,0,153},{51,0,102},{51,0,51},{51,0,0},

                                   {0,255,255},{0,255,204},{0,255,153},{0,255,102},{0,255,51},{0,255,0},
                                   {0,204,255},{0,204,204},{0,204,153},{0,204,102},{0,204,51},{0,204,0},
                                   {0,153,255},{0,153,204},{0,153,153},{0,153,102},{0,153,51},{0,153,0},
//                                   {0,126,255},{0,126,204},{0,126,153},{0,126,102},{0,126,51},{0,126,0}, //多的6个
                                   {0,102,255},{0,102,204},{0,102,153},{0,102,102},{0,102,51},{0,102,0},
                                   {0,51,255},{0,51,204},{0,51,153},{0,51,102},{0,51,51},{0,51,0},
                                   {0,0,255},{0,0,204},{0,0,153},{0,0,102},{0,0,51},{0,0,0},
                                  //以上216种，往下在鲜艳区再加40种
                                  {25,25,25},{25,25,75},{25,25,125},{25,25,175},{25,25,225},
                                  {25,75,25},{25,75,75},{25,75,125},{25,75,175},{25,75,225},
                                  {25,125,25},{25,125,75},{25,125,125},{25,125,175},{25,125,225},

                                   {75,25,25},{75,25,75},{75,25,125},{75,25,175},{75,25,225},
                                   {75,75,25},{75,75,75},{75,75,125},{75,75,175},{75,75,225},
                                   {75,125,25},{75,125,75},{75,125,125},{75,125,175},{75,125,225},

                                   {125,25,25},{125,25,75},{125,25,125},{125,25,175},{125,25,225},
                                   {125,75,25},{125,75,75},{125,75,125},{125,75,175},{125,75,225}
                                  };




/********************************************
 *function:将opencv格式的图片容器转换成QT格式的图片容器
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/4
 *******************************************/
QImage Mat2QImage(cv::Mat cvImg)
{
    QImage qImg;
    if(cvImg.channels()==3)                             //3 channels color image
    {

        cv::cvtColor(cvImg,cvImg,CV_BGR2RGB);
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols, cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }
    else if(cvImg.channels()==1)                    //grayscale image
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_Indexed8);
    }
    else
    {
        qImg =QImage((const unsigned char*)(cvImg.data),
                    cvImg.cols,cvImg.rows,
                    cvImg.cols*cvImg.channels(),
                    QImage::Format_RGB888);
    }

    return qImg;
}
