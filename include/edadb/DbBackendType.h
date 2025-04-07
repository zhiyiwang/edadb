/***
 * @file DbBackendType.hpp
 * @brief Database backend type enum.
 */

 #pragma once

 namespace edadb {
 
 /**
  * @class DbBackend Type
  * @brief Database backend type.
  */
 enum class DbBackendType {
     UNKNOWN = 0,
     SQLITE,
     DUCKDB, 
     MAX
 };
 
 } // namespace edadb