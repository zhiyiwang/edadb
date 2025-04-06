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

TABLE4CLASS_COLNAME( IdbSite, "IdbSite_table", (name, width, height), ("name_col","width_col","height_col") );



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
    edadb::DbMap<IdbSite> &dbm = edadb::DbMap<IdbSite>::i();
    if (!dbm.init("sqlite.db")) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }

    std::cout << "[DbMap CreateTable]" << std::endl;
    dbm.createTable();
    std::cout << std::endl << std::endl;

    // insert
    std::cout << "[DbMap Insert]" << std::endl;
    IdbSite p1("Site1",100,110), p2("Site2",200,210), p3("Site3",300,310), p4("Site4",400,410), p5("Site5",500,510);
    
    // insert records
    edadb::DbMap<IdbSite>::Writer writer;
    if (writer.insert(&p1) == false) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    if (writer.insert(&p2) == false) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }  
    if (writer.insert(&p3) == false) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    if (writer.insert(&p4) == false) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    if (writer.insert(&p5) == false) {
        std::cerr << "DbMap::Writer::insert failed" << std::endl;
        return 1;
    }
    writer.finalize(); 
    std::cout << std::endl << std::endl;

    // scan
    std::cout << "[DbMap Scan]" << std::endl;
    edadb::DbMap<IdbSite>::Reader reader(dbm);
    reader.prepare();

    IdbSite got; 
    bool got_flag = false;
    uint32_t cnt = 0;
    while (got_flag = reader.read(&got)) {
        std::cout << "IdbSite [" << cnt++ << "] :  ";
        got.print();
    }

    reader.finalize();
    std::cout << std::endl << std::endl;

    // update 
    std::cout << "[DbMap Update]" << std::endl;
    IdbSite p1_new("Site1_new",1000,1100), p2_new("Site2_new",2000,2100);
    writer.update(&p1, &p1_new);
    writer.update(&p2, &p2_new);

    // scan
    reader.prepare();
    cnt = 0;
    while (got_flag = reader.read(&got)) {
        std::cout << "IdbSite [" << cnt++ << "] :  ";
        got.print();
    }
    reader.finalize();
    std::cout << std::endl << std::endl;

    // delete
    std::cout << "[DbMap Delete]" << std::endl;
    writer.deleteByPrimaryKeys(&p1_new);
    writer.deleteByPrimaryKeys(&p2_new);

    // scan
    reader.prepare();
    cnt = 0;
    while (got_flag = reader.read(&got)) {
        std::cout << "IdbSite [" << cnt++ << "] :  ";
        got.print();
    }
    reader.finalize();
    std::cout << std::endl << std::endl;

    // lookup
    std::cout << "[DbMap Lookup]" << std::endl;
    IdbSite lookup_idsite("Site3",0,0);
    reader.prepare(&lookup_idsite);
    got_flag = reader.read(&got);
    if (got_flag) {
        std::cout << "IdbSite :  ";
        got.print();
    } else {
        std::cout << "IdbSite not found" << std::endl;
    }
    reader.finalize();


    return 0;
}


void testSqlite3() {
    // scan sqlite3 directly
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open("sqlite.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return ;
    }

    // prepare sql
    std::string sql = "SELECT * FROM IdbSite;";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    // step and get column
    uint32_t cnt = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::cout<<"IdbSite ["<<cnt++<<"] :  ";
        std::cout<<"name: "<<sqlite3_column_text(stmt, 0) << " ";
        std::cout<<"width: "<<sqlite3_column_int(stmt, 1) << " ";
        std::cout<<"height: "<<sqlite3_column_int(stmt, 2) << " ";
        std::cout<<std::endl;
    }

    // finalize
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}



int main() {
//    testTypeMetaDataPrinter();
    testSqlStatement();
    testDbMap();
//    testSqlite3();

    return 0;
}

