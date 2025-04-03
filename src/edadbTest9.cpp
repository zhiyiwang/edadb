// #include "lib/edadb/edadbTestMacro.hpp"
#include "lib/edadb/edadbTest9.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
/*
* DbMapAll全部写在宏中的版本 新的class定义
*/

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<int32_t> point_t;
typedef bg::model::polygon<point_t> polygon_t;

struct IdbCoordinate {
  polygon_t poly;
  int32_t x;
  int32_t y;
  IdbCoordinate(int32_t _x = 0, int32_t _y = 0) : x(_x), y(_y) {}
};

#include <istream>
#include <ostream>
#include <streambuf>

class DbIstream : public std::istream {
  DbIstream(std::istream& is) : std::istream(is.rdbuf()) {}
  
};
/* Table4ExternalClass (myclass, tablename, writeFunc, readFunc, isBinary)
  namespace edadb{\
  template<>\
  struct CppTypeToDbType<myclass>{\
      static const DbTypes ret = DbTypes::kPrivate;\
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
TABLE4CLASS(IdbCoordinate, "coordinate_table", (x, y));
TABLE4CLASS(IdbLayerShape, "layer_shape_table", (_name, _layer));
TABLE4CLASS(IdbVia, "via_table", (_name, _coord));

// TABLE4CLASS( IdbSites, "", (name) );
// TABLE2TABLE_1_N_VEC( IdbSites, (idbsite_array, array2_array) );

TABLE4CLASSWVEC(IdbPort, "port_table", (_name), (_layer_shapes, _vias));


int main() {

  edadb::dbgPrint();

  IdbLayerShape* shape1 = new IdbLayerShape("METAL1", "M1");
  shape1->_rects.emplace_back(0, 0, 100, 100);  
  IdbLayerShape* shape2 = new IdbLayerShape("METAL2", "M2");
  shape2->_rects.emplace_back(200, 200, 300, 300);

  IdbVia* via1 = new IdbVia("VIA1", 50, 50);
  IdbVia* via2 = new IdbVia("VIA2", 250, 250);

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
  db_Port.connectToDb("db=sqlite.db foreign_keys=1");
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
    for (auto& shape : port._layer_shapes) {
      std::cout << "  "; shape.print();
    }
    for (auto& via : port._vias) {
      std::cout << "  "; via.print(); 
    }
  }

  std::cout<<"\n-----------update records-------------\n";
  via1->_coord.x = 100; 
  via1->_coord.y = 100;
  db_Via.updateDb(via1);

  std::vector<IdbVia> via_vec;
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

