#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"globalvariable.h"

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    quanti = NULL;
    clusters = NULL;
    split = NULL;
    track = NULL;
    spline = NULL;
    fills = NULL;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_triggered()
{
    QFileDialog *fd = new QFileDialog(this);
    fd->setWindowTitle(tr("选择图片文件"));
    fd->setDirectory(filePath);//默认路径
    QStringList  filters;
    filters<<"Image Files(*.bmp;*.jpg;*.png;*.)"
            <<"All Files(*.*)";
    fd->setNameFilters(filters);

    if(fd->exec() == QDialog::Accepted)
    {
        QString path = fd->selectedFiles()[0]; //所选文件的完整路径  包括文件名和后缀
        QString path2 = fd->directory().path();//所选文件所在文件夹的路径   不含文件名和后缀
        filePath = path2;
        int path_size = path.size();
        int path2_size = path2.size()+1;//除去文件名
        fileName = path.right(path_size - path2_size);
        QStringList picName = fileName.split(".");
        fileName = picName[0];

        qDebug()<<path2;
        qDebug()<<fileName;

        Image = imread(path.toLatin1().data()); //根据路径读取图片
        Image_quantity = Image.clone();
        Image_cluster = Image.clone();
        Image_edge = Image.clone();
        Image_track = Image.clone();
        Image_fill = Image.clone();

        image_width = Image.cols;
        image_height = Image.rows;

        qImage = Mat2QImage(Image);//将Mat数据转成QImage
        ui->showLabel->setPixmap(QPixmap::fromImage(qImage));
        ui->showLabel->resize(ui->showLabel->pixmap()->size());
        cv::cvtColor(Image,Image,CV_RGB2BGR);
    }
}

/********************************************
 *function:颜色量化按钮槽函数
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/6
 *******************************************/
void MainWindow::on_quantification_btn_clicked()
{
    quanti = new Quantification;
    qDebug()<<Image.rows<<" "<<Image.cols<<" "<<Image.rows*Image.cols;

    if(quantity_first)
    {
        quanti->RGB216_quantity();
    }
    else
    {
//        quanti->filterProcess();
        quanti->Lab_quantity();
    }

//    qImage = Mat2QImage(Image_quantity);//将Mat数据转成QImage
//    ui->showLabel->setPixmap(QPixmap::fromImage(qImage));
//    ui->showLabel->resize(ui->showLabel->pixmap()->size());

    namedWindow("原图", CV_WINDOW_AUTOSIZE);
    imshow("原图", Image);
//    waitKey(0);
//    destroyWindow("Window");


    namedWindow("量化后", CV_WINDOW_AUTOSIZE);
    imshow("量化后", Image_quantity);
//    waitKey(0);
//    destroyWindow("MyWindow");


//    delete quanti;
//    quanti = NULL;
}

/********************************************
 *function:颜色聚类按钮槽函数
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/6
 *******************************************/
void MainWindow::on_quantification_btn_2_clicked()
{
    clusters = new Cluster;
    if(quantity_first)
    {
        clusters->RGB_Head = quanti->Head_216;
        clusters->K_Means_Cluster();
    }
    else
    {
        clusters->reGenerateRGB_LabPointer();
        clusters->K_Means_Cluster();

//        clusters->filterProcess();
//        clusters->reGenerateRGB_LabPointer();
//        clusters->K_Means_Cluster();

//        clusters->filterProcess();
//        clusters->filterProcess();
//        clusters->reGenerateRGB_LabPointer();
//        clusters->K_Means_Cluster();

        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();
        clusters->reduceNosiePoint3();

//        clusters->filterProcess();
//        clusters->reGenerateRGB_LabPointer();
//        clusters->K_Means_Cluster();


////        clusters->reduceNosiePoint();
////        clusters->reduceNosiePoint();
////        clusters->reduceNosiePoint();
//        clusters->reGenerateRGB_LabPointer();
//        clusters->K_Means_Cluster();

//            clusters->reduceNosiePoint();
//            clusters->reduceNosiePoint();
////            clusters->reduceNosiePoint();
////            clusters->reduceNosiePoint();
////            clusters->reduceNosiePoint();
////            clusters->reduceNosiePoint();
//            clusters->reGenerateRGB_LabPointer();
//            clusters->K_Means_Cluster();
    }

//    clusters->ADPC_cluster();
////    clusters->reduceNosiePoint();
//    clusters->filterProcess();
//    clusters->ADPC_cluster();
//    clusters->filterProcess();
//    clusters->ADPC_cluster();
//    clusters->filterProcess();
//    clusters->ADPC_cluster();
////    clusters->reduceNosiePoint();
//    clusters->filterProcess();
//    clusters->ADPC_cluster();

    namedWindow("聚类后", CV_WINDOW_AUTOSIZE);
    imshow("聚类后", Image_quantity);

//    RGB_Lab *col1,*col2;
//    col1 = new RGB_Lab;
//    col2 = new RGB_Lab;
//    col1->Red = 112;
//    col1->Green = 176;
//    col1->Blue = 96;
//    col2->Red = 96;
//    col2->Green = 176;
//    col2->Blue = 80;
//    clusters->compareColorNBS(col1,col2);
//    delete col1;
//    delete col2;

    delete clusters;
    clusters = NULL;
}

void MainWindow::on_quantification_btn_3_clicked()
{
    split = new Spliter;
}

/********************************************
 *function:链码追踪
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2018/1/2
 *******************************************/
void MainWindow::on_quantification_btn_4_clicked()
{
    track = new Tracker;
    track->getAllPicCode();
    track->getPropertyChainList();
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/1/11
 *******************************************/
void MainWindow::on_quantification_btn_5_clicked()
{
    spline = new curveFit;
//    offset = new BezierOffset;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/3/14
 *******************************************/
void MainWindow::on_quantification_btn_6_clicked()
{
    fills = new fillColors;
    showPic  test;
    test.exec();
}
