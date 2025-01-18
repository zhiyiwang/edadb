#include "lib/edadb/edadb.hpp"


/// @class IdbSite  // example 
class IdbSite {
public:
  std::string name;
  int width;
  int height;
  int not_saved;
  
  IdbSite() : name("Default"), width(123), height(456), not_saved(789) {}

  IdbSite(std::string n, int w = 123,int h = 456,int t = 789) : name(n), width(w), height(h), not_saved(t){}

  void print(){
    std::cout<<"IdbSite(name="<<name<<", width="<<width<<", height="<<height<<", not_saved="<<not_saved<<")\n";
  }
  
};

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
TABLE4CLASS( (IdbSite, name, width, height) );

int main() {
  edadb::dbgPrint();

  IdbSite *p1 = new IdbSite("Site1",100,110);
  IdbSite *p2 = new IdbSite("Site2",200,210);
  IdbSite *p3 = new IdbSite("Site3",300,310); 
  IdbSite p4("Site4",400,410); 
  IdbSite p5("Site5",500,510); 

  edadb::DbMap<IdbSite> db;
  db.connectToDb("sqlite.db");
  db.createTable("IdbSite_table");

  std::cout<<"\n-----------insert 5 IdbSite records-------------\n";

  db.insertToDb(p1);
  db.insertToDb(p2);
  db.insertToDb(p3);
  db.insertToDb(&p4);
  db.insertToDb(&p5);


  std::vector<IdbSite> vec;
  db.selectFromDb(&vec);
  for(auto i : vec) i.print();

  std::cout<<"\n-----------update Site1, delete Site2-------------\n";

  p1->height = 150;
  db.updateDb(p1);

  db.deleteFromDb(p2);


  vec.clear();
  db.selectFromDb(&vec);
  for(auto i : vec) i.print();

  std::cout<<"\n-----------choose Sites with width < 350--------------\n";

  vec.clear();
  db.selectFromDb(&vec, "width < 350");
  for(auto i : vec) i.print();

  return 0;
}

