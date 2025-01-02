

#include "lib/edadb/edadb.hpp"




/// @class IdbSite  // Layout 
struct IdbSite {
    int tmp;
    int widthX;
    double heightY;
    IdbSite(int a1 = 123,double a2 = 2.34,int t = 555) : widthX(a1), heightY(a2), tmp(t){}
    IdbSite& operator=(const IdbSite& p) {
        widthX = p.widthX;
        heightY = p.heightY;
        return *this;
    }
    
    
};


// Both should be persistable
// SPECIALIZE_BUN_HELPER((IdbSite, widthX));

// SPECIALIZE_BUN_HELPER( (test::IdbSite, name, uname, age, heightY) );
TABLE4CLASS( (IdbSite, widthX, heightY) );

int main() {
    edadb::print();
    
  edadb::DbMap<IdbSite> db;
  db.connectToDb("sqlite.db");
  db.setTableName("IdbSite_table");
  
  IdbSite *p5 = db.selectFromDb("widthX = 2");
  std::cout << "p5->widthX = "<<p5->widthX << "  p5->heightY = "<< p5->heightY <<
  "  p5->tmp = " << p5->tmp << " \n";
  int widthX = 4;
  IdbSite *p6 = db.selectWithPK(widthX);
  std::cout << "p6->widthX = "<<p6->widthX << "  p6->heightY = "<< p6->heightY <<
  "  p6->tmp = " << p6->tmp << " \n";

    //db.select

    
    

	    
    return 0;
}

