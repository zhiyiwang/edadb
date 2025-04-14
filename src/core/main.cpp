#include "edadb.h"
//#include "Test.hpp"


/// @class IdbSite example 
class IdbSite {
public:
    std::string name;
    int width;
    double height;
    int not_saved;
    
    IdbSite() : name("Default"), width(123), height(456), not_saved(789) {}
  
    IdbSite(std::string n, int w = 123, int h = 456, int t = 789) : name(n), width(w), height(h), not_saved(t){}
  
    void print() {
      std::cout<<"IdbSite(name="<<name<<", width="<<width<<", height="<<height<<", not_saved="<<not_saved<<")\n";
    }
};

//TABLE4CLASS_COLNAME( IdbSite, "IdbSite_table", (name, width, height), ("name_col","width_col","height_col") );

_EDADB_DEFINE_TABLE_BY_COLUMN_(IdbSite, "IdbSite_table", (name, width, height), ("name_col","width_col","height_col") );


void testTypeMetaDataPrinter() {
    IdbSite p1("Site1",100,110);
    IdbSite p2("Site2",200,210);

    edadb::TypeMetaDataPrinter<IdbSite> printer;
    printer.printStatic();
    printer.print(&p1);
    printer.print(&p2);
    std::cout << std::endl << std::endl;
}


void testSqlStatement() {
    IdbSite p1("Site1",100,110), p2("Site2",200,210);
    edadb::SqlStatement<IdbSite> sql_stmt;
    sql_stmt.print(&p1, &p2);
}
    

int testDbMap() {
    // init database and create table
//    edadb::DbMap<IdbSite> &dbm = edadb::DbMap<IdbSite>::i();
//    if (!dbm.init("sqlite.db")) {
//        std::cerr << "DbMap::init failed" << std::endl;
//        return 1;
//    }
    if (!edadb::initDatabase<IdbSite>("sqlite.db")) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }

//    std::cout << "[DbMap CreateTable]" << std::endl;
//    dbm.createTable();
//    std::cout << std::endl << std::endl;
    if (!edadb::createTable<IdbSite>()) {
        std::cerr << "DbMap::createTable failed" << std::endl;
        return 1;
    }


     // insert
     std::cout << "[DbMap Insert]" << std::endl;
     IdbSite p1("Site1",100,110), p2("Site2",200,210), p3("Site3",300,310), p4("Site4",400,410), p5("Site5",500,510);
     std::vector<IdbSite*> vec;
     vec.push_back(&p2);
     vec.push_back(&p3);
     vec.push_back(&p4);
     vec.push_back(&p5);
 
//    // insert records
//    edadb::DbMap<IdbSite>::Writer writer;
//    if (writer.insert(&p1) == false) {
//        std::cerr << "DbMap::Writer::insert failed" << std::endl;
//        return 1;
//    }
//    if (writer.insert(&p2) == false) {
//        std::cerr << "DbMap::Writer::insert failed" << std::endl;
//        return 1;
//    }  
//    if (writer.insert(&p3) == false) {
//        std::cerr << "DbMap::Writer::insert failed" << std::endl;
//        return 1;
//    }
//    if (writer.insert(&p4) == false) {
//        std::cerr << "DbMap::Writer::insert failed" << std::endl;
//        return 1;
//    }
//    if (writer.insert(&p5) == false) {
//        std::cerr << "DbMap::Writer::insert failed" << std::endl;
//        return 1;
//    }
//    writer.finalize(); 

    if (!edadb::insertObject<IdbSite>(&p1)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }

    if (!edadb::insertVector<IdbSite>(vec)) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }

    std::cout << std::endl << std::endl;


    // scan
    std::cout << "[DbMap Scan]" << std::endl;
//    edadb::DbMap<IdbSite>::Reader reader(dbm);
//    reader.prepareByPredicate();
//
//    IdbSite got; 
//    bool got_flag = false;
//    uint32_t cnt = 0;
//    while (got_flag = reader.read(&got)) {
//        std::cout << "IdbSite [" << cnt++ << "] :  ";
//        got.print();
//    }
//
//    reader.finalize();
//    std::cout << std::endl << std::endl;

    edadb::DbMapReader<IdbSite> *rd = nullptr;
    IdbSite got;
    std::string pred = "";
    while (edadb::readByPredicate<IdbSite>(rd, &got, pred) > 0) {
        std::cout << "IdbSite :  ";
        got.print();
    } 
    assert(rd == nullptr);


    // update 
    std::cout << "[DbMap Update]" << std::endl;
    IdbSite p1_new("Site1_new",1000,1100), p2_new("Site2_new",2000,2100), p3_new("Site3_new",3000,3100);
//    writer.updateBySqlStmt(&p1, &p1_new);
//    writer.updateBySqlStmt(&p2, &p2_new);
    if (!edadb::updateObject<IdbSite>(&p1, &p1_new)) {
        std::cerr << "DbMap::Writer::update failed" << std::endl;
        return 1;
    }
    std::vector<IdbSite*> org_vec, new_vec;
    org_vec.push_back(&p2);
    org_vec.push_back(&p3);
    new_vec.push_back(&p2_new);
    new_vec.push_back(&p3_new);

    if (!edadb::updateVector<IdbSite>(org_vec, new_vec)) {
        std::cerr << "DbMap::Writer::update failed" << std::endl;
        return 1;
    }


    // scan
//    reader.prepareByPredicate();
//    cnt = 0;
//    while (got_flag = reader.read(&got)) {
//        std::cout << "IdbSite [" << cnt++ << "] :  ";
//        got.print();
//    }
//    reader.finalize();
//    std::cout << std::endl << std::endl;
    while (edadb::readByPredicate<IdbSite>(rd, &got, pred) > 0) {
        std::cout << "IdbSite :  ";
        got.print();
    }
    std::cout << std::endl << std::endl;


    // delete
    std::cout << "[DbMap Delete]" << std::endl;
//    writer.deleteByPrimaryKeys(&p1_new);
//    writer.deleteByPrimaryKeys(&p2_new);
    if (!edadb::deleteObject<IdbSite>(&p1_new)) {
        std::cerr << "DbMap::Writer::delete failed" << std::endl;
        return 1;
    }
    if (!edadb::deleteObject<IdbSite>(&p2_new)) {
        std::cerr << "DbMap::Writer::delete failed" << std::endl;
        return 1;
    }
    std::cout << std::endl << std::endl;

    // scan
//    reader.prepareByPredicate();
//    cnt = 0;
//    while (got_flag = reader.read(&got)) {
//        std::cout << "IdbSite [" << cnt++ << "] :  ";
//        got.print();
//    }
//    reader.finalize();
//    std::cout << std::endl << std::endl;
    while (edadb::readByPredicate<IdbSite>(rd, &got, pred) > 0) {
        std::cout << "IdbSite :  ";
        got.print();
    }
    std::cout << std::endl << std::endl;


    // lookup
    std::cout << "[DbMap Lookup]" << std::endl;
    IdbSite lookup_idsite("Site4",0,0);
//    reader.prepareByPrimaryKey(&lookup_idsite);
//    got_flag = reader.read(&got);
//    if (got_flag) {
//        std::cout << "IdbSite :  ";
//        got.print();
//    } else {
//        std::cout << "IdbSite not found" << std::endl;
//    }
//    reader.finalize();
    if (edadb::readByPrimaryKey<IdbSite>(&lookup_idsite) > 0) {
        std::cout << "IdbSite :  ";
        lookup_idsite.print();
    } else {
        std::cout << "IdbSite not found" << std::endl;
    }


    return 0;
}


int main() {
//    testTypeMetaDataPrinter();
    testSqlStatement();
    testDbMap();

    return 0;
}

