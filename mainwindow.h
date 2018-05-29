#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"cv.h"
#include"highgui.h"
#include<QFileDialog>
#include<quantification.h>
#include<cluster.h>
#include<spliter.h>
#include<tracker.h>
#include<curvefit.h>
#include<bezieroffset.h>
#include<fillcolors.h>
#include<anyanglefill.h>
#include<yfillcolors.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_open_triggered();

    void on_quantification_btn_clicked();

    void on_quantification_btn_2_clicked();

    void on_quantification_btn_3_clicked();

    void on_quantification_btn_4_clicked();

    void on_quantification_btn_5_clicked();

    void on_quantification_btn_6_clicked();

private:
    Ui::MainWindow *ui;
    Quantification *quanti;
    Cluster  *clusters;
    Spliter *split;
    Tracker *track;
    curveFit *spline;
    BezierOffset *offset;
    fillColors *fills;
    YFillColors *yFill;
};

#endif // MAINWINDOW_H
