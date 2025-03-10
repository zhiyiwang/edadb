/**
 * @file intfc_duckdb.cpp
 * @brief test the duckdb C interface
 */

#include <stdio.h>
#include <stdlib.h>
#include "duckdb.h"

void check_duckdb_error(duckdb_state state, const char* message) {
    if (state != DuckDBSuccess) {
        fprintf(stderr, "Error: %s\n", message);
        exit(1);
    }
}

int test_duckdb_interface(void) {
    duckdb_database db;
    duckdb_connection conn;
    
    check_duckdb_error(duckdb_open(NULL, &db), "Failed to open database");
    check_duckdb_error(duckdb_connect(db, &conn), "Failed to connect to database");


    check_duckdb_error(duckdb_query(conn, "CREATE TABLE users(id INTEGER, name VARCHAR);", NULL), "Failed to create table");

    check_duckdb_error(duckdb_query(conn, "INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob');", NULL), "Failed to insert data");

    duckdb_result result;
    check_duckdb_error(duckdb_query(conn, "SELECT * FROM users;", &result), "Failed to execute query");

    size_t row_count = duckdb_row_count(&result);
    size_t col_count = duckdb_column_count(&result);
    printf("Query returned %zu rows and %zu columns\n", row_count, col_count);

    // scan
    for (size_t row = 0; row < row_count; row++) {
        int id = duckdb_value_int32(&result, 0, row);
        const char *name = duckdb_value_varchar(&result, 1, row);
        printf("ID: %d, Name: %s\n", id, name);
        duckdb_free((void*)name); 
    }

    duckdb_destroy_result(&result);

    duckdb_disconnect(&conn);
    duckdb_close(&db);

    printf("DuckDB C API test completed successfully.\n");
    return 0;
}
