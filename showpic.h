#ifndef SHOWPIC_H
#define SHOWPIC_H

#include <QDialog>
#include<QPainter>
#include<QDebug>
#include<QVector>
#include"globalvariable.h"

namespace Ui {
class showPic;
}

class showPic : public QDialog
{
    Q_OBJECT

public:
    explicit showPic(QWidget *parent = 0);
    ~showPic();

private:
    Ui::showPic *ui;
    QPixmap *pix;
    QPainter *pen1;
    float  zoom1;
    QVector<color_Splines2> Bezier_curves;
    float  chazhi_num;
    float  chazhi_precesion;

private:
    void drawOutline();
    void drawOutline2();
    void drawOutline3();
    void drawOutline_Bezier();
    void outAndFill();
    void spline2Bezier();
    QVector<bezier> translateCtrPoint(Spline_Node input);
    point_f splinePoint(float u, point_f input1, point_f input2, point_f input3, point_f input4);

protected:
    void paintEvent(QPaintEvent *);
};

#endif // SHOWPIC_H
