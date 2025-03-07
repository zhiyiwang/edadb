#include "edadb.hpp"


/// @class IdbSite example 
class IdbSite {
public:
  std::string name;
  int width;
  int height;
  int not_saved;
  
  IdbSite() : name("Default"), width(123), height(456), not_saved(789) {}

  IdbSite(std::string n, int w = 123, int h = 456, int t = 789) : name(n), width(w), height(h), not_saved(t){}

  void print() {
    std::cout<<"IdbSite(name="<<name<<", width="<<width<<", height="<<height<<", not_saved="<<not_saved<<")\n";
  }
  
};

TABLE4CLASS_COLNAME( IdbSite, "table_name", (name, width, height), ("iname","iwidth","iheight") );
    

int main() {

    // checkt TypeMetaData<IdbSite>
    std::cout<<"TypeMetaData<IdbSite>::class_name() = "<<edadb::TypeMetaData<IdbSite>::class_name()<<"\n";
    std::cout <<"TypeMetaData<IdbSite>::table_name() = "<<edadb::TypeMetaData<IdbSite>::table_name()<<"\n";
    std::cout<<"TypeMetaData<IdbSite>::member_names() = ";
    auto names = edadb::TypeMetaData<IdbSite>::member_names();
    for(auto n : names) std::cout<<n<<" ";
    std::cout<<"\n";

    std::cout<<"TypeMetaData<IdbSite>::column_names() = ";
    auto cols = edadb::TypeMetaData<IdbSite>::column_names();
    for(auto c : cols) std::cout<<c<<" ";
    std::cout<<"\n";

    std::cout<<"TypeMetaData<IdbSite>::tuple_type_pair() = ";
    boost::fusion::for_each(edadb::TypeMetaData<IdbSite>::tuple_type_pair(), [](auto p){std::cout<<p.second<<" ";});
    std::cout<<"\n";


    edadb::TypeMetaDataValuePrinter<IdbSite> printer;

    IdbSite p1("Site1",100,110);
    // use getVal get TupType and output the values and the type of the values
    std::cout<<"IdbSite p1 values: " << std::endl;
    
    printer.print( edadb::TypeMetaData<IdbSite>::getVal(&p1) );
    std::cout<<"\n";

    IdbSite p2("Site2",200,210);
    std::cout<<"IdbSite p2 values: " << std::endl;
    
    printer.print(edadb::TypeMetaData<IdbSite>::getVal(&p2));
    std::cout<<"\n";
    


//  IdbSite *p1 = new IdbSite("Site1",100,110);
//  IdbSite *p2 = new IdbSite("Site2",200,210);
//  IdbSite *p3 = new IdbSite("Site3",300,310); 
//  IdbSite p4("Site4",400,410); 
//  IdbSite p5("Site5",500,510); 

//  edadb::DbMap<IdbSite> db;
//  db.connectToDb("sqlite.db");
//  db.createTable("IdbSite_table");
//
//  std::cout<<"\n-----------insert 5 IdbSite records-------------\n";
//
//  db.insertToDb(p1);
//  db.insertToDb(p2);
//  db.insertToDb(p3);
//  db.insertToDb(&p4);
//  db.insertToDb(&p5); 
//
//
//  std::vector<IdbSite> vec;
//  db.selectFromDb(&vec);
//  for(auto i : vec) i.print();
//
//  std::cout<<"\n-----------update Site1, delete Site2-------------\n";
//
//  p1->height = 150;
//  db.updateDb(p1);
//
//  db.deleteFromDb(p2);
//
//
//  vec.clear();
//  db.selectFromDb(&vec);
//  for(auto i : vec) i.print();
//
//  std::cout<<"\n-----------choose Sites with width < 350--------------\n";
//
//  vec.clear();
//  db.selectFromDb(&vec, "width < 350");
//  for(auto i : vec) i.print();

  return 0;
}

