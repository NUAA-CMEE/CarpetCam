#include "spliter.h"

Spliter::Spliter()
{
    count = 0;
    sideWidth = 5;
    //test extracting the color image
    RGB_Lab *target= new RGB_Lab;
     Cluster_point *temp = cluster_Head;
     while(temp)
     {
         target->Blue = temp->Blue;
         target->Green = temp->Green;
         target->Red = temp->Red;
//         qDebug()<<"分离"<<target->Red<<" "<<target->Green<<" "<<target->Blue;
         extractColorImage(target);
         temp = temp->Next;
     }
     total_pic_number = count;
     edgeDetection();
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
void Spliter::extractColorImage(RGB_Lab *rgb)
{
    count ++;
    Mat output(Image_quantity.rows+2*sideWidth,Image_quantity.cols+2*sideWidth,Image_quantity.type());
    for(int i = 0;i<output.rows;i++)
    {
        for(int j = 0;j<output.cols;j++)
        {
            output.at<Vec3b>(i,j)[0] = 0; //0
            output.at<Vec3b>(i,j)[1] = 0; //0
            output.at<Vec3b>(i,j)[2] = 0; //0
        }
    }
    //generate the black background color image
    for(int i = sideWidth;i<(Image_quantity.rows+sideWidth);i++)
    {
        for(int j = sideWidth;j<(Image_quantity.cols+sideWidth);j++)
        {
            if(Image_quantity.at<Vec3b>(i-sideWidth,j-sideWidth)[0]==(int)rgb->Blue&&Image_quantity.at<Vec3b>(i-sideWidth,j-sideWidth)[1]==(int)rgb->Green&&Image_quantity.at<Vec3b>(i-sideWidth,j-sideWidth)[2]==(int)rgb->Red)
            {
                output.at<Vec3b>(i,j)[0] = rgb->Blue;
                output.at<Vec3b>(i,j)[1] = rgb->Green;
                output.at<Vec3b>(i,j)[2] = rgb->Red;
            }
//            else
//            {
//                output.at<Vec3b>(i,j)[0] = 0;
//                output.at<Vec3b>(i,j)[1] = 0;
//                output.at<Vec3b>(i,j)[2] = 0;
//            }
        }
    }
    //以jpg格式保存，图像会被压缩，像素值失真。例如0 0 0 （纯黑）会变成 0 1 4，以bmp保存就不会
    QString fileSave = filePath +"/"+fileName + QString("%1").arg(count) + ".bmp";
    imwrite(fileSave.toLatin1().data(),output);
}

/********************************************
 *function:extracting edge of image
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/27
 *******************************************/
void Spliter::edgeDetection()
{
    Mat pureColor;
    Mat gray;
    Mat output(Image_quantity.rows+2*sideWidth,Image_quantity.cols+2*sideWidth,Image_quantity.type());
    Cluster_point *temp = cluster_Head;
    for(int i = 1;i<count+1;i++)
    {
        QString fileSave = filePath +"/"+fileName + QString("%1").arg(i) + ".bmp"; //必须按bmp或tiff
        pureColor = imread(fileSave.toLatin1().data());
        cvtColor(pureColor,gray,CV_BGR2GRAY);
        Canny(gray,gray,3,9,5);//  40  50 3
        cvtColor(gray,pureColor,CV_GRAY2BGR);

        for(int m = 0;m<(output.rows);m++)
        {
            for(int n = 0;n<(output.cols);n++)
            {
                if(gray.at<uchar>(m,n)==255)
                {
                    output.at<Vec3b>(m,n)[0] = temp->Blue;
                    output.at<Vec3b>(m,n)[1] = temp->Green;
                    output.at<Vec3b>(m,n)[2] = temp->Red;
                }
                else
                {
                    output.at<Vec3b>(m,n)[0] = 0;//0
                    output.at<Vec3b>(m,n)[1] = 0;//0
                    output.at<Vec3b>(m,n)[2] = 0;//0
                }
            }
        }

        imwrite(fileSave.toLatin1().data(),output);

//        namedWindow(QString("%1").arg(i).toLatin1().data(), CV_WINDOW_AUTOSIZE);
//        imshow(QString("%1").arg(i).toLatin1().data(), output);

        temp = temp->Next;
    }
}
