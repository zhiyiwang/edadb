// #include "lib/edadb/edadbTestMacro.hpp"
#include "lib/edadb/edadbTest18.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
/*
* 支持vector的嵌套
*/

struct IdbRect {
  int x;
  IdbRect(int _x){
    x = _x;
  }
};

struct IdbRect2 {
  int y;
  IdbRect2(int _y){
    y = _y;
  }
};

/// @class IdbLayerShape // example 
class IdbLayerShape {
  public:
    std::string _name;      
    std::string _layer;     
    std::vector<IdbRect> _rects; 
    std::vector<IdbRect2> _rect2s; 

  public:
    IdbLayerShape() = default;
    IdbLayerShape(std::string n, std::string l) : _name(n), _layer(l) {}

    void print() const {
        std::cout << "LayerShape(type=" << _name 
                  << ", layer=" << _layer 
                  // << ", rects.size =" << _rects.size() 
                  << ")\n";
    }
};

/// @class IdbPort  // example 
class IdbPort {
  public:
    std::string _name;
    std::vector<IdbLayerShape> _layer_shapes; // 替换idbsite_array
    std::vector<IdbRect> port_rects; 

public:
    IdbPort() = default;
    IdbPort(std::string n) : _name(n) {}

    void print() const {
        std::cout << "Port(name=" << _name << ")\n";
        for (auto& s : _layer_shapes) s.print();
    }
};

TABLE4CLASS(IdbRect, "rect_table", (x));
TABLE4CLASS(IdbRect2, "rect2_table", (y));

TABLE4CLASSWVEC(IdbLayerShape, "layer_shape_table", (_name, _layer), (_rects, _rect2s));

TABLE4CLASSWVEC(IdbPort, "port_table", (_name), (_layer_shapes, port_rects));

/*
1.只定义DbMap<IdbRect>,DbMap<IdbLayerShape>, DbMap<IdbPort>
在定义DbMap<IdbLayerShape>时，通过某种方式知道其内包含ArrayField
比如在DbMap<IdbLayerShape>里有一个成员变量DbMap<IdbRect> _rects
在DbMap<IdbPort>里有一个成员变量DbMap<IdbLayerShape> _layer_shapes
还是用宏拼出，具体方法类似定义DbMapAll

2.可以通过某种方法记录比如在metadata<myclass>中ARRAY_FIELD_TUP成boost sequence

3.定义DbType是composite vector
可以在TABLE4CLASS_COLNAME定义加一个参数，来决定template<>\
struct CppTypeToDbType<myclass>{\
    static const DbTypes ret = DbTypes::kComposite;\
};\还是DbTypes::composite_vector 

4.在宏展开中只包含函数声明，具体函数里要进行递归调用，写一个template<T>实现在宏的前面，具体函数实现写在后面
*/

int main() {

  edadb::dbgPrint();

  std::cout<<"\n-----------The former tests-------------\n";

  IdbLayerShape* shape1 = new IdbLayerShape("METAL1", "M1");
  shape1->_rects.emplace_back(11);  
  IdbLayerShape* shape2 = new IdbLayerShape("METAL2", "M2");
  shape2->_rects.emplace_back(22);


  IdbPort* port1 = new IdbPort("Port1");
  port1->_layer_shapes.push_back(*shape1);

  IdbPort* port2 = new IdbPort("Port2");
  port2->_layer_shapes.push_back(*shape2);


  edadb::DbMap<IdbLayerShape> db_LayerShape;
  edadb::DbMap<IdbPort> db_Port;

  // edadb::DbMapAll<IdbPort> db_Port;

  db_LayerShape.connectToDb("sqlite.db");
  // db_LayerShape.createTable("layer_shape_table");
  IdbLayerShape ls1("test_name", "test_layer");
  ls1._rects.push_back(IdbRect(123));
  // db_LayerShape.insertToDbAll(&ls1);

  db_Port.createTable("port_table");
  IdbPort port("port_name");
  port._layer_shapes.push_back(ls1);
  db_Port.insertToDb(&port);


  return 0;
}

