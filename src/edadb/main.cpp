#include "edadb.hpp"
#include "Test.hpp"


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

TABLE4CLASS_COLNAME( IdbSite, "table_name", (name, width, height), (":name",":width",":height") );



void testTypeMetaDataPrinter() {
    IdbSite p1("Site1",100,110);
    IdbSite p2("Site2",200,210);

    edadb::TypeMetaDataPrinter<IdbSite> printer;
    printer.printTypeMetaDataStaticMembers();
    printer.print(&p1);
    printer.print(&p2);
    std::cout << std::endl << std::endl;
}

void test_perf_int () {
    test_edadb_perf_int(); std::remove("sqlite.db");
    test_edadb_perf_int(); std::remove("sqlite.db");
    test_edadb_perf_int(); std::remove("sqlite.db");
    test_edadb_perf_int(); std::remove("sqlite.db");
    test_edadb_perf_int(); std::remove("sqlite.db");

    test_edadb_perf_str(); std::remove("sqlite.db");
    test_edadb_perf_str(); std::remove("sqlite.db");
    test_edadb_perf_str(); std::remove("sqlite.db");
    test_edadb_perf_str(); std::remove("sqlite.db");
    test_edadb_perf_str(); std::remove("sqlite.db");
}

void testSqlStatement() {
    IdbSite p1("Site1",100,110);
    IdbSite p2("Site2",200,210);
    edadb::SqlStatement<IdbSite> sql_stmt;
    std::cout << "[sqlstatement debug] " << std::endl;
    std::cout << "Create Table SQL: " << sql_stmt.createTableStatement() << std::endl;
    std::cout << "Insert Place Holder SQL: " << sql_stmt.insertPlaceHolderStatement() << std::endl;
    std::cout << "Insert p1 SQL: " << sql_stmt.insertStatement (&p1) << std::endl;
    std::cout << "Insert p2 SQL: " << sql_stmt.insertStatement (&p2) << std::endl;
    std::cout << "Scan SQL: " << sql_stmt.scanStatement() << std::endl;
    std::cout << std::endl << std::endl;
}
    

int testDbMap() {
    edadb::DbMap<IdbSite> dbm;
    if (!dbm.init("sqlite.db", "IdbSite")) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }

    std::cout << "[DbMap CreateTable]" << std::endl;
    dbm.createTable();
    std::cout << std::endl << std::endl;

    // insert
    std::cout << "[DbMap Insert]" << std::endl;
    IdbSite p1("Site1",100,110), p2("Site2",200,210), p3("Site3",300,310), p4("Site4",400,410), p5("Site5",500,510);
    
    edadb::DbMap<IdbSite>::Inserter inserter(dbm);
    inserter.prepare();
    inserter.insert(&p1);
    inserter.insert(&p2);
    inserter.insert(&p3);
    inserter.insert(&p4);
    inserter.insert(&p5);
    inserter.finalize();
    std::cout << std::endl << std::endl;

    // scan
    std::cout << "[DbMap Scan]" << std::endl;
    edadb::DbMap<IdbSite>::Fetcher fetcher(dbm);
    fetcher.prepare();

    IdbSite got; 
    bool got_flag = false;
    uint32_t cnt = 0;
    while (got_flag = fetcher.fetch(&got)) {
        std::cout << "IdbSite [" << cnt++ << "] :  ";
        got.print();
    }

    fetcher.finalize();
    std::cout << std::endl << std::endl;

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
//    testSqlStatement();
    testDbMap();
//    testSqlite3();

    return 0;
}

