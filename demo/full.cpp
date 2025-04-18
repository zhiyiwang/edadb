#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>


#include "edadb.h"

/**
 * @brief IdbCoordinate: This is a class to represent a coordinate.
 * @details The class contains two integer coordinates x and y.
 */
struct IdbCoordinate {
public:
    int32_t _x;
    int32_t _y;

public:
    IdbCoordinate (int32_t x = 0, int32_t y = 0) : _x(x), _y(y) {}
    IdbCoordinate (const IdbCoordinate& coord) : _x(coord._x), _y(coord._y) {}
    ~IdbCoordinate() = default;

public:
    void print(const std::string& pref = "") const {
        std::cout << pref << "Coordinate(_x=" << _x << ", _y=" << _y << ")" << std::endl;
    }
};


/**
 * @brief IdbRect: This is a class to represent a rectangle.
 * @details The class contains two coordinates: lower left and upper right.
 */
struct IdbRect {
public:
    IdbCoordinate _ll; // lower left coordinate 
    IdbCoordinate _ur; // upper right coordinate

public:
    IdbRect(int32_t llx = 0, int32_t lly = 0, int32_t urx = 0, int32_t ury = 0) 
        : _ll(llx, lly), _ur(urx, ury) {}

    IdbRect(const IdbCoordinate& ll, const IdbCoordinate& ur) : _ll(ll), _ur(ur) {}
    IdbRect(const IdbRect& rect) : _ll(rect._ll), _ur(rect._ur) {}

    ~IdbRect() = default;

public:
    void print(const std::string& pref = "") {
        std::cout << pref << "Rectangle" << std::endl;
        std::string next_pref = pref + "  ";
        _ll.print(next_pref);
        _ur.print(next_pref);
    }
};


/**
 * @brief IdbLayerShape: This is a class to represent a layer shape.
 * @details The class contains a name, a layer, and a vector of rectangles.
 */
class IdbLayerShape {
public:
    std::string _name;  // layer shape name
    std::string _layer; // layer string  
    std::vector<IdbRect> _rects; // vector of rectangles

public:
    IdbLayerShape() = default;
    IdbLayerShape(std::string n, std::string l) : _name(n), _layer(l) {}

public:
    void print(const std::string& pref = "") {
        std::cout << pref << "LayerShape" << std::endl;
        std::string next_perf = pref + "  ";
        std::cout << next_perf << "name=" << _name << std::endl;
        std::cout << next_perf << "layer=" << _layer << std::endl;
        std::cout << next_perf << "rects.size=" << _rects.size() << std::endl;
        for (auto& r : _rects) {
            std::cout << next_perf << "rect" << std::endl;
            r.print(next_perf);
        }
    } // print
};


/**
 * @brief IdbVia: This is a class to represent a via.
 * @details The class contains a name, a coordinate, a type, and an enum.
 */
class IdbVia {
public:
    std::string _name;
    IdbCoordinate _coord;

public:
    IdbVia() = default;
    IdbVia(std::string n, int32_t x, int32_t y) : _name(n), _coord(x, y) {}

public:
    void print(const std::string& pref = "") {
        std::cout << pref << "Via" << std::endl;
        std::string next_pref = pref + "  ";
        std::cout << next_pref << "name=" << _name << std::endl;
        std::cout << next_pref << "coord" << std::endl;
        _coord.print(next_pref);
    }
};


/**
 * @brief IdbPort: This is a class to represent a port.
 * @details The class contains a name, a vector of layer shapes, and a vector of vias.
 */
class IdbPort {
public:
    std::string _name; // port name
    std::vector<IdbLayerShape> _layer_shapes; // vector of layer shapes
    std::vector<IdbVia> _vias; // vector of vias

public:
    IdbPort() = default;
    IdbPort(std::string n) : _name(n) {}

    void print(const std::string& pref = "") {
        std::cout << "Port" << std::endl;
        std::string next_pref = pref + "  ";
        std::cout << next_pref << "name=" << _name << std::endl;
        std::cout << next_pref << "layer_shapes.size=" << _layer_shapes.size() << std::endl;
        std::cout << next_pref << "vias.size=" << _vias.size() << std::endl;
        for (auto& ls : _layer_shapes)
            ls.print(next_pref);
        for (auto& v : _vias)
            v.print(next_pref); 
    }
};



// define table 4 class, alias of edadb::Table4Class
_EDADB_DEFINE_TABLE_BY_CLASS_(IdbCoordinate, "coordinate_table", (_x, _y));
_EDADB_DEFINE_TABLE_BY_CLASS_(IdbLayerShape, "layer_shape_table", (_name, _layer));
_EDADB_DEFINE_TABLE_BY_CLASS_(IdbVia, "via_table", (_name, _coord));



// utlity functions
template<typename T>
int scanTable() {
    edadb::DbMapReader<T> *rd = nullptr;
    IdbCoordinate got;
    std::string pred = "";
    while (edadb::readByPredicate<T>(rd, &got, pred) > 0) {
        std::cout << "scanTable<" << typeid(T).name() << "> :  ";
        got.print();
    } 
    assert(rd == nullptr);
    return 0;
} // scanTable



// demo test for IdbCoordinate, a simple class
int testIdbCoordinate(const std::string& conn_param) {
    std::cout << "DBMap<" << typeid(IdbCoordinate).name() << ">" << std::endl;

    // init database and create table
    std::cout << "[DbMap Init]" << std::endl;
    if (!edadb::initDatabase<IdbCoordinate>(conn_param)) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }
    if (edadb::executeSql<IdbCoordinate>("PRAGMA foreign_keys = ON;") == false) {
        std::cerr << "DbMap::executeSql failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    std::cout << "[DbMap CreateTable]" << std::endl;
    if (!edadb::createTable<IdbCoordinate>()) {
        std::cerr << "DbMap::createTable failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;


    // insert objects 
    std::cout << "[DbMap Insert]" << std::endl;
    IdbCoordinate c1(1,1), c2(2,2), c3(3,3), c4(4,4);
    std::vector<IdbCoordinate*> coord_vec;
    coord_vec.push_back(&c1); c1.print();
    coord_vec.push_back(&c2); c2.print();
    coord_vec.push_back(&c3); c3.print();
    coord_vec.push_back(&c4); c4.print();
    if (!edadb::insertVector<IdbCoordinate>(coord_vec)) {
        std::cerr << "DbMap::insert failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // read variables
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable<IdbCoordinate>();
    std::cout << std::endl << std::endl;


    // update objects
    std::cout << "[DbMap Update]" << std::endl;
    IdbCoordinate c3_new (30, 30);
    if (!edadb::updateObject<IdbCoordinate>(&c3, &c3_new)) {
        std::cerr << "DbMap::update failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    std::cout << "[DbMap Scan]" << std::endl;
    scanTable<IdbCoordinate>();
    std::cout << std::endl << std::endl;


    // delete objects
    std::cout << "[DbMap Delete]" << std::endl;
    if (!edadb::deleteObject<IdbCoordinate>(&c4)) {
        std::cerr << "DbMap::delete failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    std::cout << "[DbMap Scan]" << std::endl;
    scanTable<IdbCoordinate>();
    std::cout << std::endl << std::endl;

    return 1;
} // testIdbCoordinate


// demo test for IdbVia, a complex class 
int testIdbVia(const std::string& conn_param) {
    std::cout << "DBMap<" << typeid(IdbVia).name() << ">" << std::endl;

    // init database and create table
    std::cout << "[DbMap Init]" << std::endl;
    if (!edadb::initDatabase<IdbVia>(conn_param)) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }
    if (edadb::executeSql<IdbVia>("PRAGMA foreign_keys = ON;") == false) {
        std::cerr << "DbMap::executeSql failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    std::cout << "[DbMap CreateTable]" << std::endl;
    if (!edadb::createTable<IdbVia>()) {
        std::cerr << "DbMap::createTable failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;


    // insert objects
    std::cout << "[DbMap Insert]" << std::endl;
    IdbVia v1("via1", 10, 11), v2("via2", 20, 21), v3("via3", 30, 31), v4("via4", 40, 41);
    edadb::SqlStatement<IdbVia> sql_stmt;
    sql_stmt.print(&v1, &v2);


//    std::vector<IdbVia*> via_vec;
//    via_vec.push_back(&v1); v1.print();
//    via_vec.push_back(&v2); v2.print();
//    via_vec.push_back(&v3); v3.print();
//    via_vec.push_back(&v4); v4.print();
//    if (!edadb::insertVector<IdbVia>(via_vec)) {
//        std::cerr << "DbMap::insert failed" << std::endl;
//        return 1;
//    }

    return 0;
}


int main () {
    const std::string conn_param = "full.db";
//    testIdbCoordinate(conn_param);
    testIdbVia(conn_param);

    return 0;
} // main