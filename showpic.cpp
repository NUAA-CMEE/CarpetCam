#include "showpic.h"
#include "ui_showpic.h"

showPic::showPic(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::showPic)
{
    ui->setupUi(this);
    if(image_width>image_height)
    {
        zoom1 = 400.0/image_width;
    }
    else
    {
        zoom1 = 400.0/image_height;
    }
    zoom1 = 1;
    chazhi_num = 100.0;
    chazhi_precesion = 1.0/chazhi_num;
    int width = (int)(image_width*zoom1);
    int height = (int)(image_height*zoom1);
    resize(width*2,height*2);
    pix =new QPixmap(width*2,height*2);
    pix->fill(Qt::white);
    pen1 = new QPainter(pix);  //将画笔与画布捆绑
//    spline2Bezier();
    update();
}

showPic::~showPic()
{
    delete ui;
}

/********************************************
 *function:将图形的外轮廓绘制到画布上
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/18
 *******************************************/
void showPic::drawOutline()
{
     int size1 = Bezier_curves.size();
     for(int i= 0;i<size1;i++)
     {
         color_Splines2  one_color = Bezier_curves.at(i);
         QPen pen;
         QColor  rgb(one_color.R,one_color.G,one_color.B);
         pen .setColor(rgb);
         pen.setWidth(1);
         pen1->setPen(pen);
         int size2 = one_color.curves.size();
         for(int j = 0;j<size2;j++)
         {
             QVector<bezier> a_loop = one_color.curves.at(j);
             int size3 = a_loop.size();
             for(int k = 0;k<size3;k++)
             {
                 bezier a_section = a_loop.at(k);
                 QPainterPath path(a_section.point1);
                 path.cubicTo(a_section.point2,a_section.point3,a_section.point4);
                 pen1->drawPath(path);
             }
         }
     }
}

/********************************************
 *function:直接按B样条离散成直线画出来
 *input:
 *output:
 *adding: for curvefit solution1
 *author: wong
 *date: 2018/1/18
 *******************************************/
void showPic::drawOutline2()
{
    int size1 = total_content.outlines.size();  //封闭曲线个数
    color_Splines single_color;
    color_Splines2  single_color2;
    single_color2.curves.clear();
    for(int i = 0;i<size1;i++)  //size1
    {
        single_color = total_content.outlines.at(i); //按颜色
        QPen pen;
        QColor  rgb(single_color.R,single_color.G,single_color.B);
        pen .setColor(rgb);
        pen.setWidth(1);
        pen1->setPen(pen);
//        pen1->scale(1.5,1.5);
        int size2 = single_color.curves.size();
        Spline_Node ctl_point;
        for(int j = 0;j<size2;j++)
        {
            ctl_point = single_color.curves.at(j);
            int size3 = ctl_point.points.size();
            for(int k = 0;k<=size3-4;k++) //for循环的边界要明确  不带“=”会导致最后一段
            {
                point_f start,end;
                point_f p1,p2,p3,p4;
                p1 = ctl_point.points.at(k);
                p2 = ctl_point.points.at(k+1);
                p3 = ctl_point.points.at(k+2);
                p4 = ctl_point.points.at(k+3);
                for(float m = 0;m<=1.0-chazhi_precesion;m = m+chazhi_precesion)
                {
                    start = splinePoint(m,p1,p2,p3,p4);
                    end = splinePoint(m+0.02,p1,p2,p3,p4);
                    pen1->drawLine(start.y,start.x,end.y,end.x);
                }
            }
        }
    }
}

/********************************************
 *function:直接按B样条离散成直线画出来  是用第二种B样条计算方法计算出来的
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/24
 *******************************************/
void showPic::drawOutline3()
{
    int size1 = total_content.outlines2.size();  //颜色个数
    color_Splines_2 single_color;
    for(int i = 0;i<size1;i++)  //size1
    {
        single_color = total_content.outlines2.at(i); //按颜色
        QPen pen;
        QColor  rgb(single_color.R,single_color.G,single_color.B);
        pen .setColor(rgb);
        pen.setWidth(1);
        pen1->setPen(pen);
//        pen1->scale(1.5,1.5);
        int size2 = single_color.curves.size();
        QVector<Spline_Node> section; //一个封闭环
        Spline_Node ctl_point;
        for(int j = 0;j<size2;j++)
        {
            section = single_color.curves.at(j);
            int size3 = section.size();
            for(int m = 0;m<size3;m++)
            {
                ctl_point = section.at(m);
                int size4 = ctl_point.points.size();
                for(int k = 0;k<=size4-4;k++)
                {
                    point_f start,end;
                    point_f p1,p2,p3,p4;
                    p1 = ctl_point.points.at(k);
                    p2 = ctl_point.points.at(k+1);
                    p3 = ctl_point.points.at(k+2);
                    p4 = ctl_point.points.at(k+3);
                    for(float m = 0;m<=1.0-0.02;m = m+0.02)
                    {
                        start = splinePoint(m,p1,p2,p3,p4);
                        end = splinePoint(m+0.02,p1,p2,p3,p4);
                        pen1->drawLine(start.y,start.x,end.y,end.x);
                    }
                }
            }
        }
    }
}

/********************************************
 *function:绘制拟合出的Bezier曲线
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/2/9
 *******************************************/
void showPic::drawOutline_Bezier()
{
    int size1 = total_content.outlines4.size();  //颜色个数
    for(int i = 0;i<size1;i++) //size1
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        //设置画笔的颜色
        QPen pen;
        QColor  rgb(color_bezier.R,color_bezier.G,color_bezier.B);
        pen .setColor(rgb);
        pen.setWidth(1);
        pen1->setPen(pen);

        int size2 = color_bezier.curves.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<bezier> close_curve = color_bezier.curves.at(j); //封闭环
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3;k++)
            {
                bezier  draw_bezier = close_curve.at(k);
                QPointF point1,point2,point3,point4;
                float m = 2;  //放大倍数
                point1.setX(draw_bezier.point1.y()*m);point1.setY(draw_bezier.point1.x()*m);
                point2.setX(draw_bezier.point2.y()*m);point2.setY(draw_bezier.point2.x()*m);
                point3.setX(draw_bezier.point3.y()*m);point3.setY(draw_bezier.point3.x()*m);
                point4.setX(draw_bezier.point4.y()*m);point4.setY(draw_bezier.point4.x()*m);
                QPainterPath path(point1);
                path.cubicTo(point2,point3,point4);
                pen1->drawPath(path);
            }
        }

    }
}

/********************************************
 *function:绘制离散成直线的Bezier曲线
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/3
 *******************************************/
void showPic::drawOutline_Bezier2Line()
{
    int size1 = total_content.outlines4.size();  //颜色个数
    for(int i = 0;i<size1;i++)
    {
        color_Splines2  color_bezier = total_content.outlines4.at(i);
        //设置画笔的颜色
        QPen pen;
        QColor  rgb(color_bezier.R,color_bezier.G,color_bezier.B);
        pen .setColor(rgb);
        pen.setWidth(1);
        pen1->setPen(pen);

        int size2 = color_bezier.curves2.size();  //同一个颜色的封闭环个数
        for(int j = 0;j<size2;j++)
        {
            QVector<Bezier2Line> close_curve = color_bezier.curves2.at(j); //封闭环
            int size3 = close_curve.size(); //组成封闭环的曲线的段数
            for(int k = 0;k<size3;k++)
            {
                Bezier2Line  draw_line = close_curve.at(k);
                int size4 = draw_line.lines.size();
                for(int L = 0;L<size4;L++)
                {
                    Line_type line = draw_line.lines.at(L);
                    QPointF start,end;  //直线起点和终点
                    float m = 1;  //放大倍数
                    start.setX(line.start.y*m);start.setY(line.start.x*m);
                    end.setX(line.end.y*m);end.setY(line.end.x*m);
                    pen1->drawLine(start,end);
                }
            }
        }
    }
}

/********************************************
 *function:按颜色来画出外轮廓和填充，仿真地毯的织造过程
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/18
 *******************************************/
void showPic::outAndFill()
{

}

/********************************************
 *function:将三次均匀B样条的控制点转换成3次贝塞尔曲线的控制点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/18
 *******************************************/
void showPic::spline2Bezier()
{
    int size1 = total_content.outlines.size();  //封闭曲线个数
    color_Splines single_color;
    color_Splines2  single_color2;
    single_color2.curves.clear();
    for(int i = 0;i<size1;i++)
    {
        single_color = total_content.outlines.at(i); //按颜色
        int size2 = single_color.curves.size();
        single_color2.R = single_color.R;
        single_color2.G = single_color.G;
        single_color2.B = single_color.B;
        Spline_Node ctl_point;
        QVector<bezier> ctl_point2;
        ctl_point2.clear();
        for(int j = 0;j<size2;j++)
        {
            ctl_point = single_color.curves.at(j);
            ctl_point2 = translateCtrPoint(ctl_point);
            single_color2.curves.append(ctl_point2);
            ctl_point2.clear();
        }
        Bezier_curves.append(single_color2);
        single_color2.curves.clear();
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:每段B样条的四个控制点都转换成对应的Bezier曲线的控制点
 *author: wong
 *date: 2018/1/18
 *******************************************/
QVector<bezier> showPic::translateCtrPoint(Spline_Node input)
{
    QVector<bezier> result;
    bezier a_curve;
    float p1x,p2x,p3x,p4x;
    float p1y,p2y,p3y,p4y;
    int size = input.points.size();
    for(int i = 0;i<size;i++)
    {
        if(i ==size -3)
            break;
        else
        {
            p1x = (1*input.points.at(i).x + 4*input.points.at(i+1).x + 1*4*input.points.at(i+2).x)/6;
            p1y = (1*input.points.at(i).y + 4*input.points.at(i+1).y + 1*4*input.points.at(i+2).y)/6;
            p2x = (4*input.points.at(i+1).x + 2*input.points.at(i+2).x)/6;
            p2y = (4*input.points.at(i+1).y + 2*input.points.at(i+2).y)/6;
            p3x = (2*input.points.at(i+1).x + 4*input.points.at(i+2).x)/6;
            p3y = (2*input.points.at(i+1).y + 4*input.points.at(i+2).y)/6;
            p4x = (1*input.points.at(i+1).x + 4*input.points.at(i+2).x + 1*input.points.at(i+3).x)/6;
            p4y = (1*input.points.at(i+1).y + 4*input.points.at(i+2).y + 1*input.points.at(i+3).y)/6;
//            a_curve.point1.setX(p1x);a_curve.point1.setY(p1y);
//            a_curve.point2.setX(p2x);a_curve.point2.setY(p2y);
//            a_curve.point3.setX(p3x);a_curve.point3.setY(p3y);
//            a_curve.point4.setX(p4x);a_curve.point4.setY(p4y);
            a_curve.point1.setX(p1y);a_curve.point1.setY(p1x);
            a_curve.point2.setX(p2y);a_curve.point2.setY(p2x);
            a_curve.point3.setX(p3y);a_curve.point3.setY(p3x);
            a_curve.point4.setX(p4y);a_curve.point4.setY(p4x);
            result.append(a_curve);
//            qDebug()<<p1x<<","<<p1y<<" "<<p2x<<","<<p2y<<" "<<p3x<<","<<p3y<<" "<<p4x<<","<<p4y;
        }
    }
    return result;
}

/********************************************
 *function:B样条取点
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/22
 *******************************************/
point_f showPic::splinePoint(float u, point_f input1, point_f input2, point_f input3, point_f input4)
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
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/18
 *******************************************/
void showPic::paintEvent(QPaintEvent *)
{
//    drawOutline();
//    drawOutline2();
//    drawOutline3();

//    drawOutline_Bezier();
    drawOutline_Bezier2Line();
    QPainter pen2(this);
    pen2.scale(1.7,1.7);
    pen2.drawPixmap(0,0,*pix);  //把画布上的东西画出来
}
