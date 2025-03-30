/**
 * @file CppTypeToDbType.hpp
 * @brief CppTypeToDbType.hpp provides a way to convert C++ types to
 *        database types.
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>


#ifdef _EDADB_SQLITE3_
#include "CppTypeToSqliteType.hpp"
#else
#include "CppTypeToDuckDBType.hpp"
#endif