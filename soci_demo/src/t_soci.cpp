

#include "lib/edadb/edadb.hpp"




/// @class IdbSite  // Layout 
struct IdbSite {
    int tmp;
    int width;
    double height;
    IdbSite(int a1 = 123,double a2 = 2.34,int t = 555) : width(a1), height(a2), tmp(t){}
    IdbSite& operator=(const IdbSite& p) {
        width = p.width;
        height = p.height;
        return *this;
    }
    
    
};


// Both should be persistable
// SPECIALIZE_BUN_HELPER((IdbSite, width));

// SPECIALIZE_BUN_HELPER( (test::IdbSite, name, uname, age, height) );
TABLE4CLASS( (IdbSite, width, height) );

int main() {
	std::cout<<121<<std::endl;
    edadb::print();
    IdbSite *p1 = new IdbSite(1,5.55);
    IdbSite *p2 = new IdbSite(2,6.66);
    IdbSite *p3 = new IdbSite();
    IdbSite *p4 = new IdbSite(4,8.88);
  edadb::DbMap<IdbSite> db;
  db.connectToDb("sqlite.db");
  db.createTable("IdbSite_table");
  db.insertToDb(p1);
  db.insertToDb(p2);
  db.insertToDb(p3);
  db.insertToDb(p4);
//   double input = 0;
//   std::cout<<"update p1->width to ? ";std::cin>>input;
  p1->height = 11.11;
  db.updateDb(p1);
  db.deleteFromDb(p2);
  IdbSite *p5 = db.selectFromDb("width = 2");
  std::cout << "p5->width = "<<p5->width << "  p5->height = "<< p5->height <<
  "  p5->tmp = " << p5->tmp << " \n";
  int width = 4;
  IdbSite *p6 = db.selectWithPK(width);
  std::cout << "p6->width = "<<p6->width << "  p6->height = "<< p6->height <<
  "  p6->tmp = " << p6->tmp << " \n";

    //db.select

    
    

	    
    return 0;
}

