#ifndef ENTITYTYPE_H
#define ENTITYTYPE_H


enum  fitType{LineType,SplineType};   //曲线拟合类型: 直线和三次B样条

typedef struct Point
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
    Point  start;
    Point  end;
};

class SplineEntity : public baseEntity
{
    Point pot1;
    Point pot2;
    Point pot3;
    Point pot4;
};

#endif // ENTITYTYPE_H
