#pragma once

#include <string>
#include <memory>
#include <set>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <bitset>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/preprocessor.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/copy.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "Shadow.h"
#include "TypeInfoTrait.h"
#include "Table4Class.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"
#include "DbBackendType.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"
#include "DbManager.h"
#include "DbManager4Sqlite.h"
#include "DbMapAll.h"


namespace edadb {

/**
 * @brief Initialize the database connection.
 * @param T The class type.
 * @param dbName The database name.
 * @return use decltype to return the type of the init function, which is bool.
 */
inline
bool initDatabase(const std::string& dbName) {
    bool res = false;
    if ((res = DbMapBase::i().init(dbName)) == false) {
        std::cerr << "DbMap::init failed" << std::endl;
        return res;
    }
    return res;
}

inline
bool executeSql(const std::string& sql) {
    return DbMapBase::i().executeSql(sql);
}

inline
bool beginTransaction() {
    return DbMapBase::i().beginTransaction();
}

inline
bool commitTransaction() {
    return DbMapBase::i().commitTransaction();
}



template<typename T>
bool createTable(DbMap<T> &dbmap, bool self_txn = true) {
    if (self_txn) {
        return beginTransaction() && dbmap.createTable() && commitTransaction();
    } else {
        return dbmap.createTable();
    }
}// createTable

template<typename T>
bool dropTable() {
    return DbMap<T>::i().dropTable();
} 

/**
 * @brief Insert the object into the database.
 * @param obj The object pinter to insert.
 * @return use decltype to return the type of the insert function, which is bool.
 */
template <typename T>
bool insertObject(DbMap<T> &dbmap, T* obj, bool self_txn = true) {
    typename DbMap<T>::Writer writer(dbmap);
    if (self_txn) {
        return beginTransaction() 
            && writer.insertOne(obj) 
            && commitTransaction();
    } else {
        return writer.insertOne(obj);
    }
} // insertObject

/**
 * @brief Insert the object vector into the database.
 * @param obj_vec The object pointer vector to insert.
 * @return use decltype to return the type of the insert function, which is bool.
 */
template <typename T>
bool insertVector(DbMap<T> &dbmap, std::vector<T*>& obj_vec, bool self_txn = true) {
    typename DbMap<T>::Writer writer(dbmap); 
    if (self_txn) {
        return beginTransaction() 
            && writer.insertVector(obj_vec) 
            && commitTransaction();
    } else {
        return writer.insertVector(obj_vec);
    }
} // insertVector


/**
 * @brief DbMapWriter: This is a type alias for the DbMap Writer class.
 * @tparam T The class type.
 */
template <typename T>
using DbMapWriter = typename edadb::DbMap<T>::Writer;

/**
 * @fn updateObject
 * @brief Update the object in the database.
 * @param dbmap The database map to update the object.
 * @param obj The object pointer to update.
 * @param self_txn If true, the function will begin a transaction and commit it after the update.
 * @return int Returns 1 if update successfully, 0 if no more row, -1 if error.
 */
template <typename T>
int updateObject(DbMap<T> &dbmap, T* obj, bool self_txn = true) { 
    typename DbMap<T>::Writer writer(dbmap);
    if (self_txn) {
        return beginTransaction() 
            && writer.updateOne(obj) 
            && commitTransaction();
    } else {
        return writer.updateOne(obj);
    }
} // updateObject

template <typename T>
bool updateVector(DbMap<T> &dbmap, std::vector<T*>& objs, bool self_txn = true) {
    typename DbMap<T>::Writer writer(dbmap);
    if (self_txn) {
        return beginTransaction() 
            && writer.updateVector(objs) 
            && commitTransaction();
    } else {
        return writer.updateVector(objs);
    } 
} // updateVector 


/**
 * @brief Delete the object from the database.
 * @param obj The object pointer to delete.
 * @return use decltype to return the type of the delete function, which is bool.
 */
template <typename T>
bool deleteObject(DbMap<T> &dbmap, T* obj, bool self_txn = true) {
    typename DbMap<T>::Writer writer(dbmap);
    if (self_txn) {
        return beginTransaction() 
            && writer.deleteOne(obj) 
            && commitTransaction();
    } else {
        return writer.deleteOne(obj);
    }
} // deleteObject

  

/**
 * @brief DbMapReader: This is a type alias for the DbMap Reader class.
 * @tparam T The class type.
 */
template<typename T>
using DbMapReader = typename edadb::DbMap<T>::Reader;

/**
 * @fn readGeneric
 * @brief generic read function to read the object from the database.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @param prepare The prepare function to prepare the reader.
 * @return int Returns 1 if read successfully, 0 if no more row, -1 if error.
 */
template <typename T, typename PrepareFunc>
int readGeneric( typename edadb::DbMap<T>::Reader*& reader, DbMap<T>& dbmap, T* obj,
        PrepareFunc prepare)
{
    // prepare the reader if not prepared
    if (reader == nullptr) {
        reader = new typename edadb::DbMap<T>::Reader(dbmap);
        if (!prepare(*reader)) {
            std::cerr << "DbMap::Reader::prepare failed" << std::endl;
            delete reader;
            reader = nullptr;
            return -1;
        } 
    } 

    // read until no more row
    bool ok = reader->read(obj);
    if (!ok) {
        reader->finalize();
        delete reader;
        reader = nullptr;
    }
    return ok ? 1 : 0;
} // readGeneric


/**
 * @fn read2Scan
 * @brief read the object from the database by predicate.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @return int Returns 1 if read successfully, 0 if no more row, -1 if error.
 */
template <typename T>
int read2Scan(typename edadb::DbMap<T>::Reader*& reader, DbMap<T>& dbmap, T* obj) {
    return readGeneric(reader, dbmap, obj,
        [](auto& r) { return r.prepare2Scan(); }
    );
}


/**
 * @fn readByPredicate
 * @brief read the object from the database by predicate.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @param predicate The predicate to filter the object.
 * @return int Returns 1 if read successfully, 0 if no more row, -1 if error.
 */    
template <typename T>
int readByPredicate(typename edadb::DbMap<T>::Reader*& reader, DbMap<T>& dbmap, T* obj,
            const std::string& predicate) {
    return readGeneric(reader, dbmap, obj,
        [&](auto& r) { return r.prepareByPredicate(predicate); }
    );
}

/**
 * @fn readByPrimaryKey: use readGeneric lambda function to read the object
 * @brief read the object from the database by primary key.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @return int Returns 1 if read successfully, -1 if error.
 */
template <typename T>
int readByPrimaryKey(typename edadb::DbMap<T>::Reader*& reader, DbMap<T>& dbmap, T* obj) {
    return readGeneric(
        reader, dbmap, obj,
        [&](auto& r) { return r.prepareByPrimaryKey(obj); }
    );
}

/**
 * @fn readByPrimaryKey
 * @brief read the object from the database by primary key.
 * @param dbmap The database map to read the object.
 * @param obj The object to read.
 * @return int Returns 1 if read successfully, -1 if error.
 */
template <typename T>
int readByPrimaryKey(DbMap<T> &dbmap, T* obj) {
    // primary key read is one time only
    typename edadb::DbMap<T>::Reader reader(dbmap);
    bool ok = false;

    if (!(ok = reader.prepareByPrimaryKey())) {
        std::cerr << "DbMap::Reader::prepareByPrimaryKey: prepare failed" << std::endl;
    }
    
    if (ok && !(ok = reader.read(obj))) {
        std::cerr << "DbMap::Reader::read: read failed" << std::endl;
    }

    reader.finalize();
    return ok ? 1 : -1;
}

} // namespace edadb