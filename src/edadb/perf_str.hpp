/**
 * @file perf_int.hpp
 * @brief perf_int.hpp provides a way to test the performance of the edadb library.
 */

#pragma once

#include <stdint.h>
#include <string>
#include <chrono>

#include "edadb.hpp"

#define RECD_NUM  10000
#define QUERY_NUM 10000

class PerfStr {
public:
    int id;
    std::string str1;
    std::string str2;
    std::string str3;
};


TABLE4CLASS_COLNAME( PerfStr, "PerfStr", (id, str1, str2, str3), (":id", ":str1", ":str2", ":str3") );

int test_edadb_perf_str(uint64_t recd_num = RECD_NUM, uint64_t query_num = QUERY_NUM) {

    edadb::DbMap<PerfStr> dbm;
    if (!dbm.init("sqlite.db", "PerfStr")) {
        std::cerr << "DbMap::init failed" << std::endl;
        return 1;
    }

    dbm.createTable();

    auto start_insert = std::chrono::high_resolution_clock::now();
    edadb::DbMap<PerfStr>::Inserter inserter(&dbm);
    dbm.beginTransaction();
    inserter.prepare();
    for (uint64_t i = 0; i < recd_num; i++) {
        PerfStr perf_str;
        perf_str.id = i;
        perf_str.str1 = "str1";
        perf_str.str2 = "str2";
        perf_str.str3 = "str3";
        inserter.insert(&perf_str);
    }
    inserter.finalize();
    dbm.commitTransaction();


    auto start_scan = std::chrono::high_resolution_clock::now();
    uint64_t cnt = 0, sum_id = 0, sum_str1 = 0, sum_str2 = 0, sum_str3 = 0;
    for (int i = 0; i < query_num; i++) {
        edadb::DbMap<PerfStr>::Fetcher fetcher(&dbm);
        fetcher.prepare();
    
        PerfStr got;
        while (fetcher.fetch(&got)) {
            cnt++;
            sum_id += got.id;
            sum_str1 += got.str1.size();
            sum_str2 += got.str2.size();
            sum_str3 += got.str3.size();
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