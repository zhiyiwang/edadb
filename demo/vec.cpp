/**
 * @file vec.cpp
 * @brief vec.cpp provides a way to test the nested class and vector class.
*/

#include "edadb.h"

struct IdbRect {
public:
    int _x;

public:
    ~IdbRect(void) = default;
    IdbRect (void) = default;
    IdbRect (int x) : _x(x) {}
    IdbRect (const IdbRect& obj) : _x(obj._x) {}

public:
    void print(const std::string& pref = "") const {
        std::cout << pref << "[IdbRect]: _x=" << _x << std::endl;
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
    void print(const std::string& pref = "") const {
        std::cout << pref << "[IdbRect2]: _y=" << _y << std::endl;
    }
};


class IdbLayerShape {
public:
    std::string _name;      
    std::string _layer;     
    std::vector<IdbRect> _rects; 
    std::vector<IdbRect2> _rect2s; 

public:
    ~IdbLayerShape(void) = default;
    IdbLayerShape(void) = default;
    IdbLayerShape(std::string n, std::string l) : _name(n), _layer(l) {}
    IdbLayerShape(const IdbLayerShape& obj) = default;

public:
    void print(const std::string& pref = "") const {
        std::string vec_pref = pref + "  ";
        std::cout << pref << "[LayerShape] Begin ----------------------------------------" << std::endl;
        std::cout << pref << "_name=\"" << _name << "\"" << std::endl;
        std::cout << pref << "_layer=\"" << _layer << "\"" << std::endl;
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


_EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_(IdbLayerShape, "layer_shape_table", (_name, _layer), (_rects, _rect2s))
//_EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_(IdbPort, "port_table", (_name), (_layer_shapes, _rects))


int main(void) {
    IdbLayerShape shape1("IdbLayerShape 1", "Type 1"), shape2("IdbLayerShape 2", "Type 2");
    shape1._rects.emplace_back(11); shape1._rect2s.emplace_back(12);
    shape2._rects.emplace_back(21); shape2._rect2s.emplace_back(22);

    IdbPort port1("Port 1"), port2("Port 2");
    port1._layer_shapes.push_back(shape1); port1._rects.emplace_back(111);
    port2._layer_shapes.push_back(shape2); port2._rects.emplace_back(222);

    // print objects
    std::cout << "================ Demo Vector Init ================" << std::endl;
    port1.print();
    port2.print();

    edadb::VecMetaDataPrinter<IdbLayerShape> printer;
    printer.printStatic();

    // TODO:
    // 1. create table
    // 2. insert objects
    // 3. select objects
    // 4. delete objects
    // 5. update objects

    return 0;
} // main