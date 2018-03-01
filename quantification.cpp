#include "quantification.h"

Quantification::Quantification(QObject *parent) : QObject(parent)
{
    Head = NULL;
    Head_216 = NULL;
    target_colorNum = 256;
}

/********************************************
 *function:析构函数
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/6
 *******************************************/
Quantification::~Quantification()
{
//    RGB_Lab *temp = Head;
//    RGB_Lab *del;
//    while(temp)
//    {
//        del = temp->Next;
//        delete temp;
//        temp = del;
//    }
//    temp = Head_216;
//    while(temp)
//    {
//        del = temp->Next;
//        delete temp;
//        temp = del;
//    }
//    qDebug()<<"delete succesfully";
}

/********************************************
 *function:RGB空间的调色板量化方法
 *input:
 *output:
 *adding:在RGB空间中选取256个采样点作为调色板选项
 *author: wang
 *date: 2017/12/4
 *******************************************/
void Quantification::RGB216_quantity()
{
    QProgressDialog process;
    process.setWindowTitle(tr("颜色量化"));
    process.setLabelText(tr("正在量化图片..."));
    process.setRange(0,Image.rows);//范围
    process.setModal(true);
    process.show();

    Pixel point1,point2;//用来存储像素点的RGB值
    //遍历所有像素点
    for(int i = 0;i<Image.rows;i++)
    {
        process.setValue(i);//更新计算进度条
        QCoreApplication::processEvents(); //防止冻结，让进度条显示出来
        for(int j = 0;j<Image.cols;j++)
        {
            //注意通道对应的意义
            point1.B = Image.at<Vec3b>(i,j)[0];
            point1.G = Image.at<Vec3b>(i,j)[1];
            point1.R = Image.at<Vec3b>(i,j)[2];
            point2 = getReplacePixel(point1);
            Image_quantity.at<Vec3b>(i,j)[0] = point2.B;
            Image_quantity.at<Vec3b>(i,j)[1] = point2.G;
            Image_quantity.at<Vec3b>(i,j)[2] = point2.R;
        }
    }
    //统计量化后的颜色种类
    int color_num = 0;
    color_num = countRGB();
    qDebug()<<"第一种量化方法使用颜色数目"<<color_num;

}

/********************************************
 *function:
 *input:
 *output:
 *adding:参考论文  在均匀颜色空间中实现彩色图像的颜色量化 任智斌
 *author: wang
 *date: 2017/12/5
 *******************************************/
void Quantification::Lab_quantity()
{
    int color_num,origine_num;
    fill_RGB_Node();//读取图片
    color_num = countSize();
     qDebug()<<"Lab读取图片中的颜色种类"<<color_num;
    origine_num = color_num;
    if(color_num<=target_colorNum) //图片中颜色种类比目标值小不需要进行量化处理，可以直接进行颜色聚类
    {
        for(int i = 0;i<Image_quantity.rows;i++)
        {
            for(int j = 0;j<Image_quantity.cols;j++)
            {
                if(Image_quantity.at<Vec3b>(i,j)[0]==255&&\
                   Image_quantity.at<Vec3b>(i,j)[1]==255&&\
                        Image_quantity.at<Vec3b>(i,j)[2]==255)
                {
                    continue;
                }
                Image_quantity.at<Vec3b>(i,j)[0] =compress(Image_quantity.at<Vec3b>(i,j)[0]) ;  //利用整型的地板除法特性压缩颜色种类，最多16*16*16=4096种
                Image_quantity.at<Vec3b>(i,j)[1] =compress(Image_quantity.at<Vec3b>(i,j)[1]);
                Image_quantity.at<Vec3b>(i,j)[2] = compress(Image_quantity.at<Vec3b>(i,j)[2]);
            }
        }
    }
    else //图片颜色超过目标值，需要转到Lab空间进行颜色量化
    {
        QProgressDialog process;
        process.setWindowTitle(tr("颜色量化"));
        process.setLabelText(tr("正在压缩图像像素信息..."));
        process.setRange(0,color_num - target_colorNum);//范围
        process.setModal(true);
        process.show();

        qDebug()<<"第二种量化：";
        qDebug()<<color_num<<"lager than "<<target_colorNum;
        RGB2Lab();//先将链表中的颜色转换到Lab空间
        while(color_num>target_colorNum)//超过目标值时一直合并颜色
        {
            mergeColor();
            color_num = countSize();
            process.setValue(origine_num-color_num);//更新计算进度条
            QCoreApplication::processEvents(); //防止冻结，让进度条显示出来
             qDebug()<<color_num;
        }
        Lab2RGB();//再将链表中的颜色转换到RGB空间，得到标准调色板；


        QProgressDialog process2;
        process2.setWindowTitle(tr("颜色量化"));
        process2.setLabelText(tr("正在生成量化后图像..."));
        process2.setRange(0,Image.rows);//范围
        process2.setModal(true);
        process2.show();

        Pixel point1,point2;//用来存储像素点的RGB值
        //遍历所有像素点，找到替代的值
        for(int i = 0;i<Image.rows;i++)
        {
            process2.setValue(i);//更新计算进度条
            QCoreApplication::processEvents(); //防止冻结，让进度条显示出来
            for(int j = 0;j<Image.cols;j++)
            {
                //注意通道对应的意义
                point1.B = Image.at<Vec3b>(i,j)[0];
                point1.G = Image.at<Vec3b>(i,j)[1];
                point1.R = Image.at<Vec3b>(i,j)[2];
                point2 = getReplacePixel2(point1);

                if(point2.B ==208&&point2.G == 192&&point2.R == 192)
                    qDebug()<<"存在该像素";
                Image_quantity.at<Vec3b>(i,j)[0] = point2.B;
                Image_quantity.at<Vec3b>(i,j)[1] = point2.G;
                Image_quantity.at<Vec3b>(i,j)[2] = point2.R;
            }
        }
    }

    RGB_Lab *temp = Head;
    while(temp)
    {
//        qDebug()<<" "<<temp->Blue<<""<<temp->Green<<""<<temp->Red;
//        qDebug()<<"个数"<<temp->count;
        temp = temp->Next;
    }

}

/********************************************
 *function:高斯滤波
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
void Quantification::filterProcess()
{
    GaussianBlur(Image_quantity,Image_quantity,Size(kernel_size,kernel_size),0,0);
}

/********************************************
 *function:在RGB颜色空间中找出替代输入像素的
 *input:像素值
 *output:替代的像素RGB
 *adding:用于第一种量化方法
 *author: wang
 *date: 2017/12/4
 *******************************************/
Pixel Quantification::getReplacePixel(Pixel input)
{
    Pixel replace;
    float temp_distance = 0;
    float  min_distance = 0;
    int Pixel_Index = 0;
    min_distance = caculateOdistance(input,standRGB[0]);
    for(int i = 1;i<256;i++)
    {
        temp_distance = caculateOdistance(input,standRGB[i]);
        if(temp_distance<min_distance)
        {
            min_distance = temp_distance;
            Pixel_Index = i;
        }
    }
    replace.R = standRGB[Pixel_Index][0];
    replace.G = standRGB[Pixel_Index][1];
    replace.B = standRGB[Pixel_Index][2];
    return replace;
}

/********************************************
 *function:在RGB调色板中找到替代原像素的标准色
 *input:
 *output:
 *adding:用于第二种量化方法
 *author: wang
 *date: 2017/12/5
 *******************************************/
Pixel Quantification::getReplacePixel2(Pixel input)
{
    Pixel replace;
    float temp_distance = 0;
    float  min_distance = 0;
    RGB_Lab *temp = Head;
    RGB_Lab *min_Node = Head;
    min_distance = caculateOdistance(input,temp);
    while(temp)
    {
        temp_distance = caculateOdistance(input,temp);
        if(temp_distance<min_distance)
        {
            min_distance = temp_distance;
            min_Node = temp;
        }
        temp = temp->Next;
    }
    replace.R = min_Node->Red;
    replace.G = min_Node->Green;
    replace.B = min_Node->Blue;
    return replace;
}

/********************************************
 *function:计算输入的像素与RGB空间标准位置间的距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/4
 *******************************************/
float Quantification::caculateOdistance(Pixel input, int *array)
{
    float distance;
    float r,g,b;
    r =  input.R - array[0];
    g = input.G - array[1];
    b = input.B - array[2];
    distance = sqrt(r*r+g*g+b*b);
    return distance;
}

/********************************************
 *function:计算在Lab空间中的欧氏距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
float Quantification::caculateOdistance(RGB_Lab *Node1, RGB_Lab *Node2)
{
    float x,y,z,distance;
    x = Node1->Red - Node2->Red;
    y = Node1->Green - Node2->Green;
    z = Node1->Blue - Node2->Blue;
    distance = sqrt(x*x + y*y +z*z);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
float Quantification::caculateOdistance(Pixel input, RGB_Lab *Node)
{
    float x,y,z,distance;
    x = input.R - Node->Red;
    y = input.G - Node->Green;
    z = input.B - Node->Blue;
    distance = sqrt(x*x + y*y +z*z);
    return distance;
}

/********************************************
 *function:读取图片像素值，填充单向链表
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
void Quantification::fill_RGB_Node()
{

    QProgressDialog process;
    process.setWindowTitle(tr("颜色量化"));
    process.setLabelText(tr("正在分析图像颜色组成..."));
    process.setRange(0,Image.rows);//范围
    process.setModal(true);
    process.show();

    RGB_Lab *cur_Node;
    int Red,Green,Blue;
    //遍历所有像素点
    for(int i = 0;i<Image.rows;i++)
    {
        process.setValue(i);//更新计算进度条
        QCoreApplication::processEvents(); //防止冻结，让进度条显示出来
        for(int j = 0;j<Image.cols;j++)
        {
            if(i == 0 && j == 0)
            {
                Head = new RGB_Lab;
                if(Image.at<Vec3b>(i,j)[0]==255&&\
                   Image.at<Vec3b>(i,j)[1]==255&&\
                        Image.at<Vec3b>(i,j)[2]==255)
                {
                    Blue = 255; Green = 255; Red = 255;
                }
                else
                {
                    Blue =compress(Image.at<Vec3b>(i,j)[0]) ;  //利用整型的地板除法特性压缩颜色种类，最多16*16*16=4096种
                    Green =compress(Image.at<Vec3b>(i,j)[1]);
                    Red = compress(Image.at<Vec3b>(i,j)[2]);
                }
                Head->Blue = Blue;
                Head->Green = Green;
                Head->Red = Red;
                Head->count = 1;
                Head->pre = NULL;
                Head->Next = NULL;
                cur_Node = Head;
            }
            else
            {
                Pixel input;
                if(Image.at<Vec3b>(i,j)[0]==255&&\
                   Image.at<Vec3b>(i,j)[1]==255&&\
                        Image.at<Vec3b>(i,j)[2]==255)
                {
                    input.B = 255;
                    input.G = 255;
                    input.R = 255;
                }
                else
                {
                    input.B = compress(Image.at<Vec3b>(i,j)[0]); //利用整型的地板除法特性压缩颜色种类，最多16*16*16=4096种
                    input.G = compress(Image.at<Vec3b>(i,j)[1]);
                    input.R = compress(Image.at<Vec3b>(i,j)[2]);
                }

                if(checkSameNode(input,true))
                {

                }
                else
                {
                    RGB_Lab *temp = new RGB_Lab;
                    if(Image.at<Vec3b>(i,j)[0]==255&&\
                       Image.at<Vec3b>(i,j)[1]==255&&\
                            Image.at<Vec3b>(i,j)[2]==255)
                    {
                        Blue = 255; Green = 255; Red = 255;
                    }
                    else
                    {
                        Blue = compress(Image.at<Vec3b>(i,j)[0]); //利用整型的地板除法特性压缩颜色种类，最多16*16*16=4096种
                        Green = compress(Image.at<Vec3b>(i,j)[1]);
                        Red =compress(Image.at<Vec3b>(i,j)[2]);
                    }
                    temp->Blue = Blue;
                    temp->Green = Green;
                    temp->Red = Red;
                    temp->count = 1;
                    temp->pre = cur_Node;
                    temp->Next = NULL;
                    cur_Node->Next = temp;
                    cur_Node = temp;
                }
            }
        }
    }
}

/********************************************
 *function:将读取到的像素压缩压缩到16*16*16种
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/8
 *******************************************/
int Quantification::compress(int color)
{
//    if(color == 255)
//        return 255;
    int num;
    int return_color;
    num = color/16;
//    if((color - 16*num)<(16*(num +1) - color))
//        ;
//    else
//        num = num + 1;
    return_color = 16*num;
    return return_color;

}

/********************************************
 *function:检查链表中是否已经存在该颜色
 *input:
 *output:
 *adding:index 0:第一种量化   1：第二种量化
 *author: wang
 *date: 2017/12/5
 *******************************************/
bool Quantification::checkSameNode(Pixel input,bool index)
{
         RGB_Lab *temp;
        if(index)
        {
            temp = Head;
        }
        else
        {
            temp = Head_216;
        }

        while(temp)
        {
            if(input.R == temp->Red && input.G == temp->Green && input.B == temp->Blue)
            {
                temp->count ++;
                return true;
            }
            else
            {
                temp = temp->Next;
            }
        }
        return false;
}

/********************************************
 *function:统计RGB链表中的颜色种类
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
int Quantification::countSize()
{
    int count = 0;
    if(Head == NULL)
        return -1;
    RGB_Lab *temp = Head;
    while(temp)
    {
        count++;
        temp = temp->Next;
    }
//    qDebug()<<"Lab读取图片中的颜色种类"<<count;
    return count;
}

/********************************************
 *function:将颜色链表中的RGB数值转成Lab
 *input:
 *output:
 *adding:先从RGB转到XYZ空间，再从XYZ空间转到Lab
 *author: wang
 *date: 2017/12/5
 *******************************************/
void Quantification::RGB2Lab()
{
    float X,Y,Z;

    RGB_Lab *temp = Head;
    while(temp)
    {
        if(temp->Red == 255 && temp->Green == 255 && temp->Blue == 255)
        {
            temp->Red = 255;
        }
        X = (0.412453*temp->Red + 0.35758*temp->Green +0.180423*temp->Blue)/(255.0*0.950456);
        Y = (0.21267 *temp->Red + 0.71516 * temp->Green + 0.072169*temp->Blue)/255;
        Z = (0.019334*temp->Red + 0.119193*temp->Green + 0.950227*temp->Blue)/(255.0*1.088854);
        //计算L值
        if(Y<0.008856)
        {
            temp->Red = 903.3*Y;
        }
        else
        {
            temp->Red = 116.0*caculate1(Y) - 16.0;
        }
        //计算a值
        temp->Green = 500*(caculate1(X) - caculate1(Y));
        //计算b值
        temp->Blue = 200 *(caculate1(Y) - caculate1(Z));

        temp = temp->Next;
    }
}

/********************************************
 *function:把颜色链表中的Lab再转回RGB
 *input:
 *output:
 *adding:先从Lab转到XYZ,再从XYZ转到RGB
 *           http://blog.sina.com.cn/s/blog_5309cefc0101cdeg.html
 *author: wang
 *date: 2017/12/5
 *******************************************/
void Quantification::Lab2RGB()
{
    double X,Y,Z;
    double P;
    RGB_Lab *temp = Head;
    while(temp)
    {
        P = (temp->Red +16.0) /116.0;
        if(temp->Red > 7.9996)
        {
            Y = P*P*P;
        }
        else
        {
            Y = temp->Red/903.3;
        }
        double yr = Y,fy;
        if(yr>0.008856)
        {
            fy = pow(yr,1.0/3.0);
        }
        else
        {
            fy = 7.787*yr + 16.0/116.0;
        }

        double fx = temp->Green/500.0 + fy,fz = fy - temp->Blue/200.0;
        if(fx>0.2069)
        {
            X = 0.950456 * fx*fx*fx;
        }
        else
        {
            X = 0.950456/7.787*(fx - 16.0/116.0);
        }

        if(fz>0.2069)
        {
            Z = 1.088854 * fz *fz*fz;
        }
        else
        {
            Z = 1.088854/7.787*(fz - 16.0/116.0);
        }
        double R = 3.240479*X - 1.537150*Y - 0.498535 * Z;
        double G = -0.969256*X + 1.875992*Y + 0.041556*Z;
        double B = 0.055648 *X - 0.204043 *Y + 1.057311 *Z;
        R *= 255; G *= 255; B *= 255;

        temp->Red = (R<0)? 0 : ((R>255)? 255: R);
        temp->Green = (G<0)? 0 : ((G>255)? 255: G);
        temp->Blue = (B<0)? 0 : ((B>255)? 255: B);

        temp = temp->Next;
    }
}

/********************************************
 *function:数学计算公式  f(m)
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
float Quantification::caculate1(float input)
{
    if(input>0.008856)
        return pow(input,0.3333333);
    else
        return (7.787*input + 16.0/116.0);
}

/********************************************
 *function:合并颜色链表中颜色最相近的两个
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/5
 *******************************************/
void Quantification::mergeColor()
{
     RGB_Lab *Node1,*Node2;
     float min_NBS,distance;//色差最小原则
     RGB_Lab *temp = Head;
     RGB_Lab *temp2;

     Node1 = temp;
     Node2 = temp->Next;
     min_NBS = caculateOdistance(Node1,Node2);

     //找出色差最小的两个节点
     while(temp)
     {
         temp2 = temp->Next;
         while(temp2)
         {
             distance = caculateOdistance(temp,temp2);
             if(distance < min_NBS)
             {
                 Node1 = temp;
                 Node2 = temp2;
                 min_NBS = distance;
             }
             temp2 = temp2->Next;
         }
         temp = temp->Next;
     }

     //将两个节点进行合并
     float L,a,b;
     int count;
     count = Node1->count + Node2->count;
     L = (Node1->Red * Node1->count + Node2->Red*Node2->count)/count;
     a = (Node1->Green * Node1->count + Node2->Green*Node2->count)/count;
     b = (Node1->Blue * Node1->count + Node2->Blue*Node2->count)/count;

     Node1->Red = L;
     Node1->Green = a;
     Node1->Blue = b;
     Node1->count = count;

     if(Node2->Next!= NULL)
     {
         Node2->pre->Next = Node2->Next;
         Node2->Next->pre = Node2->pre;
     }
     else
     {
         Node2->pre->Next = NULL;
     }

     delete Node2;

}

/********************************************
 *function:统计量化后图像
 *input:
 *output:
 *adding:根据第一种量化方法得到的Image_quantity,来分析图像中的颜色组成，颜色种类和每种颜色的像素数目
 *author: wang
 *date: 2017/12/6
 *******************************************/
int Quantification::countRGB()
{
    //生成颜色链表
    RGB_Lab *cur_Node;
    int Red,Green,Blue;
    for(int i = 0;i<Image_quantity.rows;i++)
    {
        for(int j = 0;j<Image_quantity.cols;j++)
        {
            if(i == 0 && j == 0)
            {
                Head_216 = new RGB_Lab;
                Blue = Image_quantity.at<Vec3b>(i,j)[0];
                Green = Image_quantity.at<Vec3b>(i,j)[1];
                Red = Image_quantity.at<Vec3b>(i,j)[2];
                Head_216->Blue = Blue;
                Head_216->Green = Green;
                Head_216->Red = Red;
                Head_216->count = 1;
                Head_216->pre = NULL;
                Head_216->Next = NULL;
                cur_Node = Head_216;
            }
            else
            {
                Pixel input;
                input.B = Image_quantity.at<Vec3b>(i,j)[0];
                input.G = Image_quantity.at<Vec3b>(i,j)[1];
                input.R = Image_quantity.at<Vec3b>(i,j)[2];
                if(checkSameNode(input,false))
                {

                }
                else
                {
                    RGB_Lab *temp = new RGB_Lab;
                    Blue = Image_quantity.at<Vec3b>(i,j)[0];
                    Green = Image_quantity.at<Vec3b>(i,j)[1];
                    Red = Image_quantity.at<Vec3b>(i,j)[2];
                    temp->Blue = Blue;
                    temp->Green = Green;
                    temp->Red = Red;
                    temp->count = 1;
                    temp->pre = cur_Node;
                    temp->Next = NULL;
                    cur_Node->Next = temp;
                    cur_Node = temp;
                }
            }
        }
    }

    //统计颜色数目
    int total_color_num = 0;
    if(Head_216 == NULL)
        return -1;
    RGB_Lab *temp = Head_216;
    while(temp)
    {
        total_color_num++;
//        qDebug()<<" "<<temp->Blue<<""<<temp->Green<<""<<temp->Red;
//        qDebug()<<"个数"<<temp->count;
        temp = temp->Next;
    }
    return total_color_num;

}
