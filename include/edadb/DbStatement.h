/***
 * @file DbStatement.h
 * @brief DbStatement.h provides a way to manage the Sqlite3 statement.
 */

#pragma once

#include <string>
#include <iostream>

#include "Config.h"
#include "DbBackendType.h"

namespace edadb {

/**
 * @brief DbStatementImpl: This struct holds the db statement and error message.
 *     User need to specify the database backend type using DbBackendType.
 * @tparam DBType The database backend type.
 */
template <DbBackendType DBType>
struct DbStatementImpl {
    static_assert(DBType != DBType,
        "DbStatement is not implemented for this backend type.");
};


/**
 * DbStatement is defined in backend, such as backend/DbBackendType/DbStatement4Sqlite.h:
 *   using DbStatement = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
 *         DbStatementImpl<DbBackendType::SQLITE>>;
 */

} // namespace edadb