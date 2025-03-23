/**
 * @file DbMap.hpp
 * @brief DbMap.hpp provides a way to map objects to relations.
 */

#pragma once

#include <fmt/format.h>
#include <string>
#include <stdint.h>

#include "SqlStatement.hpp"
#include "DbStatement.h"
#include "DbManager.hpp"

namespace edadb {


/**
 * @brief DbMap class maps objects to relations.
 * @tparam T The class type.
 */
template<typename T>
class DbMap {
public:
    class Inserter; // insert object to database
    class Fetcher;  // fetch  object from database
    class Updater;  // update object to database
    class Deleter;  // delete object from database
    

protected:
    std::string table_name;
    DbManager   manager;

public:
    DbMap () = default;
    ~DbMap() = default;

public:
    const std::string& getTableName() { return table_name; }
    DbManager&         getManager  () { return manager   ; }

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

        const std::string sql =
            fmt::format(SqlStatement<T>::createTableStatement(), table_name);
        #if DEBUG_SQLITE3_API
            std::cout << "Create Table SQL: " << std::endl;
            std::cout << sql << std::endl;
        #endif
        return manager.exec(sql);
    }

    bool dropTable() {
        if (!inited()) {
            std::cerr << "DbMap::dropTable: not inited" << std::endl;
            return false;
        }

        const std::string sql = fmt::format("DROP TABLE IF EXISTS \"{}\";", table_name);
        return manager.exec(sql);
    }

public:
    bool beginTransaction() {
        return manager.exec("BEGIN TRANSACTION;");
    }

    bool commitTransaction() {
        return manager.exec("COMMIT;");
    }
}; // class DbMap



// insert object to database
template<typename T>
class DbMap<T>::Inserter {
protected:
    DbMap     &dbmap;
    DbManager &manager;

    DbStatement dbstmt;
    uint32_t index = 0;

public:
    Inserter(DbMap &m) : dbmap(m), manager(m.getManager()) { resetIndex(); }

    DbManager& getManager() {
        return manager;
    }

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
            fmt::format(SqlStatement<T>::insertPlaceHolderStatement(), dbmap.getTableName());
        #if DEBUG_SQLITE3_API
            std::cout << "DbMap::Inserter::prepare: " << sql << std::endl;
        #endif

        return manager.prepare(dbstmt, sql);
    }

    bool insert(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::insert: not inited" << std::endl;
            return false;
        }

        bindObject(obj);

        if (!manager.bindStep(dbstmt)) {
            return false;
        }

        if (!manager.reset(dbstmt)) {
            return false;
        }

        return true;
    }

    bool finalize() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::insertFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize(dbstmt);
    }

private:
    /** reset index to begin to bind */      
    void resetIndex() {
        index = manager.s_bind_column_begin_index;
    }

    /**
     * @brief bind the object to the database.
     * @param obj The object to bind.
     */
    void bindObject(T* obj) {
        // reset index to begin to bind 
        resetIndex();

        // iterate through the values and bind them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }

public:
    /**
     * @brief Operator to bind each element in obj 
     * @param elem The element pointer to bind, which is defined as a cpp type pointer.
     */
    template <typename ElemType>
    void operator()(const ElemType &elem) {
        manager.bindColumn(dbstmt, index++, elem);
    }
};



// fetch values for scan statement
template<typename T>
class DbMap<T>::Fetcher {
protected:
    DbMap     &dbmap;
    DbManager &manager;

    DbStatement dbstmt;
    uint32_t index = 0;

public:
    Fetcher(DbMap &m) : dbmap(m), manager(m.getManager()) { resetIndex(); }

    DbManager& getManager() {
        return manager;
    }

public:
    bool prepare2Scan() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Fetcher::prepare: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::scanStatement(), dbmap.getTableName());
        return manager.prepare(dbstmt, sql);
    }


    bool prepare2Lookup(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Fetcher::prepare: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::lookupStatement(obj), dbmap.getTableName());
        return manager.prepare(dbstmt, sql);
    }


    bool fetch(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Inserter::insert: not inited" << std::endl;
            return false;
        }

        if (!manager.fetchStep(dbstmt)) {
            return false; // no more row
        }

        fetchObject(obj);
        return true;
    }

    bool finalize() {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::scanFinalize: not inited" << std::endl;
            return false;
        }

        return manager.finalize(dbstmt);
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
    void fetchObject(T* obj) {
        // reset index to begin to fetch
        resetIndex();

        // iterate through the values and fetch them
        auto values = TypeMetaData<T>::getVal(obj);
        boost::fusion::for_each(values, *this);
    }

public:
    /**
     * @brief Operator to fetch each element in obj
     *   invoked by boost::fusion::for_each @ fetchValues
     * @param elem The element to fetch from column in db row.
     */
    template <typename ElemType>
    void operator()(ElemType &elem) {
        auto dbtype = CppTypeToDbType<ElemType>::dbType;
        manager.fetchColumn(dbstmt, index++, elem);
    }
};



// delete object from database
template<typename T>
class DbMap<T>::Deleter {
protected:
    DbMap     &dbmap;
    DbManager &manager;

public:
    Deleter(DbMap &m) : dbmap(m), manager(m.getManager()) {}

    DbManager& getManager() {
        return manager;
    }

public:
    bool deleteByPrimaryKeys(T* obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Deleter::deleteOne: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::deleteStatement(obj), dbmap.getTableName());
        return manager.exec(sql);
    }
}; // class Deleter



// update object to database
template<typename T>
class DbMap<T>::Updater {
protected:
    DbMap     &dbmap;
    DbManager &manager;

public:
    Updater(DbMap &m) : dbmap(m), manager(m.getManager()) {}

    DbManager& getManager() {
        return manager;
    }

public:
    bool update(T* org_obj, T* new_obj) {
        if (!dbmap.inited()) {
            std::cerr << "DbMap::Updater::update: not inited" << std::endl;
            return false;
        }

        const std::string sql =
            fmt::format(SqlStatement<T>::updateStatement(org_obj, new_obj), dbmap.getTableName());
        return manager.exec(sql);
    }
}; // class Updater



} // namespace edadb