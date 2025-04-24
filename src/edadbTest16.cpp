// #include "lib/edadb/edadbTestMacro.hpp"
#include "lib/edadb/edadbTest15.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
/*
* 支持指针
*/

class ExternalClass1{
  private:
    int pra_field_ex1;
  public:
    int get() const { return pra_field_ex1; }
    void set(int val) { pra_field_ex1 = val; }
    void print() const { // 不用const修饰无法被const对象调用
      std::cout << "pra_field_ex1: " << pra_field_ex1<<"\n";
    }
};

class ExternalClass2{
  private:
    int pra_field_ex2;
  public:
    int get() const { return pra_field_ex2; }
    void set(int val) { pra_field_ex2 = val; }
    void print() const { // 不用const修饰无法被const对象调用
      std::cout << "pra_field_ex2: " << pra_field_ex2<<"\n";
    }
};


template<>
class edadb::Shadow<ExternalClass1>{
  public:
    int sha_field_ex1;
  public:
    void fromShadow(ExternalClass1* obj){
      obj->set(sha_field_ex1);
    }
    void toShadow(ExternalClass1* obj){
      sha_field_ex1 = obj->get();
    }
};

template<>
class edadb::Shadow<ExternalClass2>{
  public:
    int sha_field_ex2;
  public:
    void fromShadow(ExternalClass2* obj){
      obj->set(sha_field_ex2);
    }
    void toShadow(ExternalClass2* obj){
      sha_field_ex2 = obj->get();
    }
};

struct IdbCoordinate {
  public:
    ExternalClass1 ex1;
    ExternalClass2 ex2;
  public:
    int32_t x;
    int32_t y;
    IdbCoordinate(int32_t _x = 0, int32_t _y = 0) : x(_x), y(_y) {ex1.set(4),ex2.set(8);}

    void print() const {
      std::cout << "Coordinate(x=" << x 
                << ", y=" << y 
                << ", ex1=" << ex1.get()  
                << ", ex2=" << ex2.get()   
                << ")\n";
    }
};


struct IdbRect {
  IdbCoordinate ll; // lower left 
  IdbCoordinate ur; // upper right
  IdbRect(int32_t x1 = 0, int32_t y1 = 0, int32_t x2 = 0, int32_t y2 = 0) 
      : ll(x1, y1), ur(x2, y2) {}
};

/// @class IdbLayerShape // example 
class IdbLayerShape {
  public:
    std::string _name;      
    std::string _layer;     
    std::vector<IdbRect> _rects; 

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

enum MyNormalEnum { 
    kEnum1 = 0, 
    kEnum2 = 1 
};

/// @class IdbVia  // example 
class IdbVia {
  public:
    enum class ViaType { 
        kVia1 = 0, 
        kVia2 = 1 
    };

    std::string _name;
    IdbCoordinate _coord;
    ViaType _type;
    MyNormalEnum _normal_enum;

  public:
    IdbVia() = default;
    IdbVia(std::string n, int32_t x, int32_t y, ViaType t = ViaType::kVia1, MyNormalEnum e = kEnum1)
        : _name(n), _coord(x, y), _type(t), _normal_enum(e) {}

    void print() const {
        std::cout << "Via(name=" << _name 
                  << ", x=" << _coord.x 
                  << ", y=" << _coord.y
                  <<", _coord.ex1 = " << _coord.ex1.get()
                  <<", _coord.ex2 = " << _coord.ex2.get()
                  << ", type=" << static_cast<int>(_type)
                  << ", normal_enum=" << _normal_enum
                  << ")\n";
    }
};

/// @class IdbPort  // example 
class IdbPort {
  public:
    std::string _name;
    std::vector<IdbLayerShape> _layer_shapes; // 替换idbsite_array
    std::vector<IdbVia> _vias;                // 替换array2_array
    
    std::vector<ExternalClass1> _ex1s;

public:
    IdbPort() = default;
    IdbPort(std::string n) : _name(n) {}

    void print() const {
        std::cout << "Port(name=" << _name << ")\n";
        for (auto& s : _layer_shapes) s.print();
        for (auto& v : _vias) v.print();
        for (auto& e : _ex1s) e.print();
    }
};

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
// #define TABLE4CLASSWEXTERNAL(myclass, tablename, CLASS_ELEMS_TUP, EXTERNAL_TUP)

// Table4ExternalClass(ExternalClass1, writeFuncType1, readFuncType1);
// Table4ExternalClass(ExternalClass2, writeFuncType2, readFuncType2);
Table4ExternalClass(ExternalClass1, (sha_field_ex1));
Table4ExternalClass(ExternalClass2, (sha_field_ex2));


// TABLE4CLASSWEXTERNAL(IdbCoordinate, "coordinate_table", (x, y), (ex1,ex2));
TABLE4CLASS(IdbCoordinate, "coordinate_table", (x, y, ex1, ex2));
TABLE4CLASS(IdbLayerShape, "layer_shape_table", (_name, _layer));
TABLE4CLASS(IdbVia, "via_table", (_name, _coord, _type, _normal_enum));



TABLE4CLASSWVEC(IdbPort, "port_table", (_name), (_layer_shapes, _vias, _ex1s));


int main() {

  edadb::dbgPrint();

  IdbCoordinate * coord1 = new IdbCoordinate(1,1);
  IdbCoordinate * coord2 = new IdbCoordinate(2,2);
  IdbCoordinate * coord3 = new IdbCoordinate(3,3);
  IdbCoordinate * coord4 = new IdbCoordinate(4,4);
  coord1->ex1.set(2);
  coord1->ex2.set(4);
  coord2->ex1.set(8);
  coord2->ex2.set(16);
  coord3->ex1.set(32);
  coord3->ex2.set(64);
  coord4->ex1.set(128);
  coord4->ex2.set(256);
  edadb::DbMap<IdbCoordinate> db_Coordinate;
  db_Coordinate.connectToDb("db=sqlite.db foreign_keys=1");
  // db_Coordinate.connectToDb("sqlite.db");
  db_Coordinate.createTable("coordinate_table");

  std::cout<<"\n-----------insert records-------------\n";
  db_Coordinate.insertToDb(coord1);
  db_Coordinate.insertToDb(coord2);
  db_Coordinate.insertToDb(coord3);
  db_Coordinate.insertToDb(coord4);

  std::cout<<"\n-----------update records-------------\n";
  // coord1->x *= 2; 主键不改
  coord3->y *= 2;
  coord3->ex1.set(32*2);
  db_Coordinate.updateDb(coord3);


  std::cout<<"\n-----------select test-------------\n";
  std::vector<IdbCoordinate> coord_results;
  db_Coordinate.selectFromDb(&coord_results);

  for(auto& coord : coord_results) {
    coord.print();
  }

  std::cout<<"\n-----------delete records-------------\n";
  db_Coordinate.deleteFromDb(coord4); // 不变

  coord_results.clear();
  db_Coordinate.selectFromDb(&coord_results);

  for(auto& coord : coord_results) {
    coord.print();
  }

  std::cout<<"\n-----------The former tests-------------\n";

  IdbLayerShape* shape1 = new IdbLayerShape("METAL1", "M1");
  shape1->_rects.emplace_back(0, 0, 100, 100);  
  IdbLayerShape* shape2 = new IdbLayerShape("METAL2", "M2");
  shape2->_rects.emplace_back(200, 200, 300, 300);

  IdbVia* via1 = new IdbVia("VIA1", 50, 50, IdbVia::ViaType::kVia1);
  IdbVia* via2 = new IdbVia("VIA2", 250, 250, IdbVia::ViaType::kVia2);

  IdbPort* port1 = new IdbPort("Port1");
  port1->_layer_shapes.push_back(*shape1);
  port1->_vias.push_back(*via1);

  IdbPort* port2 = new IdbPort("Port2");
  port2->_layer_shapes.push_back(*shape2);
  port2->_vias.push_back(*via2);

  edadb::DbMap<IdbLayerShape> db_LayerShape;
  edadb::DbMap<IdbVia> db_Via;
  edadb::DbMapAll<IdbPort> db_Port;

  // db_Port.connectToDb("sqlite.db");
  db_LayerShape.createTable("layer_shape_table");
  db_Via.createTable("via_table");
  db_Port.createTableAll("port_table");

  std::cout<<"\n-----------insert records-------------\n";

  db_Via.insertToDb(via1);
  db_Via.insertToDb(via2);

  db_Port.insertToDbAll(port1); // 前提是已经createTableAll
  db_Port.insertToDbAll(port2); // 插入port2及其关联的shape2和via2

  std::vector<IdbPort> port_results;
  db_Port.selectFromDbAll(&port_results);
  
  for (auto& port : port_results) {
    port.print();
  }

  std::cout<<"\n-----------update records-------------\n";
  std::cout<<"-------before update records:---------\n";
  std::vector<IdbVia> via_vec;
  db_Via.selectFromDb(&via_vec);
  for(auto i : via_vec) i.print();

  via1->_coord.x = 100; 
  via1->_coord.y = 100;
  via1->_type = IdbVia::ViaType::kVia1;
  via1->_normal_enum = kEnum2;
  db_Via.updateDb(via1);

  std::cout<<"\n-------after update records:---------\n";

  via_vec.clear();
  db_Via.selectFromDb(&via_vec);
  for(auto i : via_vec) i.print();

  via1->_type = IdbVia::ViaType::kVia1; // 不能用int赋值，赋值一个大数不允许
  via1->_normal_enum = kEnum2; // 不能用int赋值，赋值一个大数不允许
  std::cout<<"via1->_normal_enum = "<<via1->_normal_enum<<"\n"; // 1
  db_Via.updateDb(via1);

  std::cout<<"\n-------enum test --- after update records:---------\n";

  via_vec.clear();
  db_Via.selectFromDb(&via_vec);
  for(auto i : via_vec) i.print();

  std::cout<<"\n-----------delete records-------------\n";
  db_Port.deleteFromDbAll(port1);

  via_vec.clear();
  db_Via.selectFromDb(&via_vec);
  for(auto i : via_vec) i.print();

  std::cout<<"\n-----------select test-------------\n";
  port_results.clear();
  db_Port.selectFromDbAll(&port_results);

  for (auto& port : port_results) {
    port.print();
  }

  return 0;
}

