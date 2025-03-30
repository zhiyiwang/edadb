/**
 * @file DbType.hpp
 * @brief DbType.hpp provides a way to manage the database type.
 */

#pragma once

#include "DbBackendType.hpp"
#include "SqlType.hpp"

namespace edadb {


/**
 * @enum DbType Traits to set the database type for the backend.
 */
template <DbBackendType BT>
struct DbTypeTraits;


/**
 * @struct base template declaration
 * @details SqlType to DbType conversion: only declear the template, no implementation.
 *        The implementation is in the specializations by the backend.
 */
template<DbBackendType BT, SqlType ST>
struct Sql2DbType;


/**
 * @struct DBType: declare the database type.
 * @brief This is the database type for the backend.
 *     We comment out here to avoid error: "redeclared as different kind of entity"
 *     Each backend will implement the DbType in their backend:
 *       such as DbTypeSqlite, DbTypeDuckDb, etc.
 */
//template<DbBackendType BT>
//class DbType;


/**
 * @fn getDbTypeString
 * @brief get the database type string for the given C++ type. 
 * @return std::string Returns the database type equivalent.
 */
template<DbBackendType BT, typename T>
std::string const &getDbTypeString();


} // namespace edadb
