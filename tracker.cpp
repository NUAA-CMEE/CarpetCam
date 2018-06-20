#include "tracker.h"


Tracker::Tracker(QObject *parent) : QObject(parent)
{
    isEnd = false;
    allPic.number = 0;
    allPic.codes.clear();
//    total_pic_number = 7;
    right_slope = 0;
    left_slope = 0;
    jump = false;
}

/********************************************
 *function:get all the freeman code from the split pictures
 *input:none
 *output:none
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
void Tracker::getAllPicCode()
{
    Cluster_point *temp = cluster_Head;
    for(int i = 1;i<total_pic_number+1;i++)
    {
        codes_in_aPic.chain_lists.clear();
        codes_in_aPic.number = 0;
        codes_in_aPic.R = temp->Red;
        codes_in_aPic.G = temp->Green;
        codes_in_aPic.B = temp->Blue;

        float whiteDistance = caculateDistance(temp,255,255,255);
        float blackDistance = caculateDistance(temp,0,0,0);
        if(whiteDistance>blackDistance)
        {
            backgroundColor.R = 255 ;
            backgroundColor.G = 255 ;
            backgroundColor.B = 255 ;
        }
        else
        {
            backgroundColor.R = 0 ;
            backgroundColor.G = 0 ;
            backgroundColor.B = 0 ;
        }


        QString fileRead = filePath +"/"+fileName + QString("%1").arg(i) + ".bmp";
        getOnePicCode(fileRead);
        allPic.number++;
        allPic.codes.append(codes_in_aPic);
        temp = temp->Next;

    }

//    Pic_codes temp0;
//    int size = allPic.number;
//    for(int x = 0;x<size;x++)
//    {
//        temp0 = allPic.codes.at(x);
//        Code temp2;
//        int size1 = temp0.number;
//        for(int y = 0;y<size1;y++)
//        {
//            temp2 = temp0.chain_lists.at(y);
//            qDebug()<<temp0.R<<" "<<temp0.G<<" "<<temp0.B;
//            Image_IJ temp3;
//            int size2 = temp2.point.size();
//            for(int z = 0;z<size2;z++)
//            {
//                temp3 = temp2.point.at(z);
//                qDebug()<<temp3.i<<" "<<temp3.j;
//            }
//        }
//    }

}

/********************************************
 *function:get all the freeman code in a picture
 *input: path: Picture path
 *output:none
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
void Tracker::getOnePicCode(QString path)
{
    Pic = imread(path.toLatin1().data());
//    qDebug()<<Pic.at<Vec3b>(316,350)[0]<<" "<<Pic.at<Vec3b>(316,350)[1]<<" "<<Pic.at<Vec3b>(316,350)[2];
//     qDebug()<<Pic.at<Vec3b>(350,316)[0]<<" "<<Pic.at<Vec3b>(350,316)[1]<<" "<<Pic.at<Vec3b>(350,316)[2];
//    qDebug()<<Pic.at<Vec3b>(Point(316,350))[0]<<" "<<Pic.at<Vec3b>(Point(316,350))[1]<<" "<<Pic.at<Vec3b>(Point(316,350))[2];
//     qDebug()<<Pic.at<Vec3b>(Point(350,316))[0]<<" "<<Pic.at<Vec3b>(Point(350,316))[1]<<" "<<Pic.at<Vec3b>(Point(350,316))[2];
//    namedWindow(QString("%1").arg(path).toLatin1().data(), CV_WINDOW_AUTOSIZE);
//    imshow(QString("%1").arg(path).toLatin1().data(), Pic);
    //break the loop when it search to the last pixel
    while(!isEnd)
    {
       getSingleChainCode();
    }
    isEnd = false;
}

/********************************************
 *function:get a chain code from a picture
 *input:
 *output:
 *adding:cv::Mat的坐标系是反的,要么用 (n,m) 要么用pointm,n)
 *author: wang
 *date: 2018/1/2
 *******************************************/
void Tracker::getSingleChainCode()
{
    for(int m = 0;m<(Pic.rows);m++)
    {
        for(int n = 0;n<(Pic.cols);n++)
        {
            if(Pic.at<Vec3b>(m,n)[0]==backgroundColor.R&&Pic.at<Vec3b>(m,n)[1]==backgroundColor.G&&Pic.at<Vec3b>(m,n)[2]==backgroundColor.B)
            {
                if(m == (Pic.rows-1)&&n == (Pic.rows-1))
                    isEnd = true;
                continue;
            }
            else
            {
                qDebug()<<Pic.at<Vec3b>(m,n)[0]<<" "<<Pic.at<Vec3b>(m,n)[1]<<" "<<Pic.at<Vec3b>(m,n)[2];
                if(traceChain(m,n)<0)
                    return;
                codes_in_aPic.number++;
                return;
            }
        }
    }
}

/********************************************
 *function:trace the single chain from the start point
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
int Tracker::traceChain(int row, int col)
{
    Image_IJ cur;
    cur.i = row;
    cur.j = col;
    Code result;
    result.outer = true;
    result.depth = 0;
    result.X = row;
    result.Y = col;
    result.minX = row;
    result.minY = col;
    result.maxX = row;
    result.maxY = col;
    result.point.clear();
    result.point.append(cur);
    int intoCode = 0;
    if(col == 0)
        intoCode = 6;  //
    else
        intoCode = 0;
    do
    {
        if(isEvenNumber(intoCode))  //偶数  +1  逆时针旋转45°
        {
            intoCode = loopAdd(intoCode,1);
        }
        else  //奇数  +2   逆时针旋转90°
        {
            intoCode = loopAdd(intoCode,2);
        }
        cur = nextPoint(cur,intoCode);
        if(cur.i == -1 && cur.j == -1) //返回-1 表示是抓取到的不是一个封闭环，放弃提取. 并将开放链用黑色填充，保证整幅图像的提取循环正常结束
       {
            Image_IJ temp;
            int size = result.point.size();
            for(int i = 0;i<size;i++)
            {
                temp=result.point.at(i);
                Pic.at<Vec3b>(temp.i,temp.j)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i,temp.j)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i,temp.j)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i-1,temp.j-1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i-1,temp.j-1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i-1,temp.j-1)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i-1,temp.j)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i-1,temp.j)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i-1,temp.j)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i-1,temp.j+1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i-1,temp.j+1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i-1,temp.j+1)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i,temp.j-1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i,temp.j-1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i,temp.j-1)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i,temp.j+1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i,temp.j+1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i,temp.j+1)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i+1,temp.j-1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i+1,temp.j-1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i+1,temp.j-1)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i+1,temp.j)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i+1,temp.j)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i+1,temp.j)[2] = backgroundColor.B;

                Pic.at<Vec3b>(temp.i+1,temp.j+1)[0] = backgroundColor.R;
                Pic.at<Vec3b>(temp.i+1,temp.j+1)[1] = backgroundColor.G;
                Pic.at<Vec3b>(temp.i+1,temp.j+1)[2] = backgroundColor.B;
            }
            return -1;
        }
        result.point.append(cur);
        intoCode = lastCode;
    }while(!(cur.i==row&&cur.j==col));

    //clear the points have been searched
    Image_IJ temp;
    int size = result.point.size();
    for(int i = 0;i<size;i++)
    {
        temp=result.point.at(i);
        Pic.at<Vec3b>(temp.i,temp.j)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i,temp.j)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i,temp.j)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i-1,temp.j-1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i-1,temp.j-1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i-1,temp.j-1)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i-1,temp.j)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i-1,temp.j)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i-1,temp.j)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i-1,temp.j+1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i-1,temp.j+1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i-1,temp.j+1)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i,temp.j-1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i,temp.j-1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i,temp.j-1)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i,temp.j+1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i,temp.j+1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i,temp.j+1)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i+1,temp.j-1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i+1,temp.j-1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i+1,temp.j-1)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i+1,temp.j)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i+1,temp.j)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i+1,temp.j)[2] = backgroundColor.B;

        Pic.at<Vec3b>(temp.i+1,temp.j+1)[0] = backgroundColor.R;
        Pic.at<Vec3b>(temp.i+1,temp.j+1)[1] = backgroundColor.G;
        Pic.at<Vec3b>(temp.i+1,temp.j+1)[2] = backgroundColor.B;
    }

    if(result.point.size()>=4)  //防止单个的点作为封闭环 加入数据。过滤作用
    {
        codes_in_aPic.chain_lists.append(result);
    }
    else
    {
        return -1;
    }
}

/********************************************
 *function:check the input number is a even number or odd number
 *input:
 *output: true: even  false:odd
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
bool Tracker::isEvenNumber(int input)
{
    if(input%2==0)
        return true;
    else
        return false;
}

/********************************************
 *function:addtion function for freeman code
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
int Tracker::loopAdd(int src, int add)
{
    int result;
    result = src + add;
    if(result>=8)
        result = result -8;
    return result;
}

/********************************************
 *function:sub function for freeman code
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
int Tracker::loopSub(int src, int sub)
{
    int result;
    result = src - sub;
    if(result<0)
        result = result +8;
    return result;
}

/********************************************
 *function:according to the last point and code value to find the next point
 *input:
 *output:
 *adding:根据链码值确定搜索的起点，绕原点顺时针搜索一圈
 *author: wang
 *date: 2018/1/2
 *******************************************/
Image_IJ Tracker::nextPoint(Image_IJ input, int codeValue)
{
    Image_IJ get;
    codeValue ++;
    for(int i = 0;i<8;i++)
    {
        codeValue = loopSub(codeValue,1);
        get = surroundPoint(input,codeValue);
        if(Pic.at<Vec3b>(get.i,get.j)[0]==backgroundColor.R&&Pic.at<Vec3b>(get.i,get.j)[1]==backgroundColor.G&&Pic.at<Vec3b>(get.i,get.j)[2]==backgroundColor.B)
        {
            continue;
        }
        else
        {
            lastCode = codeValue;
            return get;
        }
    }
    //经过8次搜索后发现都是黑色的像素0 0 0；说明到达了一个开放链的终点，返回-1报错
    get.i = -1;get.j = -1;
    return get;
}

/********************************************
 *function:according to the last point and code value to find the surround point
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
Image_IJ Tracker::surroundPoint(Image_IJ input, int codeValue)
{
    Image_IJ point;
    point  = input;
    switch(codeValue)
    {
        case 0:
        {
            point.j = point.j +1;
            break;
        }
        case 1:
        {
            point.i = point.i -1;
            point.j = point.j +1;
            break;
        }
        case 2:
        {
            point.i = point.i -1;
            break;
        }
        case 3:
        {
            point.i = point.i -1;
            point.j = point.j -1;
            break;
        }
        case 4:
        {
            point.j = point.j -1;
            break;
        }
        case 5:
        {
            point.i = point.i +1;
            point.j = point.j -1;
            break;
        }
        case 6:
        {
            point.i = point.i +1;
            break;
        }
        case 7:
        {
            point.i = point.i +1;
            point.j = point.j +1;
            break;
        }
    }
    return point;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:根据追踪到的轨迹轮廓来获得每个轮廓的各个属性，包括：内环外环   最小包围盒坐标  轮廓深度 起点坐标
 *author: wang
 *date: 2018/1/4
 *******************************************/
void Tracker::getPropertyChainList()
{
    findLimitValue();
    caculateChainDepth();
    judgeOuter();
    Pic_codes temp0;
    int size = allPic.number;
    for(int x = 0;x<size;x++)
    {
        temp0 = allPic.codes.at(x);
        Code temp2;
        int size1 = temp0.number;
        for(int y = 0;y<size1;y++)
        {
            temp2 = temp0.chain_lists.at(y);
//            qDebug()<<temp2.minX<<","<<temp2.minY<<" "<<temp2.maxX<<","<<temp2.maxY;
//            qDebug()<<temp2.depth;
            qDebug()<<temp2.outer;
        }
    }
}

/********************************************
 *function:find the limit coordinate
 *input:
 *output:
 *adding:找到每条封闭链的的最小包围盒，一个矩形的左上角坐标和矩形的右下角坐标
 *           左上角： minx  miny  右下角：maxx  maxy
 *author: wang
 *date: 2018/1/5
 *******************************************/
void Tracker::findLimitValue()
{
    Pic_codes *temp0;
    int size = allPic.number;
    for(int x = 0;x<size;x++)
    {
        temp0 = (Pic_codes *)&(allPic.codes.at(x));
        Code *temp2 = NULL;
        int size1 = temp0->number;
        for(int y = 0;y<size1;y++)
        {
            temp2 = (Code *)&(temp0->chain_lists.at(y));
            //给最小包围盒的顶点赋初值
            temp2->minX = temp2->point.at(0).i;
            temp2->minY = temp2->point.at(0).j;
            temp2->maxX = temp2->point.at(0).i;
            temp2->maxY = temp2->point.at(0).j;
            Image_IJ temp3;
            int size2 = temp2->point.size();
            for(int z = 0;z<size2;z++)
            {
                temp3 = temp2->point.at(z);
                if(temp3.i<temp2->minX)
                {
                    temp2->minX = temp3.i;
                }
                if(temp3.i>temp2->maxX)
                {
                    temp2->maxX = temp3.i;
                }
                if(temp3.j<temp2->minY)
                {
                    temp2->minY = temp3.j;
                }
                if(temp3.j>temp2->maxY)
                {
                    temp2->maxY = temp3.j;
                }
            }
        }
    }
}

/********************************************
 *function:caculate the depth of the chain in then picture
 *input:
 *output:
 *adding:计算每一个封闭环在其所在图像内的深度   深度定义:比当前封闭环的包围盒大的封闭环个数
 *author: wang
 *date: 2018/1/5
 *******************************************/
void Tracker::caculateChainDepth()
{
    Pic_codes *temp0;
    int size = allPic.number;
    for(int x = 0;x<size;x++)
    {
        temp0 = (Pic_codes *)&(allPic.codes.at(x)); //每一幅图像
        Code *temp2 = NULL;
        int size1 = temp0->number;
        for(int y = 0;y<size1;y++)
        {
            temp2 = (Code *)&(temp0->chain_lists.at(y));  //每一个封闭环

            Code *temp3 = NULL;
            int size2 = temp0->number;
            for(int z = 0;z<size2;z++)
            {
                temp3 = (Code *)&(temp0->chain_lists.at(z));
                if(temp2 == temp3)
                    continue;
                else
                {
                    if(  (temp3->minX<temp2->minX &&temp3->minY<temp2->minY) &&(temp3->maxX>temp2->maxX&&temp3->maxY>temp2->maxY))
                        temp2->depth++;
                }
            }
        }
    }
}

/********************************************
 *function:judge the chain list is out looper or inner loop
 *input:
 *output:
 *adding:先根据每个封闭环的深度(最小包围盒方法)来判断内外环，但最小包围盒存在着误判的情况
 *           需要结合射线法来判断内外环
 *author: wang
 *date: 2018/1/5
 *******************************************/
void Tracker::judgeOuter()
{
    Pic_codes *temp0;
    int size = allPic.number;
    for(int x = 0;x<size;x++)
    {
        temp0 = (Pic_codes *)&(allPic.codes.at(x));  //一幅图片中所有的封闭环
        Code *temp2 = NULL;
        int size1 = temp0->number;
        for(int y = 0;y<size1;y++)
        {
            temp2 = (Code *)&(temp0->chain_lists.at(y));  //一个图中的某一个封闭环
            Image_IJ temp2_start;
            temp2_start.i = temp2->X;
            temp2_start.j = temp2->Y;
            int corss_number = radial(temp0,y);
            qDebug()<<tr("交点个数")<<corss_number;
            //第一次判断内外环
            if(isEvenNumber(temp2->depth))  //深度层数是偶数
            {
                 temp2->outer = true;  //外环
            }
            else   //深度层数是奇数
            {
                temp2->outer = false;  //内环
            }

            //包围盒在判断内环时存在缺陷，需要再次判断
            if(temp2->outer==false)
            {
                if(isEvenNumber(corss_number))  //水平向左的射线与其他封闭环的交点个数为偶数
                {
                    temp2->outer = true;
                }
                else
                {
                    temp2->outer = false;
                }
            }

        }
    }
}

/********************************************
 *function:return the number of intersection of fadial line with other chain list
 *input: aPic 一幅图像中所有的封闭环链   index 求第几个封闭环的射线交点个数
 *output:射线向左方向与其余封闭环的交点个数
 *adding:在根据最小包围盒计算出深度后用射线向左与其他分闭环求交，返回交点个数判断内外环
 *            需要注意三种特殊情况：1、射线经过封闭环的顶点(尖角部分)     算作两次
 *                                               2、射线与封闭环多边形的边重合          算作两次
 *author: wang
 *date: 2018/1/5
 *******************************************/
int Tracker::radial(Pic_codes *aPic, int index)
{
    int result = 0;
    for(int i = 0;i<aPic->number;i++)
    {
        if(i == index)
            continue;
        else
        {
            result = result + radial_with_aCode(aPic->chain_lists.at(i),aPic->chain_lists.at(index).X,aPic->chain_lists.at(index).Y);
        }
    }
    return result;
}

/********************************************
 *function:计算某一个封闭环
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/7
 *******************************************/
int Tracker::radial_with_aCode(Code chain, int row,int col)
{
    int result = 0;
    for(int i = 0;i<chain.point.size()-1;i++)
    {
        if(chain.point.at(i).i == row&&chain.point.at(i).j<col)
        {
            if(isVertex(chain,i)) //顶点的情况
            {
                result = result +2;
            }
            else if(isBorder(chain,i)) //边界线的端点
            {
                if(jump)
                {
                    jump = false;
                    continue;
                }
                else
                {
                    result = result +1;
                }
            }
            else if(isBorder2(chain,i)) //边界线的中间点
            {
                continue;
            }
            else
            {
                 result ++;  //正常情况下
            }
        }
    }
    return result;
}

/********************************************
 *function:判断当前点是否是顶点
 *input:
 *output:
 *adding:可以是上顶点也可以是下顶点
 *author: wang
 *date: 2018/1/7
 *******************************************/
bool Tracker::isVertex(Code chain, int index)
{
    if(index == 0)  //起点
    {
        if((chain.point.at(chain.point.size()-1).i<chain.point.at(index).i)&&(chain.point.at(index+1).i<chain.point.at(index).i)) //上顶点
        {
            return true;
        }
        if((chain.point.at(chain.point.size()-1).i>chain.point.at(index).i)&&(chain.point.at(index+1).i>chain.point.at(index).i)) //下顶点
        {
            return true;
        }
        return false;
    }
    else if(index == chain.point.size()-1) //终点
    {
        if((chain.point.at(index-1).i<chain.point.at(index).i)&&(chain.point.at(0).i<chain.point.at(index).i)) //上顶点
        {
            return true;
        }
        if((chain.point.at(index-1).i>chain.point.at(index).i)&&(chain.point.at(0).i>chain.point.at(index).i)) //下顶点
        {
            return true;
        }
        return false;
    }
    else
    {
        if((chain.point.at(index-1).i<chain.point.at(index).i)&&(chain.point.at(index+1).i<chain.point.at(index).i)) //上顶点
        {
            return true;
        }
        if((chain.point.at(index-1).i>chain.point.at(index).i)&&(chain.point.at(index+1).i>chain.point.at(index).i)) //下顶点
        {
            return true;
        }
        return false;
    }
}

/********************************************
 *function:判断当前点是否是 多边形线段上的点
 *input:
 *output:
 *adding:左右端点总是成对出现
 *author: wang
 *date: 2018/1/7
 *******************************************/
bool Tracker::isBorder(Code chain, int index)
{
    if(index == 0)  //起点
    {
        if((chain.point.at(chain.point.size()-1).i == chain.point.at(index).i)&&(chain.point.at(index).i!=chain.point.at(index+1).i)) //右端点
        {
            right_slope = chain.point.at(index+1).i - chain.point.at(index).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        if((chain.point.at(chain.point.size()-1).i != chain.point.at(index).i)&&(chain.point.at(index).i==chain.point.at(index+1).i))  //左端点
        {
            left_slope = chain.point.at(index).i - chain.point.at(chain.point.size()-1).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        return false;
    }
    else if(index == chain.point.size()-1) //终点
    {
        if((chain.point.at(index-1).i==chain.point.at(index).i)&&(chain.point.at(0).i!=chain.point.at(index).i)) //右端点
        {
            right_slope = chain.point.at(0).i - chain.point.at(index).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        if((chain.point.at(index-1).i!=chain.point.at(index).i)&&(chain.point.at(0).i==chain.point.at(index).i)) //左端点
        {
            left_slope = chain.point.at(index).i - chain.point.at(index-1).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        return false;
    }
    else
    {
        if((chain.point.at(index-1).i==chain.point.at(index).i)&&(chain.point.at(index+1).i!=chain.point.at(index).i)) //右端点
        {
            right_slope = chain.point.at(index+1).i - chain.point.at(index).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        if((chain.point.at(index-1).i!=chain.point.at(index).i)&&(chain.point.at(index+1).i==chain.point.at(index).i)) //左端点
        {
            left_slope = chain.point.at(index).i - chain.point.at(index-1).i;
            if(left_slope == right_slope)
            {
                jump = true;
                right_slope = 0;
                left_slope = 0;
            }
            return true;
        }
        return false;
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/8
 *******************************************/
bool Tracker::isBorder2(Code chain, int index)
{
    if(index == 0)  //起点
    {
        if((chain.point.at(chain.point.size()-1).i == chain.point.at(index).i)&&(chain.point.at(index).i==chain.point.at(index+1).i))
        {
            return true;
        }
        return false;
    }
    else if(index == chain.point.size()-1) //终点
    {
        if((chain.point.at(index-1).i==chain.point.at(index).i)&&(chain.point.at(0).i==chain.point.at(index).i))
        {
            return true;
        }
        return false;
    }
    else
    {
        if((chain.point.at(index-1).i==chain.point.at(index).i)&&(chain.point.at(index+1).i==chain.point.at(index).i))
        {
            return true;
        }
        return false;
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/28
 *******************************************/
float Tracker::caculateDistance(Cluster_point *color, int R, int G, int B)
{
    float delta_R = color->Red - R;
    float delta_G = color->Green - G;
    float delta_B = color->Blue - B;
    float distance = sqrt(delta_R*delta_R+delta_G*delta_G+delta_B*delta_B);
    return distance;
}
