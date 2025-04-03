#include <iostream>

template<typename T>
class Shadow{

};

template<boost::Polygon>
class Shadow{
    public:
    // begin_points 和 set_points  不考虑inners
    // begin_holes
        // std::vector<Shadow<Point>> inners; // 只存 outer
        std::vector<Shadow<Point>> outer;
    public:
        void fromShadow(boost::Polygon* obj);
        void toShadow(boost::Polygon* obj);
};

Table4ClassExternal(Point, (x,y, ...)){
    
    cpptypetodbtype -> dbTypes::kExternal
    Table4Class(Shadow<boost::Point>,"",(x,y, ...))
}

Table4ClassExternal(boost::Polygon, (outer, ...)){
    cpptypetodbtype -> dbTypes::kExternal
    Table4Class(Shadow<boost::Polygon>,"",(outer, ...))
}

Table4Class(FatherClass,"table_name",(, , polygon))// 语法不需要任何修改
// 如何在createTable中定义pk   子类没有主键的问题
