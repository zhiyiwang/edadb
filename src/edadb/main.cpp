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

TABLE4CLASS_COLNAME( IdbSite, "table_name", (name, width, height), (":name",":width",":height") );
    

int main() {
    if (0) {
        std::cout<<"[TypeMetaData<IdbSite> Debug]" << std::endl;
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
        std::cout<<std::endl<<std::endl;
    }

    if (0) {
        std::cout<<"[TypeMetaDataValuePrinter<IdbSite> Debug]" << std::endl;
        edadb::TypeMetaDataValuePrinter<IdbSite> printer;

        IdbSite p1("Site1",100,110);
        // use getVal get TupType and output the values and the type of the values
        std::cout<<"IdbSite p1 values: " << std::endl;

        printer.print( edadb::TypeMetaData<IdbSite>::getVal(&p1) );
        std::cout<<"\n";

        IdbSite p2("Site2",200,210);
        std::cout<<"IdbSite p2 values: " << std::endl;

        printer.print( edadb::TypeMetaData<IdbSite>::getVal(&p2) );
        std::cout<<std::endl<<std::endl;
    }

   
    if (0) {
        // sqlstatement
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


    // ObjectRelationMapper
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
    dbm.insertPrepare();
    dbm.insert(&p1);
    dbm.insert(&p2);
    dbm.insert(&p3);
    dbm.insert(&p4);
    dbm.insert(&p5);
    dbm.insertFinalize();
    std::cout << std::endl << std::endl;

    // scan
    std::cout << "[DbMap Scan]" << std::endl;
    dbm.scanPrepare();
    bool got_flag = false;
    IdbSite got; 
    #if DEBUG_SQLITE3_INSERT
        std::cout << "Insert obj address: " << &got << std::endl;
        std::cout << "obj->name: " << got.name << " address: " << &got.name << std::endl;
        std::cout << "obj->width: " << got.width << " address: " << &got.width << std::endl;
        std::cout << "obj->height: " << got.height << " address: " << &got.height << std::endl;
        std::cout << std::endl;
    #endif

    uint32_t cnt = 0;
    while (got_flag = dbm.scan(&got)) {
        std::cout<<"IdbSite ["<<cnt++<<"] :" << std::endl;
        got.print();
    }

    dbm.scanFinalize();
    std::cout << std::endl << std::endl;

    // scan sqlite3 directly
    if (0) {
        sqlite3 *db;
        char *zErrMsg = 0;
        int rc;
        rc = sqlite3_open("sqlite.db", &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return(0);
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
            std::cout<<"name: "<<sqlite3_column_text(stmt, 0)<<std::endl;
            std::cout<<"width: "<<sqlite3_column_int(stmt, 1)<<std::endl;
            std::cout<<"height: "<<sqlite3_column_int(stmt, 2)<<std::endl;
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

  return 0;
}

