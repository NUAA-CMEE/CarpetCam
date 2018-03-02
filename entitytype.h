#ifndef ENTITYTYPE_H
#define ENTITYTYPE_H


enum  fitType{LineType,SplineType};   //曲线拟合类型: 直线和三次B样条

typedef struct Point_mine
{
    float x;
    float y;
}point_f;

class baseEntity
{
public:
    fitType  sonType; //子类数据类型
};

class LineEntity : public baseEntity
{
    Point_mine  start;
    Point_mine  end;
};

class SplineEntity : public baseEntity
{
    Point_mine pot1;
    Point_mine pot2;
    Point_mine pot3;
    Point_mine pot4;
};

#endif // ENTITYTYPE_H
