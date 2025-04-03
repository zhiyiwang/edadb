// #include "lib/edadb/edadbTestMacro.hpp"
#include "lib/edadb/edadbTest11.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
/*
* 外部类 序列化函数
*/

class ExternalClass1{
  private:
    int pra_field_ex1;
  public:
    int get() const { return pra_field_ex1; }
    void set(int val) { pra_field_ex1 = val; }
};

class ExternalClass2{
  private:
    int pra_field_ex2;
  public:
    int get() const { return pra_field_ex2; }
    void set(int val) { pra_field_ex2 = val; }
};

// 序列化函数
bool writeFuncType1(ExternalClass1* obj, edadb::DbOstream* so) {
  int val = obj->get();
  so->write(reinterpret_cast<const char*>(&val), sizeof(val));
  return so->good(); // 写入过程出问题会返回false
}

bool writeFuncType2(ExternalClass2* obj, edadb::DbOstream* so) {
  int val = obj->get();
  so->write(reinterpret_cast<const char*>(&val), sizeof(val));
  return so->good();
}

// 反序列化函数
bool readFuncType1(ExternalClass1* obj, edadb::DbIstream* si) {
  int val;
  si->read(reinterpret_cast<char*>(&val), sizeof(val));

  if (si->gcount() != sizeof(val)) {
      return false; // 读取长度不匹配
  }

  obj->set(val);
  return true;
}

bool readFuncType2(ExternalClass2* obj, edadb::DbIstream* si) {
  int val;
  si->read(reinterpret_cast<char*>(&val), sizeof(val));

  if (si->gcount() != sizeof(val)) {
      return false; // 读取长度不匹配
  }

  obj->set(val);
  return true;
}

struct IdbCoordinate {
  public:
    ExternalClass1 ex1;
    ExternalClass2 ex2;
  public:
    int32_t x;
    int32_t y;
    IdbCoordinate(int32_t _x = 0, int32_t _y = 0) : x(_x), y(_y) {}

    void print() const {
      std::cout << "Coordinate(x=" << x 
                << ", y=" << y 
                << ", ex1=" << ex1.get()  
                << ", ex2=" << ex2.get()   
                << ")\n";
    }
};

// Table4ExternalClass (myclass, tablename, writeFunc, readFunc, isBinary)
/* Table4ExternalClass (ExternalClass, writeFunc, readFunc, isBinary)
  namespace edadb{\
  template<>\
  struct CppTypeToDbType<myclass>{\
      static const DbTypes ret = DbTypes::kExternalB;\
  };\



  create在这个后面
      if (ret == DbTypes::kComposite) // 尽量减少字符串比较
        // if (type == "__COMPOSITE__") // CppTypeToDbType<O>::ret 不能直接判等
        {
            // sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_");
            std::vector<std::string> sub_member_names;
            sql += ", " + edadb::createTableStrSubObj<ObjType>(x.second + "_", sub_member_names);
            member_names.insert(member_names.end(), sub_member_names.begin(), sub_member_names.end());
        }
        else {
            SqlString<T>::member_names.push_back(x.second);
            sql += ", " + x.second + " " + type;
        }
    有
    else if((ret == DbTypes::kExternalB){
            SqlString<T>::member_names.push_back(x.second);
            sql += ", " + x.second + " " + type;
    }

    其实也不用怎么变，大概需要改type为BLOB
    using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                std::string type = edadb::cppTypeToDbTypeString<typename edadb::ConvertCPPTypeToSOCISupportType<ObjType>::type>();

    insert fill那里改一下？
*/


/*
写readfunc从串恢复 writefunc得到串 对于geometry的例子，如何  写demo 想如何结合到edadb
*/
/* Table4ExternalClass (myclass, writeFunc, readFunc, isBinary)
  namespace edadb{\
  template<>\
  struct CppTypeToDbType<myclass>{\
      static const DbTypes ret = DbTypes::kExternal;\
  };\

*/
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
                  << ", rects=" << _rects.size() << ")\n";
    }
};


/// @class IdbVia  // example 
class IdbVia {
  public:
    std::string _name;
    IdbCoordinate _coord;

  public:
    IdbVia() = default;
    IdbVia(std::string n, int32_t x, int32_t y) 
        : _name(n), _coord(x, y) {}

    void print() const {
        std::cout << "Via(name=" << _name 
                  << ", x=" << _coord.x 
                  << ", y=" << _coord.y << ")\n";
    }
};

/// @class IdbPort  // example 
class IdbPort {
  public:
    std::string _name;
    std::vector<IdbLayerShape> _layer_shapes; // 替换idbsite_array
    std::vector<IdbVia> _vias;                // 替换array2_array

public:
    IdbPort() = default;
    IdbPort(std::string n) : _name(n) {}

    void print() const {
        std::cout << "Port(name=" << _name << ")\n";
        for (auto& s : _layer_shapes) s.print();
        for (auto& v : _vias) v.print();
    }
};

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
// TABLE4CLASS( SubClass, "",(sid,data1,data2));
// TABLE4CLASS( IdbSite, "table_name", (name, width, height, subobj) );
// TABLE4CLASS(IdbCoordinate, "coordinate_table", (x, y, ex));

// TABLE4CLASS(IdbCoordinate, "coordinate_table", (x, y));
// #define TABLE4CLASSWEXTERNAL(myclass, tablename, CLASS_ELEMS_TUP, EXTERNAL_TUP)
Table4ExternalClass(ExternalClass1, writeFuncType1, readFuncType1);
Table4ExternalClass(ExternalClass2, writeFuncType2, readFuncType2);
TABLE4CLASSWEXTERNAL(IdbCoordinate, "coordinate_table", (x, y), (ex1,ex2));
TABLE4CLASS(IdbLayerShape, "layer_shape_table", (_name, _layer));
TABLE4CLASS(IdbVia, "via_table", (_name, _coord));

// TABLE4CLASS( IdbSites, "", (name) );
// TABLE2TABLE_1_N_VEC( IdbSites, (idbsite_array, array2_array) );

TABLE4CLASSWVEC(IdbPort, "port_table", (_name), (_layer_shapes, _vias));


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
  db_Coordinate.connectToDb("sqlite.db");
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

  std::cout<<"\n-----------delete records-------------\n";
  db_Coordinate.deleteFromDb(coord4); // 不变

  std::cout<<"\n-----------select test-------------\n";
  std::vector<IdbCoordinate> coord_results;
  db_Coordinate.selectFromDb(&coord_results);

  for(auto& coord : coord_results) {
    coord.print();
  }

  // IdbLayerShape* shape1 = new IdbLayerShape("METAL1", "M1");
  // shape1->_rects.emplace_back(0, 0, 100, 100);  
  // IdbLayerShape* shape2 = new IdbLayerShape("METAL2", "M2");
  // shape2->_rects.emplace_back(200, 200, 300, 300);

  // IdbVia* via1 = new IdbVia("VIA1", 50, 50);
  // IdbVia* via2 = new IdbVia("VIA2", 250, 250);

  // IdbPort* port1 = new IdbPort("Port1");
  // port1->_layer_shapes.push_back(*shape1);
  // port1->_vias.push_back(*via1);

  // IdbPort* port2 = new IdbPort("Port2");
  // port2->_layer_shapes.push_back(*shape2);
  // port2->_vias.push_back(*via2);

  // edadb::DbMap<IdbLayerShape> db_LayerShape;
  // edadb::DbMap<IdbVia> db_Via;
  // edadb::DbMapAll<IdbPort> db_Port;

  // db_Port.connectToDb("sqlite.db");
  // db_LayerShape.createTable("layer_shape_table");
  // db_Via.createTable("via_table");
  // db_Port.createTableAll("port_table");

  // std::cout<<"\n-----------insert records-------------\n";

  // db_Via.insertToDb(via1);
  // db_Via.insertToDb(via2);

  // db_Port.insertToDbAll(port1); // 前提是已经createTableAll
  // db_Port.insertToDbAll(port2); // 插入port2及其关联的shape2和via2

  // std::vector<IdbPort> port_results;
  // db_Port.selectFromDbAll(&port_results);
  
  // for (auto& port : port_results) {
  //   port.print();
  //   for (auto& shape : port._layer_shapes) {
  //     std::cout << "  "; shape.print();
  //   }
  //   for (auto& via : port._vias) {
  //     std::cout << "  "; via.print(); 
  //   }
  // }

  // std::cout<<"\n-----------update records-------------\n";
  // via1->_coord.x = 100; 
  // via1->_coord.y = 100;
  // db_Via.updateDb(via1);

  // std::vector<IdbVia> via_vec;
  // db_Via.selectFromDb(&via_vec);
  // for(auto i : via_vec) i.print();

  // std::cout<<"\n-----------delete records-------------\n";
  // db_Port.deleteFromDbAll(port1);

  // via_vec.clear();
  // db_Via.selectFromDb(&via_vec);
  // for(auto i : via_vec) i.print();

  // std::cout<<"\n-----------select test-------------\n";
  // port_results.clear();
  // db_Port.selectFromDbAll(&port_results);

  // for (auto& port : port_results) {
  //   port.print();
  // }

  return 0;
}

