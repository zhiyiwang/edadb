/**
 * @file test_sugar_interface.cpp
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */

#include <iostream>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

/**
 * @brief test the soci interface
 * https://soci.sourceforge.net/doc/master/interfaces/
 */
int test_sugar_interface(void) {
    try {
        // create session: connect to SQLite database
        soci::session sql(soci::sqlite3, "sugar.db");

        // create table
        sql << "CREATE TABLE IF NOT EXISTS person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";

        // insert records
        // each statement will be prepared and executed separately
        sql << "INSERT INTO person (name, age) VALUES ('Alice', 30)";
        sql << "INSERT INTO person (name, age) VALUES ('Bob', 25)";

        // query records
        // session::prepare to prepare a statement
        // rowset to iterate over the result set
        soci::rowset<soci::row> rs = sql.prepare << "SELECT id, name, age FROM person";

        // iterate each row and get the value by column name
        // the name is defined when creating the table
        printf("Get Value by Column Name:\n");
        for (const auto& row : rs) {
            int id = row.get<int>("id");
            std::string name = row.get<std::string>("name");
            int age = row.get<int>("age");

            std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        }

        // iterate each row and get the value by column index
        // the index is the order of the columns in the SELECT statement
        printf("Get Value by Column Index:\n");
        // reset the rowset to query again
        rs = sql.prepare << "SELECT id, name, age FROM person";
        for (const auto& row : rs) {
            int id = row.get<int>(0);
            std::string name = row.get<std::string>(1);
            int age = row.get<int>(2);

            std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
} // test_suagr_interface
