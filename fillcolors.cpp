#include "fillcolors.h"

#include"globalvariable.h"

fillColors::fillColors()
{
    num_points_judge = 80;
    num_split_point = 4;
    chain_judge = 5;
    filter_distance = 10;
    splitBezierCurve();
    sortLoopArea();
    findWorkArea(); //find all process area

    planAreaFill();  //填充所有的颜色区域
}

/********************************************
 *function:按弦长将所有的Bezier曲线离散成直线
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::splitBezierCurve()
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
 *function:寻找封闭环的最小包围盒并计算面积
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::findBoxArea(loop &input)
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
 *function:寻找加工区域  一个外环和若干个内环
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::findWorkArea()
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
 *function:将离散出来的直线input2添加到input1中
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::addLines(loop &input1, QVector<Line_type2> input2)
{
    int size = input2.size();
    for(int i = 0;i<size;i++)
    {
        input1.lines.append(input2.at(i));
    }
}

/********************************************
 *function:将Bezier曲线离散
 *input:
 *output:
 *adding:
 *           离散时，数目过多会导致线段太短；数目过少会导致精度缺失
 *           需要考虑到部分Bezier曲线为直线的特殊情况，就不需要离散，直接用Line_type2来表示
 *author: wong
 *date: 2018/3/14
 *******************************************/
QVector<Line_type2> fillColors::convertBezier2Lines(bezier curve)
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
 *function:判断输入的Bezier曲线是否为直线
 *input:
 *output:true  是直线    false   不是直线
 *adding:
 *author: wong
 *date: 2018/4/2
 *******************************************/
bool fillColors::bezierIsLine(bezier curve)
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
 *date: 2018/3/14
 *******************************************/
float fillColors::bezierCurveLength(bezier curve)
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
 *function:根据封闭环的最小包围盒的面积来排序
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::sortLoopArea()
{
    int size1 = all_color.size(); //颜色种类
    for(int i = 0;i<size1;i++)
    {
        std::sort(all_color[i].outers.begin(),all_color[i].outers.end(),sortByArea);
        std::sort(all_color[i].inners.begin(),all_color[i].inners.end(),sortByArea);
    }
}

/********************************************
 *function:排序模板函数
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
bool fillColors::sortByArea(const loop &v1, const loop &v2)
{
    return v1.acreage<v2.acreage;  //升序
}

/********************************************
 *function:将一种颜色内的所有环进行分组，组成实际的加工区域
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
QVector<processArea> fillColors::groupProcessArea(color_loops input)
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
 *date: 2018/3/15
 *******************************************/
QVector<loop> fillColors::findInnerLoopsForOuterLoop(loop outer, QVector<loop> &inners)
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
 *function:判断内环是否被外环所包围
 *input:
 *output:true  外环包含内环      false  不包含
 *adding:
 *author: wong
 *date: 2018/3/15
 *******************************************/
bool fillColors::outerIncludeInner(loop inner, loop outer)
{
    if(inner.minX>=outer.minX&&inner.maxX<=outer.maxX&&inner.minY>=outer.minY&&inner.maxY<=outer.maxY)
    {
        return true;
    }
    else
        return false;
}

/********************************************
 *function:针对一块加工区域来生成行切轨迹
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void fillColors::planAreaFill()
{
    int size = all_colorWorkArea.size();  //颜色数目
    for(int i =0;i<1;i++)   //  size=7
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
 *date: 2018/3/19
 *******************************************/
color_linesFillColor fillColors::computeAcolor(QVector<processArea> input)
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
 *date: 2018/3/19
 *******************************************/
processAreaFill fillColors:: computeAarea(processArea input)
{
    processAreaFill result;

    /*建立单调链*/
    QVector<MonotonicChain>  chains_outer = findMontonicChain2(input.outer);  //找到外环的单调链
    QVector<QVector<MonotonicChain>> chains_inners =findMontonicChains(input.inners); //找到内环的单调链

    /*建立活性边表*/
    activeEdgeTable_Node *AET_Head =createAetChain(input);

    /*利用单调链建立活性边链表的纵向关系和跳转关系*/
    //先用外环
    point_Node *Last_Node = NULL;
    int size = chains_outer.size();
    for(int i = 0;i<size;i++)
    {
        if(Last_Node==NULL)
        {
            Last_Node = buildYRelation(chains_outer.at(i),AET_Head);  //存在返回的指针一直为空的情况
            continue;
        }

        point_Node *chain_Head = FirstChainNode(chains_outer.at(i),AET_Head);//当前单调链与活性边的第一个交点
        if(chain_Head == NULL)
        {
            continue;
        }
        else
        {
            Last_Node->chainPcritical = chain_Head;  //建立跳转关系
            point_Node *temp = buildYRelation(chains_outer.at(i),AET_Head);//当前交点与活性的最后一个交点，不会为空，因为能进入这个分支说明已经存在第一个交点了
            Last_Node = temp;
        }
    }

    //再用若干个内环
    int size1 = chains_inners.size();
    for(int i = 0;i<size1;i++)
    {
        QVector<MonotonicChain>  inner = chains_inners.at(i);
        point_Node *Last_Node2 = NULL;
        int size2 = inner.size();
        for(int j = 0;j<size2;j++)
        {
            if(Last_Node2 == NULL)
            {
                Last_Node2= buildYRelation(inner.at(j),AET_Head);
                continue;
            }

            point_Node *chain_Head = FirstChainNode(inner.at(j),AET_Head);
            if(chain_Head==NULL)
            {
                continue;
            }
            else
            {
                Last_Node2->chainPcritical = chain_Head;//建立跳转关系
               point_Node *temp =  buildYRelation(inner.at(j),AET_Head);
               Last_Node2 = temp;
            }
        }
    }

    /********生成填充轨迹*******/
    while(!isAllWorked(AET_Head))  //还有节点没有加工到就一直循环
    {
        QVector<float_Point> section_line;
        point_Node *temp = findGoodPoint(AET_Head);
        while(1)
        {
            temp->process = true;
            float_Point point;
            point.x = temp->x;
            point.y = temp->y;
            section_line.append(point);


            if(temp->linked!=NULL&&temp->linked->process==false)     //路径横行   水平方向上存在可连接节点
            {
                temp = temp->linked;
                continue;
            }
            else if(temp->chainNext!=NULL&&temp->chainNext->process==false)   //路径下行   单调链的垂直向下存在可连接节点
            {
                temp = temp->chainNext;
                continue;
            }
            else if(temp->chainBefore!=NULL&&temp->chainBefore->process==false) //路径上行   单调链的垂直向上存在可连接节点
            {
                temp = temp->chainBefore;
                continue;
            }
//            else if(temp->chainPcritical!=NULL&&temp->chainPcritical->process==false) //单调链的转接节点
//            {
//                temp = temp->chainPcritical;
//                continue;
//            }
            else
                break;
        }
        int size = section_line.size();
        if(2==size)
        {
            float distance = computeDistance(section_line.at(0),section_line.at(1));
            if(distance<filter_distance)
                continue;
        }
        result.points.append(section_line) ;
    }
    return result;
}


/********************************************
 *function:找出一个封闭环的单调链
 *input:
 *output:
 *adding:根据是否存在极值点来判断是否是单调链
 *            忽略所有的水平边
 *author: wong
 *date: 2018/3/19
 *******************************************/
QVector<MonotonicChain> fillColors::findMontonicChain(loop input)
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

     while(!lines.isEmpty())
     {
         int size1 = lines.size();
         MonotonicChain  chain;
         Line_type2   last_line;
         int delete_num = 0;
         float minX,maxX;  //临时变量用于比较
         for(int i = 0;i<size1;i++)
         {
             if(i == 0)
             {
                 chain.lines.append(lines.at(i));
                 if(lines.at(i).start.x>lines.at(i).end.x)
                 {
                     maxX = lines.at(i).start.x;
                     minX = lines.at(i).end.x;
                 }
                 else
                 {
                     maxX = lines.at(i).end.x;
                     minX = lines.at(i).start.x;
                 }
                 last_line = lines.at(i);
                 delete_num++;
             }
             else
             {
                 if(last_line.end.x==lines.at(i).start.x&&last_line.end.y==lines.at(i).start.y) //连续的
                 {
                     if((last_line.start.x- last_line.end.x)*(lines.at(i).end.x - lines.at(i).start.x)>0)  //同号为极值点  不单调
                     {
                         break;
                     }
                     else //非极值点   单调
                     {
                         last_line = lines.at(i);
                         if(lines.at(i).end.x>maxX)
                             maxX = lines.at(i).end.x;
                         if(lines.at(i).end.x<minX)
                             minX = lines.at(i).end.x;
                         chain.lines.append(lines.at(i));
                         delete_num++;
                     }
                 }
                 else
                 {
                     break;
                 }
             }
         }
         chain.minX = minX;
         chain.maxX = maxX;
         result.append(chain);
         chain.lines.clear();
         for(int j = 0;j<delete_num;j++)
         {
             lines.removeFirst();
         }
     }
     return result;
}

/********************************************
 *function:找出一个封闭环中的单调链，不根据极值点判断，直接根据X值的变化趋势来判断
 *input:
 *output:
 *adding:
 *            根据X值变化趋势来确定单调链。当前后两条直线不连续，但单调性一致时，通过两直线首末点距离来判断是否合并到一条链上
 *
 *author: wong
 *date: 2018/3/31
 *******************************************/
QVector<MonotonicChain> fillColors:: findMontonicChain2(loop input)
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
 *function:计算两点间的距离
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/4/2
 *******************************************/
float fillColors::computeDistance(float_Point end, float_Point start)
{
    float delta_x = end.x - start.x;
    float delta_y = end.y - start.y;
    float distance = sqrt(delta_x*delta_x+delta_y*delta_y);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/19
 *******************************************/
QVector<QVector<MonotonicChain>> fillColors::findMontonicChains(QVector<loop> input)
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
 *function:判断是否是水平线
 *input:
 *output:true  是水平线  false  非水平
 *adding:
 *author: wong
 *date: 2018/3/20
 *******************************************/
bool fillColors::isHorizontalLine(Line_type2 input)
{
    if(input.start.x == input.end.x)
        return true;
    else
        return false;
}

/********************************************
 *function:重节点位置判断
 *input:
 *output:0:不存在重节点
 *           1:起点在左上，重节点为当前直线终点
 *           2:起点在右上，重节点为当前直线起点
 *           3:起点在右下，重节点为当前直线终点
 *           4:起点在坐下，重节点为当前直线起点
 *adding:
 *author: wong
 *date: 2018/4/3
 *******************************************/
int fillColors::heavyNode(Line_type2 pre, Line_type2 next)
{
    if(pre.start.x<next.end.x&&pre.start.y<next.end.y)  //左上
    {
        float k1 = (pre.end.y-pre.start.y)/(pre.end.x-pre.start.x);
        float k2 = (next.end.y - next.start.y)/(next.end.x - next.start.x);  //方向相同
        if(k1*k2>0)
            return 1;
    }

    if(pre.start.x<next.end.x&&pre.start.y>next.end.y) //右上
    {
        float k1 = (pre.end.y-pre.start.y)/(pre.end.x-pre.start.x);
        float k2 = (next.end.y - next.start.y)/(next.end.x - next.start.x);
        if(k1*k2>0)
            return 2;
    }

    if(pre.start.x>next.end.x&&pre.start.y>next.end.y)  //右下
    {
        float k1 = (pre.end.y-pre.start.y)/(pre.end.x-pre.start.x);
        float k2 = (next.end.y - next.start.y)/(next.end.x - next.start.x);
        if(k1*k2>0)
            return 3;
    }

    if(pre.start.x>next.end.x&&pre.start.y<next.end.y)  //左下
    {
        float k1 = (pre.end.y-pre.start.y)/(pre.end.x-pre.start.x);
        float k2 = (next.end.y - next.start.y)/(next.end.x - next.start.x);
        if(k1*k2>0)
            return 4;
    }

    return 0;

}

/********************************************
 *function:根据加工区域和行距来生成活性边表
 *input:
 *output:活性边表的头
 *adding:
 *author: wong
 *date: 2018/3/20
 *******************************************/
activeEdgeTable_Node *fillColors::createAetChain(processArea input)
{
    activeEdgeTable_Node * Head = NULL;
    activeEdgeTable_Node * Last = NULL;
    float up = input.outer.minX;  //上边界
    float down = input.outer.maxX;  //下边界
    float scan_x = up; //扫描线的起始位置
    float range = down - up;  //加工范围
    int scan_num = int (range/Stitch); //扫描线个数
    for(int i = 1;i<=scan_num;i++)
    {
        scan_x = scan_x + Stitch;
        if(i == 1)
        {
            activeEdgeTable_Node * row = computeScanAet(scan_x,input);
            row->pBefore = NULL;
            row->pNext = NULL;
            Head = row;
            Last = row;
        }
        else
        {
            activeEdgeTable_Node * row = computeScanAet(scan_x,input);
            row->pNext = NULL;
            row->pBefore = Last;
            Last->pNext = row;
            Last = row;
        }
    }
    return Head;
}

/********************************************
 *function:根据扫描线x坐标和加工区域来计算出节点链表
 *input:
 *output:
 *adding:需要考虑和奇异点的求交问题
 *           在过顶点时，对于非极值点只算一次，对于极值点算两次
 *           对于水平线计算返回-1，跳过
 *author: wong
 *date: 2018/3/20
 *******************************************/
activeEdgeTable_Node *fillColors::computeScanAet(float scan_x, processArea input)
{
    activeEdgeTable_Node *result = new activeEdgeTable_Node;
    result->X = scan_x;
    result->pBefore = NULL;
    result->pNext = NULL;
    result->pHead = NULL;
    QVector<float_Point>  points;

    loop outer = input.outer;
    Line_type2 last_line ;
    if(scan_x>=outer.minX&&scan_x<=outer.maxX)
    {
        int size = outer.lines.size();
        for(int i =0;i<size;i++)
        {
            if(i==0)
            {
                last_line = outer.lines.at(i);
                float t = computeCrossPoint(scan_x,outer.lines.at(i));  //如果是水平线，返回-1
                if(t>=0&&t<=1)
                {
                    float_Point  cross;
                    cross.x = (1-t)*outer.lines.at(i).start.x + t*outer.lines.at(i).end.x;
                    cross.y = (1-t)*outer.lines.at(i).start.y + t*outer.lines.at(i).end.y;
                    points.append(cross);
                }
            }
            else
            {
                float t = computeCrossPoint(scan_x,outer.lines.at(i));
                if(t>=0&&t<=1)
                {
                    if(t==0) //在多边形的顶点上
                    {
                        if(existLimitValue(last_line,outer.lines.at(i)))//是极值点才添加到points容器中去
                        {
                            float_Point  cross;
                            cross.x = (1-t)*outer.lines.at(i).start.x + t*outer.lines.at(i).end.x;
                            cross.y = (1-t)*outer.lines.at(i).start.y + t*outer.lines.at(i).end.y;
                            points.append(cross);
                        }
                    }
                    else
                    {
                        float_Point  cross;
                        cross.x = (1-t)*outer.lines.at(i).start.x + t*outer.lines.at(i).end.x;
                        cross.y = (1-t)*outer.lines.at(i).start.y + t*outer.lines.at(i).end.y;
                        points.append(cross);
                    }
                }
                if(t==-1)  //水平线
                {
                    if(i+1<size)
                    {
                        int judge = heavyNode(last_line,outer.lines.at(i+1));
                        if(1==judge)
                        {
                            float_Point  cross;
                            cross.x = outer.lines.at(i).start.x;
                            cross.y = outer.lines.at(i).start.y;
                            points.append(cross);
                        }
                        if(2==judge)
                        {
                            float_Point  cross;
                            cross.x = outer.lines.at(i).end.x;
                            cross.y = outer.lines.at(i).end.y;
                            points.append(cross);
                        }
                        if(3==judge)
                        {
                            float_Point  cross;
                            cross.x = outer.lines.at(i).start.x;
                            cross.y = outer.lines.at(i).start.y;
                            points.append(cross);
                        }
                        if(4==judge)
                        {
                            float_Point  cross;
                            cross.x = outer.lines.at(i).end.x;
                            cross.y = outer.lines.at(i).end.y;
                            points.append(cross);
                        }
                    }
                }
                last_line = outer.lines.at(i);
            }
        }
    }

    QVector<loop> inners = input.inners;
    int size = inners.size();
    for(int i = 0;i<size;i++)
    {
        loop inner = inners.at(i);
        if(scan_x>=inner.minX&&scan_x<=inner.maxX)
        {
            int size = inner.lines.size();
            for(int i =0;i<size;i++)
            {
                if(i==0)
                {
                    last_line = inner.lines.at(0);
                    float t = computeCrossPoint(scan_x,inner.lines.at(i));
                    if(t>=0&&t<=1)
                    {
                        float_Point  cross;
                        cross.x = (1-t)*inner.lines.at(i).start.x + t*inner.lines.at(i).end.x;
                        cross.y = (1-t)*inner.lines.at(i).start.y + t*inner.lines.at(i).end.y;
                        points.append(cross);
                    }
                }
                else
                {
                    float t = computeCrossPoint(scan_x,inner.lines.at(i));
                    if(t>=0&&t<=1)
                    {
                        if(t==0)
                        {
                            if(existLimitValue(last_line,inner.lines.at(i)))//是极值点才添加到points容器中去
                            {
                                float_Point  cross;
                                cross.x = (1-t)*inner.lines.at(i).start.x + t*inner.lines.at(i).end.x;
                                cross.y = (1-t)*inner.lines.at(i).start.y + t*inner.lines.at(i).end.y;
                                points.append(cross);
                            }
                        }
                        else
                        {
                            float_Point  cross;
                            cross.x = (1-t)*inner.lines.at(i).start.x + t*inner.lines.at(i).end.x;
                            cross.y = (1-t)*inner.lines.at(i).start.y + t*inner.lines.at(i).end.y;
                            points.append(cross);
                        }
                    }
                    else if(t==-1)  //水平线
                    {
                        if(i+1<size)
                        {
                            int judge = heavyNode(last_line,inner.lines.at(i+1));
                            if(1==judge)
                            {
                                float_Point  cross;
                                cross.x = inner.lines.at(i).end.x;
                                cross.y = inner.lines.at(i).end.y;
                                points.append(cross);
                            }
                            if(2==judge)
                            {
                                float_Point  cross;
                                cross.x = inner.lines.at(i).start.x;
                                cross.y = inner.lines.at(i).start.y;
                                points.append(cross);
                            }
                            if(3==judge)
                            {
                                float_Point  cross;
                                cross.x = inner.lines.at(i).end.x;
                                cross.y = inner.lines.at(i).end.y;
                                points.append(cross);
                            }
                            if(4==judge)
                            {
                                float_Point  cross;
                                cross.x = inner.lines.at(i).start.x;
                                cross.y = inner.lines.at(i).start.y;
                                points.append(cross);
                            }
                        }
                    }
                    last_line = inner.lines.at(i);
                }
            }
        }
    }

    //排序
    sortScanCrossPoint(points);

    //建立水平联系
    int points_size = points.size();  //应该是偶数
    if(points_size%2!=0)
    {
        qDebug()<<"odd number!!!!!!!!!"<<points.at(0).x;
        for(int i = 0;i<points_size;i++)
        {
            qDebug()<<points.at(i).y;
        }
    }

    bool  linked = true;
    result->numOfPoints = points_size;
    point_Node * last_point = NULL;
    for(int i = 0;i<points_size;i++)
    {
        point_Node *point = createPointNode();
        if(0==i)
        {
            point->index = i;
            point->x = points.at(i).x;
            point->y = points.at(i).y;
            result->pHead = point;
            last_point = point;
        }
        else
        {
            if(linked)
            {
                last_point->linked = point;
                point->linked =last_point;
                linked =false;
            }
            else
            {
                last_point->disconnect = point;
                point->disconnect = last_point;
                linked = true;
            }
            last_point->Next_Node = point;
            point->index = i;
            point->x = points.at(i).x;
            point->y = points.at(i).y;
            last_point = point;
        }
    }
    return result;
}

/********************************************
 *function:扫描线与直线求交
 *input:
 *output: 负值：无交点
 *            正值：直线的参数解t
 *adding:
 *author: wong
 *date: 2018/3/20
 *******************************************/
float fillColors::computeCrossPoint(float x, Line_type2 input)
{
    float t;
    float minX,maxX;
    if(input.start.x==input.end.x&&x==input.start.x)  //水平线返回-1
    {
        return -1;
    }
    else if(input.start.x<input.end.x)
    {
        minX = input.start.x;
        maxX =input.end.x;
    }
    else
    {
        minX = input.end.x;
        maxX =input.start.x;
    }

    if(x>=minX&&x<=maxX)
    {
         t = (x-input.start.x)/(input.end.x - input.start.x);
        return t;
    }
    else
    {
        return -2;
    }
}

/********************************************
 *function:扫描线与加工区域交点按y值升序排列
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/22
 *******************************************/
void fillColors::sortScanCrossPoint(QVector<float_Point> &points)
{
    std::sort(points.begin(),points.end(),sortCrossPoints);
}

/********************************************
 *function:点坐标按Y坐标升序排列
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/22
 *******************************************/
bool fillColors::sortCrossPoints(const float_Point &v1, const float_Point &v2)
{
    return v1.y<v2.y;  //升序
}

/********************************************
 *function:判断相邻两条直线的交点是否是极值点
 *input:pre 前一条直线   next后一条直线
 *output: true 是极值点   false   非极值点
 *adding:
 *author: wong
 *date: 2018/3/22
 *******************************************/
bool fillColors::existLimitValue(Line_type2 pre, Line_type2 next)
{
    float  judge = (pre.start.x-pre.end.x)*(next.end.x-next.start.x);
    if(judge==0) //前一条直线是水平线   当前直线不会是水平线。如果当前线是水平线，在算交点时就会返回-1  也不会走到这一步了
    {
        return  true;  //这种情况下，首端点也是要加入的
    }
    else if(judge>0) //极值点
    {
        return true;
    }
    else  //非极值点
    {
        return false;
    }
}

/********************************************
 *function:分配内存空间
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/23
 *******************************************/
point_Node *fillColors::createPointNode()
{
    point_Node *result = new point_Node;
    result->skip = false; //非下针点
    result->process = false;//未加工
    result->Next_Node = NULL;
    result->linked = NULL;
    result->disconnect = NULL;
    result->chainBefore = NULL;
    result->chainNext = NULL;
    result->chainPcritical = NULL;
    result->pNext = NULL;
    return result;
}

/********************************************
 *function:输入的单调链与扫描线的第一个交点
 *input:
 *output:
 *adding:
 *           与找出最后一个节点一样，找第一个节点也要按单调性来分情况讨论
 *author: wong
 *date: 2018/3/24
 *******************************************/
point_Node *fillColors::FirstChainNode(MonotonicChain chain, activeEdgeTable_Node *AET_Head)
{
    point_Node *First_Node = NULL;
    bool isIncrease;//判断单调性 true 递增  false 递减
    float  minX,maxX;
    int size = chain.lines.size();
    for(int i = 0;i<size;i++)  //这边还是要循环的，因为并不是单调链的第一段或前几段就一定和活性边有交点，前几条线可能在两个活性边的中间
    {
        Line_type2 line = chain.lines.at(i);
        if(line.start.x<line.end.x)
        {
            minX = line.start.x;
            maxX = line.end.x;
            isIncrease = true;
        }
        else
        {
            minX = line.end.x;
            maxX = line.start.x;
            isIncrease = false;
        }

        if(isIncrease)  //X单调递增时
        {
            //扫描线快速定位到单调链的起始X位置
            activeEdgeTable_Node *cur_Edge = AET_Head;
            while(cur_Edge)
            {
                if(cur_Edge->X>minX)
                {
                    break;
                }
                else
                {
                    cur_Edge  = cur_Edge->pNext;
                }
            }

            if(cur_Edge == NULL)
            {
                continue;
            }

            while(cur_Edge->X<=maxX)
            {
                float t = computeCrossPoint(cur_Edge->X,line);
                if(t>0&&t<=1)
                {
                    float_Point  cross;
                    cross.x = (1-t)*line.start.x + t*line.end.x;
                    cross.y = (1-t)*line.start.y + t*line.end.y;

                    point_Node* Node = cur_Edge->pHead;
                    while(Node)
                    {
                        if(fabs(Node->y-cross.y)<0.01)
                        {
                            if(Node->chainNext==NULL)
                            {
                                First_Node = Node;
                                return First_Node;
                            }
                            else
                            {
                                if(Node->chainNext->y == cross.y)  //对重节点要特殊处理，找出真正的节点
                                {
                                    //一共2*4=8种情况，通过向量和起终点的信息来判断
                                    if(line.start.x == cur_Edge->X)  //如果重合的是起点
                                    {
                                        if(line.end.x<line.start.x&&line.end.y<line.start.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.end.x>line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                    }
                                    if(line.end.x == cur_Edge->X)  //如果重合的是线段终点
                                    {
                                        if(line.start.x<line.end.x&&line.start.y>line.end.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.start.x<line.end.x&&line.start.y<line.end.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y<line.end.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y>line.end.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                    }
                                }
                                else
                                {
                                    First_Node = Node;
                                    return First_Node;
                                }
                            }
                        }
                        Node = Node->Next_Node;
                    }
                }
                cur_Edge  = cur_Edge->pNext;
                if(cur_Edge== NULL)
                    break;
            }
        }
        else   //X单调减
        {
            //扫描线快速定位到单调链的结束X位置
            activeEdgeTable_Node *cur_Edge = AET_Head;
            while(cur_Edge)
            {
                if(cur_Edge->X<maxX)
                {
                    cur_Edge  = cur_Edge->pNext;
                }
                else
                {
                    break;
                }
            }

            if(cur_Edge == NULL)
            {
                continue;
            }

            while(cur_Edge->X>=minX)
            {
                float t = computeCrossPoint(cur_Edge->X,line);
                if(t>0&&t<=1)
                {
                    float_Point  cross;
                    cross.x = (1-t)*line.start.x + t*line.end.x;
                    cross.y = (1-t)*line.start.y + t*line.end.y;

                    point_Node* Node = cur_Edge->pHead;
                    while(Node)
                    {
                        if(fabs(Node->y-cross.y)<0.01)
                        {
                            if(Node->chainNext==NULL)
                            {
                                First_Node = Node;
                                return First_Node;
                            }
                            else
                            {
                                if(Node->chainNext->y == cross.y)  //对重节点要特殊处理，找出真正的节点
                                {
                                    //一共2*4=8种情况，通过向量和起终点的信息来判断
                                    if(line.start.x == cur_Edge->X)  //如果重合的是起点
                                    {
                                        if(line.end.x<line.start.x&&line.end.y<line.start.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.end.x>line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                    }
                                    if(line.end.x == cur_Edge->X)  //如果重合的是线段终点
                                    {
                                        if(line.start.x<line.end.x&&line.start.y>line.end.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                        if(line.start.x<line.end.x&&line.start.y<line.end.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y<line.end.y)
                                        {
                                            First_Node = Node;
                                            return First_Node;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y>line.end.y)
                                        {
                                            First_Node = Node->chainNext;
                                            return First_Node;
                                        }
                                    }
                                }
                                else
                                {
                                    First_Node = Node;
                                    return First_Node;
                                }
                            }

                        }
                        Node = Node->Next_Node;
                    }
                }
                cur_Edge  = cur_Edge->pBefore;
                if(cur_Edge== NULL)
                    break;
            }
        }
    }
    return First_Node;
}

/********************************************
 *function:利用单调链建立活性边表Y方向上的联系
 *input:
 *output:返回当前单调链与扫描线的最后一个节点的指针
 *adding:
 *           如果单调链的长度较短就存在单调链正好落在两个活性边中间，不与任何活性边相交。
 *           那么就存在返回值为NULL的情况，引起后续错误。
 *
 *           要修改，原来是按活性边从上倒下扫描，错误。必须结合单调链的方向来.....
 *author: wong
 *date: 2018/3/23
 *******************************************/
point_Node *fillColors::buildYRelation(MonotonicChain chain, activeEdgeTable_Node *AET_Head)
{
    point_Node *final_Node = NULL; //最后一个节点
    float  minX,maxX;
    bool  isFirst = true;  //标志位   是求出的第一个交点
    bool  isIncrease; //true  递增    false   递减
    point_Node* Last_Node = NULL;
    int size = chain.lines.size();
    for(int i = 0;i<size;i++)
    {
        Line_type2 line = chain.lines.at(i);
        if(line.start.x<line.end.x)
        {
            minX = line.start.x;
            maxX = line.end.x;
            isIncrease = true;
        }
        else
        {
            minX = line.end.x;
            maxX = line.start.x;
            isIncrease = false;
        }

        if(isIncrease) //单调链是X递增的情况
        {
            //扫描线快速定位到单调链的起始X位置
            activeEdgeTable_Node *cur_Edge = AET_Head;
            while(cur_Edge)
            {
                if(cur_Edge->X>minX)
                {
                    break;
                }
                else
                {
                    cur_Edge  = cur_Edge->pNext;
                }
            }

            if(cur_Edge== NULL)
            {
                continue;
            }

            while(cur_Edge->X<=maxX)
            {
                float t = computeCrossPoint(cur_Edge->X,line);
                if(t>0&&t<=1)
                {
                    float_Point  cross;
                    cross.x = (1-t)*line.start.x + t*line.end.x;
                    cross.y = (1-t)*line.start.y + t*line.end.y;

                    point_Node*  target_Node = NULL;
                    point_Node* Node = cur_Edge->pHead;
                    while(Node)
                    {
                        if(fabs(Node->y-cross.y)<0.01)
                        {
                            if(Node->Next_Node!= NULL)
                            {
                                if(Node->Next_Node->y == cross.y)  //对重节点要特殊处理，找出真正的节点
                                {
                                    //一共2*4=8种情况，通过向量和起终点的信息来判断
                                    if(line.start.x == cur_Edge->X)  //如果重合的是起点
                                    {
                                        if(line.end.x<line.start.x&&line.end.y<line.start.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.end.x>line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                    }
                                    if(line.end.x == cur_Edge->X)  //如果重合的是线段终点
                                    {
                                        if(line.start.x<line.end.x&&line.start.y>line.end.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.start.x<line.end.x&&line.start.y<line.end.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y<line.end.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y>line.end.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    target_Node = Node;
                                    break;
                                }
                            }
                            else
                            {
                                target_Node = Node;
                                break;
                            }

                        }
                        Node = Node->Next_Node;
                    }

                    if(isFirst)
                    {
                        Last_Node = target_Node;
                        final_Node = target_Node;//更新最后一个节点
                        isFirst = false;
                    }
                    else
                    {
                        Last_Node->chainNext = target_Node;  //双向链表  互指
                        target_Node->chainBefore = Last_Node;
                        Last_Node = target_Node;
                        final_Node = target_Node;
                    }
                }
                cur_Edge  = cur_Edge->pNext;
                if(cur_Edge==NULL)
                    break;
            }
        }
        else  //单调链是X递减的情况
        {
            //扫描线快速定位到单调链的结束X位置
            activeEdgeTable_Node *cur_Edge = AET_Head;
            while(cur_Edge)
            {
                if(cur_Edge->X<maxX)
                {
                    cur_Edge  = cur_Edge->pNext;
                }
                else
                {
                    break;
                }
            }

            if(cur_Edge== NULL)
            {
                continue;
            }

            while(cur_Edge->X>=minX)
            {
                float t = computeCrossPoint(cur_Edge->X,line);
                if(t>0&&t<=1)
                {
                    float_Point  cross;
                    cross.x = (1-t)*line.start.x + t*line.end.x;
                    cross.y = (1-t)*line.start.y + t*line.end.y;

                    point_Node*  target_Node = NULL;
                    point_Node* Node = cur_Edge->pHead;
                    while(Node)
                    {
                        if(fabs(Node->y-cross.y)<0.01)
                        {
                            if(Node->Next_Node!= NULL)
                            {
                                if(fabs(Node->Next_Node->y -cross.y)<0.01)  //对重节点要特殊处理，找出真正的节点
                                {
                                    //一共2*4=8种情况，通过向量和起终点的信息来判断
                                    if(line.start.x == cur_Edge->X)  //如果重合的是起点
                                    {
                                        if(line.end.x<line.start.x&&line.end.y<line.start.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.end.x>line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.end.x<line.start.x&&line.end.y>line.start.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                    }
                                    if(line.end.x == cur_Edge->X)  //如果重合的是线段终点
                                    {
                                        if(line.start.x<line.end.x&&line.start.y>line.end.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                        if(line.start.x<line.end.x&&line.start.y<line.end.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y<line.end.y)
                                        {
                                            target_Node = Node;
                                            break;
                                        }
                                        if(line.start.x>line.end.x&&line.start.y>line.end.y)
                                        {
                                            target_Node = Node->Next_Node;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    target_Node = Node;
                                    break;
                                }
                            }
                            else
                            {
                                target_Node = Node;
                                break;
                            }
                        }
                        Node = Node->Next_Node;
                    }

                    if(isFirst)
                    {
                        Last_Node = target_Node;
                        final_Node = target_Node;//更新最后一个节点
                        isFirst = false;
                    }
                    else
                    {
                        Last_Node->chainBefore = target_Node;  //双向链表  互指
                        target_Node->chainNext = Last_Node;
                        Last_Node = target_Node;
                        final_Node = target_Node;
                    }
                }
                cur_Edge  = cur_Edge->pBefore;
                if(cur_Edge==NULL)
                    break;
            }
        }
    }
    return final_Node;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/23
 *******************************************/
void fillColors::buildAScanYRelation(MonotonicChain chain, activeEdgeTable_Node *AET_Head)
{

}

/********************************************
 *function:判断是否已经全部加工
 *input:
 *output:true  全部加工     false   还有未加工的
 *adding:
 *author: wong
 *date: 2018/3/24
 *******************************************/
bool fillColors::isAllWorked(activeEdgeTable_Node *AET_Head)
{
    activeEdgeTable_Node *Next_Edge = AET_Head;
    while(Next_Edge)
    {
        point_Node* Node = Next_Edge->pHead;
        while(Node)
        {
            if(Node->process==false)  //还有未加工的
            {
                return false;
            }
            Node = Node->Next_Node;
        }
        Next_Edge = Next_Edge->pNext;
    }
    return true;
}

/********************************************
 *function:寻找合适的加工区域起点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/24
 *******************************************/
point_Node *fillColors::findGoodPoint(activeEdgeTable_Node *AET_Head)
{
    point_Node *result = NULL;
    activeEdgeTable_Node * temp = AET_Head;  //要想取到最上最左的起始节点，必须借助有向的扫描线链表
    while (temp)
    {
        if(temp->pHead->process== false)//当前扫描线的起始节点尚未加工
        {
            result = temp->pHead;
            return result;
        }
        temp = temp->pNext;
    }

    //如果所有扫描线的起始点都已经有过加工，就要从扫描线的初始节点重新开始遍历
    temp = AET_Head;
    while(temp)
    {
        point_Node *Node = temp->pHead;
        while(Node)
        {
            if(Node->process == true)
            {
                Node = Node->Next_Node;
            }
            else if(Node->process == false)
            {
                result = Node;
                return result;
            }
        }
        temp = temp->pNext;
    }
}


