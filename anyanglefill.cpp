#include "anyanglefill.h"

#include"globalvariable.h"

AnyAngleFill::AnyAngleFill(QObject *parent) : QObject(parent)
{
    num_points_judge = 80;
    num_split_point = 4;
    chain_judge = 5;
    filter_distance = 10;
    isVertical = false;
    fill_angle=0;//填充角度
    computeK(fill_angle);

    splitBezierCurve();
    sortLoopArea(); //按颜色分拣出所有的内外环，并按内外环排序
    findWorkArea(); //

    planAreaFill();  //填充所有的颜色区域
}

/********************************************
 *function:将输入的角度换算成斜率
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
void AnyAngleFill::computeK(float angle)
{
    if(angle>=0&&angle<90)
    {
        fill_K=tan(angle/PI);
        return;
    }

    if(angle>90<=180)
    {
        fill_K=-tan((180-angle)/PI);
        return;
    }

    if(90==angle)
    {
        isVertical = true;
        return;
    }
}

/********************************************
 *function:按弦长将所有的Bezier曲线离散成直线
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::splitBezierCurve()
{
    int size1 =  total_content.outlines4.size();
    for(int i = 0;i<size1;i++)
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        color_loops  color; //存一个颜色所有的封闭环
        color.R = color_bezier.R;
        color.G = color_bezier.G;
        color.B = color_bezier.B;
        int size2 = color_bezier.curves.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<bezier> close_curve = color_bezier.curves.at(j); //封闭环
            loop  a_close_loop; //存一个封闭环的所有直线
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3;k++)
            {
                bezier  origin = close_curve.at(k);
                QVector<Line_type2> lines = convertBezier2Lines(origin);
                addLines(a_close_loop,lines);
            }
            a_close_loop.isouter = color_bezier.curves.at(j).at(0).isouter;
            findBoxArea(a_close_loop); //找到最小包围盒
            if(a_close_loop.isouter)  //外环
            {
                color.outers.append(a_close_loop);
            }
            else  //内环
            {
                color.inners.append(a_close_loop);
            }
        }
        all_color.append(color);
    }
}

/********************************************
 *function:根据封闭环的最小包围盒的面积来排序
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::sortLoopArea()
{
    int size1 = all_color.size(); //颜色种类
    for(int i = 0;i<size1;i++)
    {
        std::sort(all_color[i].outers.begin(),all_color[i].outers.end(),sortByArea);
        std::sort(all_color[i].inners.begin(),all_color[i].inners.end(),sortByArea);
    }
}

/********************************************
 *function:寻找加工区域  一个外环和若干个内环
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::findWorkArea()
{
    int size = all_color.size(); //按颜色分的内外环集
    for(int i = 0;i<size;i++)
    {
        color_loops  a_color = all_color.at(i);
        QVector<processArea>  a_color_workArea = groupProcessArea(a_color);
        all_colorWorkArea.append(a_color_workArea);
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::planAreaFill()
{
    int size = all_colorWorkArea.size();  //颜色数目
    for(int i =0;i<size;i++)   //  size=7
    {
        color_linesFillColor  one_color = computeAcolor(all_colorWorkArea.at(i));
        final_fill.append(one_color);
    }
}

/********************************************
 *function:计算一种颜色的水平线填充
 *input: 一种颜色的所有封闭加工区域
 *output:加工区域的填充点集
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
color_linesFillColor AnyAngleFill::computeAcolor(QVector<processArea> input)
{
    color_linesFillColor  color_result;
    color_result.R = input.at(0).R;
    color_result.G = input.at(0).G;
    color_result.B = input.at(0).B;
    int size = input.size();
    for(int i = 0;i<size;i++)
    {
        processAreaFill fill = computeAarea(input.at(i));
        color_result.areas.append(fill);
    }
    return color_result;
}

/********************************************
 *function:计算一个区域的水平线填充
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
processAreaFill AnyAngleFill::computeAarea(processArea input)
{
    processAreaFill result;

    /*建立单调链*/
    QVector<MonotonicChain>  chains_outer = findMontonicChain(input.outer);  //找到外环的单调链
    QVector<QVector<MonotonicChain>> chains_inners =findMontonicChains(input.inners); //找到内环的单调链

//    /*建立活性边表*/
//    activeEdgeTable_Node *AET_Head =createAetChain(input);

//    /*利用单调链建立活性边链表的纵向关系和跳转关系*/
//    //先用外环
//    point_Node *Last_Node = NULL;
//    int size = chains_outer.size();
//    for(int i = 0;i<size;i++)
//    {
//        if(Last_Node==NULL)
//        {
//            Last_Node = buildYRelation(chains_outer.at(i),AET_Head);  //存在返回的指针一直为空的情况
//            continue;
//        }

//        point_Node *chain_Head = FirstChainNode(chains_outer.at(i),AET_Head);//当前单调链与活性边的第一个交点
//        if(chain_Head == NULL)
//        {
//            continue;
//        }
//        else
//        {
//            Last_Node->chainPcritical = chain_Head;  //建立跳转关系
//            point_Node *temp = buildYRelation(chains_outer.at(i),AET_Head);//当前交点与活性的最后一个交点，不会为空，因为能进入这个分支说明已经存在第一个交点了
//            Last_Node = temp;
//        }
//    }

//    //再用若干个内环
//    int size1 = chains_inners.size();
//    for(int i = 0;i<size1;i++)
//    {
//        QVector<MonotonicChain>  inner = chains_inners.at(i);
//        point_Node *Last_Node2 = NULL;
//        int size2 = inner.size();
//        for(int j = 0;j<size2;j++)
//        {
//            if(Last_Node2 == NULL)
//            {
//                Last_Node2= buildYRelation(inner.at(j),AET_Head);
//                continue;
//            }

//            point_Node *chain_Head = FirstChainNode(inner.at(j),AET_Head);
//            if(chain_Head==NULL)
//            {
//                continue;
//            }
//            else
//            {
//                Last_Node2->chainPcritical = chain_Head;//建立跳转关系
//               point_Node *temp =  buildYRelation(inner.at(j),AET_Head);
//               Last_Node2 = temp;
//            }
//        }
//    }

//    /********生成填充轨迹*******/
//    while(!isAllWorked(AET_Head))  //还有节点没有加工到就一直循环
//    {
//        QVector<float_Point> section_line;
//        point_Node *temp = findGoodPoint(AET_Head);
//        while(1)
//        {
//            temp->process = true;
//            float_Point point;
//            point.x = temp->x;
//            point.y = temp->y;
//            section_line.append(point);


//            if(temp->linked!=NULL&&temp->linked->process==false)     //路径横行   水平方向上存在可连接节点
//            {
//                temp = temp->linked;
//                continue;
//            }
//            else if(temp->chainNext!=NULL&&temp->chainNext->process==false)   //路径下行   单调链的垂直向下存在可连接节点
//            {
//                temp = temp->chainNext;
//                continue;
//            }
//            else if(temp->chainBefore!=NULL&&temp->chainBefore->process==false) //路径上行   单调链的垂直向上存在可连接节点
//            {
//                temp = temp->chainBefore;
//                continue;
//            }
////            else if(temp->chainPcritical!=NULL&&temp->chainPcritical->process==false) //单调链的转接节点
////            {
////                temp = temp->chainPcritical;
////                continue;
////            }
//            else
//                break;
//        }
//        int size = section_line.size();
//        if(2==size)
//        {
//            float distance = computeDistance(section_line.at(0),section_line.at(1));
//            if(distance<filter_distance)
//                continue;
//        }
//        result.points.append(section_line) ;
//    }
//    return result;
}

/********************************************
 *function:找出一个封闭环中的单调链，不根据极值点判断，直接根据X值的变化趋势来判断
 *input:
 *output:
 *adding:
 *            根据X值变化趋势来确定单调链。当前后两条直线不连续，但单调性一致时，通过两直线首末点距离来判断是否合并到一条链上
 *
 *author: wong
 *date: 2018/5/23
 *******************************************/
QVector<MonotonicChain> AnyAngleFill::findMontonicChain(loop input)
{
    QVector<MonotonicChain>  result;
    QVector<Line_type2>  lines; //存储所有的非水平线

    //找出非水平线
    int size = input.lines.size();
    for(int i =0;i<size;i++)
    {
        if(isHorizontalLine(input.lines.at(i)))
        {
            continue;
        }
        else  //非水平线
        {
            lines.append(input.lines.at(i));
        }
    }

    bool  lastState,curState;
    int size1 = lines.size(); //非水平线集
    MonotonicChain chain;
    Line_type2   last_line;
    for(int i = 0;i<size1;i++)
    {
        if(i==0)
        {
            Line_type2 temp = lines.at(i);
            if(temp.end.x>temp.start.x)
            {
                lastState = true;  //递增
            }
            else
            {
                lastState = false;//递减
            }

            if(lines.at(i).start.x>lines.at(i).end.x)
            {
                chain.maxX = lines.at(i).start.x;
                chain.minX = lines.at(i).end.x;
            }
            else
            {
                chain.maxX = lines.at(i).end.x;
                chain.minX = lines.at(i).start.x;
            }
            chain.lines.append(lines.at(i));
            last_line = lines.at(i);
        }
        else
        {
            Line_type2 temp = lines.at(i);
            if(temp.end.x>temp.start.x)
            {
                curState = true;  //递增
            }
            else
            {
                curState = false;//递减
            }

            if(curState==lastState) //单调性相同
            {
                if(fabs(last_line.end.x-temp.start.x)<0.01&&fabs(last_line.end.y-temp.start.y)<0.01)  //前后两条直线连续
                {
                    chain.lines.append(temp);
                    if(temp.start.x<chain.minX)
                    {
                        chain.minX = temp.start.x;
                    }
                    if(temp.start.x>chain.maxX)
                    {
                        chain.maxX = temp.start.x;
                    }
                    if(temp.end.x<chain.minX)
                    {
                        chain.minX = temp.end.x;
                    }
                    if(temp.end.x>chain.maxX)
                    {
                        chain.maxX = temp.end.x;
                    }
                }
                else
                {
                    float distance = computeDistance(last_line.end,temp.start);
                    if(distance<=chain_judge)//不连续  但距离足够近  可以认为属于同一条单调链
                    {
                        chain.lines.append(lines.at(i));
                        if(temp.start.x<chain.minX)
                        {
                            chain.minX = temp.start.x;
                        }
                        if(temp.start.x>chain.maxX)
                        {
                            chain.maxX = temp.start.x;
                        }
                        if(temp.end.x<chain.minX)
                        {
                            chain.minX = temp.end.x;
                        }
                        if(temp.end.x>chain.maxX)
                        {
                            chain.maxX = temp.end.x;
                        }
                    }
                    else  //太远了  需划分到新的单调链
                    {
                         result.append(chain);
                         chain.lines.clear();
                         if(temp.start.x<temp.end.x)
                         {
                             chain.minX = temp.start.x;
                             chain.maxX = temp.end.x;
                         }
                         else
                         {
                             chain.minX = temp.end.x;
                             chain.maxX = temp.start.x;
                         }
                         chain.lines.append(temp);
                    }
                }
            }
            else  //单调性不同
            {
                result.append(chain);
                chain.lines.clear();
                if(temp.start.x<temp.end.x)
                {
                    chain.minX = temp.start.x;
                    chain.maxX = temp.end.x;
                }
                else
                {
                    chain.minX = temp.end.x;
                    chain.maxX = temp.start.x;
                }
                chain.lines.append(temp);
            }
            lastState = curState;
            last_line = temp;
        }

    }
    result.append(chain);
    return result;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
QVector<QVector<MonotonicChain> > AnyAngleFill::findMontonicChains(QVector<loop> input)
{
    QVector<QVector<MonotonicChain>> result;
    int size =input.size();
    for(int i = 0;i<size;i++)
    {
        QVector<MonotonicChain> a_loop = findMontonicChain2(input.at(i));
        result.append(a_loop);
    }
    return result;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/24
 *******************************************/
QVector<MonotonicChain> AnyAngleFill::findMontonicChain2(loop input)
{
    QVector<MonotonicChain>  result;
    QVector<Line_type2>  lines; //存储所有的非水平线

    //找出非水平线
    int size = input.lines.size();
    for(int i =0;i<size;i++)
    {
        if(isHorizontalLine(input.lines.at(i)))
        {
            continue;
        }
        else  //非水平线
        {
            lines.append(input.lines.at(i));
        }
    }

    bool  lastState,curState;
    int size1 = lines.size(); //非水平线集
    MonotonicChain chain;
    Line_type2   last_line;
    for(int i = 0;i<size1;i++)
    {
        if(i==0)
        {
            Line_type2 temp = lines.at(i);
            if(temp.end.x>temp.start.x)
            {
                lastState = true;  //递增
            }
            else
            {
                lastState = false;//递减
            }

            if(lines.at(i).start.x>lines.at(i).end.x)
            {
                chain.maxX = lines.at(i).start.x;
                chain.minX = lines.at(i).end.x;
            }
            else
            {
                chain.maxX = lines.at(i).end.x;
                chain.minX = lines.at(i).start.x;
            }
            chain.lines.append(lines.at(i));
            last_line = lines.at(i);
        }
        else
        {
            Line_type2 temp = lines.at(i);
            if(temp.end.x>temp.start.x)
            {
                curState = true;  //递增
            }
            else
            {
                curState = false;//递减
            }

            if(curState==lastState) //单调性相同
            {
                if(fabs(last_line.end.x-temp.start.x)<0.01&&fabs(last_line.end.y-temp.start.y)<0.01)  //前后两条直线连续
                {
                    chain.lines.append(temp);
                    if(temp.start.x<chain.minX)
                    {
                        chain.minX = temp.start.x;
                    }
                    if(temp.start.x>chain.maxX)
                    {
                        chain.maxX = temp.start.x;
                    }
                    if(temp.end.x<chain.minX)
                    {
                        chain.minX = temp.end.x;
                    }
                    if(temp.end.x>chain.maxX)
                    {
                        chain.maxX = temp.end.x;
                    }
                }
                else
                {
                    float distance = computeDistance(last_line.end,temp.start);
                    if(distance<=chain_judge)//不连续  但距离足够近  可以认为属于同一条单调链
                    {
                        chain.lines.append(lines.at(i));
                        if(temp.start.x<chain.minX)
                        {
                            chain.minX = temp.start.x;
                        }
                        if(temp.start.x>chain.maxX)
                        {
                            chain.maxX = temp.start.x;
                        }
                        if(temp.end.x<chain.minX)
                        {
                            chain.minX = temp.end.x;
                        }
                        if(temp.end.x>chain.maxX)
                        {
                            chain.maxX = temp.end.x;
                        }
                    }
                    else  //太远了  需划分到新的单调链
                    {
                         result.append(chain);
                         chain.lines.clear();
                         if(temp.start.x<temp.end.x)
                         {
                             chain.minX = temp.start.x;
                             chain.maxX = temp.end.x;
                         }
                         else
                         {
                             chain.minX = temp.end.x;
                             chain.maxX = temp.start.x;
                         }
                         chain.lines.append(temp);
                    }
                }
            }
            else  //单调性不同
            {
                result.append(chain);
                chain.lines.clear();
                if(temp.start.x<temp.end.x)
                {
                    chain.minX = temp.start.x;
                    chain.maxX = temp.end.x;
                }
                else
                {
                    chain.minX = temp.end.x;
                    chain.maxX = temp.start.x;
                }
                chain.lines.append(temp);
            }
            lastState = curState;
            last_line = temp;
        }

    }
    result.append(chain);
    return result;
}

/********************************************
 *function:将离散出来的直线input2添加到input1中
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::addLines(loop &input1, QVector<Line_type2> input2)
{
    int size = input2.size();
    for(int i = 0;i<size;i++)
    {
        input1.lines.append(input2.at(i));
    }
}

/********************************************
 *function:寻找封闭环的最小包围盒并计算面积
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
void AnyAngleFill::findBoxArea(loop &input)
{
    int size = input.lines.size();
    float minX,maxX,minY,maxY;
    minX = input.lines.at(0).start.x;
    maxX = input.lines.at(0).start.x;
    minY = input.lines.at(0).start.y;
    maxY = input.lines.at(0).start.y;
    for(int i = 0;i<size;i++)
    {
        if(input.lines.at(i).start.x<minX)
            minX = input.lines.at(i).start.x;
        if(input.lines.at(i).start.x>maxX)
            maxX = input.lines.at(i).start.x;
        if(input.lines.at(i).start.y<minY)
            minY = input.lines.at(i).start.y;
        if(input.lines.at(i).start.y>maxY)
            maxY = input.lines.at(i).start.y;
        if(input.lines.at(i).end.x<minX)
            minX = input.lines.at(i).end.x;
        if(input.lines.at(i).end.x>maxX)
            maxX = input.lines.at(i).end.x;
        if(input.lines.at(i).end.y<minY)
            minY = input.lines.at(i).end.y;
        if(input.lines.at(i).end.y>maxY)
            maxY = input.lines.at(i).end.y;
    }
    input.minX = minX;
    input.maxX = maxX;
    input.minY = minY;
    input.maxY = maxY;

    float length1 = fabs(minX-maxX);
    float length2 = fabs(minY-maxY);
    input.acreage = length1*length2;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *           离散时，数目过多会导致线段太短；数目过少会导致精度缺失
 *           需要考虑到部分Bezier曲线为直线的特殊情况，就不需要离散，直接用Line_type2来表示
 *author: wong
 *date: 2018/5/21
 *******************************************/
QVector<Line_type2> AnyAngleFill::convertBezier2Lines(bezier curve)
{
    if(bezierIsLine(curve))  // Bezier曲线表示的是直线
    {
        QVector<Line_type2> result;
        Line_type2 line;
        line.start.x = curve.point1.x();
        line.start.y = curve.point1.y();
        line.end.x = curve.point4.x();
        line.end.y = curve.point4.y();
        result.append(line);
        return result;
    }
    else
    {
        int numOfLine = 0; //离散成的线段数目
        float distance = bezierCurveLength(curve);
        if(distance<=num_points_judge)
        {
            numOfLine = num_split_point;
        }
        else
        {
             numOfLine = int((distance/num_points_judge)*num_split_point*1.0);
        }

        QVector<Line_type2> result;
        Line_type2 temp;
        float t = 0;
        float delta= 1.00/numOfLine; //t的增量
        for(int i = 0;i<numOfLine-1;i++)
        {
            temp.isouter = curve.isouter;
            t = i*delta;
            float para0 = (1-t)*(1-t)*(1-t);
            float para1 = 3*(1-t)*(1-t)*t;
            float para2 = 3*(1-t)*t*t;
            float para3 = t*t*t;
            temp.start.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
            temp.start.y= para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
            t = t +delta;
            para0 = (1-t)*(1-t)*(1-t);
            para1 = 3*(1-t)*(1-t)*t;
            para2 = 3*(1-t)*t*t;
            para3 = t*t*t;
            temp.end.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
            temp.end.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
            result.append(temp);
        }
        //最后一段加上
        temp.isouter = curve.isouter;
        t = delta*(numOfLine-1);
        float para0 = (1-t)*(1-t)*(1-t);
        float para1 = 3*(1-t)*(1-t)*t;
        float para2 = 3*(1-t)*t*t;
        float para3 = t*t*t;
        temp.start.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
        temp.start.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
        temp.end.x = curve.point4.x();
        temp.end.y = curve.point4.y();
        result.append(temp);

        return result;
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
bool AnyAngleFill::bezierIsLine(bezier curve)
{
    QPointF  point1=curve.point1;
    QPointF  point2=curve.point2;
    QPointF  point3=curve.point3;
    QPointF  point4=curve.point4;
    if(point1.x()==point2.x()&&point2.x()==point3.x()&&point3.x()==point4.x())  //水平
    {
        return true;
    }
    else if(point1.y()==point2.y()&&point2.y()==point3.y()&&point3.y()==point4.y())  //竖着
    {
        return true;
    }
    else
    {
        float k1,k2,k3;
        k1 = (point1.x()-point2.x())/(point1.y()-point2.y());
        k2 = (point2.x()-point3.x())/(point2.y()-point3.y());
        k3 = (point3.x()-point4.x())/(point3.y()-point4.y());
        if(fabs(k1-k2)<0.01&&fabs(k2-k3)<0.01)  //斜率相同，一般情况下的四点共线
            return true;
        else
            return false;
    }
}

/********************************************
 *function:计算bezier曲线的长度
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
float AnyAngleFill::bezierCurveLength(bezier curve)
{
    float total_distance = 0;
    for(float t = 0;t<=0.9;t = t+0.1)
    {
        float_Point  point1;
        float para0 = (1-t)*(1-t)*(1-t);
        float para1 = 3*(1-t)*(1-t)*t;
        float para2 = 3*(1-t)*t*t;
        float para3 = t*t*t;
        point1.x = para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
        point1.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();

        float_Point  point2; //后一个点
        float m = t+0.1;
        para0 = (1-m)*(1-m)*(1-m);
        para1 = 3*(1-m)*(1-m)*m;
        para2 = 3*(1-m)*m*m;
        para3 = m*m*m;
        point2.x = para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
        point2.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();

        float delta_x = point1.x - point2.x;
        float delta_y = point1.y - point2.y;
        float distance = sqrt(delta_x*delta_x + delta_y*delta_y);
        total_distance = total_distance + distance;
    }
    return total_distance;
}

/********************************************
 *function:排序模板函数
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/21
 *******************************************/
bool AnyAngleFill::sortByArea(const loop &v1, const loop &v2)
{
    return v1.acreage<v2.acreage;  //升序
}

/********************************************
 *function:将一种颜色内的所有环进行分组，组成实际的加工区域
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
QVector<processArea> AnyAngleFill::groupProcessArea(color_loops input)
{
    QVector<processArea> result;
    int inner_size = input.inners.size();
    if(inner_size == 0)//没有内环，所以每个外环都是一个加工区域
    {
        int outer_size = input.outers.size();
        for(int i = 0;i<outer_size;i++)
        {
            processArea  area;
            area.R = input.R;
            area.G = input.G;
            area.B = input.B;
            area.outer = input.outers.at(i);
            result.append(area);
        }
        return result;
    }
    else //有内环就要从外环找起
    {
        int outer_size = input.outers.size();
        for(int i = 0;i<outer_size;i++)
        {
            loop outer = input.outers.at(i);
            QVector<loop>  inners = findInnerLoopsForOuterLoop(outer,input.inners);
            processArea  area;
            area.R = input.R;
            area.G = input.G;
            area.B = input.B;
            area.outer = outer;
            if(inners.isEmpty())
            {
                continue;
            }
            else
            {
                area.inners.append(inners);
            }
            result.append(area);
        }
        if(input.inners.isEmpty())
        {
            return result;
        }
        else
        {
            qDebug()<<"inners is error!!!";
        }
    }
}

/********************************************
 *function:从inners中找到属于outer的内环，根据包围盒的嵌套关系
 *input:
 *output:
 *adding:遍历inners，找出的符合条件的内环，要从inners中删除
 *author: wong
 *date: 2018/5/23
 *******************************************/
QVector<loop> AnyAngleFill::findInnerLoopsForOuterLoop(loop outer, QVector<loop> &inners)
{
    QVector<loop> result;
    QVector<int> index;
    int size = inners.size();
    for(int i = 0;i<size;i++)
    {
        if(outerIncludeInner(inners.at(i),outer))
        {
            index.append(i);
            result.append(inners.at(i));
        }
    }
    int size2 = index.size();
    if(size2 == 0)
    {
        return result;
    }
    else
    {
        for(int i = size2-1;i>=0;i--)
        {
            inners.removeAt(index.at(i));
        }
    }
}

/********************************************
 *function::判断内环是否被外环所包围
 *input:
 *output:true  外环包含内环      false  不包含
 *adding:
 *author: wong
 *date: 2018/5/23
 *******************************************/
bool AnyAngleFill::outerIncludeInner(loop inner, loop outer)
{
    if(inner.minX>=outer.minX&&inner.maxX<=outer.maxX&&inner.minY>=outer.minY&&inner.maxY<=outer.maxY)
    {
        return true;
    }
    else
        return false;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/24
 *******************************************/
bool AnyAngleFill::isHorizontalLine(Line_type2 input)
{
    if(input.start.x == input.end.x)
        return true;
    else
        return false;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/24
 *******************************************/
float AnyAngleFill::computeDistance(float_Point end, float_Point start)
{
    float delta_x = end.x - start.x;
    float delta_y = end.y - start.y;
    float distance = sqrt(delta_x*delta_x+delta_y*delta_y);
    return distance;
}
