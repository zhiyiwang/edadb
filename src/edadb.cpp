#include "lib/edadb/edadb.hpp"


/// @class IdbSite  // Layout 
class IdbSite {
public:
  int width;
  double height;
  int tmp;

  IdbSite(int w = 123,double h = 2.34,int t = 555) : width(w), height(h), tmp(t){}

  IdbSite& operator=(const IdbSite& p) {
      width = p.width;
      height = p.height;
      return *this;
  }   
};

// TABLE4CLASS( (classname, primary_key, field_to_persist,...) );
TABLE4CLASS( (IdbSite, width, height) );

int main() {
  edadb::dbgPrint();

  IdbSite *p1 = new IdbSite(1,5.55);
  IdbSite *p2 = new IdbSite(2,6.66);
  IdbSite *p3 = new IdbSite(); //default：123,2.34
  IdbSite p4(4,4.88); 

  edadb::DbMap<IdbSite> db;
  db.connectToDb("sqlite.db");
  db.createTable("IdbSite_table");

  db.insertToDb(p1);
  db.insertToDb(p2);
  db.insertToDb(p3);
  db.insertToDb(&p4);

  p1->height = 11.11;
  db.updateDb(p1);

  db.deleteFromDb(p2);

  IdbSite p5;
  db.selectFromDb("width = 4", &p5); //应该 return vector<obj>
  std::cout << "p5->width = "<<p5.width << "  p5->height = "<< p5.height <<
  "  p5->tmp = " << p5.tmp << " \n";
//   int width = 4;
//   IdbSite *p6 = db.selectWithPK(width); 
//   std::cout << "p6->width = "<<p6->width << "  p6->height = "<< p6->height <<
//   "  p6->tmp = " << p6->tmp << " \n";
    return 0;
}

