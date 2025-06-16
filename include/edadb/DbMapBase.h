/**
 * @file DbMapBase.h
 * @brief DbMapBase.h defines DbMapBase class, which is the base class for all DbMap classes.
 */

#pragma once

#include <string>

#include "Singleton.h"
#include "DbManager.h"
#include "DbManager4Sqlite.h"


namespace edadb {


/**
 * @brief DbMapBase class is the base class for all DbMap classes and manages the database connection.
 */
class DbMapBase : public Singleton<DbMapBase> {
private:
    /**
     * @brief friend class for Singleton pattern.
     */
    friend class Singleton<DbMapBase>;

protected:
    // Singleton DbManager ins to connect to database
    inline static DbManager &manager = DbManager::i();

protected:
    DbMapBase() = default;

public:
    virtual ~DbMapBase() = default;

public: 
    DbManager &getManager() { return manager; }

public:
    /**
     * @brief Initialize the backend database connection.
     * @param c The database connection string.
     * @return true if success; otherwise, false.
     */
    bool init(const std::string &c) {
        // no need to check inited, singleton manager will check it
        return manager.connect(c);
    }

    /**
     * @brief Execute the SQL statement directly.
     * @param sql The SQL statement.
     * @return true if executed; otherwise, false.
     */
    bool executeSql(const std::string &sql) {
        return manager.exec(sql);
    }

    /**
     * @brief Begin a transaction.
     * @return true if success; otherwise, false.
     */
    bool beginTransaction() {
        return manager.exec("BEGIN TRANSACTION;");
    }

    /**
     * @brief Commit a transaction.
     * @return true if success; otherwise, false.
     */
    bool commitTransaction() {
        return manager.exec("COMMIT;");
    }
}; // DbMapBase


} // namespace edadb