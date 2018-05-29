#include "cluster.h"
#include"globalvariable.h"

Cluster::Cluster(QObject *parent) : QObject(parent)
{
    RGB_Head = NULL;
    NBS_Head = NULL;
    NBS_threshold = 0;
    cluster_Head = NULL;
    convergence_judge = 0.05;
    last_cluster_head = NULL;
    kernel_size = 5;  //13
    normal_K = 100;
    cluster_num = 7;
    sort_density_Head = NULL;
    cluster_density_Head = NULL;
}

/********************************************
 *function:聚类析构函数
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/8
 *******************************************/
Cluster::~Cluster()
{

}

/********************************************
 *function:自适应K均值聚类算法
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/9
 *******************************************/
void Cluster::K_Means_Cluster()
{

    create_NBS(); //创建NBS链表
    caculate_weight();  //计算全局加权阈值
//    create_clusterCenter();//生成初始聚类中心链表
//    create_clusterCenter2();//生成初始聚类中心链表
    create_clusterCenter3();//生成初始聚类中心链表
    copyCluster_first();//复制当前聚类中心链表
    distributePoint2Cluster();//分配像素点到聚类中心  第一次
    updateClusters();//更新聚类中心的像素值
    while(compareCluster())
    {
        copyCluster_other();
        distributePoint2Cluster2();//分配像素点到聚类中心
        updateClusters();//更新聚类中心的像素值
    }

    fill_cluster_Image();//生成聚类后的图像
}

/********************************************
 *function:创建NBS链表
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/9
 *******************************************/
void Cluster::create_NBS()
{
    NBS *NBS_last = NULL;
    RGB_Lab *RGB_temp = RGB_Head;
    RGB_Lab *RGB_temp2 = NULL;
    float nbs_distance = 0;
    //遍历所有的颜色组合
    while(RGB_temp)
    {
        RGB_temp2 = RGB_temp->Next;
        while(RGB_temp2)
        {
            if(NBS_last == NULL)  //第一个节点
            {
                nbs_distance = caculate_NBS(RGB_temp,RGB_temp2);
//                qDebug()<<"计算nbs_distance"<<nbs_distance;
//                if(nbs_distance !=nbs_distance)
//                {
//                    qDebug()<<"无穷大的节点1为"<<RGB_temp->Blue<<" "<<RGB_temp->Green<<" "<<RGB_temp->Red;
//                    qDebug()<<"无穷大的节点2为"<<RGB_temp2->Blue<<" "<<RGB_temp2->Green<<" "<<RGB_temp2->Red;
//                }
                if(nbs_distance>NBS_yuzhi)
                {
                    NBS_last = new NBS_Node;
                    NBS_last->palette1 = RGB_temp;
                    NBS_last->palette2 = RGB_temp2;
                    NBS_last->NBS_distance = nbs_distance;
                    NBS_last->weight = (NBS_last->palette1->count + NBS_last->palette2->count)/((float)image_width*image_height);
                    NBS_last->Next = NULL;
                    NBS_Head = NBS_last;
                }
            }
            else//后续节点
            {
                nbs_distance = caculate_NBS(RGB_temp,RGB_temp2);
//                qDebug()<<"计算nbs_distance"<<nbs_distance;
                if(nbs_distance>NBS_yuzhi)
                {
                    NBS *NBS_temp;
                    NBS_temp = new NBS_Node;
                    NBS_temp->palette1 = RGB_temp;
                    NBS_temp->palette2 = RGB_temp2;
                    NBS_temp->NBS_distance = nbs_distance;
                    NBS_temp->weight = (NBS_temp->palette1->count + NBS_temp->palette2->count)/((float)image_width*image_height);
                    NBS_temp->Next = NULL;
                    NBS_last->Next = NBS_temp;
                    NBS_last = NBS_temp;
                }
            }
            RGB_temp2 = RGB_temp2->Next;
        }
        RGB_temp = RGB_temp->Next;
    }
}

/********************************************
 *function:遍历NBS链表，计算加权阈值
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/9
 *******************************************/
void Cluster::caculate_weight()
{
    float distance = 0,weight = 0;
    NBS_Node *temp = NBS_Head;
    while(temp)
    {
        distance = distance + temp->weight*temp->NBS_distance;
        weight = weight + temp->weight;
        temp = temp->Next;
    }
    NBS_threshold = distance/(weight*2);
    qDebug()<<"NBS_threshold"<<NBS_threshold;
}

/********************************************
 *function:生成聚类中心链表
 *input:
 *output:
 *adding:遍历NBS链表得到聚类中心链表，链表生成后确定了聚类的数目和初始聚类中心的数值，
 *           但后续会对聚类中心链表进行迭代，聚类数目不变，聚类中心的数值发生变化
 *author: wang
 *date: 2017/12/10
 *******************************************/
void Cluster::create_clusterCenter()
{
    RGB_Lab *temp = RGB_Head;
    RGB_Lab *max = NULL;
    int max_color_count = 0;
    //遍历颜色链表找到数目最多的颜色，作为第一个聚类中心
    while(temp)
    {
        if(temp->count>max_color_count)
        {
            max_color_count = temp->count;
            max = temp;
        }
        temp = temp->Next;
    }
    cluster_Head = new Cluster_point;
    cluster_Head->Red = max->Red;
    cluster_Head->Green = max->Green;
    cluster_Head->Blue = max->Blue;
    cluster_Head->Next = NULL;

    if(!RGB_Head->Next)  //如果颜色模板中只有一种颜色就结束，不存在第二个聚类中心
        return;

    //遍历颜色链表找出与第一个聚类中心NBS改进后欧式距离最大的点，作为第二个聚类中心
    temp = RGB_Head;
    max = NULL;
    float max_distance = 0,temp_distance;
    while(temp)
    {
        temp_distance = caculate_NBS(cluster_Head,temp);
        if(temp_distance>max_distance)
        {
            max_distance = temp_distance;
            max = temp;
        }
        temp = temp->Next;
    }
    Cluster_point *second = new Cluster_point;
    second->Red = max->Red;
    second->Green = max->Green;
    second->Blue = max->Blue;
    second->Next = NULL;
    cluster_Head->Next = second;

    //根据NBS加权阈值，自动寻找其余类聚中心
    float  judge1,judge2;
    Cluster_point *cluster_temp;
    Cluster_point *cluster_last = second;
    temp = RGB_Head;
    while(temp)
    {
//        if(temp->Blue == 125&&temp->Green ==25&&temp->Red==25)
//        {
//            qDebug()<<"second"<<second;
//        }
        judge1 = caculate_NBS(cluster_Head,temp); //计算和第一个聚类中心的距离
        judge2 = caculate_NBS(second,temp);//计算和第二个聚类中心的距离

//        if(temp->Blue == 125&&temp->Green ==25&&temp->Red==25)
//        {
//            qDebug()<<"judge1"<<judge1;
//            qDebug()<<"judge2"<<judge2;
//            qDebug()<<"NBS_threshold"<<NBS_threshold;
//        }
        if(judge1>=NBS_threshold && judge2 >=NBS_threshold)//满足条件的就是新的聚类中心  NBS_threshold
        {
//            qDebug()<<"judge1"<<judge1;
//            qDebug()<<"judge2"<<judge2;
//            qDebug()<<"NBS_threshold"<<NBS_threshold;

            cluster_temp = new Cluster_point;
            cluster_temp->Red = temp->Red;
            cluster_temp->Green = temp->Green;
            cluster_temp->Blue = temp->Blue;
            cluster_temp->Next = NULL;
            cluster_last->Next = cluster_temp;
            cluster_last = cluster_temp;
        }
        temp = temp->Next;
    }

    float  total = 0;
    cluster_temp = cluster_Head;
    while(cluster_temp)
    {
//        qDebug()<<"生成聚类";
//        qDebug()<<cluster_temp->Blue<<" "<<cluster_temp->Green<<" "<<cluster_temp->Red;
        total++;
        cluster_temp = cluster_temp->Next;
    }
    qDebug()<<"聚类颜色数"<<total;

}

/********************************************
 *function:改进后的聚类中心挑选方法
 *input:
 *output:
 *adding:每一次挑选出的新的聚类中心，都会被添加到比较对象中，找出第一个和第二聚类中心
 *            图片中像素种类少于3种时，这段代码会有内存问题，但一般不会遇到
 *author: wang
 *date: 2017/12/17
 *******************************************/
void Cluster::create_clusterCenter2()
{
    RGB_Lab *temp = RGB_Head;
    RGB_Lab *max = NULL;
    int max_color_count = 0;
    //遍历颜色链表找到数目最多的颜色，作为第一个聚类中心
    while(temp)
    {
        if(temp->count>max_color_count)
        {
            max_color_count = temp->count;
            max = temp;
        }
        temp = temp->Next;
    }
    cluster_Head = new Cluster_point;
    cluster_Head->Red = max->Red;
    cluster_Head->Green = max->Green;
    cluster_Head->Blue = max->Blue;
    cluster_Head->Next = NULL;

    if(!RGB_Head->Next)  //如果颜色模板中只有一种颜色就结束，链表中不存在第二个节点
        return;

    //从RGB链表中将被选为第一个聚类中心的节点删除
    if(max->pre == NULL)  //数目最大的节点正好是链表中的链表头
    {
        RGB_Head = max->Next;
        delete max;
        RGB_Head->pre = NULL;
    }
    else if(max->Next == NULL)  //数目最大的节点正好是链表中的最后一个节点
    {
        max->pre->Next = NULL;
        delete max;
    }
    else//数目最大的节点是链表的中间节点  不是前两种情况，说明链表中至少有三个节点了，保证了此分支操作的正确性
    {
        max->Next->pre = max->pre;
        max->pre->Next = max->Next;
        delete max;
    }

    //遍历颜色链表找出与第一个聚类中心NBS改进后欧式距离最大的点，作为第二个聚类中心
    temp = RGB_Head;
    max = NULL;
    float max_distance = 0,temp_distance;
    while(temp)
    {
        temp_distance = caculate_NBS(cluster_Head,temp);
        if(temp_distance>max_distance)
        {
            max_distance = temp_distance;
            max = temp;
        }
        temp = temp->Next;
    }
    Cluster_point *second = new Cluster_point;
    second->Red = max->Red;
    second->Green = max->Green;
    second->Blue = max->Blue;
    second->Next = NULL;
    cluster_Head->Next = second;

    //从RGB链表中将被选为第二个聚类中心的节点删除
    if(max->pre == NULL)  //NBS最大的节点正好是链表中的链表头
    {
        RGB_Head = max->Next;
        delete max;
        RGB_Head->pre = NULL;
    }
    else if(max->Next == NULL)  //NBS最大的节点正好是链表中的最后一个节点
    {
        max->pre->Next = NULL;
        delete max;
    }
    else//NBS最大的节点是链表的中间节点  不是前两种情况，说明链表中至少有三个节点了，保证了此分支操作的正确性
    {
        max->Next->pre = max->pre;
        max->pre->Next = max->Next;
        delete max;
    }

    //find the rest clusters in RGB linked list
    RGB_Lab *max_number_color = NULL;
    Cluster_point *LastNode = second;
    while(1)
    {
        max_number_color = findMaxNumColor();
        if(isAcluster(max_number_color))  //当前节点可以作为聚类中心
        {
            //add the cluster node to the custer linked list
            Cluster_point *temp_Node = new Cluster_point;
            temp_Node->Red = max_number_color->Red;
            temp_Node->Green = max_number_color->Green;
            temp_Node->Blue = max_number_color->Blue;
            temp_Node->Next = NULL;
            LastNode->Next = temp_Node;
            LastNode = temp_Node;
        }

        //release the Node of RGB linked list, because it has been visited
        if(max_number_color->pre == NULL)  //the node fund is the head of RGB linked list
        {
            RGB_Head = max_number_color->Next;
            delete max_number_color;
            if(RGB_Head !=NULL)
            {
                RGB_Head->pre = NULL;
            }
        }
        else if(max_number_color->Next == NULL)  //the node fund is the last one node of the RGB linked list
        {
            max_number_color->pre->Next = NULL;
            delete max_number_color;
        }
        else//
        {
            max_number_color->Next->pre = max_number_color->pre;
            max_number_color->pre->Next = max_number_color->Next;
            delete max_number_color;
        }

        if(RGBlist_isEmpty())  //if RGB list is empty ,break the while loop
            break;
    }

    //count the number of cluster
    Cluster_point *temp1 = cluster_Head;
     float count = 0;
    while(temp1)
    {
        count ++;
        temp1 = temp1->Next;
    }
    qDebug()<<"the num of clusters is:"<<count;

}

/********************************************
 *function:create the cluster center by reading txt file
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
void Cluster::create_clusterCenter3()
{
    QString path = "./clusters.txt";
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"fail to open the file";
        return;
    }
    QTextStream data(&file);
    QString  cur_line;
    QStringList  list;
    Cluster_point *last = NULL;
    Cluster_point *temp = NULL;
    bool  isFirst = true;

    while(!data.atEnd())
    {
        cur_line = data.readLine();
        list = cur_line.split(".");
        if(isFirst)
        {
            cluster_Head = new Cluster_point;
            cluster_Head->Red = list.at(0).toInt();
            cluster_Head->Green =  list.at(1).toInt();
            cluster_Head->Blue = list.at(2).toInt();
            cluster_Head->Next = NULL;
            last = cluster_Head;
            isFirst = false;
        }
        else
        {
            temp = new Cluster_point;
            temp->Red = list.at(0).toInt();
            temp->Green =  list.at(1).toInt();
            temp->Blue = list.at(2).toInt();
            temp->Next = NULL;
            last->Next = temp;
            last = temp;
        }
    }
    file.close();

    temp = cluster_Head;
    while(temp)
    {
        qDebug()<<temp->Red<<" "<<temp->Green<<" "<<temp->Blue;
        temp = temp->Next;
    }
}

/********************************************
 *function:根据改进后欧式距离将量化后图片的每个像素点分配到聚类中心
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
void Cluster::distributePoint2Cluster()
{
    Pixel point;
    cluster_center *temp,*target;
    float temp_distance,min_distance;
    for(int i = 0;i<Image_quantity.rows;i++)
    {
        for(int j = 0;j<Image_quantity.cols;j++)
        {
            Image_IJ points;
            points.i = i;
            points.j = j;
            point.B = Image_quantity.at<Vec3b>(i,j)[0];
            point.G = Image_quantity.at<Vec3b>(i,j)[1];
            point.R = Image_quantity.at<Vec3b>(i,j)[2];

            temp = cluster_Head;
            min_distance = caculate_Odistance(temp,point);
//            min_distance = caculate_NBS(temp,point);
            target = temp;

            while(temp)
            {
                temp_distance = caculate_Odistance(temp,point);
//                temp_distance = caculate_NBS(temp,point);
                if(temp_distance<min_distance)  //找到最小距离
                {
                    min_distance = temp_distance;
                    target = temp;
                }
                temp = temp->Next;
            }
            target->points.push_back(points);
        }
    }

    int total = 0;
    temp = cluster_Head;
    while(temp)
    {
        total = total + temp->points.size();
        temp = temp->Next;
    }
    qDebug()<<"像素点总个数"<<total;
}

/********************************************
 *function:将像素点分配到各个聚类中心
 *input:
 *output:
 *adding:因为不是第一次分配，所以要先清空vector
 *author: wang
 *date: 2017/12/13
 *******************************************/
void Cluster::distributePoint2Cluster2()
{

    Pixel point;
    Cluster_point *temp,*target;
    float temp_distance,min_distance;

    //先清空
    temp = cluster_Head;
    while(temp)
    {
        temp->points.clear();
        temp = temp->Next;
    }

    for(int i = 0;i<Image_quantity.rows;i++)
    {
        for(int j = 0;j<Image_quantity.cols;j++)
        {
            Image_IJ points;
            points.i = i;
            points.j = j;
            point.B = Image_quantity.at<Vec3b>(i,j)[0];
            point.G = Image_quantity.at<Vec3b>(i,j)[1];
            point.R = Image_quantity.at<Vec3b>(i,j)[2];
            temp = cluster_Head;
            min_distance = caculate_Odistance(temp,point);
//            min_distance = caculate_NBS(temp,point);
            target = temp;
            while(temp)
            {
                temp_distance = caculate_Odistance(temp,point);
//                temp_distance = caculate_NBS(temp,point);
                if(temp_distance<min_distance)  //找到最小距离
                {
                    min_distance = temp_distance;
                    target = temp;
                }
                temp = temp->Next;
            }
            target->points.push_back(points);
        }
    }

    int total = 0;
    temp = cluster_Head;
    while(temp)
    {
        total = total + temp->points.size();
        temp = temp->Next;
    }
    qDebug()<<"像素点总个数"<<total;
}

/********************************************
 *function:聚类中心根据属于它的像素点来更新聚类中心的数值
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
void Cluster::updateClusters()
{
     Cluster_point *temp = cluster_Head;
     while(temp)
     {
         double total_R = 0,total_G = 0,total_B = 0;
         for(int i = 0;i<temp->points.size();i++)
         {
             total_B =total_B+ Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[0];
             total_G = total_G+Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[1];
             total_R = total_R+Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[2];
         }
         temp->Blue = total_B/temp->points.size();
         temp->Green = total_G/temp->points.size();
         temp->Red = total_R/temp->points.size();
         temp = temp->Next;
     }
}


/********************************************
 *function:复制当前聚类中心的RGB，留作下次比较
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/13
 *******************************************/
void Cluster::copyCluster_other()
{
    RGB_Lab *temp1 = last_cluster_head;
    Cluster_point *temp2 = cluster_Head;
    while(temp1)
    {
        temp1->Red = temp2->Red;
        temp1->Green = temp2->Green;
        temp1->Blue = temp2->Blue;
        temp1 = temp1->Next;
        temp2 = temp2->Next;
    }
}

/********************************************
 *function:复制当前聚类中心的RGB，留作下次比较
 *input:
 *output:
 *adding:xuya
 *author: wang
 *date: 2017/12/13
 *******************************************/
void Cluster::copyCluster_first()
{
    bool first = true;
    RGB_Lab *last,*temp;

    Cluster_point *temp_cluster = cluster_Head;
    while(temp_cluster)
    {
        if(first)
        {
            last_cluster_head = new RGB_Lab;
            last_cluster_head->Red = temp_cluster->Red;
            last_cluster_head->Green = temp_cluster->Green;
            last_cluster_head->Blue = temp_cluster->Blue;
            last_cluster_head->Next = NULL;
            last = last_cluster_head;
            first = false;
        }
        else
        {
            temp = new RGB_Lab;
            temp->Red = temp_cluster->Red;
            temp->Green = temp_cluster->Green;
            temp->Blue = temp_cluster->Blue;
            temp->Next = NULL;
            last->Next = temp;
            last = temp;
        }
        temp_cluster = temp_cluster->Next;
    }
}

/********************************************
 *function:比较新的聚类中心和上一次的聚类中心，判断是否收敛
 *input:
 *output:true:还未收敛     false:已经收敛
 *adding:
 *author: wang
 *date: 2017/12/13
 *******************************************/
bool Cluster::compareCluster()
{
    RGB_Lab *temp1 = last_cluster_head;  //上一次聚类中心链表
    Cluster_point *temp2 = cluster_Head;

    float judge;
    while(temp1)
    {
        judge = caculate_Odistance(temp2,temp1);
//        qDebug()<<"收敛判据："<<judge;
        if(judge > convergence_judge) //收敛条件
            return true;
        temp1 = temp1->Next;
        temp2 = temp2->Next;
        judge = 0;
    }
    return false;
}

/********************************************
 *function:生成聚类后的图像
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/13
 *******************************************/
void Cluster::fill_cluster_Image()
{
    Cluster_point *temp = cluster_Head;
    while(temp)
    {
        for(int i = 0;i<temp->points.size();i++)
        {
            Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[0] = temp->Blue;
            Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[1] = temp->Green;
            Image_quantity.at<Vec3b>(temp->points.at(i).i,temp->points.at(i).j)[2] = temp->Red;
        }
        temp = temp->Next;
    }
}

/********************************************
 *function:根据量化后图片生成实际的调色板链表
 *input:
 *output:
 *adding:如果直接用量化中合并后的链表有问题，因为有的节点没有像素
 *           他是先生成调色链表再将像素分配过去。而直接根据量化后的图像则不会
 *author: wang
 *date: 2017/12/15
 *******************************************/
void Cluster::reGenerateRGB_LabPointer()
{
    RGB_Lab *cur_Node;
    for(int i = 0;i<Image_quantity.rows;i++)
    {
        for(int j = 0;j<Image_quantity.cols;j++)
        {
            if(i == 0 && j == 0)
            {
                RGB_Head = new RGB_Lab;
                RGB_Head->Blue = Image_quantity.at<Vec3b>(i,j)[0];
                RGB_Head->Green = Image_quantity.at<Vec3b>(i,j)[1];
                RGB_Head->Red = Image_quantity.at<Vec3b>(i,j)[2];
                RGB_Head->count = 1;
                RGB_Head->pre = NULL;
                RGB_Head->Next = NULL;
                cur_Node = RGB_Head;
            }
            else
            {
                Pixel input;
                input.B = Image_quantity.at<Vec3b>(i,j)[0]; //利用整型的地板除法特性压缩颜色种类，最多16*16*16=4096种
                input.G = Image_quantity.at<Vec3b>(i,j)[1];
                input.R = Image_quantity.at<Vec3b>(i,j)[2];
                if(checkSameNode(input))
                {

                }
                else
                {
                    RGB_Lab *temp = new RGB_Lab;
                    temp->Blue = Image_quantity.at<Vec3b>(i,j)[0];
                    temp->Green = Image_quantity.at<Vec3b>(i,j)[1];
                    temp->Red = Image_quantity.at<Vec3b>(i,j)[2];
                    temp->count = 1;
                    temp->pre = cur_Node;
                    temp->Next = NULL;
                    cur_Node->Next = temp;
                    cur_Node = temp;
                }
            }
        }
    }

    int total_color_num = 0;
    RGB_Lab *temp = RGB_Head;
    while(temp)
    {
        total_color_num++;
//        qDebug()<<" "<<temp->Blue<<""<<temp->Green<<""<<temp->Red;
//        qDebug()<<"个数"<<temp->count;
        temp = temp->Next;
    }
    qDebug()<<"聚类前颜色种类 "<<total_color_num;
}

/********************************************
 *function:测试函数，用来计算输入的
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/17
 *******************************************/
void Cluster::compareColorNBS(RGB_Lab *Node1, RGB_Lab *Node2)
{
    float nbs = caculate_NBS(Node1,Node2);
    qDebug()<<"测试两颜色NBS距离"<<nbs;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/15
 *******************************************/
bool Cluster::checkSameNode(Pixel input)
{
    RGB_Lab *temp =RGB_Head ;
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
 *function:check whether the linked list has the node
 *input:
 *output:
 *adding:true: list already has the node  false:otherwise
 *author: wang
 *date: 2017/12/18
 *******************************************/
bool Cluster::checkSameNode(Pixel input,RGB_Lab *Head)
{
    RGB_Lab *temp = Head;
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
 *function:根据调色板节点信息计算两标准色的NBS距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/9
 *******************************************/
float Cluster::caculate_NBS(RGB_Lab *Node1, RGB_Lab *Node2)
{
     HVC hvc_node1 = RGB2HVC(Node1);
     HVC hvc_node2 = RGB2HVC(Node2);
     float distance= NBS_function(hvc_node1,hvc_node2);
     return distance;
}

/********************************************
 *function:计算调色板中的颜色和聚类中心的NBS距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
float Cluster::caculate_NBS(Cluster_point *input1, RGB_Lab *input2)
{
    RGB_Lab *Node1 = new RGB_Lab;
    Node1->Red = input1->Red;
    Node1->Green = input1->Green;
    Node1->Blue = input1->Blue;
    HVC hvc_node1 = RGB2HVC(Node1);
    HVC hvc_node2 = RGB2HVC(input2);
    delete Node1;
    float distance= NBS_function(hvc_node1,hvc_node2);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
float Cluster::caculate_NBS(Cluster_point *input1, Pixel input2)
{
    RGB_Lab *Node1 = new RGB_Lab;
    Node1->Red = input1->Red;
    Node1->Green = input1->Green;
    Node1->Blue = input1->Blue;

    RGB_Lab *Node2 = new RGB_Lab;
    Node2->Red = input2.R;
    Node2->Green = input2.G;
    Node2->Blue = input2.B;

    HVC hvc_node1 = RGB2HVC(Node1);
    HVC hvc_node2 = RGB2HVC(Node2);
    delete Node1;
    delete Node2;
    float distance= NBS_function(hvc_node1,hvc_node2);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
float Cluster::caculate_NBS(Pixel input1, Pixel input2)
{
    RGB_Lab *Node1 = new RGB_Lab;
    Node1->Red = input1.R;
    Node1->Green = input1.G;
    Node1->Blue = input1.B;

    RGB_Lab *Node2 = new RGB_Lab;
    Node2->Red = input2.R;
    Node2->Green = input2.G;
    Node2->Blue = input2.B;

    HVC hvc_node1 = RGB2HVC(Node1);
    HVC hvc_node2 = RGB2HVC(Node2);
    delete Node1;
    delete Node2;
    float distance= NBS_function(hvc_node1,hvc_node2);
    return distance;
}

/********************************************
 *function:将传入的RGB值转为HVC类型
 *input:
 *output:
 *adding:结合人类视觉特性，对于亮度小于0.2的所有颜色用HVC(-1.3912,6.0916,3.0167)替代
 *           对应的HSI(0.4980,0.1765,0.1961)
 *author: wang
 *date: 2017/12/9
 *******************************************/
HVC Cluster::RGB2HVC(RGB_Lab *Node)
{
    HVC result;
    HSI  condition;
    float X,Y,Z;//RGB变换到XYZ空间
    X = (0.608*Node->Red + 0.174*Node->Green + 0.200*Node->Blue)*1.02;
    Y = (0.299*Node->Red + 0.587*Node->Green + 0.144*Node->Blue);
    Z = (0.066*Node->Green + 1.112*Node->Blue)*0.847;
    float h1,h2,h3;
    h1 = function(X) - function(Y);
    h2 = function(Z) - function(Y);
    h3 = function(Y);
    float m1,m2,m3;
    m1 = h1;
    m2 = 0.4*h2;
    m3 = 0.23*h3;
    result.H = atan(m1/(m2+0.0000000001));
    result.V = m3;
    result.C = sqrt(m1*m1 + m2*m2);

//    float s,t;//pq空间转换到st
//    float theta = atan(p/(q+0.0000000001));
//    s = (8.880 + 0.966*cos(theta)); //*p
//    t = (8.025 + 2.558*sin(theta));  //*q
//    //st空间再转到HVC空间
//    result.H = atan(s/(t+0.0000000001));
//    result.V = function(Y);
//    result.C = sqrt(s*s + t*t);

    //将RGB转成HSI ，亮度和0.2进行比较
    condition = RGB2HSI(Node);
    if(condition.I<0.2)
    {
        result.H = -1.39;
        result.V = 6.0915;
        result.C = 3.0168;
    }
    return result;
}

/********************************************
 *function:将RGB转成HSI
 *input:RGB颜色节点
 *output:
 *adding:用几何推导法将RGB转成HSI，转换前RGB要先归一化
 *author: wang
 *date: 2017/12/10
 *******************************************/
HSI Cluster::RGB2HSI(RGB_Lab *Node)
{
    float theta,R,G,B;
    float num,den;
    HSI result;
    R = Node->Red/255;
    G = Node->Green/255;
    B = Node->Blue/255;
    num = 2*R-G-B; //分子
    den = 2*sqrt((R-G)*(R-G) + (R-B)*(G-B));  //分母
    if(den<0.000000001)  //防止分母为零出现负无穷
        den = 0.01;

    if(fabs((num/den)+1)<0.001)
        theta = 3.1415926;
    else if(fabs((num/den) -1)< 0.001)
        theta = 0;
    else
        theta = acos(num/den);  //返回值在0~PI之间

    if(B<=G)
    {
        result.H = theta;
    }
    else
    {
        result.H = 2*3.1415926-theta;
    }
    result.S = 1-3*minRGB(R,G,B)/(R+G+B+0.0000000001);
    result.I = (R+G+B)/3.0;
    return result;
}

/********************************************
 *function:将RGB转成HSI
 *input:聚类节点
 *output:
 *adding:用几何推导法将RGB转成HSI，转换前RGB要先归一化
 *author: wang
 *date: 2017/12/10
 *******************************************/
HSI Cluster::RGB2HSI(Cluster_point *Node)
{
    float theta,R,G,B;
    float num,den;
    HSI result;
    R = Node->Red/255.0;
    G = Node->Green/255.0;
    B = Node->Blue/255.0;
    num = 2*R-G-B; //分子
    den = 2*sqrt((R-G)*(R-G) + (R-B)*(G-B));  //分

    if(den<0.000000001)  //防止分母为零出现负无穷
        den = 0.01;
    if(fabs((num/den)+1)<0.001)
        theta = 3.1415926;
    else if(fabs((num/den) -1)< 0.001)
        theta = 0;
    else
        theta = acos(num/den);  //返回值在0~PI之间


    if(B<=G)
    {
        result.H = theta;
    }
    else
    {
        result.H = 2*3.1415926-theta;
    }
    result.S = 1-3*minRGB(R,G,B)/(R+G+B+0.0000000001);
    result.I = (R+G+B)/3.0;
    return result;
}

/********************************************
 *function:将RGB转成HSI
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/13
 *******************************************/
HSI Cluster::RGB2HSI(Pixel Node)
{
    float theta,R,G,B;
    float num,den;
    HSI result;
    R = Node.R/255.0;
    G = Node.G/255.0;
    B = Node.B/255.0;
    num = 2*R-G-B; //分子
    den = 2*sqrt((R-G)*(R-G) + (R-B)*(G-B));  //分母
    if(den<0.000000001)  //防止分母为零出现负无穷
        den = 0.01;
    if(fabs((num/den)+1)<0.001)
        theta = 3.1415926;
    else if(fabs((num/den) -1)< 0.001)
        theta = 0;
    else
        theta = acos(num/den);  //返回值在0~PI之间

    if(B<=G)
    {
        result.H = theta;
    }
    else
    {
        result.H = 2*3.1415926-theta;
    }
    result.S = 1-3*minRGB(R,G,B)/(R+G+B+0.0000000001);
    result.I = (R+G+B)/3.0;
    return result;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
HSI Cluster::RGB2HSI(Density *Node)
{
    float theta,R,G,B;
    float num,den;
    HSI result;
    R = Node->R/255.0;
    G = Node->G/255.0;
    B = Node->B/255.0;
    num = 2*R-G-B; //分子
    den = 2*sqrt((R-G)*(R-G) + (R-B)*(G-B));  //分母
    if(den<0.000000001)  //防止分母为零出现负无穷
        den = 0.01;
    if(fabs((num/den)+1)<0.001)
        theta = 3.1415926;
    else if(fabs((num/den) -1)< 0.001)
        theta = 0;
    else
        theta = acos(num/den);  //返回值在0~PI之间

    if(B<=G)
    {
        result.H = theta;
    }
    else
    {
        result.H = 2*3.1415926-theta;
    }
    result.S = 1-3*minRGB(R,G,B)/(R+G+B+0.0000000001);
    result.I = (R+G+B)/3.0;
    return result;
}

/********************************************
 *function:求取RGB最小值
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
float Cluster::minRGB(float R, float G, float B)
{
    float min = R;
    if(G<min)
        min = G;
    if(B < min)
        min = B;
    return min;
}

/********************************************
 *function:函数计算公式f(m)
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
float Cluster::function(float input)
{
    float result;
    result = (11.6*pow(input,0.33333) )- 1.6;
    return result;
}

/********************************************
 *function:在HVC空间计算NBS距离
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/10
 *******************************************/
float Cluster::NBS_function(HVC input1, HVC input2)
{
    float deta_H,deta_V,deta_C;
    deta_H = fabs(input1.H - input2.H);
    deta_V = fabs(input1.V - input2.V);
    deta_C = fabs(input1.C - input2.C);
    float result;
//    result = 1.2*sqrt(2*input1.C*input2.C*(1-cos(2*3.1415*deta_H/100))+deta_C*deta_C + 16*deta_V*deta_V);
    result = 1.2*sqrt(2*input1.C*input2.C*(1-cos(2*3.1415*deta_H/100))+deta_C*deta_C + 16*deta_V*deta_V);
//    result = 1.2*sqrt(2*input1.C*input2.C*(1-cos(2*3.1415*deta_H/100)+deta_C*deta_C + 4*deta_V*deta_V));
    return result;
}

/********************************************
 *function:K均值聚类中的改进距离
 *input:
 *output:
 *adding:在HSI空间中进行
 *
 *            一开始有问题BGR  51 51 102和51 102 51算出来的距离一样  具有对称性，分配聚类中心时会有问题
 *
 *
 *author: wang
 *date: 2017/12/10
 *******************************************/
float Cluster::caculate_Odistance(Cluster_point *input1, Pixel input2)
{
    float distance;
    HSI first,second;
    first = RGB2HSI(input1);
    second = RGB2HSI(input2);
    float a,b,c;
    a = first.I - second.I;
    b = adjust_I(first.I)*first.S*cos(first.H) - adjust_I(second.I)*second.S*cos(second.H);
    c = adjust_I(first.I)*first.S*sin(first.H) - adjust_I(second.I)*second.S*sin(second.H);
    distance = sqrt(a*a + b*b +c*c);
    return distance;
}

/********************************************
 *function:K均值聚类中的改进距离
 *input:
 *output:
 *adding:在HSI空间中进行
 *author: wang
 *date: 2017/12/13
 *******************************************/
float Cluster::caculate_Odistance(Cluster_point *input1, RGB_Lab *input2)
{
    float distance;
    HSI first,second;
    first = RGB2HSI(input1);
    second = RGB2HSI(input2);

    float a,b,c;
    a = first.I - second.I;
    b = adjust_I(first.I)*first.S*cos(first.H) - adjust_I(second.I)*second.S*cos(second.H);
    c = adjust_I(first.I)*first.S*sin(first.H) - adjust_I(second.I)*second.S*sin(second.H);
    //    b = first.S*cos(first.H) - second.S*cos(second.H);
    //    c = first.S*sin(first.H) - second.S*sin(second.H);
    distance = sqrt(a*a + b*b +c*c);
    return distance;
}

/********************************************
 *function:调整HSI的I参数值
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/13
 *******************************************/
float Cluster::adjust_I(float I)
{
    return (1-fabs(I - 0.5)/0.5);
}

/********************************************
 *function:找出RGB链表中数目最多的节点
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/17
 *******************************************/
RGB_Lab *Cluster::findMaxNumColor()
{
    int  max = 0,temp;
    RGB_Lab *target = NULL,*cur = NULL;
    cur = RGB_Head;
    while(cur)
    {
        temp = cur->count;
        if(temp>max)
        {
            target = cur;
            max = temp;
        }
        cur = cur->Next;
    }
    return target;
}

/********************************************
 *function:将输入的RGB节点和cluster链表作对比，判断是否可以作为聚类中心
 *input:
 *output:  true  可以作为聚类中心  false 不能作为聚类中心
 *adding:
 *author: wang
 *date: 2017/12/17
 *******************************************/
bool Cluster::isAcluster(RGB_Lab *input)
{
    Cluster_point *temp = cluster_Head;
    float distance = 0;
    while(temp)
    {
        distance = caculate_NBS(temp,input);
        if(distance<NBS_yuzhi)   //NBS_yuzhi  NBS_threshold
            return false;
        temp = temp->Next;
    }
    return true;
}

/********************************************
 *function:check whether the RGB list is tempty
 *input:
 *output: true: is empty  false:is not empty
 *adding:
 *author: wang
 *date: 2017/12/17
 *******************************************/
bool Cluster::RGBlist_isEmpty()
{
    if(RGB_Head == NULL)
        return true;
    else
        return false;
}

/********************************************
 *function:find the target pixel to replace the noise point
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
Pixel Cluster::findReplacePixel(int row, int col)
{
    RGB_Lab *Last_node = NULL;
    RGB_Lab *temp_Node = NULL;
    RGB_Lab *nosie_head = NULL;
    nosie_head = new RGB_Lab;
    nosie_head->Blue = Image_quantity.at<Vec3b>(row-1,col-1)[0];
    nosie_head->Green = Image_quantity.at<Vec3b>(row-1,col-1)[1];
    nosie_head->Red = Image_quantity.at<Vec3b>(row-1,col-1)[2];
    nosie_head->count = 1;
    nosie_head->Next = NULL;
    Last_node = nosie_head;

    Pixel temp;
    temp.B = Image_quantity.at<Vec3b>(row-1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row-1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col-1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col-1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    RGB_Lab *result = NULL;
    int max = 0,cur ;
    temp_Node = nosie_head;
    while(temp_Node)
    {
        cur = temp_Node->count;
        if(cur>max)
        {
            max = cur;
            result = temp_Node;
        }
        temp_Node = temp_Node->Next;
    }

    Pixel target;
    target.R = result->Red;
    target.G = result->Green;
    target.B = result->Blue;

    RGB_Lab *next;
    temp_Node = nosie_head;
    while(temp_Node)
    {
        next = temp_Node->Next;
        delete temp_Node;
        temp_Node = next;
    }

    return target;
}

/********************************************
 *function:find the replace pixel bt NBS distance
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
Pixel Cluster::findReplacePixel2(int row, int col)
{
    float minDistance,tempDistance;
    Pixel target;

    Pixel origin;
    origin.B = Image_quantity.at<Vec3b>(row,col)[0];
    origin.G = Image_quantity.at<Vec3b>(row,col)[1];
    origin.R = Image_quantity.at<Vec3b>(row,col)[2];

    Pixel temp;
    temp.B = Image_quantity.at<Vec3b>(row-1,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col-1)[2];
    minDistance = caculate_NBS(origin,temp);
    target = temp;

    temp.B = Image_quantity.at<Vec3b>(row-1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row-1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col+1)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col-1)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col+1)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col-1)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col+1)[2];
    tempDistance = caculate_NBS(origin,temp);
    if(tempDistance<minDistance)
    {
        minDistance = tempDistance;
        target = temp;
    }

    return target;

}

/********************************************
 *function:find the replace pixel by nbs distance and number
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
Pixel Cluster::findReplacePixel3(int row, int col)
{
    RGB_Lab *Last_node = NULL;
    RGB_Lab *temp_Node = NULL;
    RGB_Lab *nosie_head = NULL;
    nosie_head = new RGB_Lab;
    nosie_head->Blue = Image_quantity.at<Vec3b>(row-1,col-1)[0];
    nosie_head->Green = Image_quantity.at<Vec3b>(row-1,col-1)[1];
    nosie_head->Red = Image_quantity.at<Vec3b>(row-1,col-1)[2];
    nosie_head->count = 1;
    nosie_head->Next = NULL;
    Last_node = nosie_head;

    Pixel temp;
    temp.B = Image_quantity.at<Vec3b>(row-1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row-1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row-1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row-1,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col-1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col-1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col-1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col-1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    temp.B = Image_quantity.at<Vec3b>(row+1,col+1)[0];
    temp.G = Image_quantity.at<Vec3b>(row+1,col+1)[1];
    temp.R = Image_quantity.at<Vec3b>(row+1,col+1)[2];
    if(checkSameNode(temp,nosie_head))
    {

    }
    else
    {
        temp_Node = new RGB_Lab;
        temp_Node->Blue = temp.B;
        temp_Node->Green = temp.G;
        temp_Node->Red = temp.R;
        temp_Node->count = 1;
        temp_Node->Next = NULL;
        Last_node->Next = temp_Node;
        Last_node = temp_Node;
    }

    RGB_Lab *result = NULL;
    int max = 0,cur ;
    int numbers = 0;
    temp_Node = nosie_head;
    while(temp_Node)
    {
        numbers++;
        cur = temp_Node->count;
        if(cur>max)
        {
            max = cur;
            result = temp_Node;
        }
        temp_Node = temp_Node->Next;
    }

    Pixel target;
    if(numbers==2&&max<5)
    {
        target = findReplacePixel2(row,col);
    }
    else
    {
        target.R = result->Red;
        target.G = result->Green;
        target.B = result->Blue;
    }

    //release the memory
    RGB_Lab *next;
    temp_Node = nosie_head;
    while(temp_Node)
    {
        next = temp_Node->Next;
        delete temp_Node;
        temp_Node = next;
    }

    return target;
}

/********************************************
 *function:计算每个颜色链表节点的
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::computerDensity()
{

}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::create_density()
{
    Density *density_last = NULL;
    Density *density_temp = NULL;
    RGB_Lab *RGB_temp = RGB_Head;
    RGB_Lab *RGB_temp2 = RGB_Head;
    float nbs_ditance = 0;
    //遍历所有的颜色组合
    while(RGB_temp)
    {
        if(density_last == NULL)
        {
            density_temp = new Density;
            density_temp->R = RGB_temp->Red;
            density_temp->G = RGB_temp->Green;
            density_temp->B = RGB_temp->Blue;
            density_temp->den = normal_K*RGB_temp->count/(image_width*image_height);
            density_temp->pre = NULL;
            density_temp->pNext = NULL;
            density_Head = density_temp;
            density_last = density_temp;
        }
        else
        {
            density_temp = new Density;
            density_temp->R = RGB_temp->Red;
            density_temp->G = RGB_temp->Green;
            density_temp->B = RGB_temp->Blue;
            density_temp->den = normal_K*RGB_temp->count/(image_width*image_height);
            density_temp->pre = density_last;
            density_temp->pNext = NULL;
            density_last->pNext = density_temp;
            density_last = density_temp;
        }

        RGB_temp2 = RGB_Head;
        while(RGB_temp2)
        {
            if(RGB_temp == RGB_temp2)
            {
                RGB_temp2 = RGB_temp2->Next;
                continue;
            }
            else
            {
                nbs_ditance = caculate_NBS(RGB_temp,RGB_temp2);
                if(nbs_ditance<0.3)
                {
                    density_temp->den = density_temp->den + normal_K*RGB_temp2->count/(image_width*image_height); //0  RGB_temp2->count
                }
                RGB_temp2 = RGB_temp2->Next;
            }
        }
        RGB_temp = RGB_temp->Next;
    }

    int count = 0;
    density_temp = density_Head;
    while(density_temp)
    {
        count++;
        qDebug()<<count<<": "<<density_temp->den;
        density_temp = density_temp->pNext;
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::create_densityDistance()
{
    RGB_Lab temp1,temp2;
    Density *density_temp1 = density_Head;
    Density *density_temp2 = density_Head;
    float distance,min_distance;
    bool isfirst;
    while(density_temp1)
    {
        density_temp2 = density_Head;
        distance = 0;
        min_distance = 0;
        isfirst = true;
        while(density_temp2)
        {
            if(density_temp1== density_temp2)
            {
                 density_temp2 = density_temp2->pNext;
                 continue;
            }
            else
            {
                if(density_temp2->den>density_temp1->den)
                {
                    temp1.Red = density_temp1->R;
                    temp1.Green = density_temp1->G;
                    temp1.Blue = density_temp1->B;
                    temp2.Red = density_temp2->R;
                    temp2.Green = density_temp2->G;
                    temp2.Blue = density_temp2->B;
                    if(isfirst)
                    {
                          distance = caculate_NBS(&temp1,&temp2);
                          min_distance = distance;
                          isfirst = false;
                    }
                else
                    {
                          distance = caculate_NBS(&temp1,&temp2);
                           if(distance<min_distance)
                             {
                               min_distance = distance;
                             }
                    }
                 }
            }
            density_temp2 = density_temp2->pNext;
        }
        density_temp1->distance = min_distance;
        density_temp1 = density_temp1->pNext;
      }

      //find the max distance between pixel points
    Density *density_temp = density_Head;
    while(density_temp)
    {
         if(density_temp->distance == 0)
             density_temp->distance = findMaxNBSditance();
        density_temp = density_temp->pNext;
    }

    density_temp = density_Head;
    while(density_temp)
    {
//        qDebug()<<density_temp->den<<" "<<density_temp->distance;
        density_temp->gama = density_temp->den*density_temp->distance;
//        qDebug()<<density_temp->gama;
        density_temp = density_temp->pNext;
    }


}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
float Cluster::findMaxNBSditance()
{
    RGB_Lab *RGB_temp = RGB_Head;
    RGB_Lab *RGB_temp2 = NULL;
    float nbs_distance = 0,maxdistance = 0;
    //遍历所有的颜色组合
    while(RGB_temp)
    {
        RGB_temp2 = RGB_temp->Next;
        while(RGB_temp2)
        {
            nbs_distance = caculate_NBS(RGB_temp,RGB_temp2);
            if(nbs_distance>maxdistance)
                maxdistance = nbs_distance;
            RGB_temp2 = RGB_temp2->Next;
        }
        RGB_temp = RGB_temp->Next;
    }
    qDebug()<<"最大距离为"<<maxdistance;
    return maxdistance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
Density *Cluster::findMaxGanma(Density *Head)
{
    Density *temp= NULL,*target = NULL;
    float max_ditance = 0;
    max_ditance = Head->gama;
    target = Head;
    temp = Head;
    while(temp)
    {
        if(temp->gama>max_ditance)
        {
            target = temp;
            max_ditance = temp->gama;
        }
        temp = temp->pNext;
    }
    return target;
}

/********************************************
 *function:sort the density list
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::sortDensityList()
{
    bool isFirst = true;
    Density *target = NULL,*temp = NULL,*last = NULL;
    while(density_Head)
    {
        if(isFirst)
        {
            target = findMaxGanma(density_Head);
            releaseDensityNode(target);
            target->pre= NULL;
            target->pNext = NULL;
            sort_density_Head = target;
            last = target;
            isFirst = false;
        }
        else
        {
            target = findMaxGanma(density_Head);
            releaseDensityNode(target);
            last->pNext = target;
            target->pre = last;
            target->pNext = NULL;
            last = target;
        }
    }

    temp = sort_density_Head;
    int count = 0;
    while(temp)
    {
        count++;
        qDebug()<<temp->gama<<" "<<temp->den<<"  "<<temp->distance<<" "<<temp->R<<" "<<temp->G<<" "<<temp->B;
//        qDebug()<<temp->gama;
//        qDebug()<<temp->R<<" "<<temp->G<<" "<<temp->B;
        temp = temp->pNext;
    }

}

/********************************************
 *function:release the target node,pay attention to the head and tail of the list
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::releaseDensityNode(Density *Node)
{
    if(Node->pre == NULL)//head of the list
    {
        if(Node->pNext== NULL) //the last one node
        {
            density_Head = NULL;
            return;
        }
        Node->pNext->pre = NULL;
        density_Head = Node->pNext;
    }
    else if(Node->pNext == NULL)//tail of the list
    {
        Node->pre->pNext = NULL;
    }
    else  //the others
    {
        Node->pre->pNext = Node->pNext;
        Node->pNext->pre = Node->pre;
    }
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::findDensityCluster()
{
    Density *temp1 = sort_density_Head->pNext;
//    qDebug()<<sort_density_Head->R<<" "<<sort_density_Head->G<<" "<<sort_density_Head->B;
    Density *temp2 = NULL;
    float distance = 0;
    while(temp1)
    {
        temp2 = temp1->pre;
        while(temp2)
        {
            distance = NBScaculate(temp1,temp2);
            if(distance <3)  //nbs threshold
            {
                temp1->pNext = NULL;
                return;
            }
            temp2 = temp2->pre;
        }
//        qDebug()<<temp1->R<<" "<<temp1->G<<" "<<temp1->B;
        temp1 = temp1->pNext;
    }

}

/********************************************
 *function:find the cluster by order and NBS distance(larger than threshold)
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
void Cluster::findDensityCluster2()
{
    cluster_density_Head = new Density;
    cluster_density_Head->R = sort_density_Head->R;
    cluster_density_Head->G = sort_density_Head->G;
    cluster_density_Head->B = sort_density_Head->B;
    cluster_density_Head->pre = NULL;
    cluster_density_Head->pNext = NULL;

    Density *temp1 = NULL,*temp2 = NULL,*last = NULL;
    last = cluster_density_Head;
    temp1= sort_density_Head;
    while(temp1)
    {
        if(canBeACluser(temp1,cluster_density_Head))
        {
            temp2 = new Density;
            temp2->R = temp1->R;
            temp2->G = temp1->G;
            temp2->B = temp1->B;
            temp2->pre = last;
            temp2->pNext = NULL;
            last->pNext = temp2;
            last = temp2;
        }
        temp1 = temp1->pNext;
    }



}

/********************************************
 *function:根据需要的颜色数目cluster_num来确定聚类中心数目
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/5/4
 *******************************************/
void Cluster::findDensityCluster3()
{
    cluster_density_Head = new Density;
    cluster_density_Head->R = sort_density_Head->R;
    cluster_density_Head->G = sort_density_Head->G;
    cluster_density_Head->B = sort_density_Head->B;
    cluster_density_Head->pre = NULL;
    cluster_density_Head->pNext = NULL;

    Density *temp1 = NULL,*temp2 = NULL,*last = NULL;
    last = cluster_density_Head;
    temp1= sort_density_Head->pNext;
    for(int i = 0;i<cluster_num-1;i++)
    {

        temp2 = new Density;
        temp2->R = temp1->R;
        temp2->G = temp1->G;
        temp2->B = temp1->B;
        temp2->pre = last;
        temp2->pNext = NULL;
        last->pNext = temp2;
        last = temp2;

        temp1 = temp1->pNext;
    }
}

/********************************************
 *function:check the input weather is a cluster
 *input:
 *output:true: is a cluster   false:not a cluster
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
bool Cluster::canBeACluser(Density *input, Density *Head)
{
    Density *temp = Head;
    float distance = 0;
    while(temp)
    {
        distance = NBScaculate(input,temp);
        if(distance<3.2)
            return false;
        temp = temp->pNext;
    }
    return true;
}

/********************************************
 *function:caculate the distance between two nodes
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
float Cluster::NBScaculate(Density *input1, Density *input2)
{
    RGB_Lab rgb1,rgb2;
    rgb1.Red = input1->R;
    rgb1.Green= input1->G;
    rgb1.Blue = input1->B;
    rgb2.Red = input2->R;
    rgb2.Green = input2->G;
    rgb2.Blue = input2->B;
    float distance = caculate_NBS(&rgb1,&rgb2);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
float Cluster::NBScaculate(RGB_Lab input1, Density *input2)
{
    RGB_Lab rgb1,rgb2;
    rgb1.Red = input1.Red;
    rgb1.Green= input1.Green;
    rgb1.Blue = input1.Blue;
    rgb2.Red = input2->R;
    rgb2.Green = input2->G;
    rgb2.Blue = input2->B;
    float distance = caculate_NBS(&rgb1,&rgb2);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/19
 *******************************************/
void Cluster::fill_Image()
{
    float count = 0;
    Density *temp = cluster_density_Head;
    qDebug()<<"聚类中心";
    while(temp)
    {
        count++;
        qDebug()<<temp->R<<" "<<temp->G<<" "<<temp->B;
        temp = temp->pNext;
    }
    qDebug()<<"一共种类"<<count;

    RGB_Lab  rgb;
    Pixel result;
    for(int i = 0;i<Image_quantity.rows;i++)
    {
        for(int j = 0;j<Image_quantity.cols;j++)
        {
            if(i == 288&&j ==160)
            {
                qDebug()<<i;
            }
            rgb.Blue = Image_quantity.at<Vec3b>(i,j)[0];
            rgb.Green = Image_quantity.at<Vec3b>(i,j)[1];
            rgb.Red = Image_quantity.at<Vec3b>(i,j)[2];
            result = findReplacePixel(rgb);
            Image_quantity.at<Vec3b>(i,j)[0] = result.B;
            Image_quantity.at<Vec3b>(i,j)[1] = result.G;
            Image_quantity.at<Vec3b>(i,j)[2] = result.R;
//            if(i == 288&&j ==160)
//            {
//                qDebug()<<Image_quantity.at<Vec3b>(i,j)[0]<<" "<<Image_quantity.at<Vec3b>(i,j)[1]<<" "<<Image_quantity.at<Vec3b>(i,j)[2];
//            }
        }
    }
}

/********************************************
 *function:find the replace pixel to fill the image
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/20
 *******************************************/
Pixel Cluster::findReplacePixel(RGB_Lab input)
{
    float distance = 0,min_ditance=0;
    Density *temp =NULL,*target=NULL;
    temp = cluster_density_Head;
    min_ditance = caculate_Odistance(&input,temp);
    target = temp;
    while(temp)
    {
        distance = caculate_Odistance(&input,temp);
        if(distance<min_ditance)
        {
            min_ditance = distance;
            target = temp;
        }
        temp = temp->pNext;
    }
    Pixel result;
    result.R = target->R;
    result.G = target->G;
    result.B = target->B;
    return result;
}

/********************************************
 *function:caculate the distance between Pixel point and cluster point
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/21
 *******************************************/
float Cluster::caculate_Odistance(RGB_Lab *input1, Density *input2)
{
    float distance;
    HSI first,second;
    first = RGB2HSI(input1);
    second = RGB2HSI(input2);
    float a,b,c;
    a = first.I - second.I;
    b = adjust_I(first.I)*first.S*cos(first.H) - adjust_I(second.I)*second.S*cos(second.H);
    c = adjust_I(first.I)*first.S*sin(first.H) - adjust_I(second.I)*second.S*sin(second.H);
    distance = sqrt(a*a + b*b +c*c);
    return distance;
}

/********************************************
 *function:
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
void Cluster::expandImage()
{
    Mat element = getStructuringElement(MORPH_RECT,Size(kernel_size,kernel_size));
//    dilate(Image_quantity,Image_quantity,element);
    erode(Image_cluster,Image_cluster,element);
//    namedWindow("膨胀后", CV_WINDOW_AUTOSIZE);
//    imshow("膨胀后", Image_cluster);
}

/********************************************
 *function:对图像进行滤波处理
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
void Cluster::filterProcess()
{
    Mat out;
//    GaussianBlur(Image_quantity,out,Size(kernel_size,kernel_size),0,0);
//    boxFilter(Image_quantity,out,-1,Size(kernel_size,kernel_size));
//    blur (Image_quantity,out,Size(kernel_size,kernel_size));
//    medianBlur(Image_quantity,out,kernel_size);
    bilateralFilter(Image_quantity,out,kernel_size,kernel_size*2,kernel_size/2);
    Image_quantity = out;
//    namedWindow("滤波后", CV_WINDOW_AUTOSIZE);
    //    imshow("滤波后", Image_quantity);
}

void Cluster::filterProcess2()
{
        Mat out;
        blur (Image_quantity,out,Size(kernel_size,kernel_size));
        Image_quantity = out;
}

/********************************************
 *function:reduce the nosie points by number
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
void Cluster::reduceNosiePoint()
{
    Mat temp = Image_quantity.clone();
    for(int i = 1;i<(Image_quantity.rows-1);i++)
    {
        for(int j = 1;j<(Image_quantity.cols-1);j++)
        {
            Pixel  result = findReplacePixel(i,j);
            temp.at<Vec3b>(i,j)[0] = result.B;
            temp.at<Vec3b>(i,j)[1] = result.G;
            temp.at<Vec3b>(i,j)[2] = result.R;
        }
    }
    Image_quantity = temp;
}

/********************************************
 *function:reduce the nosie points by nbs distance
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
void Cluster::reduceNosiePoint2()
{
    Mat temp = Image_quantity.clone();
    for(int i = 1;i<(Image_quantity.rows-1);i++)
    {
        for(int j = 1;j<(Image_quantity.cols-1);j++)
        {
            Pixel  result = findReplacePixel2(i,j);
            temp.at<Vec3b>(i,j)[0] = result.B;
            temp.at<Vec3b>(i,j)[1] = result.G;
            temp.at<Vec3b>(i,j)[2] = result.R;
        }
    }
    Image_quantity = temp;
}

/********************************************
 *function:reduce the noise point by number and nbs distance
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/26
 *******************************************/
void Cluster::reduceNosiePoint3()
{
    Mat temp = Image_quantity.clone();
    for(int i = 1;i<(Image_quantity.rows-1);i++)
    {
        for(int j = 1;j<(Image_quantity.cols-1);j++)
        {
            Pixel  result = findReplacePixel3(i,j);
            temp.at<Vec3b>(i,j)[0] = result.B;
            temp.at<Vec3b>(i,j)[1] = result.G;
            temp.at<Vec3b>(i,j)[2] = result.R;
        }
    }
    Image_quantity = temp;
}

/********************************************
 *function:密度峰
 *input:
 *output:
 *adding:
 *author: wang
 *date: 2017/12/18
 *******************************************/
void Cluster::ADPC_cluster()
{
    reGenerateRGB_LabPointer(); //生成量化后图片的颜色链表
    create_density();
    create_densityDistance();
    sortDensityList();

//    findDensityCluster2();
    findDensityCluster3();
    fill_Image();

}

/********************************************
 *function:直接根据指定的颜色中心聚类
 *input:
 *output:
 *adding:
 *author: wong
 *date: 2018/4/3
 *******************************************/
void Cluster::direct_cluster()
{
    create_clusterCenter3();//生成初始聚类中心链表
    distributePoint2Cluster();//分配像素点到聚类中心  第一次
    fill_cluster_Image();//生成聚类后的图像
}
