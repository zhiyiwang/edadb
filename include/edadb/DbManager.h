/**
 * @file DbManager.h
 * @brief DbManager.h provides a way to manage the Sqlite3.
 *      This class is a child class of DatabaseManager.
 */

#pragma once

#include <type_traits>
#include <iostream>
#include <string>
#include <stdint.h>
#include <sqlite3.h>
#include <assert.h>

#include "Config.h"
#include "Singleton.h"
#include "Cpp2SqlType.h"
#include "DbBackendType.h"
#include "DbStatement.h"
#include "DbStatement4Sqlite.h"

namespace edadb {

/**
 * @class DbManager
 * @brief This class manages the Sqlite3 database.
 *    Since Sqlite3 use R/W lock, we define Singleton pattern for this class.
 */
template<DbBackendType DBType>
class DbManagerImpl : public Singleton< DbManagerImpl<DBType> > {
    static_assert(DBType != DBType, "DbManager is not implemented for this backend type.");   
};

/**
 * DbManager is defined in backend, such as backend/DbBackendType/DbManager4Sqlite.h:
 *   using DbManager = std::enable_if_t< Config::backend_type == DbBackendType::SQLITE,
 *        DbManagerImpl<DbBackendType::SQLITE>>;
 */

} // namespace edadb