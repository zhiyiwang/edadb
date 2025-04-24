
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
  
  /*表名：port_table(允许自定义)
  列：_name
  primary key: _name
  */
  /*表名：port_table__layer_shapes      (port_table + '_' + _layer_shapes)
  列：_name, _layer ，port_table__name
  primary key: _name
  foreign key: port_table__name references port_table(_name)
  */
  /*表名：port_table_port_rects      (port_table + '_' + port_rects)
  列：x , port_table__name
  primary key: x
  foreign key: port_table__name references port_table(_name)
  */
  /*表名：port_table__layer_shapes__rects      (port_table__layer_shapes + '_' + _rects)
  列：x , port_table__layer_shapes__name
  primary key: x
  foreign key: port_table__layer_shapes__name references port_table__layer_shapes(_name)
  */
  /*表名：port_table__layer_shapes__rect2s      (port_table__layer_shapes + '_' + _rect2s)
  列：y , port_table__layer_shapes__name
  primary key: y
  foreign key: port_table__layer_shapes__name references port_table__layer_shapes(_name)
  */