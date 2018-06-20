#include "curvefit.h"

curveFit::curveFit(QObject *parent) : QObject(parent)
{
    skip_num = 1;
    groupSize = 20; //groupSize要比skip_num,否则在寻找曲率最大值像素点函数中会出现越界错误
    precision = 0.5;
    spline_size = 50;
    split_color.R = 0;
    split_color.G = 46;
    split_color.B = 184;
    split_precision = 1.1;

    //清空数据，生成自己的数据
//    clearAndCreate();

    //曲线拟合框架
//    solution1();
//    solution2();
//    solution3();
    solution4();
}

/********************************************
 *function:清理数据并创建测试数据
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::clearAndCreate()
{
    allPic.codes.clear();
    allPic.number = 1;
    Pic_codes create_codes;
    create_codes.R = 0;
    create_codes.G = 0;
    create_codes.B = 0;
    create_codes.number = 1;
    Code create_codes2;
    create_codes2.outer = true;
    Image_IJ point;
    for(int j = 7;j<409;j++)
    {
        point.i = 4;
        point.j = j;
        create_codes2.point.append(point);
    }
    for(int i = 5;i<51;i++)
    {
        point.i = i;
        point.j = 408;
        create_codes2.point.append(point);
    }
    for(int j = 407;j>6;j--)
    {
        point.i = 50;
        point.j = j;
        create_codes2.point.append(point);
    }
    for(int i = 49;i>4;i--)
    {
        point.i = i;
        point.j = 7;
        create_codes2.point.append(point);
    }
    create_codes.chain_lists.append(create_codes2);
    allPic.codes.append(create_codes);
}

/********************************************
 *function:曲线拟合框架1
 *input:
 *output:
 *adding:直接用3次B样条来拟合一个完整的封闭环，遇到型值点过近时skip
 *            拟合后存在部分封闭环的拟合曲线末端不封闭的情况
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::solution1()
{
    Pic_codes temp0;
    int size = allPic.number;
    color_Splines  RGB_curve;
    for(int x = 0;x<size;x++)
    {
        QString fileSave = filePath +"/"+fileName + QString("%1").arg(x+1) + ".bmp"; //必须按bmp或tiff
        show_it = imread(fileSave.toLatin1().data());

        temp0 = allPic.codes.at(x);  //每一个颜色
        RGB_curve.R = temp0.R;
        RGB_curve.G = temp0.G;
        RGB_curve.B = temp0.B;
        RGB_curve.number = temp0.number;
        Code temp2;
        int size1 = temp0.number;
        for(int y = 0;y<size1;y++)
        {
            temp2 = temp0.chain_lists.at(y);   //每一个封闭环
            findLimitValuePoint(temp2);
            findMaxValuePoint(temp2);
            addFeaturePoints();
            sortFeaturePoints();
            fillSplitPoints();//按指定颜色填充
           Single_curve= caculateSpline();
//           Single_curve = caculateSpline_zhu();
           Single_curve.isouter = temp2.outer;

//           while(!checkPrecision(temp2,Single_curve))
//            {
//                 //添加型值点
//               addFeaturePoints();
//               sortFeaturePoints();
////               fillSplitPoints();//按指定颜色填充型值点
//               Single_curve.points.clear();
//               Single_curve= caculateSpline();
////               Single_curve = caculateSpline_zhu();
//            }

//           qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!curve fit";
//           point_f point;
//           int size = coef_D.size();
//           for(int i = 0;i<size;i++)
//           {
//               point.x = coef_D.at(i).i;
//               point.y = coef_D.at(i).j;
//               qDebug()<<point.x<<" "<<point.y;
//           }

           feature_point.clear();
           temp_feature_point.clear();
           RGB_curve.curves.append(Single_curve);
           Single_curve.points.clear();
        }
        namedWindow(QString("%1").arg(x).toLatin1().data(), CV_WINDOW_AUTOSIZE);
        imshow(QString("%1").arg(x).toLatin1().data(), show_it);
        total_content.outlines.append(RGB_curve);
        RGB_curve.curves.clear();
    }
}

/********************************************
 *function:曲线拟合框架2
 *input:
 *output:
 *adding:在精度检查插入重复的型值点时，将封闭环断开，变成好几段来进行曲线拟合
 *            注意使用的数据结构有区别
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::solution2()
{
    Pic_codes temp0;
    int size = allPic.number;
    color_Splines_2  RGB_curve;
    for(int x = 0;x<size;x++)
    {
        QString fileSave = filePath +"/"+fileName + QString("%1").arg(x+1) + ".bmp"; //必须按bmp或tiff
        show_it = imread(fileSave.toLatin1().data());

        temp0 = allPic.codes.at(x);  //一个颜色的所有封闭环
        RGB_curve.R = temp0.R;
        RGB_curve.G = temp0.G;
        RGB_curve.B = temp0.B;
        RGB_curve.number = temp0.number;
        Code temp2;
        int size1 = temp0.number;  //一种颜色中的封闭环个数
        for(int y = 0;y<size1;y++)
        {
            temp2 = temp0.chain_lists.at(y);   //每个封闭环
            findLimitValuePoint2(temp2);
            findMaxValuePoint2(temp2);
            addFeaturePoints2();
            sortFeaturePoints2();
//            fillSplitPoints();//按指定颜色填充
           Single_curve2= caculateSpline2();
           int section_size = Single_curve2.size();
           for(int i = 0;i<section_size;i++)
           {
               Single_curve2[i].isouter = temp2.outer;
           }

           while(!checkPrecision2(temp2,Single_curve2))
            {
                 //添加型值点
               addFeaturePoints2();
               sortFeaturePoints2();
               splitProcess();//封闭环分段过程
               clearWeirdSplitPoint();
//               fillSplitPoints();//按指定颜色填充型值点
               Single_curve2.clear();
               Single_curve2= caculateSpline2();
            }

//           qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!curve fit";
//           point_f point;
//           int size = coef_D.size();
//           for(int i = 0;i<size;i++)
//           {
//               point.x = coef_D.at(i).i;
//               point.y = coef_D.at(i).j;
//               qDebug()<<point.x<<" "<<point.y;
//           }

           feature_point2.clear();
           temp_feature_point2.clear();
           RGB_curve.curves.append(Single_curve2);
           Single_curve2.clear();
        }
//        namedWindow(QString("%1").arg(x).toLatin1().data(), CV_WINDOW_AUTOSIZE);
//        imshow(QString("%1").arg(x).toLatin1().data(), show_it);
        total_content.outlines2.append(RGB_curve);
        RGB_curve.curves.clear();
    }
}

/********************************************
 *function:第三种解决方案
 *input:
 *output:
 *adding:根据第一次计算出的型值点，只对封闭环进行一次分割，而不是像方案二对曲线进行多次分割
 *           数据结构用的还是solution2的数据结构，只是处理方式不同
 *author: wong
 *date: 2018/1/25
 *******************************************/
void curveFit::solution3()
{
    Pic_codes temp0;
    int size = allPic.number;
    color_Splines_2  RGB_curve;
    for(int x = 0;x<size;x++)
    {
        temp0 = allPic.codes.at(x);  //一个颜色的所有封闭环
        RGB_curve.R = temp0.R;
        RGB_curve.G = temp0.G;
        RGB_curve.B = temp0.B;
        RGB_curve.number = temp0.number;
        Code temp2;
        int size1 = temp0.number;  //一种颜色中的封闭环个数
        for(int y = 0;y<size1;y++)
        {
            temp2 = temp0.chain_lists.at(y);   //每个封闭环
            findLimitValuePoint2(temp2);
            findMaxValuePoint2(temp2);
            addFeaturePoints2();
            sortFeaturePoints2();
            splitProcess3();
           Single_curve2= caculateSpline2();
           int section_size = Single_curve2.size();
           for(int i = 0;i<section_size;i++)
           {
               Single_curve2[i].isouter = temp2.outer;
           }

           while(!checkPrecision3(temp2,Single_curve2))
            {
                 //添加型值点
               addFeaturePoints2();
               sortFeaturePoints2();
               Single_curve2.clear();
               Single_curve2= caculateSpline2();
            }

           feature_point2.clear();
           temp_feature_point2.clear();
           RGB_curve.curves.append(Single_curve2);
           Single_curve2.clear();
        }
        total_content.outlines2.append(RGB_curve);
        RGB_curve.curves.clear();
    }
}

/********************************************
 *function:第四种曲线拟合方案
 *input:
 *output:
 *adding:采用Bezier曲线进行拟合，先根据曲率来确定分割曲线的型值点，
 *           再用分段的曲线的像素点进行Bezier曲线拟合，如果某一段的精度没有达到，就将该段继续分割，分段拟合
 *author: wong
 *date: 2018/1/28
 *******************************************/
void curveFit::solution4()
{
    Pic_codes temp0;
    int size = allPic.number;
    color_Splines2  RGB_curve;  //一种颜色的存储所有封闭环的Bezier曲线结构
    for(int x = 0;x<size;x++)  //size替换   3有问题
    {
        temp0 = allPic.codes.at(x);  //一个颜色的所有封闭环
        RGB_curve.R = temp0.R;
        RGB_curve.G = temp0.G;
        RGB_curve.B = temp0.B;
        RGB_curve.number = temp0.number;
        Code temp2;
        int size1 = temp0.number;  //一种颜色中的封闭环个数
        for(int y = 0;y<size1;y++)
        {
            temp2 = temp0.chain_lists.at(y);   //每个封闭环的像素点集合  含内外环信息
            Single_curve3 = caculateBezierCurve(temp2);  //求拟合的Bezier曲线
            if(Single_curve3.size()>=5) //过滤噪声点  一些很小的封闭环
            {
                RGB_curve.curves.append(Single_curve3);
            }
           Single_curve3.clear();
        }
        total_content.outlines4.append(RGB_curve);
        RGB_curve.curves.clear();
    }
}

/********************************************
 *function:简单粗暴地添加型值点，直接每隔若干个
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/28
 *******************************************/
void curveFit::uglyAddFeaturePoint()
{

}

/********************************************
 *function:找曲率极值点
 *input: input 一条完整的封闭环
 *output:
 *adding:找出封闭环中的曲率极值点，将极值点push到feature_point
 *author: wang
 *date: 2018/1/9
 *******************************************/
void curveFit::findLimitValuePoint(Code input)
{
    //将轮廓起点加入
    split_point  add;
    add.index = 0;
    add.x = input.point.at(0).i;
    add.y = input.point.at(0).j;
    feature_point.append(add);

    float first,second,third;  //相邻三个的曲率
    int points_size = input.point.size()-1; //终点索引
    for(int i = 2*skip_num;i<points_size;i = i+skip_num)
    {
       if((i+2*skip_num)>points_size) //超出范围
       {
           break;
       }
       else
       {
           first = caculateCurvature(input.point.at(i-2*skip_num),input.point.at(i-skip_num),input.point.at(i));
           second = caculateCurvature(input.point.at(i-skip_num),input.point.at(i),input.point.at(i+skip_num));
           third = caculateCurvature(input.point.at(i),input.point.at(i+skip_num),input.point.at(i+2*skip_num));
           if(second>first&&second>third)
           {
               add.index = i;
               add.x = input.point.at(i).i;
               add.y = input.point.at(i).j;
               feature_point.append(add);
//               if(second>0.2)
//               {
//                   feature_point.append(add);
//                   feature_point.append(add);
//                   feature_point.append(add);
//               }
           }
       }
    }

    //将轮廓终点加入
    split_point  add1;
    add1.index = input.point.size()+1;
    add1.x = input.point.at(0).i;
    add1.y = input.point.at(0).j;
    feature_point.append(add1);
}

/********************************************
 *function:第二个解决方案中求取曲率极值点
 *input:
 *output:
 *adding:和第一种找曲率极值点的区别在于找出的极值点，存放的容器不一样
 *            是封闭的，起点也被加入到最后
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::findLimitValuePoint2(Code input)
{
    //将轮廓起点加入
    QVector<split_point> entire_loop;
    split_point  add;
    add.index = 0;
    add.x = input.point.at(0).i;
    add.y = input.point.at(0).j;
    entire_loop.append(add);

    float first,second,third;  //相邻三个的曲率
    int points_size = input.point.size()-1; //终点索引
    for(int i = 2*skip_num;i<points_size;i = i+skip_num)
    {
       if((i+2*skip_num)>points_size) //超出范围
       {
           break;
       }
       else
       {
           first = caculateCurvature(input.point.at(i-2*skip_num),input.point.at(i-skip_num),input.point.at(i));
           second = caculateCurvature(input.point.at(i-skip_num),input.point.at(i),input.point.at(i+skip_num));
           third = caculateCurvature(input.point.at(i),input.point.at(i+skip_num),input.point.at(i+2*skip_num));
           if(second>first&&second>third)
           {
               add.index = i;
               add.x = input.point.at(i).i;
               add.y = input.point.at(i).j;
               entire_loop.append(add);
           }
       }
    }

    //将轮廓终点加入
    split_point  add1;
    add1.index = input.point.size()+1;
    add1.x = input.point.at(0).i;
    add1.y = input.point.at(0).j;
    entire_loop.append(add1);

    feature_point2.append(entire_loop);
}

/********************************************
 *function:计算曲率
 *input:三个点的坐标
 *output:
 *adding:
 *author: wong
 *date: 2018/1/10
 *******************************************/
float curveFit::caculateCurvature(Image_IJ input1,Image_IJ input2,Image_IJ input3)
{
    float length1,length2,length3;
    length1 = caculateDistance(input1,input2);
    length2 = caculateDistance(input2,input3);
    length3 = caculateDistance(input1,input3);
    float s = caculateArea(length1,length2,length3);
    float result = s/(length1*length2*length3);
    return result;
}

/********************************************
 *function:计算两点间距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/10
 *******************************************/
float curveFit::caculateDistance(Image_IJ input1, Image_IJ input2)
{
    float  delta_x = input1.i - input2.i;
    float  delta_y = input1.j - input2.j;
    float distance = sqrt(delta_x*delta_x + delta_y*delta_y);
    return distance;
}

/********************************************
 *function:根据三角形的三边长来计算计算面积
 *input:三边长
 *output:三角形面积
 *adding:
 *author: wong
 *date: 2018/1/10
 *******************************************/
float curveFit::caculateArea(float L1, float L2, float L3)
{
    float p = (L1+ L2 + L3)/2;
    float s = sqrt(p*(p-L1)*(p-L2)*(p-L3));
    return s;
}

/********************************************
 *function:找分段中曲率最大点,分段大小根据groupSize来
 *input:input 一条完整的封闭环
 *output:
 *adding:找出封闭环中分段的曲率最大值点，将最大值点push到feature_point
 *author: wang
 *date: 2018/1/9
 *******************************************/
void curveFit::findMaxValuePoint(Code input)
{
    float max_curvature = 0; //记录每个分组中的最大曲率   初始化为0   直线上任意一点的曲率为0
    float temp_curvature;
    int  points_count = 0; //记录已经比较的像素点个数，每当大于groupSize时，就自动清零
    split_point  add;
    int points_size = input.point.size()-1; //终点索引
    for(int i = 1;i<points_size;i = i+groupSize)
    {
        if(i+groupSize>points_size)  //防止越界，剩余像素点个数不够一组
        {
//            break;
            int rest_size = points_size - i;
            for(int j = 0;j<rest_size-1;j++)
            {
                if(j == (groupSize-1)&&max_curvature>0)
                {
                    max_curvature = 0;
                    temp_feature_point.append(add);
                }
                else
                {
                    if(i+j == 446)
                        qDebug()<<j;
                    temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                    if(temp_curvature>max_curvature)
                    {
                        add.index = j+i;
                        add.x = input.point.at(j+i).i;
                        add.y = input.point.at(j+i).j;
                        max_curvature = temp_curvature;
                    }
                }
            }
        }
        else
        {
            if(!checkVectorExistence(input,i)) //当前的一组像素点内不存在曲率极大值点，下面去寻找该分段内的曲率最大值点
            {
                for(int j = 0;j<groupSize;j++)
                {
                    if(j == (groupSize-1)&&max_curvature>0)
                    {
                        max_curvature = 0;
                        temp_feature_point.append(add);
                    }
                    else
                    {
                        if(i+j == 446)
                            qDebug()<<j;
                        temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                        if(temp_curvature>max_curvature)
                        {
                            add.index = j+i;
                            add.x = input.point.at(j+i).i;
                            add.y = input.point.at(j+i).j;
                            max_curvature = temp_curvature;
                        }
                    }
                }
            }
        }
    }
}

/********************************************
 *function:solution2的 寻找分段上的曲率最大值像素点  存储到temp_feature_point2中对应的容器中去
 *input:
 *output:
 *adding:和sloution1的区别在于找出的型值点存储的容器不同
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::findMaxValuePoint2(Code input)
{
    QVector<split_point> first_newPoint;
    split_point  add;
    float max_curvature = 0; //记录每个分组中的最大曲率   初始化为0   直线上任意一点的曲率为0
    float temp_curvature;
    int points_size = input.point.size()-1; //终点索引
    for(int i = 1;i<points_size;i = i+groupSize)
    {
        if(i+groupSize>points_size)  //防止越界，剩余像素点个数不够一组
        {
//            break;
            int rest_size = points_size - i;
            for(int j = 0;j<rest_size-1;j++)
            {
                if(j == (groupSize-1)&&max_curvature>0)
                {
                    max_curvature = 0;
                    first_newPoint.append(add);
                }
                else
                {
                    if(i+j == 446)
                        qDebug()<<j;
                    temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                    if(temp_curvature>max_curvature)
                    {
                        add.index = j+i;
                        add.x = input.point.at(j+i).i;
                        add.y = input.point.at(j+i).j;
                        max_curvature = temp_curvature;
                    }
                }
            }
        }
        else
        {
            if(!checkVectorExistence2(input,i)) //当前的一组像素点内不存在曲率极大值点，下面去寻找该分段内的曲率最大值点
            {
                for(int j = 0;j<groupSize;j++)
                {
                    if(j == (groupSize-1)&&max_curvature>0)
                    {
                        max_curvature = 0;
                        first_newPoint.append(add);
                    }
                    else
                    {
                        if(i+j == 446)
                            qDebug()<<j;
                        temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                        if(temp_curvature>max_curvature)
                        {
                            add.index = j+i;
                            add.x = input.point.at(j+i).i;
                            add.y = input.point.at(j+i).j;
                            max_curvature = temp_curvature;
                        }
                    }
                }
            }
        }
    }
    if(first_newPoint.size())
    {
        temp_feature_point2.append(first_newPoint);
    }
}

/********************************************
 *function:将精度对比后新增加的型值点temp_feature_point添加到feature_point中，并清空featur_point_temp
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/10
 *******************************************/
void curveFit::addFeaturePoints()
{
    int size = temp_feature_point.size();
    for(int i = 0;i<size;i++)
    {
        feature_point.append(temp_feature_point.at(i));
    }
    temp_feature_point.clear();
}

/********************************************
 *function:solution2中将曲率极值点和分段曲率最大值点合并到一起
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::addFeaturePoints2()
{
    int size1 = temp_feature_point2.size();
    for(int i = 0;i<size1;i++)
    {
        int size2 = temp_feature_point2[i].size();
        for(int j = 0;j<size2;j++)
        {
            feature_point2[i].append(temp_feature_point2[i][j]);
        }
    }
    temp_feature_point2.clear();
}

/********************************************
 *function:给feature_point中的型值点排序,按index
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/9
 *******************************************/
void curveFit::sortFeaturePoints()
{
    std::sort(feature_point.begin(),feature_point.end(),sortByIndex);
//    int size = feature_point.size();
//    qDebug()<<"sort!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
//    for(int i = 0;i<size;i++)
//    {
//        qDebug()<<"sort"<<feature_point.at(i).index;
//        qDebug()<<feature_point.at(i).x<<"  "<<feature_point.at(i).y;
    //    }
}

/********************************************
 *function:solution2中的给各个分段排序
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::sortFeaturePoints2()
{
    int size = feature_point2.size();
    for(int i = 0;i<size;i++)
    {
        std::sort(feature_point2[i].begin(),feature_point2[i].end(),sortByIndex);
    }
}

/********************************************
 *function:B样条分段过程
 *input:
 *output:
 *adding:根据section_split中的点来分割
 *author: wong
 *date: 2018/1/24
 *******************************************/
void curveFit::splitProcess()
{
    QVector<QVector<split_point>> temp_save = feature_point2;
    feature_point2.clear();
    int count = 0;
    int split_size = section_split.size();
    QVector<split_point>  insert_points;
    int size = temp_save.size();
    for(int m = 0;m<size;m++)
    {
        int size1 = temp_save.at(m).size();
        for(int n = 0;n<size1;n++)
        {
            if(count<split_size) //分割点还没有检测完毕
            {
                split_point point1 = temp_save.at(m).at(n);
                split_point point2 = section_split.at(count);
                if(point2.x == 13&&point2.y == 408)
                {
                    qDebug()<<"sb";
                }
                if(point1.x==point2.x&&point1.y== point2.y) //遇到分割点
                {
                    insert_points.append(point1);
                    feature_point2.append(insert_points);
                    insert_points.clear();
                    n--;
                    count++;
                }
                else
                {
                    insert_points.append(point1);
                }
            }
            else  //分割点已经检测完毕，将剩余型值点统统作为一段上的型值点保存起来
            {
                split_point point1 = temp_save.at(m).at(n);
                insert_points.append(point1);
            }
        }
        feature_point2.append(insert_points);
        insert_points.clear();
    }
    section_split.clear();
}

/********************************************
 *function:for solution3的封闭环分裂过程   根据型值点来分割
 *input:
 *output:
 *adding:比较简单 就是型值点中每两个一组
 *author: wong
 *date: 2018/1/26
 *******************************************/
void curveFit::splitProcess3()
{
    QVector<QVector<split_point>> temp_save = feature_point2;
    feature_point2.clear();
    QVector<split_point>  insert_points;
    insert_points.clear();
    int size = temp_save.at(0).size();
    for(int m = 0;m<size-1;m++)
    {
        insert_points.append(temp_save.at(0).at(m));
        insert_points.append(temp_save.at(0).at(m+1));
        feature_point2.append(insert_points);
        insert_points.clear();
    }
}

/********************************************
 *function:将feature_point2中的奇异点除去，size为1的Vector
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/25
 *******************************************/
void curveFit::clearWeirdSplitPoint()
{
    QVector<int> index;
    int size = feature_point2.size();
    for(int i = 0;i<size;i++)
    {
        int size = feature_point2.at(i).size();
        if(size == 1)
        {
            index.append(i);
        }
    }
    size = index.size();
    for(int i = size-1;i>=0;i--) //必须是从后往前删除  不然index对不上
    {
        feature_point2.remove(index.at(i));
    }
}

/********************************************
 *function:将型值点添加到图上显示出来
 *input:index 第几幅图片
 *output:
 *adding:
 *author: wong
 *date: 2018/1/11
 *******************************************/
void curveFit::fillSplitPoints()
{
    int size = feature_point.size();
    int row,col;
    for(int i = 0;i<size;i++)
    {
        row = feature_point.at(i).x;
        col = feature_point.at(i).y;
        show_it.at<Vec3b>(row,col)[0] =split_color.B;
        show_it.at<Vec3b>(row,col)[1] = split_color.G;
        show_it.at<Vec3b>(row,col)[2] = split_color.R;
    }
}

/********************************************
 *function:自定义排序函数   升序排列
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/11
 *******************************************/
 bool curveFit::sortByIndex(const split_point &v1, const split_point &v2)
{
    return v1.index<v2.index;  //升序
}

/********************************************
 *function:检查Code中从index开始的groupsize个像素点是否已经存在于型值点容器中
 *input:
 *output:true  已经存在    false  未存在
 *adding:
 *author: wong
 *date: 2018/1/11
 *******************************************/
bool curveFit::checkVectorExistence(Code input, int index)
{
   for(int i = index;i<index+groupSize;i++)
   {
       if(checkExistence(input.point.at(i).i,input.point.at(i).j))
       {
           return true;
       }
   }
   return false;
}

/********************************************
 *function:for solution 2
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/24
 *******************************************/
bool curveFit::checkVectorExistence2(Code input, int index)
{
    for(int i = index;i<index+groupSize;i++)
    {
        if(checkExistence2(input.point.at(i).i,input.point.at(i).j))
        {
            return true;
        }
    }
    return false;
}

/********************************************
 *function:检查分段最大值点是否已经在型值点容器feature_point中
 *input:
 *output:
 *adding:true 存在
 *author: wang
 *date: 2018/1/9
 *******************************************/
bool curveFit::checkExistence(int X,int Y)
{
    int size = feature_point.size();
    for(int i = 0;i<size;i++)
    {
        if(feature_point.at(i).x==X&&feature_point.at(i).y== Y)
            return true;
    }
    return false;
}

/********************************************
 *function:for solution2
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/24
 *******************************************/
bool curveFit::checkExistence2(int X, int Y)
{
    int size = feature_point2.size();
    for(int m= 0;m<size;m++)
    {
        int size1 = feature_point2.at(m).size();
        for(int i = 0;i<size1;i++)
        {
            if(feature_point2.at(m).at(i).x==X&&feature_point2.at(m).at(i).y== Y)
                return true;
        }
    }
    return false;
}

/********************************************
 *function:根据型值点的分段   计算B样条
 *input:
 *output:
 *adding:根据型值点来计算B样条，计算出的B样条存放到Single_curve中
 *           参考论文《图像无级平滑缩放算法研究》
 *           AX = D      A系数矩阵   X控制点   D型值点
 *           A = LU       将A进行LU分解，分解具有唯一性   LUX =D  令UX=Y
 *           根据下二对角阵L和矩阵D进行第一次解方程，得到矩阵Y
 *           再根据UX=Y  U是上二对角阵和矩阵Y进行第二次解方程，得到矩阵X   得到控制点坐标
 *author: wang
 *date: 2018/1/9
 *******************************************/
Spline_Node curveFit::caculateSpline()
{
    Spline_Node result;
    fillParameter();  //根据型值点填充参数
    //用追赶法求解三次均匀B样条的控制点  将系数矩阵进行LU分解，分解成单位下二对角阵和上二对角阵
    solveEquationsFirst();
    solveEquationsSecond();

//    qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!curve fit";
    result.points.clear();
    point_f point;
    point.x = 2*coef_D.at(0).i - coef_D.at(1).i;
    point.y = 2*coef_D.at(0).j - coef_D.at(1).j;
    result.points.append(point);
    int size = coef_D.size();
    for(int i = 0;i<size;i++)
    {
        point.x = coef_D.at(i).i;
        point.y = coef_D.at(i).j;
        result.points.append(point);
//        qDebug()<<point.x<<" "<<point.y;
    }
    point.x = 2*coef_D.at(size-1).i - coef_D.at(size-2).i;
    point.y = 2*coef_D.at(size-1).j - coef_D.at(size-2).j;
    result.points.append(point);

    return result;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/25
 *******************************************/
Spline_Node curveFit::caculateSpline_zhu()
{
    Spline_Node result;
    fillParameter_zhu();  //根据型值点填充参数
    //用追赶法求解三次均匀B样条的控制点  将系数矩阵进行LU分解，分解成单位下二对角阵和上二对角阵
    solveEquationsFirst();
    solveEquationsSecond();

    result.points.clear();
    point_f point;
    point.x = feature_point.at(0).x;
    point.y = feature_point.at(0).y;
    result.points.append(point);
    int size = coef_D.size();
    for(int i = 0;i<size;i++)
    {
        point.x = coef_D.at(i).i;
        point.y = coef_D.at(i).j;
        result.points.append(point);
    }
    point.x = feature_point.at(0).x;
    point.y = feature_point.at(0).y;
    result.points.append(point);

    return result;
}

/********************************************
 *function:对若干段像素点进行曲线拟合
 *input:
 *output:若干段B样条的控制点
 *adding:
 *author: wong
 *date: 2018/1/23
 *******************************************/
QVector<Spline_Node> curveFit::caculateSpline2()
{
    QVector<Spline_Node> result;
    Spline_Node section;
    fillParameter2();  //根据型值点填充参数
    //用追赶法求解三次均匀B样条的控制点  将系数矩阵进行LU分解，分解成单位下二对角阵和上二对角阵
    solveEquationsFirst2();
    solveEquationsSecond2();

    result.clear();

    int size = feature_point2.size();
    for(int m = 0;m<size;m++)
    {
        point_f point;
        point.x = 2*coef_D2.at(m).at(0).i - coef_D2.at(m).at(1).i;
        point.y = 2*coef_D2.at(m).at(0).j - coef_D2.at(m).at(1).j;
        section.points.append(point);
        int size1 = coef_D2.at(m).size();
        for(int n = 0;n<size1;n++)
        {
            point.x = coef_D2.at(m).at(n).i;
            point.y = coef_D2.at(m).at(n).j;
            section.points.append(point);
        }
        point.x = 2*coef_D2.at(m).at(size1-1).i - coef_D2.at(m).at(size1-2).i;
        point.y = 2*coef_D2.at(m).at(size1-1).j - coef_D2.at(m).at(size1-2).j;
        section.points.append(point);
        result.append(section);
        section.points.clear();
    }
    return result;
}

/********************************************
 *function:反算B样条控制点前，先根据feature_point中的型值点来填充矩阵参数  abcd
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/12
 *******************************************/
void curveFit::fillParameter()
{
    //填充D容器  型值点   size 个
    Image_IJ point;
    split_point split;
    coef_D.clear();
    int size = feature_point.size();
    for(int i = 0;i<size;i++)
    {
        split = feature_point.at(i);
        point.i = 6*split.x;
        point.j =6* split.y;
        coef_D.append(point);
    }
//    point.i = 6*feature_point.at(0).x;
//    point.j = 6*feature_point.at(0).y;
//    coef_D.append(point);//回到起点，形成封闭环

    //填充B容器   B是矩阵对角线元素   size 个
    coef_B.clear();
    coef_B.append(6.0);
    for(int i = 1;i<size-1;i++)
    {
        coef_B.append(4.0);
    }
    coef_B.append(6.0);

    //填充C元素  C是对角线上三角元素  size-1个
    coef_C.clear();
    for(int i = 0;i<size-1;i++)
    {
        if(0 == i)
        {
            coef_C.append(0.0);
        }
        else
        {
            coef_C.append(1.0);
        }
    }

    //填充A元素  A是对角线下三角元素  size-1个
    coef_A.clear();
    for(int i = 0;i<size-1;i++)
    {
        if((size-2) == i)
        {
            coef_A.append(0.0);
        }
        else
        {
            coef_A.append(1.0);
        }
    }
}

/********************************************
 *function:按朱心雄的三次B样条反算方法来填充矩阵值
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/25
 *******************************************/
void curveFit::fillParameter_zhu()
{
    //填充D容器  型值点   size +1 个
    Image_IJ point;
    split_point split;
    coef_D.clear();
    split = feature_point.at(0);
    point.i = 6*split.x;
    point.j = 6*split.y;
    coef_D.append(point);
    int size = feature_point.size();
    for(int i = 1;i<size;i++)  //从1开始
    {
        split = feature_point.at(i);
        point.i = split.x;
        point.j =split.y;
        coef_D.append(point);
    }
    point.i = 6*feature_point.at(0).x;
    point.j = 6*feature_point.at(0).y;
    coef_D.append(point);//回到起点，形成封闭环

    //填充B容器   B是矩阵对角线元素   size +1 个
    coef_B.clear();
    coef_B.append(9.0);
    coef_B.append(7.0/12.0);
    for(int i = 0;i<size-3;i++)
    {
        coef_B.append(2.0/3.0);
    }
    coef_B.append(7.0/12.0);
    coef_B.append(9.0);

    //填充C元素  C是对角线上三角元素  size个
    coef_C.clear();
    coef_C.append(-3.0);
    for(int i = 0;i<size-2;i++)
    {
         coef_C.append(1.0/6.0);
    }
    coef_C.append(1.0/4.0);

    //填充A元素  A是对角线下三角元素  size个
    coef_A.clear();
    coef_A.append(1.0/4.0);
    for(int i = 1;i<size-1;i++)
    {
        coef_A.append(1/6);
    }
    coef_A.append(-3.0);
}

/********************************************
 *function:solution2填充参数
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/23
 *******************************************/
void curveFit::fillParameter2()
{
    //填充D容器  型值点   size  个
    QVector<Image_IJ> section_points;
    int size1 = feature_point2.size();
    Image_IJ point;
    split_point split;
    coef_D2.clear();
    for(int m = 0;m<size1;m++)
    {
        int size = feature_point2.at(m).size();
        for(int i = 0;i<size;i++)
        {
            split = feature_point2.at(m).at(i);
            point.i = 6*split.x;
            point.j = 6*split.y;
            section_points.append(point);
        }
//        point.i = 6*feature_point2.at(m).at(0).x;
//        point.j = 6*feature_point2.at(m).at(0).y;
//        section_points.append(point);//回到起点，形成封闭环
        coef_D2.append(section_points);
        section_points.clear();
    }



    //填充B容器   B是矩阵对角线元素   size  个
    QVector<float> section_B;
    coef_B2.clear();
    int size2 = feature_point2.size();
    for(int m = 0;m<size2;m++)
    {
        int size = feature_point2.at(m).size();
        section_B.append(6.0);
        for(int i = 1;i<size-1;i++)
        {
            section_B.append(4.0);
        }
        section_B.append(6.0);
        coef_B2.append(section_B);
        section_B.clear();
    }


//    //填充C元素  C是对角线上三角元素  size-1个
    QVector<float> section_C;
    coef_C2.clear();
    int size3 = feature_point2.size();
    for(int m = 0;m<size3;m++)
    {
        int size = feature_point2.at(m).size();
        for(int i = 0;i<size-1;i++)
        {
            if(0 == i)
            {
                section_C.append(0.0);
            }
            else
            {
                section_C.append(1.0);
            }
        }
        coef_C2.append(section_C);
        section_C.clear();
    }


//    //填充A元素  A是对角线下三角元素  size-1个
    QVector<float> section_A;
    coef_A2.clear();
    int size4 = feature_point2.size();
    for(int m = 0;m<size4;m++)
    {
        int size = feature_point2.at(m).size();
        for(int i = 0;i<size-1;i++)
        {
            if((size-2) == i)
            {
                section_A.append(0.0);
            }
            else
            {
                section_A.append(1.0);
            }
        }
        coef_A2.append(section_A);
        section_A.clear();
    }
}

/********************************************
 *function:解方程 LUX =D  令UX=Y   LY=D  求Y
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/12
 *******************************************/
void curveFit::solveEquationsFirst()
{
    int size = coef_A.size();
    for(int i = 0;i<size;i++)
    {
        coef_A[i] = coef_A.at(i)/coef_B.at(i);  //li = ai/b(i-1)
    }

    size = coef_B.size();
    coef_B[0] = coef_B.at(0);  //u1 = b1
    for(int i = 1;i<size;i++)
    {
        coef_B[i]= coef_B.at(i) - coef_C.at(i-1)*coef_A.at(i-1);  //ui = bi -c(i-1)l(i-1)
    }

    size = coef_D.size();
    coef_D[0] = coef_D.at(0);
    for(int i= 1;i<size;i++)
    {
        coef_D[i].i = coef_D.at(i).i -coef_D.at(i-1).i*coef_A.at(i-1);
        coef_D[i].j = coef_D.at(i).j -coef_D.at(i-1).j*coef_A.at(i-1);
    }
}

/********************************************
 *function:solution1进行第一次解方程
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/24
 *******************************************/
void curveFit::solveEquationsFirst2()
{
    int size1 = feature_point2.size();
    for(int m = 0;m<size1;m++)
    {
        int size = coef_A2.at(m).size();
        for(int i = 0;i<size;i++)
        {
            coef_A2[m][i] = coef_A2.at(m).at(i)/coef_B2.at(m).at(i);  //li = ai/b(i-1)
        }
    }

    for(int m = 0;m<size1;m++)
    {
        int size = coef_B2.at(m).size();
        coef_B2[m][0] = coef_B2.at(m).at(0);  //u1 = b1
        for(int i = 1;i<size;i++)
        {
            coef_B2[m][i]= coef_B2.at(m).at(i) - coef_C2.at(m).at(i-1)*coef_A2.at(m).at(i-1);  //ui = bi -c(i-1)l(i-1)
        }
    }

    for(int m = 0;m<size1;m++)
    {
        int size = coef_D2.at(m).size();
        coef_D2[m][0] = coef_D2.at(m).at(0);
        for(int i= 1;i<size;i++)
        {
            coef_D2[m][i].i = coef_D2.at(m).at(i).i -coef_D2.at(m).at(i-1).i*coef_A2.at(m).at(i-1);
            coef_D2[m][i].j = coef_D2.at(m).at(i).j -coef_D2.at(m).at(i-1).j*coef_A2.at(m).at(i-1);
        }
    }
}

/********************************************
 *function:解方程UX=Y  根据U和Y来解出X
 *input:
 *output:
 *adding:最后得到的coef_D就是要求的控制点
 *author: wong
 *date: 2018/1/12
 *******************************************/
void curveFit::solveEquationsSecond()
{
    int size = coef_D.size();
    coef_D[size-1].i = coef_D.at(size-1).i/coef_B.at(size-1);
    coef_D[size-1].j = coef_D.at(size-1).j/coef_B.at(size-1);
    for(int i = size -2;i>=0;i--) //上三角矩阵，从n往1倒推
    {
        coef_D[i].i = (coef_D.at(i).i - coef_D.at(i+1).i*coef_C.at(i))/coef_B.at(i);
        coef_D[i].j = (coef_D.at(i).j - coef_D.at(i+1).j*coef_C.at(i))/coef_B.at(i);
    }
}

/********************************************
 *function:solution进行第二次解方程
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/24
 *******************************************/
void curveFit::solveEquationsSecond2()
{
    int size1 = feature_point2.size();
    for(int m = 0;m<size1;m++)
    {
        int size = coef_D2.at(m).size();
        coef_D2[m][size-1].i = coef_D2.at(m).at(size-1).i/coef_B2.at(m).at(size-1);
        coef_D2[m][size-1].j = coef_D2.at(m).at(size-1).j/coef_B2.at(m).at(size-1);
        for(int i = size -2;i>=0;i--) //上三角矩阵，从n往1倒推
        {
            coef_D2[m][i].i = (coef_D2.at(m).at(i).i - coef_D2.at(m).at(i+1).i*coef_C2.at(m).at(i))/coef_B2.at(m).at(i);
            coef_D2[m][i].j = (coef_D2.at(m).at(i).j - coef_D2.at(m).at(i+1).j*coef_C2.at(m).at(i))/coef_B2.at(m).at(i);
        }
    }
}

/********************************************
 *function:返回当前像素点与样条曲线的最小距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/9
 *******************************************/
float curveFit::findMinDistance()
{

}

/********************************************
 *function:检查当前拟合的样条曲线是否达到指定的精度要求
 *input:
 *output:true 达到精度要求    false 未达到精度要求
 *adding:
 *author: wang
 *date: 2018/1/9
 *******************************************/
bool curveFit::checkPrecision(Code input1,Spline_Node input2)
{
    bool result = true;
    bool Skip= false;
    int count = 1;
    split_point point;
    float max_distance = 0;
    float temp_distance = 0;
//    Image_IJ end = input1.point.at(0);
//    input1.point.append(end);
    int size = input1.point.size();
    QVector<split_point>  new_feature_point = feature_point;
//    new_feature_point.append(feature_point.at(0));//特征点最后再追加一个开始点才能和分闭环对应上
    int size2 = new_feature_point.size();//型值点个数
    for(int i = 0;i<size;i++)
    {
//        Skip = false;
        if(count ==size2) //到最后了，结束循环。防止下面访问容器越界
            break;
        int a,b,c,d;
        a = input1.point.at(i).i;
        b = new_feature_point.at(count).x;
        c = input1.point.at(i).j;
        d = new_feature_point.at(count).y;

        if(a==b&&c==d)
        {
            if(max_distance>precision)
            {
                temp_feature_point.append(point);
                result = false;
            }
            count++;
            if(count ==size2) //到最后了，结束循环。防止下面访问容器越界
                break;
            while(new_feature_point.at(count).x==new_feature_point.at(count-1).x&&new_feature_point.at(count).y==new_feature_point.at(count-1).y)
            {
                count++;  //跳过重节点
                if(count ==size2) //到最后了，结束循环。防止下面访问容器越界
                    break;
            }
            max_distance = 0;
            temp_distance = 0;
//            Skip = true;
            continue;
        }

        if(count ==size2) //到最后了，结束循环。防止下面访问容器越界
            break;
        temp_distance = distance_errror(input1.point.at(i),input2,count);
//        temp_distance = distance_error_entirety(input1.point.at(i),input2);
        if(temp_distance>max_distance)
        {
            max_distance = temp_distance;
            point.x = input1.point.at(i).i;
            point.y = input1.point.at(i).j;
            point.index = i;
        }

    }
    return result;
}

/********************************************
 *function:solution2的精度检查函数
 *input:
 *output:
 *adding:与solution1的区别在于需要分段保存
 *author: wong
 *date: 2018/1/24
 *******************************************/
bool curveFit::checkPrecision2(Code input1, QVector<Spline_Node> input2)
{
    bool result = true;
    int section_size = feature_point2.size();  //分段数目
    int section_index = 0;
    section_split.clear();
    QVector<split_point> section_new_features;
    section_new_features.clear();

    bool Skip= false;
    int count = 1;
    split_point point;
    float max_distance = 0;
    float temp_distance = 0;
    Image_IJ end = input1.point.at(0);
    input1.point.append(end);
    int size = input1.point.size();

    int size2 = feature_point2.at(section_index).size();//型值点个数
    for(int i = 0;i<size;i++)
    {
        Skip = false;
        int a,b,c,d;
        a = input1.point.at(i).i;
        b = feature_point2.at(section_index).at(count).x;
        c = input1.point.at(i).j;
        d = feature_point2.at(section_index).at(count).y;

        if(a==b&&c==d)  //像素点遇到型值点
        {
            if(max_distance>precision)
            {
                if(istooClose(point,feature_point2.at(section_index))) //如果新的型值点和原有的型值点距离过近
                {
                    section_split.append(point);
                }
                else //符合添加新的型值点的要求
                {
                    section_new_features.append(point);
                    result = false;
                }
            }
            count++;
            max_distance = 0;
            temp_distance = 0;
            Skip = true;
        }

        if(count ==size2) //边界检查
        {
            if(section_index == section_size -1)
            {
                temp_feature_point2.append(section_new_features);
                section_new_features.clear();
                break;
            }
            else
            {
                section_index++;
                size2 = feature_point2.at(section_index).size();
                count = 1;
                temp_feature_point2.append(section_new_features);
                section_new_features.clear();
            }
        }

        if(!Skip)
        {
            temp_distance = distance_errror(input1.point.at(i),input2.at(section_index),count);
    //        temp_distance = distance_error_entirety(input1.point.at(i),input2);
            if(temp_distance>max_distance)
            {
                max_distance = temp_distance;
                point.x = input1.point.at(i).i;
                point.y = input1.point.at(i).j;
                point.index = i;
            }
        }

    }

    return result;
}

/********************************************
 *function:solution3的精度检查函数
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/26
 *******************************************/
bool curveFit::checkPrecision3(Code input1, QVector<Spline_Node> input2)
{
    bool result = true;
    int section_size = feature_point2.size();  //分段数目
    int section_index = 0;
    QVector<split_point> section_new_features;
    section_new_features.clear();

    bool Skip= false;
    int count = 1;  //count代表第几个型值点
    split_point point;
    float max_distance = 0;
    float temp_distance = 0;
    Image_IJ end = input1.point.at(0);
    input1.point.append(end);
    int size = input1.point.size();

    int size2 = feature_point2.at(section_index).size();//当前段的型值点个数
    for(int i = 0;i<size;i++)
    {
        Skip = false;
        int a,b,c,d;
        a = input1.point.at(i).i;
        b = feature_point2.at(section_index).at(count).x;
        c = input1.point.at(i).j;
        d = feature_point2.at(section_index).at(count).y;

        if(a==b&&c==d)  //像素点遇到型值点
        {
            if(max_distance>precision)
            {
                section_new_features.append(point);
                result = false;
            }
            count++;
            max_distance = 0;
            temp_distance = 0;
            Skip = true;
        }

        if(count ==size2) //边界检查
        {
            if(section_index == section_size -1)
            {
                temp_feature_point2.append(section_new_features);
                section_new_features.clear();
                break;
            }
            else
            {
                section_index++;
                size2 = feature_point2.at(section_index).size();
                count = 1;
                temp_feature_point2.append(section_new_features);
                section_new_features.clear();
            }
        }

        if(!Skip)
        {
            temp_distance = distance_errror(input1.point.at(i),input2.at(section_index),count);
    //        temp_distance = distance_error_entirety(input1.point.at(i),input2);
            if(temp_distance>max_distance)
            {
                max_distance = temp_distance;
                point.x = input1.point.at(i).i;
                point.y = input1.point.at(i).j;
                point.index = i;
            }
        }

    }
    return result;
}

/********************************************
 *function:检查输入的误差较大的像素点和原有的特征点是否过小
 *input:注意input1是引用类型 当返回true时，利用该变量来传回分割点
 *output:true :过小  作为分割点      false: 距离较大  作为新的型值点
 *adding:小于split_precision就是要作为分割点
 *author: wong
 *date: 2018/1/24
 *******************************************/
bool curveFit::istooClose(split_point &input1, QVector<split_point> input2)
{
    bool result = false;
    float distance;
    int size = input2.size();
    for(int i = 0;i<size;i++)
    {
        distance =computeDistance(input1,input2.at(i));
        if(distance<split_precision)
        {
            input1.index = input2.at(i).index;
            input1.x = input2.at(i).x;
            input1.y = input2.at(i).y;
            return true;
        }
    }
    return result;
}

/********************************************
 *function:计算图上某个像素点和对应段B样条的距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/13
 *******************************************/
float curveFit::distance_errror(Image_IJ input1, Spline_Node input2, int index)
{
    float min_distance;
    float temp_distance;
    float delta = 1.0/spline_size;
    int size = input2.points.size();
    point_f cur_point;
    for(float u = 0;u<=1;u = u+delta)
    {
        cur_point = splinePoint(u,input2.points.at(index-1),input2.points.at(index),input2.points.at(index+1),input2.points.at(index+2));
        temp_distance = calculatePointDistance(input1,cur_point);
        if(u == 0)
        {
            min_distance = temp_distance;
        }
        else
        {
            if(temp_distance<min_distance)
            {
                min_distance = temp_distance;
            }
        }
    }

    for(float u = 0;u<=1;u = u+delta)
    {
        if(index ==size - 3)
            break;
        else
        {
            cur_point = splinePoint(u,input2.points.at(index),input2.points.at(index+1),input2.points.at(index+2),input2.points.at(index+3));
        }
        temp_distance = calculatePointDistance(input1,cur_point);
        if(temp_distance<min_distance)
        {
            min_distance = temp_distance;
        }
    }
    return min_distance;
}

/********************************************
 *function:计算图上某个像素点和整个封闭环的B样条的最近距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/15
 *******************************************/
float curveFit::distance_error_entirety(Image_IJ input1, Spline_Node input2)
{
    float min_distance;
    float temp_distance;
    float delta = 1.0/spline_size;
    point_f cur_point;
    int size = input2.points.size();
    for(int i = 0;i<size-3;i++)
    {
        for(float j = 0;j<=1.0;j = j+delta)
        {
            cur_point = splinePoint(j,input2.points.at(i),input2.points.at(i+1),input2.points.at(i+2),input2.points.at(i+3));
            temp_distance = calculatePointDistance(input1,cur_point);
            if(i == 0&&j == 0)
            {
                min_distance = temp_distance;
            }
            else
            {
                if(temp_distance<min_distance)
                    min_distance = temp_distance;
            }
        }
    }
    return min_distance;
}

/********************************************
 *function:计算两点间距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/13
 *******************************************/
float curveFit::calculatePointDistance(Image_IJ point1, point_f point2)
{
    float delta_x = point1.i - point2.x;
    float delta_y = point1.j - point2.y;
    float distance = sqrt(delta_x*delta_x + delta_y*delta_y);
    return distance;
}

/********************************************
 *function:计算两像素点距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/25
 *******************************************/
float curveFit::computeDistance(split_point point1, split_point point2)
{
    float delta_x = point1.x - point2.x;
    float delta_y = point1.y - point2.y;
    float distance = sqrt(delta_x*delta_x + delta_y*delta_y);
    return distance;
}

/********************************************
 *function:根据u求出某一段样条曲线上的某一个离散点
 *input:u 范围在0-1之间
 *output:
 *adding:
 *author: wong
 *date: 2018/1/13
 *******************************************/
point_f curveFit::splinePoint(float u, point_f input1, point_f input2, point_f input3, point_f input4)
{
    point_f result;
    float par1,par2,par3,par4;
    par1 = (1-u)*(1-u)*(1-u)/6;
    par2 = (3*u*u*u-6*u*u+4)/6;
    par3 = (-3*u*u*u+3*u*u+3*u+1)/6;
    par4 = u*u*u/6;
    result.x = par1*input1.x + par2*input2.x + par3*input3.x + par4*input4.x;
    result.y = par1*input1.y + par2*input2.y + par3*input3.y + par4*input4.y;
    return result;
}

/********************************************
 *function:根据封闭环上的特征点将封闭环离散成若干段曲线，留作后拟合
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/9
 *******************************************/
void curveFit::splitContinueCurve()
{

}

/********************************************
 *function:根据输入的像素点集和类内部的全局变量feature_points来求拟合的Bezier曲线控制点
 *input:图像点集chain_points(是一个封闭环)，带有内外环信息的一个封闭环
 *output:一个封闭环的Bezier表示
 *adding:隐含的输入信息还有feature_point
 *author: wong
 *date: 2018/2/2
 *******************************************/
QVector<bezier> curveFit::caculateBezierCurve(Code chain_points)
{
    QVector<bezier> result;  //返回的结果
    findLimitValuePoint_Bezier(chain_points);  //极值点  起点和终点重复  封闭
    findMaxValuePoint_Bezier(chain_points);  //分段最大值点
    addFeaturePoints_Bezier(); //添加
    sortFeaturePoints_Bezier();//排序
    QVector<Code> curves = splitChainList(chain_points);  //将封闭环打成几段连续像素
    feature_point.clear();
    temp_feature_point.clear();
    int size = curves.size();
    for(int i = 0;i<size;i++)
    {
        Code some_points = curves.at(i);
        bezier segment = segmentBezierCurve(some_points); //当前段进行第一次拟合，第一次拟合只会有一段bezier
        check_segment_precision.append(segment);
        check_segment_pixels.append(curves.at(i));
        while(!checkBezierPrecesion()) //检查精度
        {
            addFeaturePoints_Bezier();
            sortFeaturePoints_Bezier();
            check_segment_pixels.clear();
            check_segment_precision.clear();
            check_segment_pixels = splitChainList2(curves.at(i));  // 像素点分离函数有问题
            check_segment_precision = segmentBezierCurves(check_segment_pixels);
        }
        feature_point.clear();
        temp_feature_point.clear();
        int size2= check_segment_precision.size();
        for(int j = 0;j<size2;j++)
        {
            result.append(check_segment_precision.at(j));
        }
        check_segment_precision.clear();
        check_segment_pixels.clear();
    }

    //让每一段Bezier曲线带上内外环信息
    int result_size = result.size();
    for(int k = 0;k<result_size;k++)
    {
        result[k].isouter = chain_points.outer;
    }
    return result;
}

/********************************************
 *function:单段连续像素的Bezier曲线拟合
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/2
 *******************************************/
bezier curveFit::segmentBezierCurve(Code chain_points)
{
    bezier  result;
    result.isouter = chain_points.outer;
    float size = chain_points.point.size();
    float m = size -1;
    result.point1.setX((double)chain_points.point.at(0).i);
    result.point1.setY((double)chain_points.point.at(0).j);
    result.point4.setX((double)chain_points.point.at(size-1).i);
    result.point4.setY((double)chain_points.point.at(size-1).j);

    QVector<point_f>  points;

    if(m == 2)  //规避奇异值点  当m=2时，只有三个点。a=d=b=c  导致分母 ad-bc = 0
    {
        point_f insert;
        insert.x = chain_points.point.at(0).i;
        insert.y = chain_points.point.at(0).j;
        points.append(insert);
        insert.x = (chain_points.point.at(0).i +chain_points.point.at(1).i )/2 ;
        insert.y = (chain_points.point.at(0).j +chain_points.point.at(1).j )/2 ;
        points.append(insert);
        insert.x = chain_points.point.at(1).i;
        insert.y = chain_points.point.at(1).j;
        points.append(insert);
        insert.x = (chain_points.point.at(1).i +chain_points.point.at(2).i )/2 ;
        insert.y = (chain_points.point.at(1).j +chain_points.point.at(2).j )/2 ;
        points.append(insert);
        insert.x = chain_points.point.at(2).i;
        insert.y = chain_points.point.at(2).j;
        points.append(insert);
        m = 4;
    }
    else if(m == 1)   //规避奇异值点  当m=2时，只有两个点。
    {
        result.point2.setX((double)chain_points.point.at(0).i);
        result.point2.setY((double)chain_points.point.at(0).j);
        result.point3.setX((double)chain_points.point.at(size-1).i);
        result.point3.setY((double)chain_points.point.at(size-1).j);
        return result;
    }
    else
    {
        point_f insert;
        for(int i = 0;i<size;i++)
        {
            insert.x = chain_points.point.at(i).i;
            insert.y = chain_points.point.at(i).j;
            points.append(insert);
        }
    }


    float a,b,c,d;
    float gx,gy,hx,hy;
    //计算参数a
    a = 0;
    for(float i = 0;i<=m;i++)
    {
        float add = BernsteinValue(1,i/m);
        a =a + add*add;
    }

    //计算参数b
    b = 0;
    for(float i = 0;i<=m;i++)
    {
        float add = BernsteinValue(1,i/m)*BernsteinValue(2,i/m);
        b =b + add;
    }

    //参数b和c一样
    c = b;

    //计算参数d
    d = 0;
    for(float i = 0;i<=m;i++)
    {
        float add = BernsteinValue(2,i/m);
        d =d + add*add;
    }

    //计算参数g
    gx = 0; gy = 0;
    for(float i = 0;i<=m;i++)
    {
        float add_x = BernsteinValue(1,i/m)*points.at(i).x-points.at(0).x*BernsteinValue(1,i/m)*BernsteinValue(0,i/m) - points.at(m).x*BernsteinValue(1,i/m)*BernsteinValue(3,i/m) ;
        float add_y = BernsteinValue(1,i/m)*points.at(i).y-points.at(0).y *BernsteinValue(1,i/m)*BernsteinValue(0,i/m) - points.at(m).y*BernsteinValue(1,i/m)*BernsteinValue(3,i/m) ;
        gx = gx + add_x;
        gy = gy + add_y;
    }

    //计算参数h
    hx = 0;hy = 0;
    for(float i = 0;i<=m;i++)
    {
        float add_x = BernsteinValue(2,i/m)*points.at(i).x - points.at(0).x*BernsteinValue(2,i/m)*BernsteinValue(0,i/m) - points.at(m).x*BernsteinValue(2,i/m)*BernsteinValue(3,i/m);
        float add_y = BernsteinValue(2,i/m)*points.at(i).y - points.at(0).y*BernsteinValue(2,i/m)*BernsteinValue(0,i/m) - points.at(m).y*BernsteinValue(2,i/m)*BernsteinValue(3,i/m) ;
        hx =hx + add_x;
        hy =hy + add_y;
    }

    if(a*d-b*c == 0)
    {
        qDebug()<<"NAN   Bezier...........";
    }
    //AB就是中间的两个控制点
    //计算参数A
    float Ax,Ay;
    Ax = (d*gx - b*hx)/(a*d-b*c);
    Ay = (d*gy - b*hy)/(a*d-b*c);
    //计算参数B
    float Bx,By;
    Bx = (gx- a*Ax)/b;
    By = (gy-a*Ay)/b;

    result.point2.setX(Ax);
    result.point2.setY(Ay);
    result.point3.setX(Bx);
    result.point3.setY(By);

    return result;
}

/********************************************
 *function:j计算单段连续像素的多个Bezier
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/3
 *******************************************/
QVector<bezier> curveFit::segmentBezierCurves(QVector<Code> input)
{
    QVector<bezier> result;
    int size = input.size();
    for(int i = 0;i<size;i++)
    {
        bezier  seg= segmentBezierCurve(input.at(i));
        result.append(seg);
    }
    return result;
}

/********************************************
 *function:找到连续像素中的曲率极值点(Bezier曲线拟合)
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/2
 *******************************************/
void curveFit::findLimitValuePoint_Bezier(Code input)
{
    //将轮廓起点加入
    split_point  add;
    add.index = 0;
    add.x = input.point.at(0).i;
    add.y = input.point.at(0).j;
    feature_point.append(add);

    float first,second,third;  //相邻三个的曲率
    int points_size = input.point.size()-1; //终点索引
    for(int i = 2*skip_num;i<points_size;i = i+skip_num)
    {
       if((i+2*skip_num)>points_size) //超出范围
       {
           break;
       }
       else
       {
           first = caculateCurvature(input.point.at(i-2*skip_num),input.point.at(i-skip_num),input.point.at(i));
           second = caculateCurvature(input.point.at(i-skip_num),input.point.at(i),input.point.at(i+skip_num));
           third = caculateCurvature(input.point.at(i),input.point.at(i+skip_num),input.point.at(i+2*skip_num));
           if(second>first&&second>third)
           {
               add.index = i;
               add.x = input.point.at(i).i;
               add.y = input.point.at(i).j;
               feature_point.append(add);
           }
       }
    }

    //将轮廓终点加入
    split_point  add1;
    add1.index = input.point.size()+1;
    add1.x = input.point.at(0).i;
    add1.y = input.point.at(0).j;
    feature_point.append(add1);
}

/********************************************
 *function:找到分段像素中的曲率最大值点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/2
 *******************************************/
void curveFit::findMaxValuePoint_Bezier(Code input)
{
    float max_curvature = 0; //记录每个分组中的最大曲率   初始化为0   直线上任意一点的曲率为0
    float temp_curvature;
    split_point  add;
    int points_size = input.point.size()-1; //终点索引
    for(int i = 1;i<points_size;i = i+groupSize)
    {
        if(i+groupSize>points_size)  //防止越界，剩余像素点个数不够一组
        {
            int rest_size = points_size - i;
            for(int j = 0;j<rest_size-1;j++)
            {
                if(j == (groupSize-1)&&max_curvature>0)
                {
                    max_curvature = 0;
                    temp_feature_point.append(add);
                }
                else
                {
                    temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                    if(temp_curvature>max_curvature)
                    {
                        add.index = j+i;
                        add.x = input.point.at(j+i).i;
                        add.y = input.point.at(j+i).j;
                        max_curvature = temp_curvature;
                    }
                }
            }
        }
        else
        {
            if(!checkVectorExistence(input,i)) //当前的一组像素点内不存在曲率极大值点，下面去寻找该分段内的曲率最大值点
            {
                for(int j = 0;j<groupSize;j++)
                {
                    if(j == (groupSize-1)&&max_curvature>0)
                    {
                        max_curvature = 0;
                        temp_feature_point.append(add);
                    }
                    else
                    {
                        temp_curvature = caculateCurvature(input.point.at(j+i-1),input.point.at(j+i),input.point.at(j+i+1));
                        if(temp_curvature>max_curvature)
                        {
                            add.index = j+i;
                            add.x = input.point.at(j+i).i;
                            add.y = input.point.at(j+i).j;
                            max_curvature = temp_curvature;
                        }
                    }
                }
            }
        }
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/2
 *******************************************/
void curveFit::addFeaturePoints_Bezier()
{
    int size = temp_feature_point.size();
    for(int i = 0;i<size;i++)
    {
        feature_point.append(temp_feature_point.at(i));
    }
    temp_feature_point.clear();
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/3
 *******************************************/
void curveFit::sortFeaturePoints_Bezier()
{
    std::sort(feature_point.begin(),feature_point.end(),sortByIndex);
}

/********************************************
 *function:根据特征点来分割封闭环，得到分段的连续点集
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/2
 *******************************************/
QVector<Code> curveFit::splitChainList(Code input)
{
    QVector<Code> result;
    Code some_pixel;
    Image_IJ start = input.point.at(0);
    input.point.append(start);
    int count = 1;
    int size = input.point.size();
    int size2 = feature_point.size();
    for(int i = 0;i<size;i++)
    {
        some_pixel.point.append(input.point.at(i));
        if(input.point.at(i).i==feature_point.at(count).x&&input.point.at(i).j==feature_point.at(count).y)
        {
            result.append(some_pixel);
            some_pixel.point.clear();
            count++;
            i--;
            if(count == size2)
                break;
        }
    }
    return result;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/11
 *******************************************/
QVector<Code> curveFit::splitChainList2(Code input)
{
    QVector<Code> result;
    Code some_pixel;
    int count = 0;
    int size = input.point.size();
    int size2 = feature_point.size();
    for(int i = 0;i<size;i++)
    {
        some_pixel.point.append(input.point.at(i));
        if(count<size2)
        {
            if(input.point.at(i).i==feature_point.at(count).x&&input.point.at(i).j==feature_point.at(count).y)
            {
                result.append(some_pixel);
                some_pixel.point.clear();
                count++;
                i--;
            }
        }
    }
    result.append(some_pixel);
    return result;
}

/********************************************
 *function:检查Bezier的分段拟合精度
 *input:
 *output:true:达到精度要求   false:没有达到
 *adding:
 *author: wong
 *date: 2018/2/3
 *******************************************/
bool curveFit::checkBezierPrecesion()
{
    bool  result= true;
    float max_distance = 0;
    split_point  add;
    int index= 0;
    int size = check_segment_precision.size();
    for(int i = 0;i<size;i++)
    {
        bezier check_curve = check_segment_precision.at(i);
        Code check_pixels = check_segment_pixels.at(i);
        int size2 = check_pixels.point.size();
        for(int j = 0;j<size2;j++)  //求出每一小段中的最大点
        {
            Image_IJ pixel_point= check_pixels.point.at(j);
            point_f curve_point = getBezierPoint(check_curve,j,size2-1);
            float distance = caculateDistance(pixel_point,curve_point);
            if(distance>max_distance)
            {
                max_distance = distance;
                if(i == 0)  //index搞错会影响排序，就会影响分段，漏掉特征点，无法满足精度，造成死循环
                {
                    add.index = index+j;
                }
                else
                {
                    add.index = index+j -i;
                }

                add.x = pixel_point.i;
                add.y = pixel_point.j;
            }
        }
        if(max_distance>precision)
        {
            result = false;
            temp_feature_point.append(add);
            max_distance = 0;
        }
        index = index + size2;
    }
    return result;
}

/********************************************
 *function:根据像素点个序号去除对应的拟合曲线上的点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/3
 *******************************************/
point_f curveFit::getBezierPoint(bezier curve, int i, int size)
{
    point_f  result;
    float u = (float)i/(float)size;
    float a,b,c,d;
    a = (1-u)*(1-u)*(1-u);
    b = 3* (1-u)*(1-u)*u;
    c = 3*(1-u)*u*u;
    d = u*u*u;
    result.x = a*curve.point1.x() + b*curve.point2.x() +c*curve.point3.x() +d*curve.point4.x();
    result.y = a*curve.point1.y() + b*curve.point2.y() +c*curve.point3.y() +d*curve.point4.y();
    return result;
}

/********************************************
 *function:计算亮点间的距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/3
 *******************************************/
float curveFit::caculateDistance(Image_IJ input1, point_f input2)
{
    float delta_x = input1.i - input2.x;
    float delta_y = input1.j - input2.y;
    float distance = sqrt(delta_x*delta_x+delta_y*delta_y);
    return distance;
}

/********************************************
 *function:贝恩斯坦函数值
 *input: index是第几个贝恩斯坦函数   i是0~1的某个离散值
 *output:
 *adding:这边默认是3次的
 *author: wong
 *date: 2018/2/6
 *******************************************/
float curveFit::BernsteinValue(int index, float i)
{
    float result;
    switch(index)
    {
    case 0:
    {
        result = (1-i)*(1-i)*(1-i);
        break;
    }
    case 1:
    {
        result = 3*(1-i)*(1-i)*i;
        break;
    }
    case 2:
    {
        result = 3*(1-i)*i*i;
        break;
    }
    case 3:
    {
        result = i*i*i;
        break;
    }
    default:
        break;
    }
    return result;
}


