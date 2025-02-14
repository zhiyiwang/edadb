#include "lib/edadb/edadbTest2.hpp"


/// @class SubClass // example 
class SubClass { // 改 实际工程中的名字
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
  SubClass subobj;
  

  IdbSite() : name("Default"), width(11), height(22), not_saved(33), subobj() {}

  IdbSite(std::string n, int w = 11, int h = 22, int t = 33, int sub_sid = 123, double sub_data1 = 4.56, double sub_data2 = 7.89)
      : name(n), width(w), height(h), not_saved(t), subobj(sub_sid, sub_data1, sub_data2) {}

  void print() {
    std::cout << "IdbSite(name=" << name
              << ", width=" << width
              << ", height=" << height
              << ", not_saved=" << not_saved << ")\n";
    std::cout << "  Subclass: ";
    subobj.print();
  }
  
};

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
TABLE4CLASS( SubClass, "",(sid,data1,data2));
TABLE4CLASS( IdbSite, "table_name", (name, width, height, subobj) );

int main() {
  std::cout << "CppTypeToDbType<IdbSite>::ret: " << static_cast<int>(edadb::CppTypeToDbType<IdbSite>::ret) << std::endl;
  std::cout << "CppTypeToDbType<SubClass>::ret: " << static_cast<int>(edadb::CppTypeToDbType<IdbSite>::ret) << std::endl;

  edadb::dbgPrint();

  IdbSite *p1 = new IdbSite("Site1",100,110);
  IdbSite *p2 = new IdbSite("Site2",200,210);
  IdbSite *p3 = new IdbSite("Site3",300,310); 
  IdbSite p4("Site4",400,410); 
  IdbSite p5("Site5",500,510); 

  edadb::DbMap<IdbSite> db;
  db.connectToDb("sqlite.db");
  db.createTable("IdbSite_table");
  // edadb::DbMap<SubClass> subdb;
  // subdb.createTable("SubClass_table");
  // const auto vecs = edadb::TypeMetaData<SubClass>::tuple_type_pair();

  std::cout<<"\n-----------insert 5 IdbSite records-------------\n";

  db.insertToDb(p1);
  db.insertToDb(p2);
  db.insertToDb(p3);
  db.insertToDb(&p4);
  db.insertToDb(&p5); 


  // std::vector<IdbSite> vec;
  // db.selectFromDb(&vec);
  // for(auto i : vec) i.print();

  // std::cout<<"\n-----------update Site1, delete Site2-------------\n";

  p1->height = 150;
  db.updateDb(p1);

  db.deleteFromDb(p2);


  // vec.clear();
  // db.selectFromDb(&vec);
  // for(auto i : vec) i.print();

  // std::cout<<"\n-----------choose Sites with width < 350--------------\n";

  // vec.clear();
  // db.selectFromDb(&vec, "width < 350");
  // for(auto i : vec) i.print();

  return 0;
}

