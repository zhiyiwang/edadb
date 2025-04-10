#include <iostream>
#include <sqlite3.h>
#include <string>

using namespace std;

int test_sqlite_api() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open("test_api.db", &db);
    if (rc != SQLITE_OK) {
        cerr << "Could not open database: " << sqlite3_errmsg(db) << endl;
        return -1;
    }
    cout << "Open database successfully" << endl;

    // create table
    const char* createSQL = "CREATE TABLE IF NOT EXISTS COMPANY ("
                            "ID INT PRIMARY KEY NOT NULL, "
                            "NAME TEXT NOT NULL, "
                            "AGE INT NOT NULL, "
                            "ADDRESS CHAR(50), "
                            "SALARY REAL);";
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "Could not create table: " << errMsg << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return -1;
    }
    cout << "Create table successfully" << endl;

    // insert using prepared statement
    const char* insertSQL = "INSERT OR IGNORE INTO COMPANY (ID, NAME, AGE, ADDRESS, SALARY) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmtInsert = nullptr;
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmtInsert, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "Prepare INSERT statement failed: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return -1;
    }

    // insert first data: ID=1, NAME='Paul', AGE=32, ADDRESS='California', SALARY=20000.00
    rc = sqlite3_bind_int(stmtInsert, 1, 1);
    rc = sqlite3_bind_text(stmtInsert, 2, "Paul", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_int(stmtInsert, 3, 32);
    rc = sqlite3_bind_text(stmtInsert, 4, "California", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_double(stmtInsert, 5, 20000.00);

    rc = sqlite3_step(stmtInsert);
    if (rc != SQLITE_DONE) {
        cerr << "Insert first data failed: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Insert first data successfully" << endl;
    }

    // reset the prepared statement for reuse
    rc = sqlite3_reset(stmtInsert);
    if (rc != SQLITE_OK) {
        cerr << "重置 INSERT 语句失败: " << sqlite3_errmsg(db) << endl;
    }

    // insert second data: ID=2, NAME='Allen', AGE=25, ADDRESS='Texas', SALARY=15000.00
    rc = sqlite3_bind_int(stmtInsert, 1, 2);
    rc = sqlite3_bind_text(stmtInsert, 2, "Allen", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_int(stmtInsert, 3, 25);
    rc = sqlite3_bind_text(stmtInsert, 4, "Texas", -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_double(stmtInsert, 5, 15000.00);

    rc = sqlite3_step(stmtInsert);
    if (rc != SQLITE_DONE) {
        cerr << "Insert second data failed: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Insert second data successfully" << endl;
    }
    sqlite3_finalize(stmtInsert);

    // update using prepared statement
    const char* updateSQL = "UPDATE COMPANY SET SALARY = ? WHERE ID = ?;";
    sqlite3_stmt* stmtUpdate = nullptr;
    rc = sqlite3_prepare_v2(db, updateSQL, -1, &stmtUpdate, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "Prepare UPDATE statement failed: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return -1;
    }

    // update salary for ID=1
    rc = sqlite3_bind_double(stmtUpdate, 1, 25000.00);
    rc = sqlite3_bind_int(stmtUpdate, 2, 1);

    rc = sqlite3_step(stmtUpdate);
    if (rc != SQLITE_DONE) {
        cerr << "Update record failed: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Update record successfully" << endl;
    }
    sqlite3_finalize(stmtUpdate);

    // delete using prepared statement
    const char* deleteSQL = "DELETE FROM COMPANY WHERE ID = ?;";
    sqlite3_stmt* stmtDelete = nullptr;
    rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmtDelete, nullptr);
    if (rc != SQLITE_OK) {
        cerr << "Prepare DELETE statement failed: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return -1;
    }

    // delete record with ID=2
    rc = sqlite3_bind_int(stmtDelete, 1, 2);

    rc = sqlite3_step(stmtDelete);
    if (rc != SQLITE_DONE) {
        cerr << "Delete record failed: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Delete record successfully" << endl;
    }
    sqlite3_finalize(stmtDelete);

    sqlite3_close(db);
    return 0;
}
