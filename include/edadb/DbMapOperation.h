/**
 * @file DbMapOperation.h
 * @brief DbMapOperation.h defines the operations for DbMap.
 */
#pragma once

#include <string>

#include "TraitUtils.h"
#include "DbMap.h"
#include "SqlStatement.h"
#include "SqlStatement4Sqlite.h"


namespace edadb {


// enum for DbMap operation
enum class DbMapOperation {
    NONE,

    INSERT,
    UPDATE,
    DELETE,
    SCAN,

    QUERY_PREDICATE,
    QUERY_PRIMARY_KEY,
    QUERY_FOREIGN_KEY, 

    MAX
}; // DbMapOperation



/**
 * Operation traits for DbMap operation.
 */
template <typename T, DbMapOperation OP>
struct DbMapOpTrait {
    static_assert(always_false<T>::value,
        "DbMapOperation is not implemented for this operation.");
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::INSERT> {
    static constexpr const char *name() {
        return "Insert";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::insertPlaceHolderStatement(
            dbmap.getTableName(), dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::INSERT;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::UPDATE> {
    static constexpr const char *name() {
        return "Update";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::updatePlaceHolderStatement(
            dbmap.getTableName(), dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::UPDATE;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::DELETE> {
    static constexpr const char *name() {
        return "Delete";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::deletePlaceHolderStatement(dbmap.getTableName());
    }
    static DbMapOperation op() {
        return DbMapOperation::DELETE;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::SCAN> {
    static constexpr const char *name() {
        return "Scan";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::scanStatement(
            dbmap.getTableName(), dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::SCAN;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::QUERY_PREDICATE> {
    static constexpr const char *name() {
        return "QueryPredicate";
    }
    static std::string getSQL(DbMap<T> &dbmap, const std::string &pred) {
        return SqlStatement<T>::queryPredicateStatement(
            dbmap.getTableName(), dbmap.getForeignKey(), pred);
    }
    static DbMapOperation op() {
        return DbMapOperation::QUERY_PREDICATE;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::QUERY_PRIMARY_KEY> {
    static constexpr const char *name() {
        return "QueryPrimaryKey";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::queryPrimaryKeyStatement(
            dbmap.getTableName(), dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::QUERY_PRIMARY_KEY;
    }
};


template <typename T>
struct DbMapOpTrait<T, DbMapOperation::QUERY_FOREIGN_KEY> {
    static constexpr const char *name() {
        return "QueryForeignKey";
    }
    static std::string getSQL(DbMap<T> &dbmap) {
        return SqlStatement<T>::queryForeignKeyStatement(
            dbmap.getTableName(), dbmap.getForeignKey());
    }
    static DbMapOperation op() {
        return DbMapOperation::QUERY_FOREIGN_KEY;
    }
};


} // namespace edadb