/**
 * @file vec.cpp
 * @brief vec.cpp provides a way to test the nested class and vector class.
*/

#include "edadb.h"

struct IdbLevel {
public:
    int _level = 10;
    std::string _name = "level_name";

public:
    ~IdbLevel(void) = default;
    IdbLevel (void) = default;
    IdbLevel (int l, std::string n) : _level(l), _name(n) {}
    IdbLevel (const IdbLevel& obj) : _level(obj._level), _name(obj._name) {}    
public:
    void clear(void) { _level = 0; _name.clear(); }
    void print(const std::string& pref = "") const {
        std::cout << pref << "[IdbLevel]: _level=" << _level << ", _name=\"" << _name << "\"" << std::endl;
    }
};



struct IdbRect {
public:
    int _x;
    IdbLevel _level; // nested class

public:
    ~IdbRect(void) = default;
    IdbRect (void) = default;
    IdbRect (int x) : _x(x) {}
    IdbRect (const IdbRect& obj) : _x(obj._x) {}

public:
    void clear(void) { _x = 0; }

    void print(const std::string& pref = "") const {
        std::cout << pref << "[IdbRect]: _x=" << _x << std::endl;
        _level.print(pref + "  ");
    }
};


struct IdbRect2 {
public:
    int _y;

public:
    ~IdbRect2(void) = default;
    IdbRect2 (void) = default;
    IdbRect2 (int y) : _y(y) {}
    IdbRect2 (const IdbRect2& obj) : _y(obj._y) {}

public:
    void clear(void) { _y = 0; }

    void print(const std::string& pref = "") const {
        std::cout << pref << "[IdbRect2]: _y=" << _y << std::endl;
    }
};


class IdbLayerShape {
public:
    std::string _name;      
    std::string *_layer;     
    std::vector<IdbRect> _rects; 
    std::vector<IdbRect2> _rect2s; 

public:
    ~IdbLayerShape(void) = default;
    IdbLayerShape(void) = default;
    IdbLayerShape(std::string n, std::string* l) : _name(n), _layer(l) {}
    IdbLayerShape(const IdbLayerShape& obj) = default;

public:
    void clear(void) {
        _name.clear();
        _layer->clear();
        _rects.clear();
        _rect2s.clear();
    } // clear

    void print(const std::string& pref = "") const {
        std::string vec_pref = pref + "  ";
        std::cout << pref << "[LayerShape] Begin ----------------------------------------" << std::endl;
        std::cout << pref << "_name=\"" << _name << "\"" << std::endl;
        std::cout << pref << "_layer=\"" << *_layer << "\"" << std::endl;
        std::cout << pref << "_rects.size =" << _rects.size() << std::endl;
        for (auto& r : _rects) {
            r.print(vec_pref);
        }
        std::cout << pref << "_rect2s.size =" << _rect2s.size() << std::endl;
        for (auto& r : _rect2s) {
            r.print(vec_pref);
        }
        std::cout << pref << "[LayerShape] End ----------------------------------------" << std::endl;
    } // print
}; // IdbLayerShape


class IdbPort {
public:
    std::string _name; // port name
    std::vector<IdbLayerShape> _layer_shapes; // vector of layer shapes
    std::vector<IdbRect> _rects; // vector of rectangles

public:
    ~IdbPort(void) = default;
    IdbPort(void) = default;
    IdbPort(std::string n) : _name(n) {}
    IdbPort(const IdbPort& obj) = default;

public:
    void clear(void) {
        _name.clear();
        _layer_shapes.clear();
        _rects.clear();
    } // clear

    void print(const std::string& pref = "") const {
        std::string vec_pref = pref + "  ";
        std::cout << pref << "[Port] Begin ----------------------------------------" << std::endl;
        std::cout << pref << "_name=\"" << _name << "\"" << std::endl;
        std::cout << pref << "_layer_shapes.size=" << _layer_shapes.size() << std::endl;
        for (auto& ls : _layer_shapes) {
            ls.print(vec_pref);
        }
        std::cout << pref << "_rects.size=" << _rects.size() << std::endl;
        for (auto& r : _rects) {
            r.print(vec_pref);
        }
        std::cout << pref << "[Port] End ----------------------------------------" << std::endl << std::endl;
    } // print
}; // IdbPort

TABLE4CLASS(IdbLevel, "lvl_tab", (_level, _name))

TABLE4CLASS_WITH_PKEY(IdbRect, "rect_table", (_x), (_level))
TABLE4CLASS(IdbRect2, "rect2_table", (_y))

//TABLE4CLASS_WVEC(IdbLayerShape, "layer_shape_table", (_name, _layer), (_rects, _rect2s))
//TABLE4CLASS_WVEC(IdbPort, "port_table", (_name), (_layer_shapes, _rects))



// utlity functions
template<typename T>
int scanTable(edadb::DbMap<T>& dbm) {
    edadb::DbMapReader<T> *rd = nullptr;
    T got;
    while (edadb::read2Scan<T>(rd, dbm, &got) > 0) {
        std::cout << "scanTable<" << typeid(T).name() << "> :  " << std::endl;
        got.print();
        got.clear();
    } 
    assert(rd == nullptr);
    return 0;
} // scanTable



int main(void) {
    // Create Object ////////////////////////////////////////////
    std::string layer_name = "Metal1"; 
    IdbLayerShape shape1("IdbLayerShape 1", &layer_name);
    shape1._rects.emplace_back(11);
    shape1._rect2s.emplace_back(12);

    IdbPort port1("Port 1");
    port1._layer_shapes.push_back(shape1);
    port1._rects.emplace_back(111);

    std::string layer_name2 = "Metal2";
    IdbLayerShape shape2("IdbLayerShape 2", &layer_name2);
    shape2._rects.emplace_back(21);
    shape2._rect2s.emplace_back(22);

    IdbPort port2("Port 2");
//    port2._layer_shapes.push_back(shape1); // TODO: UNIQUE constraint violation
    port2._layer_shapes.push_back(shape2);
    port2._rects.emplace_back(222);

//    if (0) {
//        // Check Object ////////////////////////////////////////////
//        std::cout << "================ Demo Vector Init ================" << std::endl;
//        port1.print();
//        port2.print();
//
//        edadb::VecMetaDataPrinter<IdbLayerShape> printer;
//        printer.printStatic();
//    } // if 


    // init database 
    std::string conn_param = "vec.db";
    std::cout << "[DbMap Init]" << std::endl;
    if (!edadb::initDatabase(conn_param)) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

#if 1
    // IdbRect ////////////////////////////////////////////
    {
        IdbRect rect1(1); rect1._level._level = 100; rect1._level._name = "rect1_level"; 
        IdbRect rect2(2); rect2._level._level = 200; rect2._level._name = "rect2_level";

        std::cout << "DbMap<" << typeid(IdbRect).name() << ">" << std::endl;
        edadb::DbMap<IdbRect> dbm_rect;

        // Create table
        std::cout << "[DbMap CreateTable]" << std::endl;
        if (!edadb::createTable(dbm_rect)) {
            std::cerr << "DbMap::createTable failed" << std::endl;
            return 1; 
        }

        // Insert objects
        std::cout << "[DbMap Insert]" << std::endl;
        if (!edadb::insertObject(dbm_rect, &rect1)) {
            std::cerr << "DbMap::Writer::insert failed" << std::endl;
            return 1;
        }
        if (!edadb::insertObject(dbm_rect, &rect2)) {
            std::cerr << "DbMap::Writer::insert failed" << std::endl;
            return 1;
        }
        std::cout << std::endl << std::endl;

        // Scan table
        std::cout << "[DbMap Scan]" << std::endl;
        scanTable(dbm_rect);
        std::cout << std::endl << std::endl;
    }

#endif 


#if 0
    // IdbLayerShape ////////////////////////////////////////////

    // Define DbMap instance to operate the database table
    std::cout << "DbMap<" << typeid(IdbLayerShape).name() << ">" << std::endl;
    edadb::DbMap<IdbLayerShape> dbm_layer_shape;

    // Create table
    std::cout << "[DbMap CreateTable]" << std::endl;
    if (!edadb::createTable(dbm_layer_shape)) {
        std::cerr << "DbMap::createTable failed" << std::endl;
        return 1;
    }

    // Insert objects
    std::cout << "[DbMap Insert]" << std::endl;
    if (!edadb::insertObject(dbm_layer_shape, &shape1)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;
    std::vector<IdbLayerShape*> layer_shape_vec;
    layer_shape_vec.push_back(&shape2);
    if (!edadb::insertVector(dbm_layer_shape, layer_shape_vec)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_layer_shape);
    std::cout << std::endl << std::endl;

    // Update objects
    std::string new_layer_name = "Metal1 new";
    std::cout << "[DbMap Update]" << std::endl;
    shape1._layer = &new_layer_name;
    shape1._rects.at(0)._x = 13;
    shape1._rect2s.emplace_back(14);
    if (!edadb::updateObject(dbm_layer_shape, &shape1)) {
        std::cerr << "DbMap::update failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_layer_shape);
    std::cout << std::endl << std::endl;

    // delete objects
    std::cout << "[DbMap Delete]" << std::endl;
    if (!edadb::deleteObject(dbm_layer_shape, &shape2)) {
        std::cerr << "DbMap::delete failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_layer_shape);
    std::cout << std::endl << std::endl;
#endif


#if 0
    // IdbPort ////////////////////////////////////////////

    // Define DbMap instance to operate the database table
    std::cout << "DbMap<" << typeid(IdbPort).name() << ">" << std::endl;
    edadb::DbMap<IdbPort> dbm_port;

    // Create table
    std::cout << "[DbMap CreateTable]" << std::endl;
    if (!edadb::createTable(dbm_port)) {
        std::cerr << "DbMap::createTable failed" << std::endl;
        return 1;
    }

    // Insert objects
    std::cout << "[DbMap Insert]" << std::endl;
    if (!edadb::insertObject(dbm_port, &port1)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;
    std::vector<IdbPort*> port_vec;
    port_vec.push_back(&port2);
    if (!edadb::insertVector(dbm_port, port_vec)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_port);
    std::cout << std::endl << std::endl;

    // update objects
    std::cout << "[DbMap Update]" << std::endl;
    port1._name = "Port 1";

    std::string new_layer_name = "Metal1 new";
    port1._layer_shapes.at(0)._layer = &new_layer_name;
    port1._rects.at(0)._x = 113;
    port1._layer_shapes.at(0)._rects.at(0)._x = 113;
    port1._layer_shapes.at(0)._rect2s.emplace_back(114);
    edadb::updateObject(dbm_port, &port1);
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_port);
    std::cout << std::endl << std::endl;

    // delete objects
    std::cout << "[DbMap Delete]" << std::endl;
    if (!edadb::deleteObject(dbm_port, &port2)) {
        std::cerr << "DbMap::delete failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // Scan table
    std::cout << "[DbMap Scan]" << std::endl;
    scanTable(dbm_port);
    std::cout << std::endl << std::endl;
#endif

    return 0;
} // main