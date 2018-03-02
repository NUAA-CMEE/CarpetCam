#include "bezieroffset.h"

BezierOffset::BezierOffset()
{
    splitNum = 10.0;
    delta = 1.00/splitNum;
    num_points_judge = 10;
    num_split_point = 10;

    //构造函数大框架
    int size1 = total_content.outlines4.size();  //颜色个数
    for(int i = 0;i<size1;i++)
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        int size2 = color_bezier.curves.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<bezier> close_curve = color_bezier.curves.at(j); //封闭环
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3;k++)
            {
                bezier  origin = close_curve.at(k);
                bezier  result = offsetCurves(origin,origin.isouter);
                total_content.outlines4[i].curves[j][k] = result;
            }
        }
    }

    /*将所有Bezier曲线沿法线方向平移一定距离后，偏置曲线存在交叉现象，
       将Bezier曲线按弦长离散成直线来求交，求得封闭且连续的多边形*/

    //先按弦长将Bezier曲线离散成直线
    for(int i = 0;i<size1;i++)
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        QVector<QVector<Bezier2Line>>  color; //存一个颜色所有的封闭环的直线
        color.clear();
        int size2 = color_bezier.curves.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<bezier> close_curve = color_bezier.curves.at(j); //封闭环
            QVector<Bezier2Line>  loops; //存一个封闭环的直线
            loops.clear();
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3;k++)
            {
                bezier  origin = close_curve.at(k);
                Bezier2Line lines = convertBezier2Lines(origin);
                loops.append(lines);
            }
            color.append(loops);
        }
        total_content.outlines4[i].curves2.append(color);
    }

    //多边形求交，使成为封闭连续多边形
    for(int i = 0;i<size1;i++)
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        int size2 = color_bezier.curves.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<Bezier2Line> close_curve = color_bezier.curves2.at(j); //封闭环
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3 - 1;k++) //前一个直线组和后一个直线组求交
            {
                Bezier2Line  lines1 = close_curve.at(k); //一个Bezier曲线中的所有直线线段
                Bezier2Line  lines2 = close_curve.at(k+1); //一个Bezier曲线中的所有直线线段  紧接着的后一条曲线
                findIntersection(lines1,lines2);
                total_content.outlines4[i].curves2[j][k] = lines1;
                total_content.outlines4[i].curves2[j][k+1] = lines2;
            }
            //将最后一个直线组和第一个直线组
            Bezier2Line  lines1 = close_curve.at(size3-1);
            Bezier2Line  lines2 = close_curve.at(0);
            findIntersection(lines1,lines2);
            total_content.outlines4[i].curves2[j][size3-1] = lines1;
            total_content.outlines4[i].curves2[j][0] = lines2;
        }
    }
}

/********************************************
 *function:根据输入的Bezier曲线、偏移方向和偏移距离来计算偏移后的Bezier曲线
 *input: input 原bezier曲线方程  direction 曲线偏移方向  true  外环内偏   false  内环外偏
 *output:偏移后的曲线
 *adding:偏移距离在全局变量中设置
 *author: wong
 *date: 2018/2/26
 *******************************************/
bezier BezierOffset::offsetCurves(bezier input, bool direction)
{
    QVector<float_Point>  points;
    for(float t= 0;t<1.01;t = t+delta)
    {
        if(t>1)
            t=1;
        tangent_vector line = computeTangent(t,input);
        float_Point point = computeOffsetPoint(line,direction);
        points.append(point);
    }
    bezier  result = curveFit(points,direction);
    return result;
}

/********************************************
 *function:根据参数t和Bezier曲线确定t点处的单位切向量
 *input:参数 t   Bezier曲线方程
 *output: 单位切向量
 *adding:
 *author: wong
 *date: 2018/2/26
 *******************************************/
tangent_vector BezierOffset::computeTangent(float t, bezier curve)
{
    tangent_vector result;
    float_Point  start;
    start = computeStartPoint(t,curve);
    result.start_x = start.x;
    result.start_y = start.y;

    /*求切线*/
    float_Point  tangent;
    tangent = computeTangenLine(t,curve);
    result.tangent_x = tangent.x;
    result.tangent_y = tangent.y;
    return result;
}

/********************************************
 *function:根据t计算Bezier曲线上的某一点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/27
 *******************************************/
float_Point BezierOffset::computeStartPoint(float t, bezier curve)
{
    float_Point  result;
    float para0 = (1-t)*(1-t)*(1-t);
    float para1 = 3*(1-t)*(1-t)*t;
    float para2 = 3*(1-t)*t*t;
    float para3 = t*t*t;
    result.x = para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
    result.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
    return result;
}

/********************************************
 *function:根据t和Bezier曲线来求取切线
 *input:
 *output:
 *adding:利用Bezier曲线的递推定义来求取切线  deCasteljau方法
 *            要考虑重节点的特殊情况
 *author: wong
 *date: 2018/2/27
 *******************************************/
float_Point BezierOffset::computeTangenLine(float t, bezier curve)
{
    float_Point result;
    if(curve.point1.x()==curve.point2.x()&&curve.point1.y()==curve.point2.y()&&curve.point3.x()==curve.point4.x()&&curve.point3.y()==curve.point4.y())
    {
        result.x = curve.point3.x() - curve.point1.x();
        result.y = curve.point3.y() - curve.point1.y();
        return result;
    }
    else
    {
        float x1 = (1-t)*curve.point1.x() + t*curve.point2.x();
        float y1 = (1-t)*curve.point1.y() + t*curve.point2.y();
        float x2 = (1-t)*curve.point2.x() + t*curve.point3.x();
        float y2 = (1-t)*curve.point2.y() + t*curve.point3.y();
        float x3 = (1-t)*curve.point3.x() + t*curve.point4.x();
        float y3 = (1-t)*curve.point3.y() + t*curve.point4.y();
        float x4 = (1-t)*x1 + t*x2;
        float y4 = (1-t)*y1 + t*y2;
        float x5 = (1-t)*x2 + t*x3;
        float y5 = (1-t)*y2 + t*y3;
        result.x = x5-x4;
        result.y = y5 -y4;
        float distance = sqrt(result.x*result.x + result.y*result.y);
        result.x = result.x/distance;
        result.y = result.y/distance;
        return result;
    }
}

/********************************************
 *function:根据切线和偏移方向、偏移距离计算偏移后的点
 *input:direction:true  外环内偏   false  内环外偏
 *output:
 *adding:因为轮廓追踪是按顺时针方向来追踪的，所以内偏就将切线顺时针旋转90度，外偏就将切线逆时针旋转90度
 *author: wong
 *date: 2018/2/27
 *******************************************/
float_Point BezierOffset::computeOffsetPoint(tangent_vector input, bool direction)
{
    float_Point offset;
    if(direction)  //内偏  顺时针旋转90度
    {
        offset.x = input.tangent_y;
        offset.y = -input.tangent_x;
    }
    else              //外偏   逆时针旋转90度
    {
        offset.x = -input.tangent_y;
        offset.y = input.tangent_x;
    }

    float_Point result;
    result.x = input.start_x + offset.x;
    result.y = input.start_y + offset.y;
    return result;
}

/********************************************
 *function:根据偏移后的等距曲线上的点来进行Bezier曲线拟合
 *input:outer : true 外环  false 内环
 *output:
 *adding:
 *author: wong
 *date: 2018/2/27
 *******************************************/
bezier BezierOffset::curveFit(QVector<float_Point> input, bool outer)
{
    bezier  result;
    result.isouter = outer;
    float size = input.size();
    float m = size -1;
    result.point1.setX((double)input.at(0).x);
    result.point1.setY((double)input.at(0).y);
    result.point4.setX((double)input.at(size-1).x);
    result.point4.setY((double)input.at(size-1).y);

    QVector<point_f>  points;

    point_f insert;
    for(int i = 0;i<size;i++)
    {
        insert.x = input.at(i).x;
        insert.y = input.at(i).y;
        points.append(insert);
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
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/27
 *******************************************/
float BezierOffset::BernsteinValue(int index, float i)
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

/********************************************
 *function:将Bezier曲线按弦长离散为直线段
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/28
 *******************************************/
Bezier2Line BezierOffset::convertBezier2Lines(bezier curve)
{
    int numOfLine = 0; //离散成的线段数目
    float distance = bezierCurveLength(curve);
    if(distance<=num_points_judge)
    {
        numOfLine = num_split_point;
    }
    else
    {
         numOfLine = int((distance/num_points_judge)*num_split_point);
    }

    Bezier2Line result;
    result.isouter = curve.isouter;
    Line_type temp;
    float t = 0;
    float delta= 1.00/numOfLine; //t的增量
    for(int i = 0;i<numOfLine-1;i++)
    {
        t = i*delta;
        float para0 = (1-t)*(1-t)*(1-t);
        float para1 = 3*(1-t)*(1-t)*t;
        float para2 = 3*(1-t)*t*t;
        float para3 = t*t*t;
        temp.start.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
        temp.start.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
        t = t +delta;
        para0 = (1-t)*(1-t)*(1-t);
        para1 = 3*(1-t)*(1-t)*t;
        para2 = 3*(1-t)*t*t;
        para3 = t*t*t;
        temp.end.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
        temp.end.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
        result.lines.append(temp);
    }
    //最后一段加上
    t = delta*(numOfLine-1);
    float para0 = (1-t)*(1-t)*(1-t);
    float para1 = 3*(1-t)*(1-t)*t;
    float para2 = 3*(1-t)*t*t;
    float para3 = t*t*t;
    temp.start.x= para0*curve.point1.x() + para1*curve.point2.x() + para2*curve.point3.x() + para3*curve.point4.x();
    temp.start.y = para0*curve.point1.y() + para1*curve.point2.y() + para2*curve.point3.y() + para3*curve.point4.y();
    temp.end.x = curve.point4.x();
    temp.end.y = curve.point4.y();
    result.lines.append(temp);

    return result;
}

/********************************************
 *function:离散Bezier曲线来近似计算出曲线的弧长
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/1
 *******************************************/
float BezierOffset::bezierCurveLength(bezier curve)
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
 *function:两直线求交
 *input:
 *output:
 *adding:注意line1和line2具有方向性  line1在前,line2在后
 *author: wong
 *date: 2018/2/28
 *******************************************/
void BezierOffset::findIntersection(Bezier2Line &line1, Bezier2Line &line2)
{
    int size1 = line1.lines.size();
    int size2 = line2.lines.size();
    for(int m2 = 0;m2<size2;m2++)
    {
        for(int m1 = size1-1;m1>=0;m1--)
        {
            float_Point  cross = twoLineCross(line1.lines.at(m1),line1.lines.at(m2));
            if(cross.x ==-999&&cross.y == -999) //没有交点
            {
                
            }
            else  //根据m1和m2来截取直线
            {
                
            }
        }
    }
}

/********************************************
 *function:求两条直线的角点
 *input:
 *output:
 *adding:  1.如果有交点，求出交点并返回
 *             2.如果没有交点，将返回点的坐标设置为特殊值(暂时定为(-999，-999)
 *author: wong
 *date: 2018/3/1
 *******************************************/
float_Point BezierOffset::twoLineCross(Line_type line1, Line_type line2)
{
    float_Point result;
    float x1,x2,x3,x4;
    float y1,y2,y3,y4;
    x1 = line1.start.x;
    x2 = line1.end.x;
    x3 = line2.start.x;
    x4 = line2.end.x;

    y1 = line1.start.y;
    y2 = line1.end.y;
    y3 = line2.start.y;
    y4 = line2.end.y;

    float num= (y3-y4)*(x2-x1) - (x3-x1)*(y2-y1);  //分子
    float den = (x4-x3)*(y2-y1) - (y4-y3)*(x2-x1);  //分母

    if(den == 0)  //无解
    {
        result.x = -999;
        result.y = -999;
        return result;
    }
    else  //得到第二条直线的参数t2，求出交点
    {
        float t2 = num/den;
        if(t2>=0&&t2<=1)
        {
            result.x = (1-t2)*line2.start.x+t2*line2.end.x;
            result.y = (1-t2)*line2.start.y+t2*line2.end.y;
            return result;
        }
        else  //不在参数范围0-1内，无交点
        {
            result.x = -999;
            result.y = -999;
            return result;
        }
    }
}


