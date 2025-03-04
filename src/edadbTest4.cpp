#include "lib/edadb/edadbTest4.hpp"
/*
* 成员是vector createTable insertToDb第一版
*/

/// @class SubClass // example 
class SubClass { // IdbSites 的子类是IdbSite不如不改 vector<IdbSite*>
public:
  int sid;
  double data1;
  double data2;
  
  SubClass() : sid(123), data1(4.56), data2(7.89) {}

  SubClass(int s, double d1, double d2) : sid(s), data1(d1), data2(d2) {}

  void print() {
    std::cout << "SubClass(sid=" << sid << ", data1=" << data1 << ", data2="<< data2<< ")\n";
  }
  
};


/// @class IdbSite  // example 
class IdbSite {
public:
  std::string name;
  int width;
  int height;
  int not_saved;
  

  IdbSite() : name("Default"), width(11), height(22), not_saved(33) {}

  IdbSite(std::string n, int w = 11, int h = 22, int t = 33)
      : name(n), width(w), height(h), not_saved(t) {}

  void print() {
    std::cout << "IdbSite(name=" << name
              << ", width=" << width
              << ", height=" << height
              << ", not_saved=" << not_saved << ")\n";
  }
  
};

/// @class Array2  // example 
class Array2 {
  public:
    std::string name;
    int width;
    int height;
    int not_saved;
    
  
    Array2() : name("Default"), width(11), height(22), not_saved(33) {}
  
    Array2(std::string n, int w = 11, int h = 22, int t = 33)
        : name(n), width(w), height(h), not_saved(t) {}
  
    void print() {
      std::cout << "Array(name=" << name
                << ", width=" << width
                << ", height=" << height
                << ", not_saved=" << not_saved << ")\n";
    }
    
  };

/// @class IdbSites  // example 
class IdbSites {
  public:
    std::string name;
    // int width;
    // int height;
    // int not_saved;
    // SubClass subobj;
    std::vector<IdbSite> idbsite_array;
    std::vector<Array2> array2_array;
    
  
    IdbSites() : name("Default") {}
  
    IdbSites(std::string n)
        : name(n) {}
  
    void print() {
      std::cout << "IdbSite(name=" << name << ")\n";
    }
    
  };

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
// TABLE4CLASS( SubClass, "",(sid,data1,data2));
// TABLE4CLASS( IdbSite, "table_name", (name, width, height, subobj) );
TABLE4CLASS( IdbSite, "table_name", (name, width, height) );
TABLE4CLASS( Array2, "array_table_name", (name, width, height) );
TABLE4CLASS( IdbSites, "", (name) );

TABLE2TABLE_1_N_VEC( IdbSites, (idbsite_array, array2_array) );

/*
// 可以定义一个宏，自动定义 TABLE4CLASS 和 TABLE2TABLE_1_N_VEC
// 比如 TABLE4CLASSWVEC( IdbSites,"table_name", (name...), (columnname...), (idbsite_array, array2_array) );
// 会形成TABLE4CLASS( IdbSites,"table_name", (name...), (columnname...) );
// 和 TABLE2TABLE_1_N_VEC( IdbSites, (idbsite_array, array2_array) );
// 定义一个新的类 DbMapAll<typename T> {
//包含了 DbMap<T> 和 每一个DbMapT2T<T1,T2>
//}
这样在应用程序中只要创建DbMapAll对象 这样就解决  insertToDbAll完全可以在DbMapAll里实现
DbMapAll如有必要可以定义在宏中，声明和部分定义在宏中，宏外部定义
*/

int main() {

  edadb::dbgPrint();

  IdbSite *p1 = new IdbSite("Site1",100,110);
  IdbSite *p2 = new IdbSite("Site2",200,210);
  IdbSite *p3 = new IdbSite("Site3",300,310); 
  IdbSite p4("Site4",400,410); 
  IdbSite p5("Site5",500,510); 

  IdbSites *p6 = new IdbSites("Sites1");
  IdbSites *p7 = new IdbSites("Sites2");
  IdbSites *p8 = new IdbSites("Sites3");  
  IdbSites p9("Sites4");
  IdbSites p10("Sites5");

  edadb::DbMap<IdbSite> db2; // 改成db_表名 db_IdbSite
  edadb::DbMap<IdbSites> db1; // db_IdbSites

  edadb::DbMapT2T<IdbSites,IdbSite> db_v; // v for vector 必须体现数组变量名 才能区分多个IdbSite
  db2.connectToDb("sqlite.db");
  db2.createTable("IdbSite_table");
  db1.createTable("IdbSites_table");
  // db_v.createTable("IdbSites_IdbSite_table");
  db_v.createTable(); // '_'
  // edadb::DbMap<SubClass> subdb;
  // subdb.createTable("SubClass_table");
  // const auto vecs = edadb::TypeMetaData<SubClass>::tuple_type_pair();

  std::cout<<"\n-----------insert 5 IdbSite records-------------\n";

  db2.insertToDb(p1); // 可以 
  db2.insertToDb(p2);
  db2.insertToDb(p3);
  db2.insertToDb(&p4);
  db2.insertToDb(&p5);

  db1.insertToDb(p6);
  db1.insertToDb(p7);
  db1.insertToDb(p8);
  db1.insertToDb(&p9);
  db1.insertToDb(&p10);

  db_v.insertToDb(p6,p1); // 如果class1中没有class2的数组，仍然可以通过这种方式insert进class1_class2表中
  db_v.insertToDb(p6,p2);
  db_v.insertToDb(p6,p3); 

  std::cout<<"\n-----------insertall test-------------\n";
  p7->idbsite_array.push_back(IdbSite("SiteA", 100, 110));
  p7->idbsite_array.push_back(IdbSite("SiteB", 200, 210));
  p7->array2_array.push_back(Array2("ArrayA", 300, 310));
  p7->array2_array.push_back(Array2("ArrayB", 400, 410));

  db1.insertToDbAll(p7);

  std::cout<<"\n-----------insertall test end-------------\n";

  std::vector<IdbSite> vec;
  db2.selectFromDb(&vec);
  for(auto i : vec) i.print();

  std::cout<<"\n-----------update Site1, delete Site2-------------\n";

  p1->height = 150;
  db2.updateDb(p1);

  db2.deleteFromDb(p2);


  vec.clear();
  db2.selectFromDb(&vec);
  for(auto i : vec) i.print();

  std::cout<<"\n-----------choose Sites with width < 350--------------\n";

  vec.clear(); 
  db2.selectFromDb(&vec, "width < 350"); // 嵌套中select有问题
  for(auto i : vec) i.print();

  
  return 0;
}

