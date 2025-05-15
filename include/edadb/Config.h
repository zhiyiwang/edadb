/**
 * @file Config.h
 * @brief Config parameters
 */

#pragma once

#include "DbBackendType.h"

namespace edadb {

/**
 * @brief Config class provides the configuration parameters for the database.
 */
class Config {
public:
    static constexpr DbBackendType backend_type = DbBackendType::SQLITE; 

public:
    /**
     * @brief foreign key reference primary key column index
     */
    static constexpr const size_t fk_ref_pk_col_index = 0;
};

} // namespace edadb
