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
const int THREAD_NUM = 5;
const int PROCESS_NUM = 3;


void initialize_db() {
    sqlite3* db;
    sqlite3_open(DB_NAME, &db);
    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT);";
    sqlite3_exec(db, create_table_sql, 0, 0, 0);
    sqlite3_close(db);
}



void process_database_write(int pidx) {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE, nullptr);

    std::string insert_sql = "INSERT INTO test (value) VALUES ('Process_" + std::to_string(pidx) + "');";
    sqlite3_exec(db, "BEGIN IMMEDIATE;", 0, 0, 0); 
    bool success = (sqlite3_exec(db, insert_sql.c_str(), 0, 0, 0) != SQLITE_OK);
    if (success) {
        std::cerr << "Process ["<< pidx <<"] " << getpid() << " failed to insert: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Process ["<< pidx <<"] " << getpid() << " inserted data successfully." << std::endl;
    }
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    sqlite3_close(db);
}

void process_database_read(int pidx) {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, nullptr);

    const char* select_sql = "SELECT * FROM test;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Process ["<< pidx <<"] " << getpid() << " read: " << sqlite3_column_text(stmt, 1) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}



void thread_database_write(int thread_id) {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE, nullptr);

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

void thread_database_read(int thread_id) {
    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, nullptr);

    const char* select_sql = "SELECT * FROM test;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Thread " << thread_id << " read: " << sqlite3_column_text(stmt, 1) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void thread_statement_write_func(int thread_id, sqlite3* db) {
    const char* insert_sql = "INSERT INTO test (value) VALUES (?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);

    std::string value = "Thread_" + std::to_string(thread_id);
    sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);

    sqlite3_exec(db, "BEGIN IMMEDIATE;", 0, 0, 0); 
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Thread " << thread_id << " failed to insert: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Thread " << thread_id << " inserted data successfully." << std::endl;
    }
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    sqlite3_finalize(stmt);
}

void thread_statement_write(void) {
    std::cout << "Starting multi-threaded test statements in a single connection..." << std::endl;
    std::vector<std::thread> threads;

    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE, nullptr);

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back(thread_statement_write_func, i, db);
    }
    for (auto& th : threads) {
        th.join();
    }

    sqlite3_close(db);
}


void thread_statement_read_func(int thread_id, sqlite3* db) {
    const char* select_sql = "SELECT * FROM test;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Thread " << thread_id << " read: " << sqlite3_column_text(stmt, 1) << std::endl;
    }

    sqlite3_finalize(stmt);
}


void thread_statement_read(void) {
    std::cout << "Starting multi-threaded test statements in a single connection..." << std::endl;
    std::vector<std::thread> threads;

    sqlite3* db;
    sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READONLY, nullptr);

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back(thread_statement_read_func, i, db);
    }
    for (auto& th : threads) {
        th.join();
    }

    sqlite3_close(db);
}


int test_sqlite_lock(void) {
    initialize_db(); 

    if (0) {
        // multi process
        std::cout << "\nStarting multi-process test..." << std::endl;

        std::cout << "Multi process write..." << std::endl;
        for (int i = 0; i < PROCESS_NUM; ++i) {
            pid_t pid = fork();
            if (pid == 0) { 
                process_database_write(i);
                exit(0);
            }
        }

        // wait until all child processes finish
        int status;
        while (wait(&status) > 0);

        std::cout << "--------------------------------------------------" << std::endl;
        sleep(1); // wait for the last process to finish writing

        std::cout << "Multi process read..." << std::endl;
        for (int i = 0; i < PROCESS_NUM; ++i) {
            pid_t pid = fork();
            if (pid == 0) { 
                process_database_read(i);
                exit(0);
            }
        }
        while (wait(&status) > 0);
    }

    if (0) {
        // multi-threaded, each thread contains a database connection
        std::cout << "Starting multi-threaded test..." << std::endl;

        std::cout << "Multi-thread write..." << std::endl;
        std::vector<std::thread> threads;
        for (int i = 0; i < THREAD_NUM; ++i) {
            threads.emplace_back(thread_database_write, i);
        }
        for (auto& th : threads) {
            th.join();
        }

        std::cout << "--------------------------------------------------" << std::endl;
        sleep(1); // wait for the last process to finish writing

        std::cout << "Multi-thread read..." << std::endl;
        threads.clear();
        for (int i = 0; i < THREAD_NUM; ++i) {
            threads.emplace_back(thread_database_read, i);
        }
        for (auto& th : threads) {
            th.join();
        }
    }


    if (1) {
        // multi-threaded, all threads share a single database connection
        std::cout << "Starting multi-threaded test with a single connection..." << std::endl;

        std::cout << "Multi-thread write..." << std::endl;
        thread_statement_write();

        std::cout << "--------------------------------------------------" << std::endl;
        sleep(1); // wait for the last process to finish writing

        std::cout << "Multi-thread read..." << std::endl;
        thread_statement_read();
    }

    return 0;
}
