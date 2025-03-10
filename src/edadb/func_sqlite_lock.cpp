/**
 * @file func_sqlite_lock.cpp
 * @brief test the sqlite3 interface lock
 */

#include <sqlite3.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h> // for fork()
#include <sys/wait.h> // for wait()

const char* DB_NAME = "test_lock.db";


void initialize_db() {
    sqlite3* db;
    sqlite3_open(DB_NAME, &db);
    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT);";
    sqlite3_exec(db, create_table_sql, 0, 0, 0);
    sqlite3_close(db);
}

// insert by thread
void thread_task(int thread_id) {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr);

    const char* insert_sql = "INSERT INTO test (value) VALUES (?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);

    std::string value = "Thread_" + std::to_string(thread_id);
    sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Thread " << thread_id << " failed to insert: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Thread " << thread_id << " inserted data successfully." << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// insert by process
void process_task() {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE, nullptr);

    const char* insert_sql = "INSERT INTO test (value) VALUES ('Process_Write');";
    sqlite3_exec(db, "BEGIN IMMEDIATE;", 0, 0, 0); 
    if (sqlite3_exec(db, insert_sql, 0, 0, 0) != SQLITE_OK) {
        std::cerr << "Process " << getpid() << " failed to insert: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Process " << getpid() << " inserted data successfully." << std::endl;
    }
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    sqlite3_close(db);
}

int test_sqlite_lock(void) {
    initialize_db(); 

    // multi-threaded
    std::cout << "Starting multi-threaded test..." << std::endl;
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(thread_task, i);
    }
    for (auto& th : threads) {
        th.join();
    }


    // multi process
    std::cout << "\nStarting multi-process test..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        pid_t pid = fork();
        if (pid == 0) { 
            process_task();
            exit(0);
        }
    }

    // wait until all child processes finish
    int status;
    while (wait(&status) > 0);

    return 0;
}
