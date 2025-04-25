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
#include "Cpp2SqlType.h"
#include "TypeMetaData.h"
#include "Table4Class.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"
#include "DbBackendType.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"
#include "DbManager.h"
#include "DbManager4Sqlite.h"
#include "DbMap.h"


#define _EDADB_DEFINE_TABLE_BY_CLASS_(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP) \
    TABLE4CLASS(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP)

//#define _EDADB_DEFINE_TABLE_BY_CLASS_WITH_VECTOR_(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, VECTOR_ELEMS_TUP) \
//    TABLE4CLASSWVEC(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, VECTOR_ELEMS_TUP)
//
#define _EDADB_DEFINE_TABLE_BY_COLUMN_(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, COLNAME_TUP) \
    TABLE4CLASS_COLNAME(CLASS_NAME, TABLE_NAME, CLASS_ELEMS_TUP, COLNAME_TUP)

#define _EDADB_DEFINE_TABLE_4_EXTERNAL_CLASS_(CLASS_NAME, CLASS_ELEMS_TUP) \
    Table4ExternalClass(CLASS_NAME, CLASS_ELEMS_TUP)



namespace edadb {

/**
 * @brief Initialize the database connection.
 * @param T The class type.
 * @param dbName The database name.
 * @return use decltype to return the type of the init function, which is bool.
 */
template<typename T>
bool initDatabase(const std::string& dbName) {
    bool res = false;
    if ((res = DbMap<T>::i().init(dbName)) == false) {
        std::cerr << "DbMap::init failed" << std::endl;
        return res;
    }

    // Connection Setting: enable foreign key constraint
    if ((res = DbManager::i().exec("PRAGMA foreign_keys = ON;")) == false) {
        std::cerr << "DbMap::executeSql failed" << std::endl;
        return res;
    }

    return res;
}

template<typename T>
bool executeSql(const std::string& sql) {
    return DbMap<T>::i().executeSql(sql);
}

template<typename T>
bool createTable() {
    return DbMap<T>::i().createTable();
}

template<typename T>
bool dropTable() {
    return DbMap<T>::i().dropTable();
}

template <typename T>
bool beginTransaction() {
    return DbMap<T>::i().beginTransaction();
}

template <typename T>
bool commitTransaction() {
    return DbMap<T>::i().commitTransaction();
}

/**
 * @brief Insert the object into the database.
 * @param obj The object pinter to insert.
 * @return use decltype to return the type of the insert function, which is bool.
 */
template <typename T>
bool insertObject(T* obj) {
    typename DbMap<T>::Writer writer;
    return writer.insertOne(obj);
}

/**
 * @brief Insert the object vector into the database.
 * @param obj_ptr_vec The object pointer vector to insert.
 * @return use decltype to return the type of the insert function, which is bool.
 */
template <typename T>
bool insertVector(std::vector<T*>& obj_ptr_vec) {
    typename DbMap<T>::Writer writer; 
    return writer.insertVector(obj_ptr_vec);
}

/**
 * @brief Update the object in the database.
 * @param obj_ptr_org The original object pointer to update.
 * @param obj_ptr_new The new object pointer to update.
 * @return use decltype to return the type of the update function, which is bool.
 */
template <typename T>
bool updateObjectBySqlStmt(T* obj_ptr_org, T* obj_ptr_new) {
    typename DbMap<T>::Writer writer;
    return writer.updateBySqlStmt(obj_ptr_org, obj_ptr_new);
}

/**
 * @brief DbMapWriter: This is a type alias for the DbMap Writer class.
 * @tparam T The class type.
 */
template <typename T>
using DbMapWriter = typename edadb::DbMap<T>::Writer;


template <typename T>
int updateObject(T* org_obj_ptr, T* new_obj_ptr) {
    typename DbMap<T>::Writer writer;
    return writer.updateOne(org_obj_ptr, new_obj_ptr);
}

template <typename T>
bool updateVector(std::vector<T*>& org_vec_ptr, std::vector<T*>& new_vec_ptr) {
    typename DbMap<T>::Writer writer;
    return writer.updateVector(org_vec_ptr, new_vec_ptr);
}


/**
 * @brief Delete the object from the database.
 * @param obj_ptr The object pointer to delete.
 * @return use decltype to return the type of the delete function, which is bool.
 */
template <typename T>
bool deleteObject(T* obj_ptr) {
    typename DbMap<T>::Writer writer;
//    return writer.deleteByPrimaryKeys(obj_ptr);
    return writer.deleteOne(obj_ptr);
}
  

/**
 * @brief DbMapReader: This is a type alias for the DbMap Reader class.
 * @tparam T The class type.
 */
template<typename T>
using DbMapReader = typename edadb::DbMap<T>::Reader;

/**
 * @fn readByPredicate
 * @brief read the object from the database by predicate.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @param predicate The predicate to filter the object.
 * @return int Returns 1 if read successfully, 0 if no more row, -1 if error.
 */    
template <typename T>
int readByPredicate(typename edadb::DbMap<T>::Reader*& reader, T* obj, const std::string& predicate) {
    if (reader == nullptr) {
        reader = new typename edadb::DbMap<T>::Reader(edadb::DbMap<T>::i());
        if (reader->prepareByPredicate(predicate) == false) {
            std::cerr << "DbMap::Reader::prepareByPredicate: prepare failed" << std::endl;
            delete reader;
            reader = nullptr;
            return -1;
        }
    }

    bool got = reader->read(obj);
    if (!got) {
        reader->finalize();
        delete reader;
        reader = nullptr;
    }
    return got ? 1 : 0;
}

/**
 * @fn readByPrimaryKey
 * @brief read the object from the database by primary key.
 * @param reader The reader to read the object.
 * @param obj The object to read.
 * @return int Returns 1 if read successfully, -1 if error.
 */
template <typename T>
int readByPrimaryKey(T* obj) {
    // primary key read is one time only
    typename edadb::DbMap<T>::Reader reader(edadb::DbMap<T>::i());
    bool got = false;
    if (!(got = reader.prepareByPrimaryKey(obj))) {
        std::cerr << "DbMap::Reader::prepareByPrimaryKey: prepare failed" << std::endl;
    }
    
    if (got && !(got = reader.read(obj))) {
        std::cerr << "DbMap::Reader::read: read failed" << std::endl;
    }

    reader.finalize();
    return got ? 1 : -1;
}

} // namespace edadb