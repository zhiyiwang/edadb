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
    Unknown = 0,
    Sqlite,
    DuckDb, // TODO
    Max
};


} // namespace edadb