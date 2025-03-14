/**
 * @file perf_int.hpp
 * @brief perf_int.hpp provides a way to test the performance of the edadb library.
 */

#pragma once

#include <stdint.h>
#include <chrono>
#include "edadb.hpp"

#define RECD_NUM  10000
#define QUERY_NUM 10000

class PerfInt {
public:
    int id;
    int int1;
    int int2;
    int int3;
};


TABLE4CLASS_COLNAME( PerfInt, "PerfInt", (id, int1, int2, int3), (":id", ":int1", ":int2", ":int3") );

int test_edadb_perf_int(uint64_t recd_num = RECD_NUM, uint64_t query_num = QUERY_NUM) {

    edadb::DbMap<PerfInt> dbm;
    if (!dbm.init("sqlite.db", "PerfInt")) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }

    dbm.createTable();

    auto start_insert = std::chrono::high_resolution_clock::now();
    edadb::DbMap<PerfInt>::Inserter inserter(dbm);
    dbm.beginTransaction();
    inserter.prepare();
    for (uint64_t i = 0; i < recd_num; i++) {
        PerfInt perf_int;
        perf_int.id = i;
        perf_int.int1 = i;
        perf_int.int2 = i + 1;
        perf_int.int3 = i + 2;

        inserter.insert(&perf_int);
    }
    inserter.finalize();
    dbm.commitTransaction();


    auto start_scan = std::chrono::high_resolution_clock::now();
    uint64_t cnt = 0, sum_id = 0, sum_int1 = 0, sum_int2 = 0, sum_int3 = 0;
    for (int i = 0; i < query_num; i++) {
        edadb::DbMap<PerfInt>::Fetcher fetcher(dbm);
        fetcher.prepare();
    
        PerfInt got;
        while (fetcher.fetch(&got)) {
            cnt++;
            sum_id += got.id;
            sum_int1 += got.int1;
            sum_int2 += got.int2;
            sum_int3 += got.int3;
        } // while
        fetcher.finalize();
    }

//    std::cout << "cnt = " << cnt << "sum_id = " << sum_id << "sum_int1 = " << sum_int1 << "sum_int2 = " << sum_int2 << "sum_int3 = " << sum_int3 << std::endl;

    auto end_scan = std::chrono::high_resolution_clock::now();
    auto duration_insert = std::chrono::duration_cast<std::chrono::milliseconds>(start_scan - start_insert);
    auto duration_scan = std::chrono::duration_cast<std::chrono::milliseconds>(end_scan - start_scan) / query_num;
    
    std::cout <<  duration_insert.count() << ", " << duration_scan.count() << std::endl;

    return 0;
}