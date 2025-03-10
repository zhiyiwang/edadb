#include "lib/edadb/edadbTest6.hpp" // 6
/*
* 先改一下  class定义
*/

struct IdbCoordinate {
  int32_t x;
  int32_t y;
  IdbCoordinate(int32_t _x = 0, int32_t _y = 0) : x(_x), y(_y) {}
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

  db_Port.connectToDb("sqlite.db");
  db_LayerShape.createTable("layer_shape_table");
  db_Via.createTable("via_table");
  db_Port.createTable("port_table");


  // // edadb::DbMapT2T<IdbSites,IdbSite> db_v; // v for vector 必须体现数组变量名 才能区分多个IdbSite
  // db.connectToDb("sqlite.db");
  // db_IdbSite.createTable("IdbSite_table");
  // db.createTable("IdbSites_table");


  // std::cout<<"\n-----------insert 5 IdbSite records-------------\n";

  // db_IdbSite.insertToDb(p1);
  // db_IdbSite.insertToDb(p2);
  // db_IdbSite.insertToDb(p3);
  // db_IdbSite.insertToDb(&p4);
  // db_IdbSite.insertToDb(&p5);

  // db.insertToDb(p6);
  // db.insertToDb(p7);
  // db.insertToDb(p8);
  // db.insertToDb(&p9);
  // db.insertToDb(&p10);

  // db.createTableT2TALL(p6);
  // db.createTableT2TALL(p7);

  // db.insertArrayEleToDb(p6,p1,"idbsite_array");
  // db.insertArrayEleToDb(p6,p2,"idbsite_array");
  // db.insertArrayEleToDb(p6,p3,"idbsite_array");
  

  // std::cout<<"\n-----------insertall test-------------\n";
  // p7->idbsite_array.push_back(IdbSite("SiteA", 100, 110));
  // p7->idbsite_array.push_back(IdbSite("SiteB", 200, 210));
  // p7->array2_array.push_back(Array2("ArrayA", 300, 310));
  // p7->array2_array.push_back(Array2("ArrayB", 400, 410));

  // db.insertToDbAll(p7);

  // std::cout<<"\n-----------insertall test end-------------\n";

  // std::vector<IdbSite> vec;
  // db_IdbSite.selectFromDb(&vec);
  // for(auto i : vec) i.print();

  // // // std::cout<<"\n-----------update Site1, delete Site2-------------\n";

  // p1->height = 150;
  // p6->name = "Sites1_updated";
  // db_IdbSite.updateDb(p1);

  // db_IdbSite.deleteFromDb(p2);

  // db.updateDb(p6);
  // // db.deleteFromDb(p7); //cascade delete
  // // db.deleteFromArrDb(p6, p1, "idbsite_array");
  // db.updateArrDb(p6, p1, "idbsite_array");
  // db.updateDbAll(p6);

  // std::cout << "\n-----------test selectFromArrDb-------------\n";
  //   db.selectFromArrDb(&vec, "IdbSites_name = 'Sites1'", "idbsite_array");
  //   for (auto& site : vec) {
  //       site.print();
  //   }

  //   std::cout << "\n-----------test selectFromDbAll-------------\n";

  //   db.insertToDbAll(p6); // 
  //   std::vector<IdbSites> result;
  //   db.selectFromDbAll(&result, "name = 'Sites1'"); // 这里不是 IdbSites_name = 'Sites1'
    // for (auto& site : result) {
    //     site.print(); 
    //     for (auto& sub_site : site.idbsite_array) {
    //         sub_site.print(); 
    //     }
    //     for (auto& sub_array : site.array2_array) {
    //         sub_array.print(); 
    //     }
    // }

    

  // vec.clear();
  // db.selectFromDb(&vec);
  // for(auto i : vec) i.print();

  // std::cout<<"\n-----------choose Sites with width < 350--------------\n";

  // vec.clear(); 
  // db2.selectFromDb(&vec, "width < 350"); // 嵌套中select有问题 解决
  // for(auto i : vec) i.print();

  
  return 0;
}

