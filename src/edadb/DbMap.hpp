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
public:
    class Operator;
    class Inserter;
    class Fetcher;

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

    bool beginTransaction() {
        return manager.exec("BEGIN TRANSACTION;");
    }

    bool commitTransaction() {
        return manager.exec("COMMIT;");
    }
}; // class DbMap



template<typename T>
class DbMap<T>::Operator {
protected:
    DbMap          &dbmap;
    Sqlite3Manager &manager;
    uint32_t index{0};

public:
    Operator(DbMap &m) : dbmap(m), manager(m.manager) {}
    virtual ~Operator() = default;
}; 


// insert object to database
template<typename T>
class DbMap<T>::Inserter : public DbMap<T>::Operator {
protected:
    using DbMap<T>::Operator::dbmap;
    using DbMap<T>::Operator::manager;
    using DbMap<T>::Operator::index;

public:
    Inserter(DbMap &m) : Operator(m) { resetIndex(); }

public: // insert one 
    bool insertOne(T* obj) {
        return prepare() && insert(obj) && finalize();
    }

public: // insert many
    bool prepare() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::prepare: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::insertPlaceHolderStatement(), dbmap.table_name);
        #if DEBUG_SQLITE3_API
            std::cout << "DbMap::Inserter::prepare: " << sql << std::endl;
        #endif
        return manager.prepare(sql);
    }

    bool insert(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::insert: not inited" << std::endl;
            return false;
        }

        bindValues(obj);

        if (!manager.bindStep()) {
            return false;
        }

        if (!manager.reset()) {
            return false;
        }

        return true;
    }

    bool finalize() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::insertFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize();
    }

private:
    /** reset index to begin to bind */      
    void resetIndex() {
        index = manager.s_bind_column_begin_index;
    }

    /**
     * @brief insert the object to the database.
     * @param obj The object to bind.
     */
    void bindValues(T* obj) {
        // reset index to begin to bind 
        resetIndex();

        // iterate through the values and bind them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
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
};



// fetch values for scan statement
template<typename T>
class DbMap<T>::Fetcher : public DbMap<T>::Operator {
protected:
    using DbMap<T>::Operator::dbmap;
    using DbMap<T>::Operator::manager;
    using DbMap<T>::Operator::index;

public:
    Fetcher(DbMap &m) : Operator(m) { resetIndex(); }

public:
    bool prepare() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Fetcher::prepare: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format(SqlStatement<T>::scanStatement(), dbmap.table_name);
        return manager.prepare(sql);
    }

    bool fetch(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::insert: not inited" << std::endl;
            return false;
        }

        if (!manager.fetchStep()) {
            return false; // no more row
        }

        fetchValues(obj);
        return true;
    }

    bool finalize() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::scanFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize();
    }


private:
    /** reset index to begin to fetch */
    void resetIndex() {
        index = manager.s_fetch_column_begin_index;
    }

    /**
     * @brief fetch the object from the database.
     * @param obj The object to fetch.
     */
    void fetchValues(T* obj) {
        // reset index to begin to fetch
        resetIndex();

        // iterate through the values and fetch them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
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
};



} // namespace edadb