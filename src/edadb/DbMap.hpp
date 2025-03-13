/**
 * @file DbMap.hpp
 * @brief DbMap.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>
#include <stdint.h>

#include "SqlStatement.hpp"
#include "Sqlite3Manager.hpp"

namespace edadb {


/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template<typename T>
class DbMap {


// bind values for insert statement
struct Binder {
private:
    Sqlite3Manager &manager;
    uint32_t index{0}; 

public:
    Binder(Sqlite3Manager &m) : manager(m), index(manager.s_bind_column_begin_index) {} 

    void resetIndex() {
        index = manager.s_bind_column_begin_index;
    }

public:
    /**
     * @brief Operator to bind each element in obj 
     * @param elem The element to bind.
     */
    template <typename ElemType>
    void operator()(const ElemType &elem) {
        manager.bindValue(index++, elem);
    }

    /**
     * @brief Bind the object to the database.
     * @param obj The object to bind.
     */
    void bind(T* obj) {
        // reset index to begin to bind 
        resetIndex();

        // iterate through the values and bind them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }
};


// fetch values for scan statement
struct Fetcher {
public:
    Sqlite3Manager &manager;
    uint32_t index{0};

public:
    Fetcher(Sqlite3Manager &m) : manager(m), index(manager.s_fetch_column_begin_index) {}

    void resetIndex() {
        index = manager.s_fetch_column_begin_index;
    }


public:
    /**
     * @brief Operator to fetch each element in obj
     * @param elem The element to fetch.
     */
    template <typename ElemType>
    void operator()(ElemType &elem) {
        manager.fetchValue(index++, elem);
    }

    /**
     * @brief Fetch the object from the database.
     * @param obj The object to fetch.
     */
    void fetch(T* obj) {
        // reset index to begin to fetch
        resetIndex();

        // iterate through the values and fetch them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }
};


public:
    std::string table_name;
    Sqlite3Manager manager;

public:
    DbMap () = default;
    ~DbMap() = default;

public:
    bool init(const std::string& c, const std::string& t) {
        table_name = t;
        if (table_name.empty()) {
            std::cerr << "DbMap::init: table name is empty" << std::endl;
            return false;
        }

        if (!manager.connect(c)) {
            std::cerr << "DbMap::init: connect failed" << std::endl;
            return false;
        }

        return true;
    }

    bool inited() {
        return !table_name.empty();
    }


public:
    bool createTable() {
        if (!inited()) {
            std::cerr << "DbMap::createTable: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format(SqlStatement<T>::createTableStatement(), table_name);
        #if DEBUG_SQLITE3_API
            std::cout << "Create Table SQL: " << std::endl;
            std::cout << sql << std::endl;
        #endif
        return manager.exec(sql);
    }


public:
    bool insertPrepare() {
        if (!inited()) {
            std::cerr << "DbMap::insertPrepare: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format(SqlStatement<T>::insertPlaceHolderStatement(), table_name);
        #if DEBUG_SQLITE3_API
            std::cout << "Insert Place Holder SQL: " << sql << std::endl;
        #endif
        return manager.prepare(sql);
    }

    bool insert(T* obj) {
        if (!inited()) {
            std::cerr << "DbMap::insert: not inited" << std::endl;
            return false;
        }

        Binder b(manager);
        b.bind(obj);

        if (!manager.bindStep()) {
            return false;
        }

        if (!manager.reset()) {
            return false;
        }

        return true;
    }

    bool insertFinalize() {
        if (!inited()) {
            std::cerr << "DbMap::insertFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize();
    }


public:
    bool scanPrepare() {
        if (!inited()) {
            std::cerr << "DbMap::scanPrepare: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format(SqlStatement<T>::scanStatement(), table_name);
        return manager.prepare(sql);
    }

    bool scan(T* obj) {
        if (!inited()) {
            std::cerr << "DbMap::scan: not inited" << std::endl;
            return false;
        }

        if (!manager.fetchStep()) {
            return false; // no more row
        }

        Fetcher f(manager);
        f.fetch(obj);

        return true;
    }

    bool scanFinalize() {
        if (!inited()) {
            std::cerr << "DbMap::scanFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize();
    }
}; // class DbMap


} // namespace edadb